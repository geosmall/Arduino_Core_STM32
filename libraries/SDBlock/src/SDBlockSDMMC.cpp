/*
 * SDBlockSDMMC.cpp — SD card block device on SDMMC1. See SDBlockSDMMC.h.
 */

#include "SDBlockSDMMC.h"

#if defined(SDBLOCK_HAS_SDMMC)

namespace {

constexpr uint32_t R_NONE  = 0;                             // no response
constexpr uint32_t R_SHORT = SDMMC_CMD_WAITRESP_0;          // R1, R6, R7
constexpr uint32_t R_NOCRC = SDMMC_CMD_WAITRESP_1;          // R3
constexpr uint32_t R_LONG  = SDMMC_CMD_WAITRESP_0 | SDMMC_CMD_WAITRESP_1;  // R2

constexpr uint32_t DATA_ERRORS = SDMMC_STA_DCRCFAIL | SDMMC_STA_DTIMEOUT | SDMMC_STA_TXUNDERR |
                                 SDMMC_STA_RXOVERR | SDMMC_STA_DABORT | SDMMC_STA_IDMATE;

// Every static flag (RM0433 55.10.11).
constexpr uint32_t CLEAR_ALL = 0x1FE00FFF;

// R1 (card status): the error bits, the state field, READY_FOR_DATA, APP_CMD.
constexpr uint32_t R1_ERRORS = 0xFDFFE008;
constexpr uint32_t R1_READY = 1UL << 8;
constexpr uint32_t R1_APP_CMD = 1UL << 5;
constexpr uint32_t STATE_TRAN = 4;

constexpr uint32_t COMMAND_TIMEOUT_US = 10000;   // the SDMMC's own is 64 CK
constexpr uint32_t ACMD41_TIMEOUT_MS = 1000;
constexpr uint32_t ABORT_TIMEOUT_US = 100000;
constexpr int BEGIN_ATTEMPTS = 5;

constexpr uint32_t BLOCK_LOG2 = 9;
constexpr uint32_t ACMD41_ARG = 0x40300000;      // HCS, 3.2-3.4 V
constexpr uint32_t CMD8_ARG = 0x1AA;             // 2.7-3.6 V, check pattern

uint32_t cardState(uint32_t r1)
{
  return (r1 >> 9) & 0xF;
}

}  // namespace

// ---------------------------------------------------------------------------
// Commands

// Starts a command; its result is taken by pollResponse() or waitResponse().
void SDBlockSDMMC::startCommand(uint32_t idx, uint32_t arg, uint32_t flags)
{
  _last_cmd = uint8_t(idx);
  SDMMC1->ICR = CLEAR_ALL;
  SDMMC1->ARG = arg;
  SDMMC1->CMD = idx | flags | SDMMC_CMD_CPSMEN;
}

// The response of the command under way (R1): Busy, Ok, or an error.
SDBlockDevice::Result SDBlockSDMMC::pollResponse()
{
  const uint32_t sta = SDMMC1->STA;
  if (sta & (SDMMC_STA_CTIMEOUT | SDMMC_STA_CCRCFAIL)) {
    return fail(Result::CommandError);
  }
  if (!(sta & SDMMC_STA_CMDREND)) {
    return Result::Busy;
  }
  _last_r1 = SDMMC1->RESP1;
  return (_last_r1 & R1_ERRORS) ? fail(Result::CommandError) : Result::Ok;
}

// Waits for the end of a command of any response type; bits in the response
// are the caller's to check.
SDBlockDevice::Result SDBlockSDMMC::waitResponse(bool check_crc)
{
  const uint32_t t0 = micros();
  for (;;) {
    const uint32_t sta = SDMMC1->STA;
    if (sta & (SDMMC_STA_CMDREND | SDMMC_STA_CMDSENT)) {
      return Result::Ok;
    }
    if (sta & SDMMC_STA_CCRCFAIL) {
      return check_crc ? fail(Result::CommandError) : Result::Ok;
    }
    if ((sta & SDMMC_STA_CTIMEOUT) || micros() - t0 > COMMAND_TIMEOUT_US) {
      return fail(Result::CommandError);
    }
  }
}

// An R1 command, waited for.
SDBlockDevice::Result SDBlockSDMMC::command(uint32_t idx, uint32_t arg)
{
  startCommand(idx, arg, R_SHORT);
  const Result r = waitResponse(true);
  if (r != Result::Ok) {
    return r;
  }
  _last_r1 = SDMMC1->RESP1;
  return (_last_r1 & R1_ERRORS) ? fail(Result::CommandError) : Result::Ok;
}

// CMD13 until the card is in the transfer state and ready.
SDBlockDevice::Result SDBlockSDMMC::waitTransfer(uint32_t timeout_us)
{
  const uint32_t t0 = micros();
  for (;;) {
    const Result r = command(13, _rca);
    if (r != Result::Ok) {
      return r;
    }
    if (cardState(_last_r1) == STATE_TRAN && (_last_r1 & R1_READY)) {
      return Result::Ok;
    }
    if (micros() - t0 > timeout_us) {
      return fail(Result::Timeout);
    }
  }
}

// ---------------------------------------------------------------------------
// State

SDBlockDevice::Result SDBlockSDMMC::checkUsable(bool allow_stream) const
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

// Records the SDMMC status of a failure.
SDBlockDevice::Result SDBlockSDMMC::fail(Result r)
{
  _last_sta = SDMMC1->STA;
  return r;
}

// The SDMMC block is reset, which also ends any IDMA access to the buffer.
SDBlockDevice::Result SDBlockSDMMC::setFault(Result r)
{
  __HAL_RCC_SDMMC1_FORCE_RESET();
  __HAL_RCC_SDMMC1_RELEASE_RESET();
  _fault = r;
  _state = State::Idle;
  _phase = Phase::None;
  _queued = 0;
  return r;
}

// After a failed transfer of a waiting call: CMD12 ends it on the card and
// aborts the data path. Returns r, or faults when that does not work.
SDBlockDevice::Result SDBlockSDMMC::recover(Result r)
{
  const uint8_t cmd = _last_cmd;
  const uint32_t r1 = _last_r1;
  const uint32_t sta = _last_sta;
  _phase = Phase::None;
  _queued = 0;
  startCommand(12, 0, R_SHORT | SDMMC_CMD_CMDSTOP);
  (void)waitResponse(true);   // a card already in the transfer state refuses CMD12
  const uint32_t t0 = micros();
  bool stopped = true;
  while (SDMMC1->STA & SDMMC_STA_DPSMACT) {
    if (micros() - t0 > ABORT_TIMEOUT_US) {
      stopped = false;
      break;
    }
  }
  SDMMC1->IDMACTRL = 0;
  // CMD13 may first report the refused CMD12 (a bit cleared by reading).
  (void)command(13, _rca);
  // Erratum 2.8.3 (ES0392): the receive buffer is not reset after an IDMA
  // error, so such a failure resets the block as a fault.
  const bool ok = stopped && !(sta & SDMMC_STA_IDMATE) &&
                  waitTransfer(BUSY_TIMEOUT_US) == Result::Ok;
  _last_cmd = cmd;
  _last_r1 = r1;
  _last_sta = sta;
  return ok ? r : setFault(r);
}

// ---------------------------------------------------------------------------
// Initialisation

void SDBlockSDMMC::setClock(uint32_t max_hz, bool wide)
{
  uint32_t div = (_kernel_hz + 2 * max_hz - 1) / (2 * max_hz);
  if (div < 1) {
    div = 1;
  } else if (div > SDMMC_CLKCR_CLKDIV) {
    div = SDMMC_CLKCR_CLKDIV;
  }
  // Hardware flow control off: the IDMA reads AXI SRAM at many times the
  // card's rate, and an underrun is reported (TXUNDERR).
  SDMMC1->CLKCR = (div << SDMMC_CLKCR_CLKDIV_Pos) | (wide ? SDMMC_CLKCR_WIDBUS_0 : 0u);
  _clock_hz = _kernel_hz / (2 * div);
  // Data timeout: BUSY_TIMEOUT_US in card clocks.
  SDMMC1->DTIMER = uint32_t(uint64_t(_clock_hz) * BUSY_TIMEOUT_US / 1000000u);
}

SDBlockDevice::Result SDBlockSDMMC::begin(uint32_t clock_hz)
{
  _ready = false;
  _fault = Result::Ok;
  _state = State::Idle;
  _phase = Phase::None;
  _queued = 0;
  _blocks = 0;
  _au_blocks = 0;
  _clock_hz = 0;
  _rca = 0;
  _blocks_written = 0;
  _busy_max_us = 0;

  // SDMMC1's IDMA reaches AXI SRAM only (RM0433 2.1.6).
  const uintptr_t lo = uintptr_t(_buf);
  if (lo < D1_AXISRAM_BASE || lo + sizeof(_buf) > D1_AXISRAM_BASE + 0x80000u || clock_hz == 0) {
    return Result::BadArgument;
  }

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  GPIO_InitTypeDef gpio = {};
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio.Alternate = GPIO_AF12_SDMMC1;
  gpio.Pull = GPIO_PULLUP;
  gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
  HAL_GPIO_Init(GPIOC, &gpio);
  gpio.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOD, &gpio);
  gpio.Pull = GPIO_NOPULL;
  gpio.Pin = GPIO_PIN_12;
  HAL_GPIO_Init(GPIOC, &gpio);

  __HAL_RCC_SDMMC1_CLK_ENABLE();
  __HAL_RCC_SDMMC1_FORCE_RESET();
  __HAL_RCC_SDMMC1_RELEASE_RESET();
  _kernel_hz = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SDMMC);
  if (_kernel_hz == 0) {
    return Result::BadArgument;   // the variant does not clock the SDMMC
  }
  setClock(INIT_CLOCK_HZ, false);
  SDMMC1->POWER = SDMMC_POWER_PWRCTRL;
  delay(3);   // over 1 ms and 74 card clocks before the first command

  // CMD0, which a card accepts in any state, then CMD8.
  Result r = Result::NoCard;
  for (int attempt = 0; attempt < BEGIN_ATTEMPTS && r != Result::Ok; attempt++) {
    if (attempt) {
      delay(10);
    }
    startCommand(0, 0, R_NONE);
    r = waitResponse(true);
    delay(1);
    if (r == Result::Ok) {
      startCommand(8, CMD8_ARG, R_SHORT);
      r = waitResponse(true);
    }
  }
  if (r != Result::Ok) {
    return Result::NoCard;
  }
  if ((SDMMC1->RESP1 & 0xFFF) != CMD8_ARG) {
    return Result::UnsupportedCard;
  }

  // ACMD41 until the card is powered up (OCR bit 31).
  uint32_t ocr = 0;
  const uint32_t t0 = millis();
  do {
    r = command(55, 0);
    if (r != Result::Ok) {
      return r;
    }
    startCommand(41, ACMD41_ARG, R_NOCRC);
    r = waitResponse(false);
    if (r != Result::Ok) {
      return r;
    }
    ocr = SDMMC1->RESP1;
    if (!(ocr & (1UL << 31)) && millis() - t0 > ACMD41_TIMEOUT_MS) {
      return fail(Result::Timeout);
    }
  } while (!(ocr & (1UL << 31)));
  if (!(ocr & (1UL << 30))) {
    return Result::UnsupportedCard;   // CCS = 0: SDSC, byte addressed
  }

  startCommand(2, 0, R_LONG);
  r = waitResponse(true);
  if (r != Result::Ok) {
    return r;
  }
  startCommand(3, 0, R_SHORT);
  r = waitResponse(true);
  if (r != Result::Ok) {
    return r;
  }
  _rca = SDMMC1->RESP1 & 0xFFFF0000u;

  startCommand(9, _rca, R_LONG);
  r = waitResponse(true);
  if (r != Result::Ok) {
    return r;
  }
  if ((SDMMC1->RESP1 >> 30) != 1) {
    return Result::UnsupportedCard;   // not CSD version 2
  }
  // C_SIZE: CSD bits 69:48.
  const uint32_t c_size = ((SDMMC1->RESP2 & 0x3F) << 16) | (SDMMC1->RESP3 >> 16);
  const uint32_t blocks = (c_size + 1) * 1024;

  const uint32_t hz = clock_hz < MAX_CLOCK_HZ ? clock_hz : MAX_CLOCK_HZ;
  setClock(hz, false);
  if (_clock_hz > hz) {
    return Result::BadArgument;   // the divider's range does not reach it
  }
  r = command(7, _rca);
  if (r == Result::Ok) {
    r = waitTransfer(BUSY_TIMEOUT_US);
  }
  // 4-bit bus: card detect pull-up off D3 (ACMD42), then ACMD6.
  if (r == Result::Ok) {
    r = command(55, _rca);
  }
  if (r == Result::Ok) {
    r = command(42, 0);
  }
  if (r == Result::Ok) {
    r = command(55, _rca);
  }
  if (r == Result::Ok) {
    r = command(6, 2);
  }
  if (r != Result::Ok) {
    return r;
  }
  setClock(hz, true);

  // ACMD13: SD status, 64 bytes. AU_SIZE (bits 431:428) gives the allocation
  // unit; a card that does not answer leaves it unknown (0).
  r = command(55, _rca);
  if (r == Result::Ok) {
    r = readTransfer(13, 0, 64, 6);
  }
  if (r == Result::Ok) {
    const uint8_t au = _buf[10] >> 4;
    static const uint8_t AU_MB[6] = {8, 12, 16, 24, 32, 64};  // codes 0xA..0xF
    if (au >= 1 && au <= 9) {
      _au_blocks = 32u << (au - 1);  // 16 KB << (code - 1)
    } else if (au >= 0xA) {
      _au_blocks = uint32_t(AU_MB[au - 0xA]) * 2048u;
    }
  } else {
    r = recover(r);
    if (_fault != Result::Ok) {
      return r;
    }
  }
  r = waitTransfer(BUSY_TIMEOUT_US);
  if (r != Result::Ok) {
    return r;
  }
  _blocks = blocks;
  _ready = true;
  return Result::Ok;
}

SDBlockDevice::Result SDBlockSDMMC::setChunk(uint32_t blocks, bool pre_erase)
{
  if (blocks == 0 || blocks > MAX_CHUNK_BLOCKS) {
    return Result::BadArgument;
  }
  if (_state != State::Idle || _phase != Phase::None) {
    return Result::BadState;
  }
  _chunk = blocks;
  _pre_erase = pre_erase;
  return Result::Ok;
}

// ---------------------------------------------------------------------------
// Reads

// One data read of bytes (a power of two per block) into the buffer.
SDBlockDevice::Result SDBlockSDMMC::readTransfer(uint32_t idx, uint32_t arg, uint32_t bytes,
                                                 uint32_t size_log2)
{
  // No dirty line of the buffer may be written back over the received data.
  SCB_CleanInvalidateDCache_by_Addr(reinterpret_cast<uint32_t *>(_buf), int32_t(bytes));
  SDMMC1->DLEN = bytes;
  SDMMC1->DCTRL = (size_log2 << SDMMC_DCTRL_DBLOCKSIZE_Pos) | SDMMC_DCTRL_DTDIR;
  SDMMC1->IDMABASE0 = uint32_t(_buf);
  SDMMC1->IDMACTRL = SDMMC_IDMA_IDMAEN;
  startCommand(idx, arg, R_SHORT | SDMMC_CMD_CMDTRANS);
  Result r = waitResponse(true);
  if (r != Result::Ok) {
    return r;
  }
  _last_r1 = SDMMC1->RESP1;
  if (_last_r1 & R1_ERRORS) {
    return fail(Result::CommandError);
  }
  const uint32_t t0 = micros();
  for (;;) {
    const uint32_t sta = SDMMC1->STA;
    if (sta & DATA_ERRORS) {
      return fail(Result::ReadError);
    }
    if (sta & SDMMC_STA_DATAEND) {
      break;
    }
    if (micros() - t0 > 2 * BUSY_TIMEOUT_US) {
      return fail(Result::Timeout);
    }
  }
  SDMMC1->IDMACTRL = 0;
  SCB_InvalidateDCache_by_Addr(reinterpret_cast<uint32_t *>(_buf), int32_t(bytes));
  return Result::Ok;
}

SDBlockDevice::Result SDBlockSDMMC::readBlocks(uint32_t lba, uint8_t *dst, uint32_t n)
{
  Result r = checkUsable(false);
  if (r != Result::Ok) {
    return r;
  }
  if (n == 0 || lba >= _blocks || n > _blocks - lba) {
    return Result::BadArgument;
  }
  while (n) {
    const uint32_t k = n < MAX_CHUNK_BLOCKS ? n : MAX_CHUNK_BLOCKS;
    r = readTransfer(k > 1 ? 18 : 17, lba, k * BLOCK_SIZE, BLOCK_LOG2);
    if (r == Result::Ok && k > 1) {
      startCommand(12, 0, R_SHORT | SDMMC_CMD_CMDSTOP);
      r = waitResponse(true);
    }
    if (r == Result::Ok) {
      r = waitTransfer(BUSY_TIMEOUT_US);
    }
    if (r != Result::Ok) {
      return recover(r);
    }
    memcpy(dst, _buf, k * BLOCK_SIZE);
    lba += k;
    dst += k * BLOCK_SIZE;
    n -= k;
    yield();
  }
  return Result::Ok;
}

// ---------------------------------------------------------------------------
// Writes: one chunk, advanced a phase at a time

// Sends the first n blocks of the buffer (cleaned from the cache already).
void SDBlockSDMMC::startChunk(uint32_t lba, uint32_t n)
{
  _chunk_lba = lba;
  _chunk_n = n;
  _chunk_t0 = micros();
  if (_pre_erase && n > 1) {
    startCommand(55, _rca, R_SHORT);
    _phase = Phase::App;
  } else {
    startWrite();
  }
}

void SDBlockSDMMC::startWrite()
{
  SDMMC1->DLEN = _chunk_n * BLOCK_SIZE;
  SDMMC1->DCTRL = BLOCK_LOG2 << SDMMC_DCTRL_DBLOCKSIZE_Pos;   // host to card
  SDMMC1->IDMABASE0 = uint32_t(_buf);
  SDMMC1->IDMACTRL = SDMMC_IDMA_IDMAEN;
  startCommand(_chunk_n > 1 ? 25 : 24, _chunk_lba, R_SHORT | SDMMC_CMD_CMDTRANS);
  _phase = Phase::Write;
}

// One look at the chunk's phase, and the start of the next phase when it is
// over. Ok when the chunk is on the card, Busy before that, or the error.
SDBlockDevice::Result SDBlockSDMMC::advance()
{
  Result r;
  switch (_phase) {
  case Phase::None:
    return Result::Ok;
  case Phase::App:
    r = pollResponse();
    if (r != Result::Ok) {
      return r;
    }
    if (!(_last_r1 & R1_APP_CMD)) {
      return fail(Result::CommandError);
    }
    startCommand(23, _chunk_n, R_SHORT);
    _phase = Phase::PreErase;
    return Result::Busy;
  case Phase::PreErase:
    r = pollResponse();
    if (r != Result::Ok) {
      return r;
    }
    startWrite();
    return Result::Busy;
  case Phase::Write: {
    r = pollResponse();
    if (r != Result::Ok) {
      return r;
    }
    // DATAEND: every block sent, its CRC status good and its busy over.
    const uint32_t sta = SDMMC1->STA;
    if (sta & DATA_ERRORS) {
      return fail((sta & SDMMC_STA_DTIMEOUT) ? Result::Timeout : Result::WriteError);
    }
    if (!(sta & SDMMC_STA_DATAEND)) {
      return Result::Busy;
    }
    SDMMC1->IDMACTRL = 0;
    if (_chunk_n > 1) {
      startCommand(12, 0, R_SHORT | SDMMC_CMD_CMDSTOP);
      _phase = Phase::StopResp;
    } else {
      startCommand(13, _rca, R_SHORT);
      _phase = Phase::Status;
    }
    return Result::Busy;
  }
  case Phase::StopResp:
    r = pollResponse();
    if (r != Result::Ok) {
      return r;
    }
    _phase = Phase::StopBusy;
    return Result::Busy;
  case Phase::StopBusy:
    // R1b: D0 held low until the card has finished.
    if (SDMMC1->STA & (SDMMC_STA_BUSYD0 | SDMMC_STA_DPSMACT)) {
      return Result::Busy;
    }
    startCommand(13, _rca, R_SHORT);
    _phase = Phase::Status;
    return Result::Busy;
  case Phase::Status:
    r = pollResponse();
    if (r != Result::Ok) {
      return r;
    }
    if (cardState(_last_r1) != STATE_TRAN || !(_last_r1 & R1_READY)) {
      startCommand(13, _rca, R_SHORT);
      return Result::Busy;
    }
    _phase = Phase::None;
    return Result::Ok;
  }
  return Result::Ok;
}

SDBlockDevice::Result SDBlockSDMMC::advanceTimed()
{
  const Result r = advance();
  if (r == Result::Busy && micros() - _chunk_t0 > BUSY_TIMEOUT_US) {
    return fail(Result::Timeout);
  }
  return r;
}

SDBlockDevice::Result SDBlockSDMMC::writeBlocks(uint32_t lba, const uint8_t *src, uint32_t n)
{
  Result r = checkUsable(false);
  if (r != Result::Ok) {
    return r;
  }
  if (n == 0 || lba >= _blocks || n > _blocks - lba) {
    return Result::BadArgument;
  }
  while (n) {
    const uint32_t k = n < MAX_CHUNK_BLOCKS ? n : MAX_CHUNK_BLOCKS;
    memcpy(_buf, src, k * BLOCK_SIZE);
    SCB_CleanDCache_by_Addr(reinterpret_cast<uint32_t *>(_buf), int32_t(k * BLOCK_SIZE));
    startChunk(lba, k);
    do {
      r = advanceTimed();
    } while (r == Result::Busy);
    if (r != Result::Ok) {
      return recover(r);
    }
    lba += k;
    src += k * BLOCK_SIZE;
    n -= k;
    yield();
  }
  return Result::Ok;
}

SDBlockDevice::Result SDBlockSDMMC::sync(uint32_t timeout_us)
{
  const Result r = checkUsable(false);
  return r == Result::Ok ? waitTransfer(timeout_us) : r;
}

// ---------------------------------------------------------------------------
// Stream calls

SDBlockDevice::Result SDBlockSDMMC::streamOpen(uint32_t lba, uint32_t count)
{
  const Result r = checkUsable(false);
  if (r != Result::Ok) {
    return r;
  }
  if (count == 0 || lba >= _blocks || count > _blocks - lba) {
    return Result::BadArgument;
  }
  _state = State::Streaming;
  _queued = 0;
  _stream_lba = lba;
  _stream_left = count;
  _blocks_written = 0;
  return Result::Ok;
}

SDBlockDevice::Result SDBlockSDMMC::submit()
{
  const Result r = checkUsable(true);
  if (r != Result::Ok) {
    return r;
  }
  if (_state != State::Streaming || !blockFree()) {
    return Result::BadState;
  }
  if (_stream_left == 0) {
    return Result::BadArgument;
  }
  SCB_CleanDCache_by_Addr(reinterpret_cast<uint32_t *>(block()), int32_t(BLOCK_SIZE));
  _queued++;
  _stream_left--;
  return Result::Ok;
}

SDBlockDevice::Result SDBlockSDMMC::streamClose()
{
  const Result r = checkUsable(true);
  if (r != Result::Ok) {
    return r;
  }
  if (_state == State::Closing) {
    return Result::Ok;
  }
  if (_state != State::Streaming || !blockFree()) {
    return Result::BadState;
  }
  _state = State::Closing;
  return Result::Ok;
}

SDBlockDevice::Result SDBlockSDMMC::step()
{
  Result r = checkUsable(true);
  if (r != Result::Ok || _state == State::Idle) {
    return r;
  }
  if (_phase != Phase::None) {
    r = advanceTimed();
    if (r == Result::Busy) {
      return r;
    }
    if (r != Result::Ok) {
      return setFault(r);
    }
    const uint32_t dt = micros() - _chunk_t0;
    _busy_max_us = dt > _busy_max_us ? dt : _busy_max_us;
    _stream_lba += _chunk_n;
    _blocks_written += _chunk_n;
    _queued = 0;
  } else if (_queued >= _chunk || (_state == State::Closing && _queued)) {
    startChunk(_stream_lba, _queued);
    return Result::Ok;
  }
  if (_state == State::Closing && _queued == 0) {
    _state = State::Idle;
  }
  return Result::Ok;
}

#endif  // SDBLOCK_HAS_SDMMC
