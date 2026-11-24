/*
 * SDLog.cpp — log writer for an SD card. See SDLog.h.
 */

#include "SDLog.h"

using CR = SDBlockDevice::Result;

namespace {

const char DIR_PATH[] = "0:/LOGS";
const uint8_t ZERO_BLOCK[SDLog::BLOCK] = {};

// FAT directory entry fields.
constexpr uint32_t DIR_ENTRY = 32;
constexpr uint32_t DIR_FSTCLUSHI = 20;
constexpr uint32_t DIR_FSTCLUSLO = 26;
constexpr uint32_t DIR_FILESIZE = 28;

// Fragments read by the fast-seek link map; a log is one fragment.
constexpr uint32_t LINKMAP_ITEMS = 16;

void put32(uint8_t *p, uint32_t v)
{
  p[0] = uint8_t(v);
  p[1] = uint8_t(v >> 8);
  p[2] = uint8_t(v >> 16);
  p[3] = uint8_t(v >> 24);
}

uint16_t get16(const uint8_t *p)
{
  return uint16_t(p[0] | (p[1] << 8));
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

// Writes all n bytes, retrying the unsent remainder after a short write();
// false after DUMP_STALL_MS with no progress.
bool writeAll(Print &out, const char *buf, size_t n)
{
  uint32_t last = millis();
  while (n > 0) {
    const size_t w = out.write(reinterpret_cast<const uint8_t *>(buf), n);
    if (w > 0) {
      buf += w;
      n -= w;
      last = millis();
    } else if (millis() - last > SDLog::DUMP_STALL_MS) {
      return false;
    }
  }
  return true;
}

}  // namespace

SDLog::SDLog(uint8_t *ring, uint32_t ring_size) : _ring(ring), _ring_size(ring_size)
{
}

uint32_t SDLog::crc32(const uint8_t *p, uint32_t n, uint32_t crc)
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

uint16_t SDLog::crc16(const uint8_t *p, uint32_t n)
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

// ---------------------------------------------------------------------------
// Names

bool SDLog::parseName(const char *name, uint32_t *number) const
{
  // "LOGnnnnn.<ext>", upper case as FatFs returns 8.3 names.
  if (strncmp(name, "LOG", 3) != 0) {
    return false;
  }
  uint32_t n = 0;
  for (int i = 3; i < 8; i++) {
    if (name[i] < '0' || name[i] > '9') {
      return false;
    }
    n = n * 10 + uint32_t(name[i] - '0');
  }
  if (name[8] != '.' || strcmp(name + 9, _ext) != 0 || n == 0) {
    return false;
  }
  *number = n;
  return true;
}

void SDLog::makeName(char *dst, uint32_t number) const
{
  memcpy(dst, "LOG", 3);
  for (int i = 7; i >= 3; i--) {
    dst[i] = char('0' + number % 10);
    number /= 10;
  }
  dst[8] = '.';
  strcpy(dst + 9, _ext);
}

void SDLog::makePath(char *dst, const char *name) const
{
  strcpy(dst, DIR_PATH);
  strcat(dst, "/");
  strcat(dst, name);
}

SDLog::Result SDLog::fsError(FRESULT fr)
{
  _fs_result = fr;
  return Result::FsError;
}

// ---------------------------------------------------------------------------
// Disarmed: begin, prepare, recovery

void SDLog::resetLog()
{
  _head = _tail = _count = 0;
  _logging = _draining = false;
  _state = State::Idle;
  _log_bytes = _stored_bytes = _blocks = _seg_end = _committed = 0;
}

SDLog::Result SDLog::begin(SDFS_Card &fs, const SDLogConfig &cfg)
{
  _ready = _prepared = _full = false;
  resetLog();
  _name[0] = '\0';
  _fs_result = FR_OK;
  _card_result = CR::Ok;

  const size_t ext_len = cfg.ext ? strlen(cfg.ext) : 0;
  if (ext_len < 1 || ext_len > 3 || cfg.segment_blocks == 0 || cfg.first_segment_blocks == 0 ||
      cfg.min_reserve_bytes < BLOCK || cfg.reserve_bytes < cfg.min_reserve_bytes ||
      _ring_size < BLOCK) {
    return Result::BadArgument;
  }
  for (size_t i = 0; i < ext_len; i++) {
    const char c = cfg.ext[i];
    if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) {
      return Result::BadArgument;
    }
  }
  _cfg = cfg;
  memcpy(_ext, cfg.ext, ext_len + 1);

  if (!fs.isMounted() || fs.card() == nullptr) {
    return Result::NotMounted;
  }
  _fs = &fs;
  _card = fs.card();
  if (_card->streaming() || _card->fault() != CR::Ok) {
    // A stream left by an earlier begin() of this object, or a fault:
    // the card's begin() closes the stream and clears the fault.
    const CR r = _card->begin(_card->clockHz());
    if (r != CR::Ok) {
      _card_result = r;
      return Result::CardError;
    }
  }

  const FRESULT fr = f_mkdir(DIR_PATH);
  if (fr != FR_OK && fr != FR_EXIST) {
    return fsError(fr);
  }
  Result r = recoverAndScan();
  if (r != Result::Ok) {
    return r;
  }
  _ready = true;
  return prepare();
}

// Chain length of an open file in bytes, from the fast-seek link map (it
// walks the whole chain, whatever the size on the card says).
SDLog::Result SDLog::chainBytes(FIL *fil, uint32_t *bytes)
{
  DWORD tbl[LINKMAP_ITEMS];
  tbl[0] = LINKMAP_ITEMS;
  fil->cltbl = tbl;
  const FRESULT fr = f_lseek(fil, CREATE_LINKMAP);
  fil->cltbl = nullptr;
  if (fr != FR_OK) {
    return fsError(fr);
  }
  uint64_t clusters = 0;
  for (uint32_t i = 1; i + 1 < LINKMAP_ITEMS && tbl[i] != 0; i += 2) {
    clusters += tbl[i];
  }
  const uint64_t b = clusters * fil->obj.fs->csize * BLOCK;
  if (b > 0xFFFFFFFFull) {
    return Result::BadArgument;
  }
  *bytes = uint32_t(b);
  return Result::Ok;
}

// Cuts a log to len bytes and frees the rest of its chain. f_truncate frees
// clusters only past the size it believes the file has, which in-flight
// size updates may have lowered below the chain, so the open file's size is
// first set to its chain length.
SDLog::Result SDLog::cutFile(const char *path, uint32_t len)
{
  FRESULT fr = f_open(&_fil, path, FA_READ | FA_WRITE);
  if (fr != FR_OK) {
    return fsError(fr);
  }
  uint32_t chain = 0;
  Result r = chainBytes(&_fil, &chain);
  if (r != Result::Ok) {
    f_close(&_fil);
    return r;
  }
  if (len > chain) {
    len = chain;
  }
  _fil.obj.objsize = chain;
  if (len == chain) {
    // Nothing to free. f_truncate would not mark the file modified, so
    // rewrite its last byte to have f_close store the size.
    uint8_t b = 0;
    UINT n = 0;
    fr = len ? f_lseek(&_fil, len - 1) : FR_OK;
    if (fr == FR_OK && len) {
      fr = f_read(&_fil, &b, 1, &n);
    }
    if (fr == FR_OK && len) {
      fr = f_lseek(&_fil, len - 1);
    }
    if (fr == FR_OK && len) {
      fr = f_write(&_fil, &b, 1, &n);
    }
  } else {
    fr = f_lseek(&_fil, len);
    if (fr == FR_OK) {
      fr = f_truncate(&_fil);
    }
  }
  const FRESULT fc = f_close(&_fil);
  if (fr != FR_OK) {
    return fsError(fr);
  }
  if (fc != FR_OK) {
    return fsError(fc);
  }
  return Result::Ok;
}

// Scans /LOGS: deletes unused prepared files (first block zero), cuts logs
// whose chain is longer than their size, and sets the next number.
SDLog::Result SDLog::recoverAndScan()
{
  constexpr uint32_t MAX_UNUSED = 4;   // more are deleted at a later begin()
  uint32_t unused[MAX_UNUSED];
  uint32_t n_unused = 0;
  uint32_t max_number = 0;
  _recovered = 0;

  static DIR dir;
  static FILINFO fno;
  char path[sizeof(DIR_PATH) + 13];
  FRESULT fr = f_opendir(&dir, DIR_PATH);
  if (fr != FR_OK) {
    return fsError(fr);
  }
  for (;;) {
    fr = f_readdir(&dir, &fno);
    if (fr != FR_OK) {
      f_closedir(&dir);
      return fsError(fr);
    }
    if (fno.fname[0] == '\0') {
      break;
    }
    uint32_t number;
    if ((fno.fattrib & AM_DIR) || !parseName(fno.fname, &number)) {
      continue;
    }
    makePath(path, fno.fname);
    fr = f_open(&_fil, path, FA_READ);
    if (fr != FR_OK) {
      f_closedir(&dir);
      return fsError(fr);
    }
    uint32_t chain = 0;
    Result r = chainBytes(&_fil, &chain);
    UINT got = 0;
    bool zero = false;
    if (r == Result::Ok && f_lseek(&_fil, 0) == FR_OK &&
        f_read(&_fil, _dir, BLOCK, &got) == FR_OK && got == BLOCK) {
      zero = memcmp(_dir, ZERO_BLOCK, BLOCK) == 0;
    }
    const uint32_t size = uint32_t(f_size(&_fil));
    const uint32_t cluster = _fil.obj.fs->csize * BLOCK;
    f_close(&_fil);
    if (r != Result::Ok) {
      // Not a chain this writer made (fragmented beyond the link map): leave it.
      if (number > max_number) {
        max_number = number;
      }
      continue;
    }
    const uint64_t size_chain = ((uint64_t(size) + cluster - 1) / cluster) * cluster;
    if (zero) {
      if (n_unused < MAX_UNUSED) {
        unused[n_unused++] = number;
        continue;
      }
    } else if (chain > size_chain) {
      r = cutFile(path, size);
      if (r != Result::Ok) {
        f_closedir(&dir);
        return r;
      }
      _recovered++;
    }
    if (number > max_number) {
      max_number = number;
    }
  }
  f_closedir(&dir);

  for (uint32_t i = 0; i < n_unused; i++) {
    char name[13];
    makeName(name, unused[i]);
    makePath(path, name);
    fr = f_unlink(path);
    if (fr != FR_OK) {
      return fsError(fr);
    }
  }
  _number = max_number + 1;
  return Result::Ok;
}

SDLog::Result SDLog::prepare()
{
  _prepared = false;
  if (!_ready) {
    return Result::NotReady;
  }
  if (_number > MAX_NUMBER) {
    _full = true;
    return Result::Full;
  }
  char path[sizeof(DIR_PATH) + 13];
  makeName(_name, _number);
  makePath(path, _name);

  uint32_t reserve = _cfg.reserve_bytes & ~(BLOCK - 1);
  FRESULT fr = FR_DENIED;
  for (;;) {
    fr = f_open(&_fil, path, FA_CREATE_NEW | FA_WRITE);
    if (fr != FR_OK) {
      _name[0] = '\0';
      return fsError(fr);
    }
    fr = f_expand(&_fil, reserve, 1);
    if (fr == FR_OK) {
      break;
    }
    f_close(&_fil);
    f_unlink(path);
    if (fr != FR_DENIED || reserve / 2 < _cfg.min_reserve_bytes) {
      _name[0] = '\0';
      if (fr == FR_DENIED) {
        _full = true;
        return Result::Full;
      }
      return fsError(fr);
    }
    reserve = (reserve / 2) & ~(BLOCK - 1);
  }
  FATFS *vol = _fil.obj.fs;
  const uint32_t sclust = _fil.obj.sclust;
  _lba0 = uint32_t(vol->database + LBA_t(sclust - 2) * vol->csize);
  _dir_lba = uint32_t(_fil.dir_sect);
  _reserved_blocks = reserve / BLOCK;
  fr = f_close(&_fil);
  if (fr != FR_OK) {
    _name[0] = '\0';
    return fsError(fr);
  }

  // The directory sector, as f_close left it on the card.
  CR cr = _card->readBlocks(_dir_lba, _dir, 1);
  if (cr == CR::Ok) {
    cr = _card->writeBlocks(_lba0, ZERO_BLOCK, 1);
  }
  if (cr != CR::Ok) {
    _card_result = cr;
    _name[0] = '\0';
    return Result::CardError;
  }
  char entry[11];
  memset(entry, ' ', sizeof(entry));
  memcpy(entry, _name, 8);
  memcpy(entry + 8, _ext, strlen(_ext));
  _dir_off = BLOCK;
  for (uint32_t off = 0; off < BLOCK; off += DIR_ENTRY) {
    const uint8_t *e = _dir + off;
    const uint32_t cl = (uint32_t(get16(e + DIR_FSTCLUSHI)) << 16) | get16(e + DIR_FSTCLUSLO);
    if (memcmp(e, entry, sizeof(entry)) == 0 && cl == sclust) {
      _dir_off = off;
      break;
    }
  }
  if (_dir_off == BLOCK) {
    _name[0] = '\0';
    return fsError(FR_INT_ERR);
  }
  _prepared = true;
  return Result::Ok;
}

// ---------------------------------------------------------------------------
// Flight

void SDLog::ringWrite(const uint8_t *src, uint32_t n)
{
  const uint32_t first = (n < _ring_size - _head) ? n : _ring_size - _head;
  memcpy(_ring + _head, src, first);
  memcpy(_ring, src + first, n - first);
  _head = (_head + n) % _ring_size;
  _count += n;
  if (_count > _queued_max) {
    _queued_max = _count;
  }
}

SDLog::Result SDLog::startLog(const void *header, uint32_t header_len)
{
  if (!_ready || !_prepared) {
    return Result::NotReady;
  }
  if (_state != State::Idle || _count != 0) {
    return Result::Busy;
  }
  if (header_len > _ring_size) {
    return Result::BadArgument;
  }
  resetLog();
  _dir_updates = _switch_max_us = _queued_max = 0;
  _prepared = false;   // this file is now the open log
  _logging = true;
  _state = State::DirOpening;   // the size goes on the card before the first data
  _switch_t0 = micros();
  if (header_len) {
    ringWrite(static_cast<const uint8_t *>(header), header_len);
    _log_bytes = header_len;
  }
  return Result::Ok;
}

bool SDLog::push(const void *data, uint32_t n)
{
  if (!_logging || n > _ring_size - _count) {
    _dropped++;
    return false;
  }
  ringWrite(static_cast<const uint8_t *>(data), n);
  _log_bytes += n;
  return true;
}

void SDLog::endLog()
{
  _logging = false;
  if (_state != State::Idle) {
    _draining = true;
  }
}

void SDLog::fail(CR r)
{
  _card_result = r;
  _errors++;
  _logging = false;
  if (_count) {
    _dropped++;
  }
  _head = _tail = _count = 0;
  _state = State::Failed;
}

// Copies in 32-bit words when source, destination and length allow it: the
// library memcpy (newlib-nano, built for size) copies byte by byte, ~15 us
// for a block on an F405 at 168 MHz, inside service()'s budget.
// The attribute keeps GCC from turning the word loop back into a memcpy call.
__attribute__((optimize("no-tree-loop-distribute-patterns")))
static void copyBytes(uint8_t *dst, const uint8_t *src, uint32_t n)
{
  if (((uintptr_t(dst) | uintptr_t(src) | n) & 3u) == 0) {
    uint32_t *d = reinterpret_cast<uint32_t *>(dst);
    const uint32_t *s = reinterpret_cast<const uint32_t *>(src);
    uint32_t w = n / 4;
    // Eight words per pass, so the compiler can use multi-register loads
    // and stores (LDM/STM).
    for (; w >= 8; w -= 8, s += 8, d += 8) {
      const uint32_t a0 = s[0], a1 = s[1], a2 = s[2], a3 = s[3];
      const uint32_t a4 = s[4], a5 = s[5], a6 = s[6], a7 = s[7];
      d[0] = a0;
      d[1] = a1;
      d[2] = a2;
      d[3] = a3;
      d[4] = a4;
      d[5] = a5;
      d[6] = a6;
      d[7] = a7;
    }
    while (w--) {
      *d++ = *s++;
    }
  } else {
    memcpy(dst, src, n);
  }
}

// Copies n bytes (n <= BLOCK) from the ring and pads the block with zeros.
// Whole blocks leave the ring, so with a 4-byte-aligned ring whose size is a
// multiple of 4 the copies are word copies.
void SDLog::fillBlock(uint8_t *dst, uint32_t n)
{
  const uint32_t first = (n < _ring_size - _tail) ? n : _ring_size - _tail;
  copyBytes(dst, _ring + _tail, first);
  copyBytes(dst + first, _ring, n - first);
  if (n < BLOCK) {
    memset(dst + n, 0, BLOCK - n);
  }
  _tail = (_tail + n) % _ring_size;
  _count -= n;
}

// Length of the segment that starts at the next block.
uint32_t SDLog::segLen() const
{
  uint32_t seg = (_blocks == 0) ? _cfg.first_segment_blocks : _cfg.segment_blocks;
  const uint32_t left = _reserved_blocks - _blocks;
  return seg < left ? seg : left;
}

// A call either sends one piece of the block in transfer, or (buffer free)
// makes the card's busy check and fills the next block, whose first piece
// goes on the next call: a piece and a block copy never share a call.
void SDLog::streamStep()
{
  if (!_card->blockFree()) {
    const CR r = _card->step();
    if (r != CR::Ok && r != CR::Busy) {
      fail(r);
    }
    return;
  }
  if (_blocks == _seg_end) {
    // The segment's last block is out (its data response read).
    if (_blocks >= _reserved_blocks && (_count || _logging)) {
      // Reservation used up: what is queued or still to come is lost.
      _full = true;
      _logging = false;
      if (_count) {
        _dropped++;
      }
      _head = _tail = _count = 0;
    }
    const CR r = _card->streamClose();
    if (r != CR::Ok) {
      fail(r);
      return;
    }
    _switch_t0 = micros();
    _state = (_count == 0 && !_logging) ? State::FinalClosing : State::SegClosing;
    return;
  }
  uint32_t n = 0;
  if (_count >= BLOCK) {
    n = BLOCK;
  } else if (_count > 0 && !_logging) {
    n = _count;   // the last, partial block
  } else if (_count == 0 && !_logging) {
    const CR r = _card->streamClose();
    if (r != CR::Ok) {
      fail(r);
      return;
    }
    _state = State::FinalClosing;
    return;
  }
  // The busy check left by the previous block (one byte); the copy needs no
  // card, so it is made while the card may still be busy.
  const CR r = _card->step();
  if (r != CR::Ok && r != CR::Busy) {
    fail(r);
    return;
  }
  if (n) {
    fillBlock(_card->block(), n);
    const CR rs = _card->submit();
    if (rs != CR::Ok) {
      fail(rs);
      return;
    }
    _blocks++;
    _stored_bytes += n;
  }
}

void SDLog::closingStep()
{
  const CR r = _card->step();
  if (r != CR::Ok && r != CR::Busy) {
    fail(r);
    return;
  }
  if (!_card->idle()) {
    return;
  }
  switch (_state) {
  case State::SegClosing:
    _committed = _stored_bytes;
    _state = State::DirOpening;
    break;
  case State::DirClosing:
    _dir_updates++;
    _state = State::Opening;
    break;
  case State::FinalClosing:
    _committed = _stored_bytes;
    _state = State::Done;
    break;
  default:
    break;
  }
}

void SDLog::service()
{
  switch (_state) {
  case State::Opening: {
    const uint32_t seg = segLen();
    const CR r = _card->streamOpen(_lba0 + _blocks, seg);
    if (r == CR::Busy) {
      return;
    }
    if (r != CR::Ok) {
      fail(r);
      return;
    }
    _seg_end = _blocks + seg;
    if (_blocks != 0) {
      const uint32_t us = micros() - _switch_t0;
      if (us > _switch_max_us) {
        _switch_max_us = us;
      }
    }
    _state = State::Streaming;
    return;
  }
  case State::Streaming:
    streamStep();
    return;
  case State::SegClosing:
  case State::DirClosing:
  case State::FinalClosing:
    closingStep();
    return;
  case State::DirOpening: {
    if (_blocks == 0 && _count == 0 && !_logging) {
      _state = State::Done;   // nothing logged: no stream
      return;
    }
    const CR r = _card->streamOpen(_dir_lba, 1);
    if (r == CR::Busy) {
      return;
    }
    if (r != CR::Ok) {
      fail(r);
      return;
    }
    _state = State::DirFill;
    return;
  }
  case State::DirFill: {
    // The end of the segment about to be written (Betaflight's "physical
    // size"): after a power cut the file covers every block the card kept,
    // followed by stale data up to the segment's end.
    put32(_dir + _dir_off + DIR_FILESIZE, (_blocks + segLen()) * BLOCK);
    memcpy(_card->block(), _dir, BLOCK);
    const CR r = _card->submit();
    if (r != CR::Ok) {
      fail(r);
      return;
    }
    _state = State::DirSending;
    return;
  }
  case State::DirSending: {
    const CR r = _card->step();
    if (r != CR::Ok && r != CR::Busy) {
      fail(r);
      return;
    }
    if (_card->blockFree()) {
      const CR c = _card->streamClose();
      if (c != CR::Ok) {
        fail(c);
        return;
      }
      _state = State::DirClosing;
    }
    return;
  }
  case State::Idle:
  case State::Done:
  case State::Failed:
    return;
  }
}

// ---------------------------------------------------------------------------
// Disarmed: finish, list, dump, erase

SDLog::Result SDLog::finish(uint32_t timeout_ms)
{
  if (!_ready) {
    return Result::NotReady;
  }
  if (_state == State::Idle) {
    return Result::Ok;
  }
  if (_logging) {
    return Result::Busy;   // endLog() first
  }
  Result result = Result::Ok;
  const uint32_t t0 = millis();
  while (_state != State::Done && _state != State::Failed) {
    service();
    if (millis() - t0 > timeout_ms) {
      result = Result::Timeout;
      break;
    }
  }
  if (_state == State::Failed) {
    result = Result::CardError;
  }
  if (_state != State::Done) {
    // Abandon the stream; the card's begin() sends it the stop token.
    const CR r = _card->begin(_card->clockHz());
    if (r != CR::Ok) {
      _card_result = r;
      resetLog();
      return Result::CardError;
    }
  }
  char path[sizeof(DIR_PATH) + 13];
  makePath(path, _name);
  const uint32_t keep = (_state == State::Done) ? _stored_bytes : _committed;
  resetLog();
  if (keep == 0) {
    // Nothing on the card: no file, and its number is used again.
    const FRESULT fr = f_unlink(path);
    if (fr != FR_OK) {
      return fsError(fr);
    }
  } else {
    const Result rc = cutFile(path, keep);
    if (rc != Result::Ok) {
      return rc;
    }
    _number++;
  }
  const Result rp = prepare();
  return result != Result::Ok ? result : rp;
}

SDLog::Result SDLog::list(Print &out)
{
  if (!_ready) {
    return Result::NotReady;
  }
  if (_state != State::Idle) {
    return Result::Busy;
  }
  static DIR dir;
  static FILINFO fno;
  FRESULT fr = f_opendir(&dir, DIR_PATH);
  if (fr != FR_OK) {
    return fsError(fr);
  }
  char line[2 + 12 + 1 + 10 + 2];
  for (;;) {
    fr = f_readdir(&dir, &fno);
    if (fr != FR_OK || fno.fname[0] == '\0') {
      break;
    }
    uint32_t number;
    if ((fno.fattrib & AM_DIR) || !parseName(fno.fname, &number)) {
      continue;
    }
    const bool prep = _prepared && strcmp(fno.fname, _name) == 0;
    snprintf(line, sizeof(line), "%c %s %lu\n", prep ? 'P' : 'L', fno.fname,
             (unsigned long)fno.fsize);
    if (!writeAll(out, line, strlen(line))) {
      f_closedir(&dir);
      return Result::Timeout;
    }
  }
  f_closedir(&dir);
  return fr == FR_OK ? Result::Ok : fsError(fr);
}

SDLog::Result SDLog::size(const char *name, uint32_t *bytes)
{
  uint32_t number;
  if (!_ready) {
    return Result::NotReady;
  }
  if (name == nullptr || !parseName(name, &number)) {
    return Result::BadArgument;
  }
  char path[sizeof(DIR_PATH) + 13];
  makePath(path, name);
  FILINFO fno;
  const FRESULT fr = f_stat(path, &fno);
  if (fr == FR_NO_FILE) {
    return Result::NotFound;
  }
  if (fr != FR_OK) {
    return fsError(fr);
  }
  *bytes = uint32_t(fno.fsize);
  return Result::Ok;
}

SDLog::Result SDLog::dump(Print &out, const char *name, uint32_t start, uint32_t len)
{
  uint32_t number;
  if (!_ready) {
    return Result::NotReady;
  }
  if (_state != State::Idle) {
    return Result::Busy;
  }
  if (name == nullptr || !parseName(name, &number)) {
    return Result::BadArgument;
  }
  char path[sizeof(DIR_PATH) + 13];
  makePath(path, name);
  FRESULT fr = f_open(&_fil, path, FA_READ);
  if (fr == FR_NO_FILE) {
    return Result::NotFound;
  }
  if (fr != FR_OK) {
    return fsError(fr);
  }
  const uint32_t fsize = uint32_t(f_size(&_fil));
  if (start > fsize || len > fsize - start || f_lseek(&_fil, start) != FR_OK) {
    f_close(&_fil);
    return Result::BadArgument;
  }

  Result result = Result::Ok;
  char line[2 + 8 + 1 + 64 + 1 + 4 + 2];
  char *p = line;
  *p++ = 'B';
  *p++ = ' ';
  p = putHex(p, start, 8);
  *p++ = ' ';
  p = putHex(p, len, 8);
  *p++ = '\n';
  if (!writeAll(out, line, p - line)) {
    f_close(&_fil);
    return Result::Timeout;
  }
  uint8_t data[32];
  uint32_t crc = 0;
  for (uint32_t done = 0; done < len;) {
    const uint32_t want = (len - done) < sizeof(data) ? (len - done) : sizeof(data);
    const uint32_t off = start + done;
    UINT got = 0;
    fr = f_read(&_fil, data, want, &got);
    if (fr != FR_OK || got != want) {
      p = line;
      *p++ = 'X';
      *p++ = ' ';
      p = putHex(p, off, 8);
      *p++ = '\n';
      writeAll(out, line, p - line);
      result = fr != FR_OK ? fsError(fr) : fsError(FR_INT_ERR);
      break;
    }
    crc = crc32(data, want, crc);
    p = line;
    *p++ = 'D';
    *p++ = ' ';
    p = putHex(p, off, 8);
    *p++ = ' ';
    for (uint32_t i = 0; i < want; i++) {
      p = putHex(p, data[i], 2);
    }
    *p++ = ' ';
    p = putHex(p, crc16(data, want), 4);
    *p++ = '\n';
    if (!writeAll(out, line, p - line)) {
      result = Result::Timeout;
      break;
    }
    done += want;
  }
  f_close(&_fil);
  if (result != Result::Ok) {
    return result;
  }
  p = line;
  *p++ = 'E';
  *p++ = ' ';
  p = putHex(p, len, 8);
  *p++ = ' ';
  p = putHex(p, crc, 8);
  *p++ = '\n';
  return writeAll(out, line, p - line) ? Result::Ok : Result::Timeout;
}

SDLog::Result SDLog::erase(const char *name)
{
  uint32_t number;
  if (!_ready) {
    return Result::NotReady;
  }
  if (_state != State::Idle) {
    return Result::Busy;
  }
  if (name == nullptr || !parseName(name, &number)) {
    return Result::BadArgument;
  }
  char path[sizeof(DIR_PATH) + 13];
  makePath(path, name);
  const FRESULT fr = f_unlink(path);
  if (fr == FR_NO_FILE) {
    return Result::NotFound;
  }
  if (fr != FR_OK) {
    return fsError(fr);
  }
  if (_prepared && strcmp(name, _name) == 0) {
    return prepare();
  }
  return Result::Ok;
}

SDLog::Result SDLog::eraseAll()
{
  if (!_ready) {
    return Result::NotReady;
  }
  if (_state != State::Idle) {
    return Result::Busy;
  }
  static DIR dir;
  static FILINFO fno;
  char path[sizeof(DIR_PATH) + 13];
  // Rescan after each delete rather than delete while the directory is open.
  for (;;) {
    FRESULT fr = f_opendir(&dir, DIR_PATH);
    if (fr != FR_OK) {
      return fsError(fr);
    }
    bool found = false;
    for (;;) {
      fr = f_readdir(&dir, &fno);
      if (fr != FR_OK || fno.fname[0] == '\0') {
        break;
      }
      uint32_t number;
      if (!(fno.fattrib & AM_DIR) && parseName(fno.fname, &number)) {
        found = true;
        break;
      }
    }
    f_closedir(&dir);
    if (fr != FR_OK) {
      return fsError(fr);
    }
    if (!found) {
      break;
    }
    makePath(path, fno.fname);
    fr = f_unlink(path);
    if (fr != FR_OK) {
      return fsError(fr);
    }
  }
  _full = false;
  _number = 1;
  return prepare();
}
