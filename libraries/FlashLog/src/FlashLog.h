#pragma once

/*
 * FlashLog.h — append-only log storage on SPI NOR flash (PROVISIONAL
 * INTERFACE: may change until the flight-log user's craft bench passes).
 *
 * Record-format agnostic: callers push bytes; FlashLog knows logs, not
 * records.
 *
 * CALLING RULE. Call service() from the real-time task, after the outputs.
 * The core Scheduler has no fit check: a lower-priority task that blocked
 * on the flash would delay the flight frame by its full duration. service()
 * never waits on the chip and makes at most one flash write per call; its
 * worst-case time is bounded (one 2-byte status read, then at most one write
 * enable plus one page program of up to `cap` bytes).
 *
 * WRITE RULE. setWriteRule(cap, threshold): when the chip is idle and at least
 * `threshold` bytes are queued, one page program of whatever is queued, up to
 * the page boundary and up to `cap` bytes. A fixed chunk is cap == threshold.
 *
 * LAYOUT. Logs are written front to back through the region, each starting on
 * a page boundary with a 32-byte start block, then the caller's header, then
 * the caller's records. No wrap: when the region is full, logging stops and
 * full() is set; an unread log is never overwritten.
 *
 *   start block (little-endian):
 *     0  u32  magic 0x474F4C46 ("FLOG")
 *     4  u16  format version (1)
 *     6  u16  caller header length in bytes
 *     8  u32  reserved (0)
 *    12  u8[16] reserved (0)
 *    28  u32  CRC-32 (IEEE, reflected, init/xorout 0xFFFFFFFF) of bytes 0..27
 *
 * BOOT SCAN. begin() finds the end of written data by binary search for the
 * first fully erased page of the region. REQUIREMENT ON CALLERS: no log may
 * contain a whole page (256 aligned bytes) of 0xFF, or the scan can stop at
 * it. A record that carries a sequence number or timestamp meets this in
 * practice.
 *
 * DUMP. dump(out, start, len) writes text lines to a Print:
 *   "B <start hex> <len hex>\n"
 *   "D <addr hex6> <64 hex chars> <crc16 hex4>\n"  per 32 bytes (the last may
 *        be shorter); crc16 is CRC-16/CCITT-FALSE (poly 0x1021, init 0xFFFF)
 *        over the line's data bytes
 *   "E <len hex> <crc32 hex8>\n"                  CRC-32 (as above) over all
 *   On a read error: "X <addr hex6>\n" and the dump ends.
 *
 * Static allocation only: the RAM ring is supplied by the caller (see
 * FlashLogStatic<N>).
 */

#include <Arduino.h>
#include <SPIFlash.h>

class FlashLog {
public:
  enum class Result : uint8_t {
    Ok,
    NotReady,      // begin() not completed, or the chip failed it
    NotFound,      // no flash answered
    UnknownPart,   // JEDEC id not in the part table
    Busy,          // a log is open or the ring still holds data
    Full,          // region full
    BadArgument,
    Timeout,
    FlashError,    // a program or erase reported an error
  };

  static constexpr uint32_t START_BLOCK_SIZE = 32;
  static constexpr uint32_t MAGIC = 0x474F4C46;   // "FLOG"
  static constexpr uint16_t FORMAT_VERSION = 1;

  // region_size 0 = from region_start to the end of the chip.
  FlashLog(SPIFlash &flash, uint8_t *ring, uint32_t ring_size,
           uint32_t region_start = 0, uint32_t region_size = 0);

  // Starts the flash, then the boot scan. Logging is disabled on any failure;
  // the caller's flight path is unaffected.
  Result begin(uint32_t clock_hz);

  void setWriteRule(uint32_t cap, uint32_t threshold);

  // Opens a log at the next page boundary: start block, then `header`. The
  // ring must be empty (a previous log drained).
  Result startLog(const void *header, uint32_t header_len);

  // Copies n bytes into the ring, all or nothing. False (and dropped() + 1)
  // if no log is open or the ring lacks room.
  bool push(const void *data, uint32_t n);

  // The bounded write step; see CALLING RULE.
  void service();

  // Stops accepting records; service() then drains the ring regardless of
  // the threshold.
  void endLog();

  // Drains the ring, waiting on the chip (disarmed use only).
  Result flush(uint32_t timeout_us);

  // Erases the written part of the region by 64 KB blocks (disarmed use
  // only). progress(done, total) is called after each block if given.
  Result eraseUsed(void (*progress)(uint32_t done, uint32_t total) = nullptr);

  // CRC-framed text dump of [start, start + len) of the region, offsets
  // relative to the region start. See DUMP.
  Result dump(Print &out, uint32_t start, uint32_t len);

  // Raw read of the region, offsets relative to the region start.
  Result read(uint32_t offset, uint8_t *dst, uint32_t n);

  // Log enumeration: start offsets of the logs in the region, oldest first,
  // up to max. Walks the written region page by page (CLI-time use).
  uint32_t findLogs(uint32_t *starts, uint32_t max);

  bool ready() const { return _ready; }
  bool logging() const { return _logging; }
  bool full() const { return _full; }
  bool chipNotFound() const { return _not_found; }
  bool unknownPart() const { return _unknown_part; }
  uint32_t jedecId() const { return _flash.jedecId(); }
  uint32_t errors() const { return _errors; }
  uint32_t dropped() const { return _dropped; }
  uint32_t used() const { return _write_off; }
  uint32_t regionSize() const { return _region_size; }
  uint32_t queued() const { return _count; }
  uint32_t ringFree() const { return _ring_size - _count; }
  // A page program was issued and has not yet been seen to complete.
  bool programOutstanding() const { return _program_outstanding; }

  static uint32_t crc32(const uint8_t *p, uint32_t n, uint32_t crc = 0);
  static uint16_t crc16(const uint8_t *p, uint32_t n);

private:
  bool pageErased(uint32_t offset);
  uint32_t scanEnd();
  void ringWrite(const uint8_t *src, uint32_t n);

  SPIFlash &_flash;
  uint8_t *_ring;
  uint32_t _ring_size;
  uint32_t _region_start;
  uint32_t _region_size;
  uint32_t _region_size_req;

  uint32_t _head = 0;          // ring write index
  uint32_t _tail = 0;          // ring read index
  uint32_t _count = 0;         // bytes queued

  uint32_t _write_off = 0;     // next flash write offset in the region
  uint32_t _cap = SPIFlash::PAGE_SIZE;
  uint32_t _threshold = 1;

  bool _ready = false;
  bool _logging = false;
  bool _draining = false;
  bool _full = false;
  bool _not_found = false;
  bool _unknown_part = false;
  bool _program_outstanding = false;
  uint32_t _errors = 0;
  uint32_t _dropped = 0;
};

// FlashLog with its RAM ring as a member: static FlashLogStatic<2048> log(flash);
template <uint32_t N>
class FlashLogStatic : public FlashLog {
public:
  explicit FlashLogStatic(SPIFlash &flash, uint32_t region_start = 0,
                          uint32_t region_size = 0)
    : FlashLog(flash, _buf, N, region_start, region_size) {}

private:
  uint8_t _buf[N];
};
