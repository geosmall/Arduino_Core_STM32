/*
 * SPIFlash.cpp — raw polled SPI NOR flash driver. See SPIFlash.h.
 */

#include "SPIFlash.h"

#if defined(SPI_SR_TXP)
#error "SPIFlash: H7-class SPI (TXP/RXP/EOT) is not supported yet"
#endif

namespace {

constexpr uint8_t CMD_WRITE_ENABLE = 0x06;
constexpr uint8_t CMD_READ_STATUS1 = 0x05;
constexpr uint8_t CMD_READ_STATUS2 = 0x35;
constexpr uint8_t CMD_READ_DATA    = 0x03;
constexpr uint8_t CMD_PAGE_PROGRAM = 0x02;
constexpr uint8_t CMD_ERASE_64K    = 0xD8;
constexpr uint8_t CMD_JEDEC_ID     = 0x9F;

constexpr uint8_t STATUS1_BUSY     = 0x01;
constexpr uint8_t STATUS2_EP_FAIL  = 0x04;  // PY25Q128HA S10

// Clock for the JEDEC read, before the part (and its limit) is known.
constexpr uint32_t ID_CLOCK_HZ = 1000000;

// Bound on each wait for an SPI flag. One byte at the slowest supported
// clock is far below this; reaching it means the peripheral is stuck.
constexpr uint32_t SPIN_LIMIT = 100000;

// Chip select high time between commands: W25Q128FV tSHSL 50 ns (the longer
// of the two parts), with margin.
constexpr uint32_t CS_HIGH_NS = 100;

}  // namespace

SPIFlash::SPIFlash(SPI_TypeDef *instance, Pin mosi, Pin miso, Pin sclk, Pin cs)
  : _spi(instance, mosi, miso, sclk), _instance(instance), _cs(cs)
{
}

void SPIFlash::select()
{
  _cs_port->BSRR = _cs_mask << 16;
}

void SPIFlash::deselect()
{
  _cs_port->BSRR = _cs_mask;
  // Hold chip select high for CS_HIGH_NS before any following select.
  dwt_ensureRunning();
  const uint32_t cycles = (SystemCoreClock / 1000000u) * CS_HIGH_NS / 1000u + 1u;
  const uint32_t start = DWT->CYCCNT;
  while ((DWT->CYCCNT - start) < cycles) { }
}

uint32_t SPIFlash::busClockHz(SPI_TypeDef *spi)
{
  // F4/F7/G4: SPI1, SPI4, SPI5, SPI6 on APB2; SPI2, SPI3 on APB1.
  bool apb2 = (spi == SPI1);
#if defined(SPI4)
  apb2 = apb2 || (spi == SPI4);
#endif
#if defined(SPI5)
  apb2 = apb2 || (spi == SPI5);
#endif
#if defined(SPI6)
  apb2 = apb2 || (spi == SPI6);
#endif
  return apb2 ? HAL_RCC_GetPCLK2Freq() : HAL_RCC_GetPCLK1Freq();
}

SPIFlash::Result SPIFlash::xfer(const uint8_t *tx, uint8_t *rx, uint32_t n)
{
  SPI_TypeDef *const s = _instance;
  volatile uint8_t *const dr = reinterpret_cast<volatile uint8_t *>(&s->DR);

  if (rx == nullptr) {
    // Transmit only: load the next byte as soon as TXE is set, so SCK runs
    // back to back. Received bytes are discarded and cleared at the end.
    while (n--) {
      uint32_t spin = SPIN_LIMIT;
      while (!(s->SR & SPI_SR_TXE)) {
        if (--spin == 0) {
          return Result::Timeout;
        }
      }
      *dr = tx ? *tx++ : 0xFF;
    }
    uint32_t spin = SPIN_LIMIT;
    while (!(s->SR & SPI_SR_TXE)) {
      if (--spin == 0) {
        return Result::Timeout;
      }
    }
    spin = SPIN_LIMIT;
    while (s->SR & SPI_SR_BSY) {
      if (--spin == 0) {
        return Result::Timeout;
      }
    }
    // Drain the receive side (one byte on F4, up to the FIFO depth on F7/G4)
    // and clear the overrun with the DR-then-SR read sequence.
    spin = SPIN_LIMIT;
    while (s->SR & SPI_SR_RXNE) {
      (void)*dr;
      if (--spin == 0) {
        return Result::Timeout;
      }
    }
    (void)s->SR;
    return Result::Ok;
  }

  // Full duplex, one byte in flight: safe with interrupts enabled.
  while (n--) {
    uint32_t spin = SPIN_LIMIT;
    while (!(s->SR & SPI_SR_TXE)) {
      if (--spin == 0) {
        return Result::Timeout;
      }
    }
    *dr = tx ? *tx++ : 0xFF;
    spin = SPIN_LIMIT;
    while (!(s->SR & SPI_SR_RXNE)) {
      if (--spin == 0) {
        return Result::Timeout;
      }
    }
    *rx++ = *dr;
  }
  return Result::Ok;
}

SPIFlash::Result SPIFlash::begin(uint32_t clock_hz, const SPIFlashPart *table,
                                 unsigned table_count)
{
  _part = nullptr;
  _jedec = 0;
  _clock_hz = 0;

  _cs_port = get_GPIO_Port(_cs.port);
  _cs_mask = 1u << _cs.pin;
  digitalWrite(_cs, HIGH);
  pinMode(_cs, OUTPUT);

  _spi.begin();
  _spi.beginTransaction(SPISettings(ID_CLOCK_HZ, MSBFIRST, SPI_MODE0));
  _instance->CR1 |= SPI_CR1_SPE;

  uint8_t id[4] = {CMD_JEDEC_ID, 0, 0, 0};
  select();
  const Result r = xfer(id, id, sizeof(id));
  deselect();
  if (r != Result::Ok) {
    return r;
  }
  _jedec = (uint32_t(id[1]) << 16) | (uint32_t(id[2]) << 8) | id[3];
  if (_jedec == 0x000000 || _jedec == 0xFFFFFF) {
    return Result::NotFound;
  }

  for (unsigned i = 0; i < table_count; i++) {
    if (table[i].jedec_id == _jedec) {
      _part = &table[i];
      break;
    }
  }
  if (_part == nullptr) {
    return Result::UnknownPart;
  }

  const uint32_t want = (clock_hz < _part->max_clock_hz) ? clock_hz : _part->max_clock_hz;
  _spi.beginTransaction(SPISettings(want, MSBFIRST, SPI_MODE0));
  _instance->CR1 |= SPI_CR1_SPE;
  const uint32_t br = (_instance->CR1 & SPI_CR1_BR) >> SPI_CR1_BR_Pos;
  _clock_hz = busClockHz(_instance) >> (br + 1);
  if (_clock_hz > _part->max_clock_hz) {
    _part = nullptr;
    _clock_hz = 0;
    return Result::BadArgument;
  }
  return Result::Ok;
}

SPIFlash::Result SPIFlash::command(uint8_t op)
{
  select();
  const Result r = xfer(&op, nullptr, 1);
  deselect();
  return r;
}

SPIFlash::Result SPIFlash::commandAddr(uint8_t op, uint32_t addr)
{
  const uint8_t cmd[4] = {op, uint8_t(addr >> 16), uint8_t(addr >> 8), uint8_t(addr)};
  return xfer(cmd, nullptr, sizeof(cmd));
}

bool SPIFlash::isBusy(Result *result)
{
  uint8_t buf[2] = {CMD_READ_STATUS1, 0};
  select();
  const Result r = xfer(buf, buf, sizeof(buf));
  deselect();
  if (result) {
    *result = r;
  }
  return (r != Result::Ok) || (buf[1] & STATUS1_BUSY);
}

SPIFlash::Result SPIFlash::read(uint32_t addr, uint8_t *dst, uint32_t n)
{
  if (_part == nullptr) {
    return Result::NotReady;
  }
  if (n == 0 || addr >= size() || n > size() - addr) {
    return Result::BadArgument;
  }
  select();
  Result r = commandAddr(CMD_READ_DATA, addr);
  if (r == Result::Ok) {
    r = xfer(nullptr, dst, n);
  }
  deselect();
  return r;
}

SPIFlash::Result SPIFlash::pageProgram(uint32_t addr, const uint8_t *src, uint32_t n)
{
  if (_part == nullptr) {
    return Result::NotReady;
  }
  if (n == 0 || n > PAGE_SIZE || addr >= size() ||
      (addr % PAGE_SIZE) + n > PAGE_SIZE) {
    return Result::BadArgument;
  }
  Result r = command(CMD_WRITE_ENABLE);
  if (r != Result::Ok) {
    return r;
  }
  select();
  r = commandAddr(CMD_PAGE_PROGRAM, addr);
  if (r == Result::Ok) {
    r = xfer(src, nullptr, n);
  }
  deselect();
  return r;
}

SPIFlash::Result SPIFlash::erase64k(uint32_t addr)
{
  if (_part == nullptr) {
    return Result::NotReady;
  }
  if (addr >= size() || (addr % BLOCK_64K) != 0) {
    return Result::BadArgument;
  }
  Result r = command(CMD_WRITE_ENABLE);
  if (r != Result::Ok) {
    return r;
  }
  select();
  r = commandAddr(CMD_ERASE_64K, addr);
  deselect();
  return r;
}

SPIFlash::Result SPIFlash::lastProgramResult()
{
  if (_part == nullptr) {
    return Result::NotReady;
  }
  if (!_part->has_ep_fail) {
    return Result::Ok;
  }
  uint8_t buf[2] = {CMD_READ_STATUS2, 0};
  select();
  const Result r = xfer(buf, buf, sizeof(buf));
  deselect();
  if (r != Result::Ok) {
    return r;
  }
  return (buf[1] & STATUS2_EP_FAIL) ? Result::ProgramFail : Result::Ok;
}

SPIFlash::Result SPIFlash::waitIdle(uint32_t timeout_us)
{
  if (_part == nullptr) {
    return Result::NotReady;
  }
  const uint32_t start = micros();
  for (;;) {
    Result r;
    const bool busy = isBusy(&r);
    if (r != Result::Ok) {
      return r;
    }
    if (!busy) {
      return lastProgramResult();
    }
    if ((micros() - start) >= timeout_us) {
      return Result::Timeout;
    }
  }
}
