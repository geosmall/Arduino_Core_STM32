/*
 * FlashLog.cpp — append-only log storage on SPI NOR flash. See FlashLog.h.
 */

#include "FlashLog.h"

namespace {

constexpr uint32_t PAGE = SPIFlash::PAGE_SIZE;

uint8_t page_buf[SPIFlash::PAGE_SIZE];

inline uint32_t min3(uint32_t a, uint32_t b, uint32_t c)
{
  uint32_t m = a < b ? a : b;
  return m < c ? m : c;
}

void put32(uint8_t *p, uint32_t v)
{
  p[0] = uint8_t(v);
  p[1] = uint8_t(v >> 8);
  p[2] = uint8_t(v >> 16);
  p[3] = uint8_t(v >> 24);
}

uint32_t get32(const uint8_t *p)
{
  return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) |
         (uint32_t(p[3]) << 24);
}

const char HEX_DIGITS[] = "0123456789ABCDEF";

char *putHex(char *p, uint32_t v, int digits)
{
  for (int i = digits - 1; i >= 0; i--) {
    p[i] = HEX_DIGITS[v & 0xF];
    v >>= 4;
  }
  return p + digits;
}

}  // namespace

FlashLog::FlashLog(SPIFlash &flash, uint8_t *ring, uint32_t ring_size,
                   uint32_t region_start, uint32_t region_size)
  : _flash(flash), _ring(ring), _ring_size(ring_size),
    _region_start(region_start), _region_size(0), _region_size_req(region_size)
{
}

uint32_t FlashLog::crc32(const uint8_t *p, uint32_t n, uint32_t crc)
{
  crc = ~crc;
  while (n--) {
    crc ^= *p++;
    for (int k = 0; k < 8; k++) {
      crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
  }
  return ~crc;
}

uint16_t FlashLog::crc16(const uint8_t *p, uint32_t n)
{
  uint16_t crc = 0xFFFF;
  while (n--) {
    crc ^= uint16_t(*p++) << 8;
    for (int k = 0; k < 8; k++) {
      crc = (crc & 0x8000) ? uint16_t((crc << 1) ^ 0x1021) : uint16_t(crc << 1);
    }
  }
  return crc;
}

FlashLog::Result FlashLog::begin(uint32_t clock_hz)
{
  _ready = _logging = _draining = _full = false;
  _not_found = _unknown_part = _program_outstanding = false;
  _head = _tail = _count = 0;
  _write_off = 0;
  _errors = 0;

  const SPIFlash::Result r = _flash.begin(clock_hz);
  if (r == SPIFlash::Result::NotFound) {
    _not_found = true;
    return Result::NotFound;
  }
  if (r == SPIFlash::Result::UnknownPart) {
    _unknown_part = true;
    return Result::UnknownPart;
  }
  if (r != SPIFlash::Result::Ok) {
    return Result::NotReady;
  }

  const uint32_t chip = _flash.size();
  if ((_region_start % SPIFlash::BLOCK_64K) != 0 || _region_start >= chip) {
    return Result::BadArgument;
  }
  _region_size = _region_size_req ? _region_size_req : chip - _region_start;
  if ((_region_size % SPIFlash::BLOCK_64K) != 0 || _region_size > chip - _region_start) {
    return Result::BadArgument;
  }

  _write_off = scanEnd();
  _full = (_write_off >= _region_size);
  _ready = true;
  return Result::Ok;
}

void FlashLog::setWriteRule(uint32_t cap, uint32_t threshold)
{
  _cap = (cap == 0 || cap > PAGE) ? PAGE : cap;
  _threshold = threshold == 0 ? 1 : threshold;
}

bool FlashLog::pageErased(uint32_t offset)
{
  if (_flash.read(_region_start + offset, page_buf, PAGE) != SPIFlash::Result::Ok) {
    return false;
  }
  for (uint32_t i = 0; i < PAGE; i++) {
    if (page_buf[i] != 0xFF) {
      return false;
    }
  }
  return true;
}

// End of written data: first fully erased page by binary search, then the
// last non-0xFF byte of the page before it.
uint32_t FlashLog::scanEnd()
{
  const uint32_t pages = _region_size / PAGE;
  if (pageErased(0)) {
    return 0;
  }
  uint32_t lo = 0;        // known written
  uint32_t hi = pages;    // known erased (or one past the end)
  while (hi - lo > 1) {
    const uint32_t mid = lo + (hi - lo) / 2;
    if (pageErased(mid * PAGE)) {
      hi = mid;
    } else {
      lo = mid;
    }
  }
  if (_flash.read(_region_start + lo * PAGE, page_buf, PAGE) != SPIFlash::Result::Ok) {
    return (lo + 1) * PAGE;
  }
  uint32_t last = PAGE;
  while (last > 0 && page_buf[last - 1] == 0xFF) {
    last--;
  }
  return lo * PAGE + last;
}

void FlashLog::ringWrite(const uint8_t *src, uint32_t n)
{
  const uint32_t first = (n < _ring_size - _head) ? n : _ring_size - _head;
  memcpy(_ring + _head, src, first);
  memcpy(_ring, src + first, n - first);
  _head = (_head + n) % _ring_size;
  _count += n;
}

FlashLog::Result FlashLog::startLog(const void *header, uint32_t header_len)
{
  if (!_ready) {
    return Result::NotReady;
  }
  if (_full) {
    return Result::Full;
  }
  if (_logging || _count != 0 || _program_outstanding) {
    return Result::Busy;
  }
  if (header_len > 0xFFFF || START_BLOCK_SIZE + header_len > _ring_size) {
    return Result::BadArgument;
  }
  const uint32_t start = (_write_off + PAGE - 1) & ~(PAGE - 1);
  if (start + START_BLOCK_SIZE + header_len > _region_size) {
    _full = true;
    return Result::Full;
  }
  uint8_t block[START_BLOCK_SIZE];
  memset(block, 0, sizeof(block));
  put32(block, MAGIC);
  block[4] = uint8_t(FORMAT_VERSION);
  block[5] = uint8_t(FORMAT_VERSION >> 8);
  block[6] = uint8_t(header_len);
  block[7] = uint8_t(header_len >> 8);
  put32(block + 28, crc32(block, 28));

  _write_off = start;
  ringWrite(block, sizeof(block));
  if (header_len) {
    ringWrite(static_cast<const uint8_t *>(header), header_len);
  }
  _logging = true;
  _draining = false;
  return Result::Ok;
}

bool FlashLog::push(const void *data, uint32_t n)
{
  if (!_logging || n > _ring_size - _count) {
    _dropped++;
    return false;
  }
  ringWrite(static_cast<const uint8_t *>(data), n);
  return true;
}

void FlashLog::service()
{
  if (!_ready) {
    return;
  }
  if (_program_outstanding) {
    SPIFlash::Result r;
    const bool busy = _flash.isBusy(&r);
    if (r != SPIFlash::Result::Ok) {
      _errors++;
      return;
    }
    if (busy) {
      return;
    }
    _program_outstanding = false;
  }
  if (_count == 0 || (_count < _threshold && !_draining)) {
    return;
  }
  const uint32_t room = _region_size - _write_off;
  if (room == 0) {
    // Region full: the queued bytes cannot be stored.
    _full = true;
    _logging = false;
    _dropped++;
    _head = _tail = _count = 0;
    return;
  }
  uint32_t n = min3(_count, _cap, PAGE - (_write_off % PAGE));
  n = min3(n, room, _ring_size - _tail);
  if (_flash.pageProgram(_region_start + _write_off, _ring + _tail, n) !=
      SPIFlash::Result::Ok) {
    _errors++;
    return;
  }
  _tail = (_tail + n) % _ring_size;
  _count -= n;
  _write_off += n;
  _program_outstanding = true;
  if (_write_off >= _region_size) {
    // Region full: what is still queued can never be stored.
    _full = true;
    _logging = false;
    if (_count != 0) {
      _dropped++;
      _head = _tail = _count = 0;
    }
  }
}

void FlashLog::endLog()
{
  _logging = false;
  _draining = true;
}

FlashLog::Result FlashLog::flush(uint32_t timeout_us)
{
  if (!_ready) {
    return Result::NotReady;
  }
  const bool was_draining = _draining;
  _draining = true;
  const uint32_t t0 = micros();
  Result result = Result::Ok;
  while (_count != 0 || _program_outstanding) {
    if (_program_outstanding) {
      const SPIFlash::Result r = _flash.waitIdle(_flash.part()->page_program_max_us);
      if (r == SPIFlash::Result::Timeout) {
        _errors++;
        result = Result::Timeout;
        break;
      }
      if (r != SPIFlash::Result::Ok) {
        _errors++;
        result = Result::FlashError;
      }
      _program_outstanding = false;
    }
    if (_count != 0) {
      const uint32_t before = _count;
      service();
      if (_count == before && !_program_outstanding) {
        result = Result::FlashError;   // no progress: a program was refused
        break;
      }
    }
    if ((micros() - t0) >= timeout_us) {
      result = Result::Timeout;
      break;
    }
  }
  _draining = was_draining || !_logging;
  if (_count == 0 && !_logging) {
    _draining = false;
  }
  return result;
}

FlashLog::Result FlashLog::eraseUsed(void (*progress)(uint32_t done, uint32_t total))
{
  if (!_ready) {
    return Result::NotReady;
  }
  if (_logging || _count != 0 || _program_outstanding) {
    return Result::Busy;
  }
  const uint32_t blocks = (_write_off + SPIFlash::BLOCK_64K - 1) / SPIFlash::BLOCK_64K;
  for (uint32_t b = 0; b < blocks; b++) {
    if (_flash.erase64k(_region_start + b * SPIFlash::BLOCK_64K) != SPIFlash::Result::Ok) {
      _errors++;
      return Result::FlashError;
    }
    const SPIFlash::Result r = _flash.waitIdle(_flash.part()->erase_64k_max_ms * 1000u);
    if (r == SPIFlash::Result::Timeout) {
      _errors++;
      return Result::Timeout;
    }
    if (r != SPIFlash::Result::Ok) {
      _errors++;
      return Result::FlashError;
    }
    if (progress) {
      progress(b + 1, blocks);
    }
  }
  _write_off = 0;
  _full = false;
  return Result::Ok;
}

FlashLog::Result FlashLog::read(uint32_t offset, uint8_t *dst, uint32_t n)
{
  if (!_ready) {
    return Result::NotReady;
  }
  if (n == 0 || offset >= _region_size || n > _region_size - offset) {
    return Result::BadArgument;
  }
  return _flash.read(_region_start + offset, dst, n) == SPIFlash::Result::Ok
             ? Result::Ok
             : Result::FlashError;
}

// Writes all n bytes, retrying the unsent remainder after a short write();
// false after DUMP_STALL_MS with no progress.
static bool writeAll(Print &out, const char *buf, size_t n)
{
  uint32_t last = millis();
  while (n > 0) {
    const size_t w = out.write(reinterpret_cast<const uint8_t *>(buf), n);
    if (w > 0) {
      buf += w;
      n -= w;
      last = millis();
    } else if (millis() - last > FlashLog::DUMP_STALL_MS) {
      return false;
    }
  }
  return true;
}

FlashLog::Result FlashLog::dump(Print &out, uint32_t start, uint32_t len)
{
  if (!_ready) {
    return Result::NotReady;
  }
  if (start > _region_size || len > _region_size - start) {
    return Result::BadArgument;
  }
  char line[2 + 6 + 1 + 64 + 1 + 4 + 2];
  char *p = line;
  *p++ = 'B';
  *p++ = ' ';
  p = putHex(p, start, 6);
  *p++ = ' ';
  p = putHex(p, len, 6);
  *p++ = '\n';
  if (!writeAll(out, line, p - line)) {
    return Result::Timeout;
  }

  uint8_t data[32];
  uint32_t crc = 0;
  for (uint32_t done = 0; done < len;) {
    const uint32_t n = (len - done) < sizeof(data) ? (len - done) : sizeof(data);
    const uint32_t addr = start + done;
    if (_flash.read(_region_start + addr, data, n) != SPIFlash::Result::Ok) {
      p = line;
      *p++ = 'X';
      *p++ = ' ';
      p = putHex(p, addr, 6);
      *p++ = '\n';
      writeAll(out, line, p - line);
      return Result::FlashError;
    }
    crc = crc32(data, n, crc);
    p = line;
    *p++ = 'D';
    *p++ = ' ';
    p = putHex(p, addr, 6);
    *p++ = ' ';
    for (uint32_t i = 0; i < n; i++) {
      p = putHex(p, data[i], 2);
    }
    *p++ = ' ';
    p = putHex(p, crc16(data, n), 4);
    *p++ = '\n';
    if (!writeAll(out, line, p - line)) {
      return Result::Timeout;
    }
    done += n;
  }
  p = line;
  *p++ = 'E';
  *p++ = ' ';
  p = putHex(p, len, 6);
  *p++ = ' ';
  p = putHex(p, crc, 8);
  *p++ = '\n';
  if (!writeAll(out, line, p - line)) {
    return Result::Timeout;
  }
  return Result::Ok;
}

uint32_t FlashLog::findLogs(uint32_t *starts, uint32_t max)
{
  if (!_ready) {
    return 0;
  }
  uint32_t found = 0;
  uint8_t block[START_BLOCK_SIZE];
  for (uint32_t off = 0; off < _write_off && found < max; off += PAGE) {
    if (_flash.read(_region_start + off, block, sizeof(block)) != SPIFlash::Result::Ok) {
      break;
    }
    if (get32(block) == MAGIC && get32(block + 28) == crc32(block, 28)) {
      starts[found++] = off;
    }
  }
  return found;
}
