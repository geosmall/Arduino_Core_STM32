#pragma once

/*
 * SDBlockDevice.h — the interface of an SD card block device, implemented by
 * the back ends SDBlockSPI and SDBlockSDMMC and used by SDFS and SDLog
 * (PROVISIONAL INTERFACE: may change until the SD log writer's craft bench
 * passes).
 *
 * TWO KINDS OF CALL.
 * - Waiting calls, for use while disarmed: begin(), readBlocks(),
 *   writeBlocks(), sync(). Each wait is bounded by a timeout, and yield() is
 *   called between transfers (a sketch may feed a watchdog there).
 * - Stream calls, for use in flight: streamOpen(), submit(), step(),
 *   streamClose(). None of them waits on the card. Each makes bounded
 *   progress; step() and streamOpen() return Busy when the card holds busy.
 *
 * STREAM. streamOpen(lba, count) starts a run of up to count blocks at lba.
 * Blocks go through the back end's own block buffer: when blockFree(), the
 * caller fills block() with BLOCK_SIZE bytes and calls submit(). The caller
 * calls step() once per frame; each call makes one bounded action. A
 * submitted block is on its way to the card, and may still be in the back
 * end's buffer, until the stream is closed and idle(). streamClose() is
 * accepted only while blockFree(); submitted blocks are then sent by later
 * step() calls and the stream is closed when streaming() is false.
 *
 * FAULT. In a stream, a busy longer than BUSY_TIMEOUT_US or a failed
 * transfer faults the device: every call then returns the fault until
 * begin() is called again.
 *
 * Objects are the caller's, static; nothing is deleted through this class.
 */

#include <Arduino.h>

class SDBlockDevice {
public:
  enum class Result : uint8_t {
    Ok,
    Busy,            // the card holds busy; try again on a later call
    NotReady,        // begin() not completed
    NoCard,          // no card answered
    UnsupportedCard, // not an SDv2 card, or SDSC (byte addressed)
    BadArgument,     // range outside the card, a zero count, a bad clock or buffer
    BadState,        // call not valid in the stream's current state
    StreamOpen,      // a waiting call while a stream is open
    CommandError,    // error bits in the card's response, or no response
    ReadError,       // data read failed
    WriteError,      // data write failed or refused by the card
    Timeout,         // a bounded wait expired
  };

  static constexpr uint32_t BLOCK_SIZE = 512;
  // Default-speed SD bus limit (SPI mode and 4-bit mode at 3.3 V).
  static constexpr uint32_t MAX_CLOCK_HZ = 25000000;
  // Longest card busy accepted.
  static constexpr uint32_t BUSY_TIMEOUT_US = 500000;

  // Card initialisation, then the clock set to at most
  // min(clock_hz, MAX_CLOCK_HZ) and the card's size read. Clears a fault.
  virtual Result begin(uint32_t clock_hz) = 0;

  virtual bool ready() const = 0;
  virtual uint32_t blockCount() const = 0;
  // Allocation unit from the SD status (ACMD13), in blocks; 0 if not read.
  virtual uint32_t auBlocks() const = 0;
  virtual uint32_t clockHz() const = 0;
  virtual Result fault() const = 0;

  // --- Waiting calls (disarmed) --------------------------------------------
  virtual Result readBlocks(uint32_t lba, uint8_t *dst, uint32_t n) = 0;
  virtual Result writeBlocks(uint32_t lba, const uint8_t *src, uint32_t n) = 0;
  // Waits until the card is not busy, up to timeout_us.
  virtual Result sync(uint32_t timeout_us) = 0;

  // --- Stream calls (in flight; none waits on the card) --------------------
  virtual Result streamOpen(uint32_t lba, uint32_t count) = 0;
  // Where the next block goes. Fill it only while blockFree().
  virtual uint8_t *block() = 0;
  virtual bool blockFree() const = 0;
  // Takes block() as the next block of the stream.
  virtual Result submit() = 0;
  // One bounded action. Ok when it made progress or had nothing to do, Busy
  // while the card works, or the fault.
  virtual Result step() = 0;
  // Ends the stream after the blocks submitted. BadState unless blockFree().
  virtual Result streamClose() = 0;

  virtual bool streaming() const = 0;
  // No stream open and the card seen ready after the last write.
  virtual bool idle() const = 0;
  // Blocks of the stream sent to the card.
  virtual uint32_t blocksWritten() const = 0;
  // Longest time the card held up a stream, in microseconds, since begin()
  // or resetBusyMax(), as seen by step(). What it covers is per back end.
  virtual uint32_t busyMaxUs() const = 0;
  virtual void resetBusyMax() = 0;

protected:
  ~SDBlockDevice() = default;
};
