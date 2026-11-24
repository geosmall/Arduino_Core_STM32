/*
 * SDBlock.cpp — SD card block device, polled SPI back end. See SDBlock.h.
 */

#include "SDBlock.h"

#if !defined(SPI_SR_TXP)

namespace {

constexpr uint8_t TOKEN_SINGLE = 0xFE;   // start token, CMD17/CMD18/CMD24
constexpr uint8_t TOKEN_MULTI  = 0xFC;   // start token, CMD25
constexpr uint8_t TOKEN_STOP   = 0xFD;   // stop transmission, CMD25
constexpr uint8_t DATA_ACCEPTED = 0x05;  // data response, low 5 bits
constexpr uint8_t R1_IDLE = 0x01;

// Bound on each wait for an SPI flag. One byte at the init clock is far
// below this; reaching it means the peripheral is stuck.
constexpr uint32_t SPIN_LIMIT = 100000;

// Bytes read for R1 after a command (NCR is at most 8).
constexpr uint32_t R1_BYTES = 10;

constexpr uint32_t READY_TIMEOUT_US = 500000;      // before a command
constexpr uint32_t READ_TOKEN_TIMEOUT_US = 300000;
constexpr uint32_t INIT_TIMEOUT_MS = 1000;         // ACMD41 loop

// begin(): CMD0 attempts in all, and the one preceded by the stream drain.
constexpr int BEGIN_ATTEMPTS = 7;
constexpr int DRAIN_ATTEMPT = 2;

// ACMD23's block count field is 23 bits.
constexpr uint32_t ACMD23_MAX = 0x7FFFFF;

}  // namespace

SDBlockSPI::SDBlockSPI(SPI_TypeDef *instance, Pin mosi, Pin miso, Pin sclk, Pin cs)
  : _spi(instance, mosi, miso, sclk), _instance(instance), _cs(cs)
{
}

// ---------------------------------------------------------------------------
// Polled SPI (F4/F7/G4: TXE/RXNE/BSY), the loops of SPIFlash::xfer()

void SDBlockSPI::select()
{
  _cs_port->BSRR = _cs_mask << 16;
}

void SDBlockSPI::deselect()
{
  uint32_t spin = SPIN_LIMIT;
  while ((_instance->SR & SPI_SR_BSY) && --spin) { }
  _cs_port->BSRR = _cs_mask;
  // One byte with chip select high: the card releases its data out line.
  xchg(0xFF);
}

uint8_t SDBlockSPI::xchg(uint8_t out)
{
  SPI_TypeDef *const s = _instance;
  volatile uint8_t *const dr = reinterpret_cast<volatile uint8_t *>(&s->DR);
  uint32_t spin = SPIN_LIMIT;
  while (!(s->SR & SPI_SR_TXE)) {
    if (--spin == 0) {
      _spi_fault = true;
      return 0xFF;
    }
  }
  *dr = out;
  spin = SPIN_LIMIT;
  while (!(s->SR & SPI_SR_RXNE)) {
    if (--spin == 0) {
      _spi_fault = true;
      return 0xFF;
    }
  }
  return *dr;
}

// Transmit only: the transmit register is kept full so SCK runs back to
// back; the received bytes are discarded and the receive side cleared at the
// end. See SPIFlash::xfer() for the end-of-transfer sequence.
bool SDBlockSPI::txOnly(const uint8_t *p, uint32_t n)
{
  SPI_TypeDef *const s = _instance;
  volatile uint8_t *const dr = reinterpret_cast<volatile uint8_t *>(&s->DR);
  while (n--) {
    uint32_t spin = SPIN_LIMIT;
    while (!(s->SR & SPI_SR_TXE)) {
      if (--spin == 0) {
        _spi_fault = true;
        return false;
      }
    }
    *dr = *p++;
  }
  uint32_t spin = SPIN_LIMIT;
  while (!(s->SR & SPI_SR_RXNE)) {
    if (--spin == 0) {
      _spi_fault = true;
      return false;
    }
  }
  spin = SPIN_LIMIT;
  while (!(s->SR & SPI_SR_TXE)) {
    if (--spin == 0) {
      _spi_fault = true;
      return false;
    }
  }
  spin = SPIN_LIMIT;
  while (s->SR & SPI_SR_BSY) {
    if (--spin == 0) {
      _spi_fault = true;
      return false;
    }
  }
  spin = SPIN_LIMIT;
  while (s->SR & SPI_SR_RXNE) {
    (void)*dr;
    if (--spin == 0) {
      _spi_fault = true;
      return false;
    }
  }
  (void)s->SR;
  return true;
}

uint32_t SDBlockSPI::busClockHz(SPI_TypeDef *spi)
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

void SDBlockSPI::setClock(uint32_t hz)
{
  _spi.beginTransaction(SPISettings(hz, MSBFIRST, SPI_MODE0));
  _instance->CR1 |= SPI_CR1_SPE;
  const uint32_t br = (_instance->CR1 & SPI_CR1_BR) >> SPI_CR1_BR_Pos;
  _clock_hz = busClockHz(_instance) >> (br + 1);
}

// ---------------------------------------------------------------------------
// Commands (SPI mode)

// Sends a command and returns R1 (0xFF: none). Does not wait for the card to
// be ready: the caller has seen it ready. Chip select is low on return.
uint8_t SDBlockSPI::command(uint8_t idx, uint32_t arg)
{
  // CRC is checked only for CMD0 and CMD8 until CRC is enabled (CMD59).
  uint8_t crc = 0x01;
  if (idx == 0) {
    crc = 0x95;
  } else if (idx == 8) {
    crc = 0x87;
  }
  // The leading 0xFF is the byte (NRC) the card needs after a previous
  // command's response when chip select stays low between commands.
  const uint8_t frame[7] = {0xFF, uint8_t(0x40 | idx), uint8_t(arg >> 24), uint8_t(arg >> 16),
                            uint8_t(arg >> 8), uint8_t(arg), crc};
  select();
  txOnly(frame, sizeof(frame));
  uint8_t r = 0xFF;
  for (uint32_t i = 0; i < R1_BYTES && (r & 0x80); i++) {
    r = xchg(0xFF);
  }
  _last_cmd = idx;
  _last_r1 = r;
  return r;
}

uint8_t SDBlockSPI::appCommand(uint8_t idx, uint32_t arg)
{
  const uint8_t r = command(55, 0);
  if (r > R1_IDLE) {
    return r;
  }
  return command(idx, arg);
}

// One-byte busy check with chip select low: the card holds its data out
// line low while busy.
bool SDBlockSPI::readyNow()
{
  select();
  return xchg(0xFF) == 0xFF;
}

bool SDBlockSPI::waitReady(uint32_t timeout_us)
{
  const uint32_t t0 = micros();
  while (!readyNow()) {
    if (_spi_fault || (micros() - t0) > timeout_us) {
      return false;
    }
  }
  return true;
}

SDBlockSPI::Result SDBlockSPI::readData(uint8_t *dst, uint32_t n)
{
  const uint32_t t0 = micros();
  uint8_t t;
  do {
    t = xchg(0xFF);
  } while (t == 0xFF && !_spi_fault && (micros() - t0) < READ_TOKEN_TIMEOUT_US);
  if (t == 0xFF) {
    return Result::Timeout;
  }
  if (t != TOKEN_SINGLE) {
    return Result::ReadError;
  }
  while (n--) {
    *dst++ = xchg(0xFF);
  }
  xchg(0xFF);  // CRC, not checked
  xchg(0xFF);
  return Result::Ok;
}

// ---------------------------------------------------------------------------
// State

SDBlockSPI::Result SDBlockSPI::checkUsable(bool allow_stream) const
{
  if (_fault != Result::Ok) {
    return _fault;
  }
  if (!_ready) {
    return Result::NotReady;
  }
  if (!allow_stream && _state != State::Idle) {
    return Result::StreamOpen;
  }
  return Result::Ok;
}

SDBlockSPI::Result SDBlockSPI::setFault(Result r)
{
  _fault = r;
  _state = State::Idle;
  _submitted = false;
  _card_busy = false;
  _off = 0;
  _cs_port->BSRR = _cs_mask;
  return r;
}

// The busy left by a block or the stop token: one byte, never a wait.
SDBlockSPI::Result SDBlockSPI::pollBusy()
{
  if (!_card_busy) {
    return Result::Ok;
  }
  const bool rdy = (xchg(0xFF) == 0xFF);
  if (_spi_fault) {
    return setFault(Result::Timeout);
  }
  const uint32_t dt = micros() - _busy_t0;
  if (!rdy) {
    if (dt > BUSY_TIMEOUT_US) {
      return setFault(Result::Timeout);
    }
    return Result::Busy;
  }
  _card_busy = false;
  _busy_max_us = dt > _busy_max_us ? dt : _busy_max_us;
  if (_state == State::Idle) {
    deselect();  // the stop token's busy is over: the stream is closed
    _in_stream = false;
  }
  return Result::Ok;
}

// ---------------------------------------------------------------------------
// Initialisation

SDBlockSPI::Result SDBlockSPI::begin(uint32_t clock_hz)
{
  const bool was_streaming = _in_stream;
  _in_stream = false;
  _ready = false;
  _fault = Result::Ok;
  _spi_fault = false;
  _state = State::Idle;
  _submitted = false;
  _card_busy = false;
  _off = 0;
  _blocks = 0;
  _blocks_written = 0;
  _busy_max_us = 0;
  _begin_attempts = 0;
  _high_speed = false;

  // Chip select high before the pin becomes an output (the port clock first:
  // a write to an unclocked port is lost). See SPIFlash::begin().
  _cs_port = set_GPIO_Port_Clock(_cs.port);
  _cs_mask = 1u << _cs.pin;
  _cs_port->BSRR = _cs_mask;
  pinMode(_cs, OUTPUT);

  _spi.begin();
  setClock(INIT_CLOCK_HZ);
  delay(10);

  // A card left inside a CMD25 stream (this object faulted, or the MCU reset
  // with the card powered) takes command bytes as block data. 514 bytes of
  // 0xFF end any block in progress (data and CRC); once its busy is over the
  // stop token closes the stream. The block in progress, inside the stream's
  // range, is written with 0xFF. After the drain some cards answer only a
  // later CMD0 (a SanDisk SR64G reset with a block just received: the second
  // one after the drain), so a few more are tried, 1 ms apart.
  for (int attempt = 0; attempt < BEGIN_ATTEMPTS; attempt++) {
    if (attempt > DRAIN_ATTEMPT) {
      delay(1);
    }
    if (attempt == DRAIN_ATTEMPT || (attempt == 0 && was_streaming)) {
      select();
      for (uint32_t i = 0; i < BLOCK_SIZE + 2; i++) {
        xchg(0xFF);
      }
      waitReady(BUSY_TIMEOUT_US);
      const uint8_t stop = TOKEN_STOP;
      txOnly(&stop, 1);
      xchg(0xFF);
      waitReady(BUSY_TIMEOUT_US);
    }
    _cs_port->BSRR = _cs_mask;
    for (int i = 0; i < 10; i++) {
      xchg(0xFF);  // 80 clocks, chip select high
    }
    const uint8_t r = command(0, 0);
    deselect();
    _begin_attempts = uint8_t(attempt + 1);
    if (r == R1_IDLE) {
      break;
    }
    if (attempt == BEGIN_ATTEMPTS - 1 || _spi_fault) {
      return _spi_fault ? Result::Timeout : Result::NoCard;
    }
  }

  uint8_t r = command(8, 0x1AA);
  uint8_t r7[4];
  for (uint8_t &b : r7) {
    b = xchg(0xFF);
  }
  deselect();
  if (r != R1_IDLE || (r7[2] & 0x0F) != 0x01 || r7[3] != 0xAA) {
    return Result::UnsupportedCard;
  }

  const uint32_t t0 = millis();
  do {
    r = appCommand(41, 1UL << 30);  // HCS
    deselect();
  } while (r == R1_IDLE && (millis() - t0) < INIT_TIMEOUT_MS);
  if (r != 0) {
    return r == R1_IDLE ? Result::Timeout : Result::CommandError;
  }

  r = command(58, 0);
  uint8_t ocr[4];
  for (uint8_t &b : ocr) {
    b = xchg(0xFF);
  }
  deselect();
  if (r != 0) {
    return Result::CommandError;
  }
  if (!(ocr[0] & 0x40)) {
    return Result::UnsupportedCard;  // CCS = 0: SDSC, byte addressed
  }

  setClock(clock_hz < MAX_CLOCK_HZ ? clock_hz : MAX_CLOCK_HZ);
  if (_clock_hz > MAX_CLOCK_HZ) {
    _clock_hz = 0;
    return Result::BadArgument;
  }

  uint8_t csd[16];
  if (!waitReady(READY_TIMEOUT_US)) {
    deselect();
    return Result::Timeout;
  }
  if (command(9, 0) != 0) {
    deselect();
    return Result::CommandError;
  }
  const Result rr = readData(csd, sizeof(csd));
  deselect();
  if (rr != Result::Ok) {
    return rr;
  }
  if ((csd[0] >> 6) != 1) {
    return Result::UnsupportedCard;  // not CSD version 2
  }
  const uint32_t c_size = (uint32_t(csd[7] & 0x3F) << 16) | (uint32_t(csd[8]) << 8) | csd[9];
  _blocks = (c_size + 1) * 1024;

  // ACMD13: SD status (R2, then 64 bytes). AU_SIZE (bits 431:428) gives the
  // allocation unit; a card that does not answer leaves it unknown (0).
  _au_blocks = 0;
  if (waitReady(READY_TIMEOUT_US) && appCommand(13, 0) == 0) {
    (void)xchg(0xFF);  // R2 second byte
    uint8_t ssr[64];
    if (readData(ssr, sizeof(ssr)) == Result::Ok) {
      const uint8_t au = ssr[10] >> 4;
      static const uint8_t AU_MB[6] = {8, 12, 16, 24, 32, 64};  // codes 0xA..0xF
      if (au >= 1 && au <= 9) {
        _au_blocks = 32u << (au - 1);  // 16 KB << (code - 1)
      } else if (au >= 0xA) {
        _au_blocks = uint32_t(AU_MB[au - 0xA]) * 2048u;
      }
    }
  }
  deselect();
  if (_spi_fault) {
    return Result::Timeout;
  }
  if (clock_hz > MAX_CLOCK_HZ && switchHighSpeed()) {
    _high_speed = true;
    setClock(clock_hz < HIGH_SPEED_CLOCK_HZ ? clock_hz : HIGH_SPEED_CLOCK_HZ);
    if (_clock_hz > HIGH_SPEED_CLOCK_HZ) {
      return Result::BadArgument;
    }
  }
  if (_spi_fault) {
    return Result::Timeout;
  }
  _ready = true;
  return Result::Ok;
}

// CMD6, function group 1 (access mode) to function 1 (high speed), the other
// groups unchanged: asked first (mode 0), then switched (mode 1). The card
// answers with a 64-byte status block; bits 379:376 (low half of byte 16) are
// the function it will use, or used. A card without CMD6 rejects the command.
bool SDBlockSPI::switchHighSpeed()
{
  static const uint32_t ARGS[2] = {0x00FFFFF1u, 0x80FFFFF1u};
  for (uint32_t arg : ARGS) {
    if (!waitReady(READY_TIMEOUT_US)) {
      deselect();
      return false;
    }
    if (command(6, arg) != 0) {
      deselect();
      return false;
    }
    uint8_t st[64];
    const Result r = readData(st, sizeof(st));
    deselect();
    if (r != Result::Ok || (st[16] & 0x0F) != 1) {
      return false;
    }
  }
  // The new mode holds 8 clocks after the status block.
  for (int i = 0; i < 2; i++) {
    xchg(0xFF);
  }
  return !_spi_fault;
}

// ---------------------------------------------------------------------------
// Waiting calls

SDBlockSPI::Result SDBlockSPI::readBlocks(uint32_t lba, uint8_t *dst, uint32_t n)
{
  Result r = checkUsable(false);
  if (r != Result::Ok) {
    return r;
  }
  if (n == 0 || lba >= _blocks || n > _blocks - lba) {
    return Result::BadArgument;
  }
  for (uint32_t i = 0; i < n; i++) {
    if (!waitReady(READY_TIMEOUT_US)) {
      deselect();
      return Result::Timeout;
    }
    _card_busy = false;
    if (command(17, lba + i) != 0) {
      deselect();
      return Result::CommandError;
    }
    r = readData(dst + i * BLOCK_SIZE, BLOCK_SIZE);
    deselect();
    if (r != Result::Ok) {
      return r;
    }
    yield();
  }
  return _spi_fault ? setFault(Result::Timeout) : Result::Ok;
}

SDBlockSPI::Result SDBlockSPI::writeBlocks(uint32_t lba, const uint8_t *src, uint32_t n)
{
  Result r = checkUsable(false);
  if (r != Result::Ok) {
    return r;
  }
  if (n == 0 || lba >= _blocks || n > _blocks - lba) {
    return Result::BadArgument;
  }
  static const uint8_t crc[2] = {0xFF, 0xFF};
  if (!waitReady(READY_TIMEOUT_US)) {
    deselect();
    return Result::Timeout;
  }
  _card_busy = false;
  // One block: CMD24. More: ACMD23 (pre-erase count) + CMD25, stop token.
  const bool multi = n > 1;
  uint8_t r1 = multi ? appCommand(23, n < ACMD23_MAX ? n : ACMD23_MAX) : 0;
  if (r1 == 0) {
    r1 = command(multi ? 25 : 24, lba);
  }
  if (r1 != 0) {
    deselect();
    return Result::CommandError;
  }
  r = Result::Ok;
  for (uint32_t i = 0; i < n && r == Result::Ok; i++) {
    const uint8_t pre[2] = {0xFF, multi ? TOKEN_MULTI : TOKEN_SINGLE};
    txOnly(pre, sizeof(pre));
    txOnly(src + i * BLOCK_SIZE, BLOCK_SIZE);
    txOnly(crc, sizeof(crc));
    const uint8_t resp = xchg(0xFF);
    if ((resp & 0x1F) != DATA_ACCEPTED) {
      r = Result::WriteError;
    } else if (!waitReady(BUSY_TIMEOUT_US)) {
      r = Result::Timeout;
    }
    yield();
  }
  if (multi) {
    const uint8_t stop = TOKEN_STOP;
    txOnly(&stop, 1);
    xchg(0xFF);  // one byte before the card signals busy
    if (!waitReady(BUSY_TIMEOUT_US) && r == Result::Ok) {
      r = Result::Timeout;
    }
  }
  deselect();
  if (_spi_fault) {
    return setFault(Result::Timeout);
  }
  return r;
}

SDBlockSPI::Result SDBlockSPI::sync(uint32_t timeout_us)
{
  const Result r = checkUsable(false);
  if (r != Result::Ok) {
    return r;
  }
  const bool done = waitReady(timeout_us);
  deselect();
  if (_spi_fault) {
    return setFault(Result::Timeout);
  }
  if (!done) {
    return Result::Timeout;
  }
  _card_busy = false;
  return Result::Ok;
}

// ---------------------------------------------------------------------------
// Stream calls

SDBlockSPI::Result SDBlockSPI::setPieceBytes(uint32_t bytes)
{
  if (bytes < MIN_PIECE_BYTES || bytes > PIECE_BYTES) {
    return Result::BadArgument;
  }
  if (_state != State::Idle) {
    return Result::BadState;
  }
  _piece = bytes;
  return Result::Ok;
}

SDBlockSPI::Result SDBlockSPI::streamOpen(uint32_t lba, uint32_t count)
{
  Result r = checkUsable(false);
  if (r != Result::Ok) {
    return r;
  }
  if (count == 0 || lba >= _blocks || count > _blocks - lba) {
    return Result::BadArgument;
  }
  // The stop token of a previous stream may still hold the card busy.
  r = pollBusy();
  if (r != Result::Ok) {
    return r;
  }
  if (!readyNow()) {
    _cs_port->BSRR = _cs_mask;
    return _spi_fault ? setFault(Result::Timeout) : Result::Busy;
  }
  uint8_t r1 = appCommand(23, count < ACMD23_MAX ? count : ACMD23_MAX);
  if (r1 == 0) {
    r1 = command(25, lba);
  }
  if (_spi_fault) {
    return setFault(Result::Timeout);
  }
  if (r1 != 0) {
    deselect();
    return Result::CommandError;
  }
  xchg(0xFF);  // one byte gap before the first start token
  _in_stream = true;
  _state = State::Streaming;
  _submitted = false;
  _off = 0;
  _blocks_written = 0;
  return Result::Ok;
}

SDBlockSPI::Result SDBlockSPI::submit()
{
  const Result r = checkUsable(true);
  if (r != Result::Ok) {
    return r;
  }
  if (_state != State::Streaming || _submitted) {
    return Result::BadState;
  }
  _submitted = true;
  return Result::Ok;
}

SDBlockSPI::Result SDBlockSPI::streamClose()
{
  const Result r = checkUsable(true);
  if (r != Result::Ok) {
    return r;
  }
  if (_state == State::Closing) {
    return Result::Ok;
  }
  if (_state != State::Streaming || _submitted) {
    return Result::BadState;
  }
  _state = State::Closing;
  return Result::Ok;
}

SDBlockSPI::Result SDBlockSPI::step()
{
  Result r = checkUsable(true);
  if (r != Result::Ok) {
    return r;
  }
  r = pollBusy();
  if (r != Result::Ok) {
    return r;
  }

  if (_state == State::Streaming && _submitted) {
    if (_off == 0) {
      const uint8_t token = TOKEN_MULTI;
      txOnly(&token, 1);
    }
    const uint32_t n = (BLOCK_SIZE - _off < _piece) ? BLOCK_SIZE - _off : _piece;
    txOnly(_buf + _off, n);
    _off += n;
    if (_off == BLOCK_SIZE) {
      static const uint8_t crc[2] = {0xFF, 0xFF};
      txOnly(crc, sizeof(crc));
      const uint8_t resp = xchg(0xFF);
      if (_spi_fault) {
        return setFault(Result::Timeout);
      }
      if ((resp & 0x1F) != DATA_ACCEPTED) {
        return setFault(Result::WriteError);
      }
      _off = 0;
      _submitted = false;
      _blocks_written++;
      _card_busy = true;
      _busy_t0 = micros();
    }
  } else if (_state == State::Closing) {
    const uint8_t stop = TOKEN_STOP;
    txOnly(&stop, 1);
    xchg(0xFF);  // one byte before the card signals busy
    _state = State::Idle;
    _card_busy = true;
    _busy_t0 = micros();
  }
  return _spi_fault ? setFault(Result::Timeout) : Result::Ok;
}

#endif  // !SPI_SR_TXP
