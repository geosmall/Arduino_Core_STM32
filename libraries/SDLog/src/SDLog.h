#pragma once

/*
 * SDLog.h — log writer for an SD card: plain FAT files written in flight as
 * raw block streams (PROVISIONAL INTERFACE: may change until the SD log
 * writer's craft bench passes).
 *
 * Record-format agnostic: callers push bytes; a log file holds exactly the
 * bytes pushed (from startLog() to endLog()), with no header of SDLog's own,
 * so a PC opens it as written (a .bbl file in Blackbox Explorer).
 *
 * FILES. /LOGS/LOGnnnnn.<ext> (8.3 names, nnnnn from 00001, ext the
 * caller's). Each log is one contiguous file reserved ahead with f_expand.
 *
 * TWO KINDS OF CALL.
 * - Waiting calls, while disarmed: begin(), finish(), list(), dump(),
 *   erase(), eraseAll(). They use FatFs and wait on the card.
 * - Flight calls: startLog(), push(), service(), endLog(). None waits on the
 *   card; none calls FatFs.
 * Rule for callers: no FatFs use of the volume (SDFS, minIni, ...) between
 * startLog() and finish().
 *
 * PREPARED FILE. The next log's file is reserved while disarmed (in begin()
 * and in finish()): f_expand of the configured reservation (halved down to
 * min_reserve_bytes when no contiguous run is free), its first LBA and its
 * directory sector recorded, and a zero block written at its first LBA as
 * the "unused" marker. REQUIREMENT ON CALLERS: a log never starts with 512
 * zero bytes. Until it is armed, the prepared file shows on a PC as a file
 * of the reservation's size that starts with zeros.
 *
 * IN FLIGHT. startLog() only sets state; service() opens the stream. Call
 * service() once per frame, after the outputs. Each call makes one bounded
 * action: in the data phase, either one SDBlockSPI::step() that sends a 48 B
 * piece of the block in transfer, or (block buffer free) the card's one-byte
 * busy check and one 512 B copy from the ring into the card's block buffer;
 * in a segment switch, one command phase or one step(). Data is written in segments, each opened with ACMD23(count) +
 * CMD25: first_segment_blocks, then segment_blocks each. At a segment's end
 * the stream is stopped; before each segment (the first included) the
 * cached directory sector is written as a one-block stream with the entry's
 * size set to that segment's end, then the segment is opened. The ring
 * absorbs the data meanwhile; a push that does not fit is dropped and
 * counted.
 *
 * POWER CUT. The file's size on the card is the end of the segment being
 * written (as Betaflight's asyncfatfs records the allocated size), so the
 * file covers everything the card kept, followed by stale data up to the
 * segment's end (whatever the card held there): a decoder of the log must
 * tolerate that tail. The card keeps what it has committed to its flash; a
 * card may hold blocks it accepted in a buffer until then, and a power cut
 * loses them (one card measured: up to 32 KiB, kept up to the last 32 KiB
 * boundary or the start of the stream being written). begin() then cuts a
 * log whose cluster chain is longer than its size to that size, and deletes
 * a file whose first block is zero (prepared and never written).
 *
 * AFTER DISARM. endLog(), then finish() (waiting): the ring drained, the last
 * block padded with zeros, the stream stopped, the file cut to the bytes
 * logged (a log with no bytes on the card is deleted and its number used
 * again), and the next file prepared. A card fault in flight stops logging;
 * finish() restarts the card and keeps the data up to the last completed
 * segment.
 *
 * DUMP. dump(out, name, start, len) writes text lines to a Print, in
 * FlashLog's format with 8-digit offsets (read by ci/flashlog_fetch.py):
 *   "B <start hex8> <len hex8>\n"
 *   "D <offset hex8> <up to 64 hex chars> <crc16 hex4>\n"  per 32 bytes;
 *        crc16 is CRC-16/CCITT-FALSE over the line's data bytes
 *   "E <len hex8> <crc32 hex8>\n"   CRC-32 (IEEE) over all bytes dumped
 *   On a read error: "X <offset hex8>\n" and the dump ends.
 * Each line is written whole (a short write() is retried with the rest);
 * with no progress for DUMP_STALL_MS the dump stops and returns Timeout.
 *
 * Static allocation only: the ring is the caller's (see SDLogStatic<N>).
 */

#include <Arduino.h>
#include <SDFS.h>

struct SDLogConfig {
  const char *ext = "BBL";                 // 1-3 characters, A-Z 0-9
  uint32_t reserve_bytes = 64UL << 20;     // per log; below 4 GiB
  uint32_t min_reserve_bytes = 1UL << 20;  // halving stops here
  uint32_t first_segment_blocks = 16;
  uint32_t segment_blocks = 128;
};

class SDLog {
public:
  enum class Result : uint8_t {
    Ok,
    NotReady,     // begin() not completed, or no file prepared
    NotMounted,   // the SDFS volume is not mounted
    Busy,         // a log is open or not yet finished
    Full,         // no space for a reservation, or log numbers used up
    BadArgument,
    NotFound,
    Timeout,
    CardError,    // the card faulted (SDBlockSPI result in cardResult())
    FsError,      // FatFs returned an error (in fsResult())
  };

  static constexpr uint32_t BLOCK = SDBlockSPI::BLOCK_SIZE;
  static constexpr uint32_t DUMP_STALL_MS = 50;
  static constexpr uint32_t MAX_NUMBER = 99999;

  SDLog(uint8_t *ring, uint32_t ring_size);

  // Takes the card from a mounted SDFS volume, creates /LOGS, recovers logs
  // cut short, finds the next number and prepares the next file. May be
  // called again (state is reset; a stream this object left open is
  // abandoned to the card's begin()).
  Result begin(SDFS_SPI &fs, const SDLogConfig &cfg = SDLogConfig());

  // Opens the prepared log: header (may be null) goes into the ring first.
  // No card I/O. Busy if a log is open or unfinished.
  Result startLog(const void *header, uint32_t header_len);

  // Copies n bytes into the ring, all or nothing. False (and dropped() + 1)
  // if no log is accepting or the ring lacks room.
  bool push(const void *data, uint32_t n);

  // The bounded step; see IN FLIGHT.
  void service();

  // Stops accepting; service() then drains the ring.
  void endLog();

  // Completes the log after endLog() (see AFTER DISARM), waiting up to
  // timeout_ms for the drain, then prepares the next file.
  Result finish(uint32_t timeout_ms);

  // One line per log in /LOGS: "L <name> <size>\n"; the prepared file as
  // "P <name> <size>\n".
  Result list(Print &out);
  // Size of a log; NotFound if absent.
  Result size(const char *name, uint32_t *bytes);
  Result dump(Print &out, const char *name, uint32_t start, uint32_t len);
  // Deletes one log (8.3 name as listed), or all; the prepared file is
  // made again if it was deleted.
  Result erase(const char *name);
  Result eraseAll();

  bool ready() const { return _ready; }
  bool prepared() const { return _prepared; }
  bool logging() const { return _logging; }
  // A log is open: from startLog() until finish().
  bool active() const { return _state != State::Idle; }
  bool full() const { return _full; }
  const char *name() const { return _name; }
  uint32_t reservedBytes() const { return _reserved_blocks * BLOCK; }
  uint32_t logBytes() const { return _log_bytes; }
  // Log bytes submitted to the card (the last block may still be in transfer).
  uint32_t storedBytes() const { return _stored_bytes; }
  uint32_t dropped() const { return _dropped; }
  uint32_t errors() const { return _errors; }
  uint32_t queued() const { return _count; }
  // Per log (reset by startLog()): ring peak, directory updates, longest
  // segment switch. dropped() and errors() count since construction.
  uint32_t queuedMax() const { return _queued_max; }
  uint32_t ringFree() const { return _ring_size - _count; }
  uint32_t dirUpdates() const { return _dir_updates; }
  // Longest segment switch (stop to reopen), in microseconds.
  uint32_t switchMaxUs() const { return _switch_max_us; }
  uint32_t recovered() const { return _recovered; }
  SDBlockSPI::Result cardResult() const { return _card_result; }
  FRESULT fsResult() const { return _fs_result; }

  static uint32_t crc32(const uint8_t *p, uint32_t n, uint32_t crc = 0);
  static uint16_t crc16(const uint8_t *p, uint32_t n);

private:
  enum class State : uint8_t {
    Idle,          // no log open
    Opening,       // next segment's streamOpen() pending
    Streaming,
    SegClosing,    // stop token sent at a segment end; waiting for idle
    DirOpening,
    DirFill,
    DirSending,
    DirClosing,
    FinalClosing,  // stop token after the last block
    Done,          // log written; finish() pending
    Failed,        // card fault; finish() pending
  };

  Result prepare();
  Result recoverAndScan();
  Result cutFile(const char *path, uint32_t len);
  Result chainBytes(FIL *fil, uint32_t *bytes);
  Result fsError(FRESULT fr);
  bool parseName(const char *name, uint32_t *number) const;
  void makeName(char *dst, uint32_t number) const;
  void makePath(char *dst, const char *name) const;
  uint32_t segLen() const;
  void streamStep();
  void closingStep();
  void fail(SDBlockSPI::Result r);
  void fillBlock(uint8_t *dst, uint32_t n);
  void ringWrite(const uint8_t *src, uint32_t n);
  void resetLog();

  SDFS_SPI *_fs = nullptr;
  SDBlockSPI *_card = nullptr;
  SDLogConfig _cfg;
  char _ext[4] = "BBL";

  uint8_t *_ring;
  uint32_t _ring_size;
  uint32_t _head = 0;
  uint32_t _tail = 0;
  uint32_t _count = 0;
  uint32_t _queued_max = 0;

  bool _ready = false;
  bool _prepared = false;
  bool _logging = false;
  bool _draining = false;
  bool _full = false;
  State _state = State::Idle;

  // Prepared / open file.
  char _name[13] = "";
  uint32_t _number = 0;
  uint32_t _lba0 = 0;
  uint32_t _reserved_blocks = 0;
  uint32_t _dir_lba = 0;
  uint32_t _dir_off = 0;

  // Progress of the open log.
  uint32_t _log_bytes = 0;      // bytes accepted into the ring
  uint32_t _stored_bytes = 0;   // log bytes submitted to the card
  uint32_t _blocks = 0;         // blocks submitted
  uint32_t _seg_end = 0;        // block index where the open segment ends
  uint32_t _committed = 0;      // bytes on the card with the card idle after them
  uint32_t _switch_t0 = 0;

  uint32_t _dropped = 0;
  uint32_t _errors = 0;
  uint32_t _dir_updates = 0;
  uint32_t _switch_max_us = 0;
  uint32_t _recovered = 0;
  SDBlockSPI::Result _card_result = SDBlockSPI::Result::Ok;
  FRESULT _fs_result = FR_OK;

  FIL _fil = {};
  alignas(4) uint8_t _dir[BLOCK];   // cached directory sector (scratch while disarmed)
};

// SDLog with its ring as a member: static SDLogStatic<5632> sdlog; (a size that
// is a multiple of 512 keeps each block's copy out of the ring in one piece).
template <uint32_t N>
class SDLogStatic : public SDLog {
public:
  SDLogStatic() : SDLog(_buf, N) {}

private:
  alignas(4) uint8_t _buf[N];
};
