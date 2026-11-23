#pragma once

/*
 * SDBlock.h — SD card block device, polled SPI back end (PROVISIONAL
 * INTERFACE: may change until the SD log writer's craft bench passes).
 *
 * Cards: SDHC/SDXC (CSD version 2, block addressed) in SPI mode. A
 * byte-addressed SDSC card is refused at begin() (UnsupportedCard).
 *
 * TWO KINDS OF CALL.
 * - Waiting calls, for use while disarmed: begin(), readBlocks(),
 *   writeBlocks(), sync(). Each wait is bounded by a timeout, and each
 *   block read or written calls yield() (a sketch may feed a watchdog there).
 * - Stream calls, for use in flight: streamOpen(), submit(), step(),
 *   streamClose(). None of them waits on the card. Each makes bounded
 *   progress and returns Busy when the card holds busy.
 *
 * STREAM. streamOpen(lba, count) sends ACMD23 (pre-erase count) and CMD25.
 * Blocks then go through the back end's own block buffer, one at a time:
 * when blockFree(), the caller fills block() with BLOCK_SIZE bytes and calls
 * submit(). Each step() makes one write step:
 *   1. if the card was left busy, a one-byte busy check; still busy: Busy;
 *   2. otherwise, if a block is submitted, at most one piece of it
 *      (pieceBytes() bytes, 48 on this back end; the last piece of a block
 *      is shorter). A block's first piece carries the start token, its last
 *      the CRC and the data response read, after which the card is busy and
 *      the buffer is free for the next block;
 *   3. otherwise, if streamClose() was called, the stop token; the card is
 *      then busy until a later step() sees it ready, and the stream is
 *      closed.
 * Chip select is held low from streamOpen() to the stop token. The caller
 * calls step() once per frame; the step's worst case is one piece plus one
 * byte. In a stream, a busy longer than BUSY_TIMEOUT_US or a rejected data
 * response faults the device, and so does an SPI flag wait at its spin limit
 * in any call: every call then returns the fault until begin() is called
 * again. Errors of the waiting calls are returned without a fault.
 *
 * Only whole blocks are written: streamClose() is refused with a block
 * partly sent or submitted and not sent; the caller pads its last block.
 * Waiting calls are refused (StreamOpen) while a stream is open.
 *
 * The bus is assumed dedicated to the card (chip select stays low through a
 * stream). Static allocation only.
 *
 * Supported SPI blocks: STM32F4, F7, G4 (TXE/RXNE flags). H7-class SPI is
 * not supported.
 */

#include <Arduino.h>
#include <SPI.h>

class SDBlockSPI {
public:
  enum class Result : uint8_t {
    Ok,
    Busy,            // the card holds busy; try again on a later call
    NotReady,        // begin() not completed
    NoCard,          // no response to CMD0
    UnsupportedCard, // not an SDv2 card, or SDSC (byte addressed)
    BadArgument,     // range outside the card, or a zero count
    BadState,        // call not valid in the stream's current state
    StreamOpen,      // a waiting call while a stream is open
    CommandError,    // R1 error, or no R1 within the response window
    ReadError,       // no data token, or an error token
    WriteError,      // data response other than "accepted"
    Timeout,         // a bounded wait expired (card busy or SPI flag)
  };

  static constexpr uint32_t BLOCK_SIZE = 512;
  static constexpr uint32_t PIECE_BYTES = 48;
  // Default-speed SD bus limit in SPI mode.
  static constexpr uint32_t MAX_CLOCK_HZ = 25000000;
  static constexpr uint32_t INIT_CLOCK_HZ = 400000;
  // Longest card busy accepted after a block or the stop token.
  static constexpr uint32_t BUSY_TIMEOUT_US = 500000;

  SDBlockSPI(SPI_TypeDef *instance, Pin mosi, Pin miso, Pin sclk, Pin cs);

  // Card initialisation at INIT_CLOCK_HZ, then the clock set to
  // min(clock_hz, MAX_CLOCK_HZ) and the CSD read for the block count.
  // Clears a fault. A card this object left inside a stream is first given
  // the rest of its block (0xFF) and the stop token, then reset by CMD0; the
  // same is tried for any card that does not answer CMD0.
  Result begin(uint32_t clock_hz);

  bool ready() const { return _ready; }
  uint32_t blockCount() const { return _blocks; }
  // Allocation unit from the SD status (ACMD13), in blocks; 0 if not read.
  uint32_t auBlocks() const { return _au_blocks; }
  uint32_t clockHz() const { return _clock_hz; }
  uint32_t pieceBytes() const { return PIECE_BYTES; }
  Result fault() const { return _fault; }
  // The last command sent and its R1 (0xFF: none), for diagnosing a failure.
  uint8_t lastCommand() const { return _last_cmd; }
  uint8_t lastR1() const { return _last_r1; }
  // CMD0 attempts the last begin() made (1-7; the third is preceded by a
  // block's worth of 0xFF and a stop token, the fourth to seventh by 1 ms).
  uint8_t beginAttempts() const { return _begin_attempts; }

  // --- Waiting calls (disarmed) --------------------------------------------

  // Reads n blocks from lba (CMD17 per block).
  Result readBlocks(uint32_t lba, uint8_t *dst, uint32_t n);

  // Writes n blocks at lba: CMD24 for one, ACMD23 + CMD25 for more; each
  // block's busy waited out.
  Result writeBlocks(uint32_t lba, const uint8_t *src, uint32_t n);

  // Waits until the card is not busy, up to timeout_us.
  Result sync(uint32_t timeout_us);

  // --- Stream calls (in flight; none waits on the card) --------------------

  // ACMD23(count) + CMD25(lba). Busy if the card is busy; the caller tries
  // again. count is the pre-erase count: fewer blocks may be written.
  Result streamOpen(uint32_t lba, uint32_t count);

  // The back end's block buffer. Fill it only while blockFree().
  uint8_t *block() { return _buf; }
  bool blockFree() const { return !_submitted; }

  // Marks block() as holding the next block of the stream.
  Result submit();

  // One write step; see STREAM. Ok when it made progress or had nothing to
  // do, Busy while the card holds busy, or the fault.
  Result step();

  // Requests the stop token, sent by a later step() once the last block is
  // out and the card is ready. BadState with a block submitted or part sent.
  Result streamClose();

  bool streaming() const { return _state != State::Idle; }
  // No stream open and the card seen ready after the last write.
  bool idle() const { return _state == State::Idle && !_card_busy; }
  uint32_t blocksWritten() const { return _blocks_written; }
  // Longest busy seen by step() since begin() or resetBusyMax().
  uint32_t busyMaxUs() const { return _busy_max_us; }
  void resetBusyMax() { _busy_max_us = 0; }

private:
  enum class State : uint8_t { Idle, Streaming, Closing };

  uint8_t xchg(uint8_t out);
  bool txOnly(const uint8_t *p, uint32_t n);
  void select();
  void deselect();
  void setClock(uint32_t hz);
  static uint32_t busClockHz(SPI_TypeDef *spi);
  uint8_t command(uint8_t idx, uint32_t arg);
  uint8_t appCommand(uint8_t idx, uint32_t arg);
  bool readyNow();
  bool waitReady(uint32_t timeout_us);
  Result readData(uint8_t *dst, uint32_t n);
  Result checkUsable(bool allow_stream) const;
  Result setFault(Result r);
  Result pollBusy();

  SPIClass _spi;
  SPI_TypeDef *_instance;
  Pin _cs;
  GPIO_TypeDef *_cs_port = nullptr;
  uint32_t _cs_mask = 0;

  bool _ready = false;
  bool _spi_fault = false;
  Result _fault = Result::Ok;
  uint32_t _blocks = 0;
  uint32_t _au_blocks = 0;
  uint32_t _clock_hz = 0;
  uint8_t _last_cmd = 0;
  uint8_t _last_r1 = 0xFF;
  uint8_t _begin_attempts = 0;

  State _state = State::Idle;
  bool _submitted = false;
  bool _card_busy = false;
  bool _in_stream = false;      // the card may be inside CMD25 (survives a fault)
  uint32_t _busy_t0 = 0;
  uint32_t _off = 0;            // bytes of the submitted block sent
  uint32_t _blocks_written = 0;
  uint32_t _busy_max_us = 0;

  // Placed by the back end; any CPU-reachable RAM suits polled SPI.
  alignas(4) uint8_t _buf[BLOCK_SIZE];
};
