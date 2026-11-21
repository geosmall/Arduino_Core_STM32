/*
 * BlackboxBBL.cpp — encoder for Betaflight's blackbox log format. See
 * BlackboxBBL.h.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * The value encodings (variable byte, ZigZag, TAG2_3S32, TAG8_4S16 and
 * TAG8_8SVB) are Betaflight's src/main/blackbox/blackbox_encoding.c
 * (commit f97bdbcf5) with the output redirected to a Buffer; the frame,
 * header and event layout follow Betaflight's blackbox.c and the reference
 * decoder (blackbox-tools src/parser.c, commit 57c7695).
 */

#include "BlackboxBBL.h"

#include <string.h>

namespace bbl {

namespace {

// ---------------------------------------------------------------------------
// Value encodings (Betaflight blackbox_encoding.c)

uint32_t zigzagEncode(int32_t value)
{
  return (uint32_t)((value << 1) ^ (value >> 31));
}

void writeUnsignedVB(Buffer &out, uint32_t value)
{
  // While this isn't the final byte (only 7 bits per byte)
  while (value > 127) {
    out.put((uint8_t)(value | 0x80));  // high bit: more bytes follow
    value >>= 7;
  }
  out.put((uint8_t)value);
}

void writeSignedVB(Buffer &out, int32_t value)
{
  writeUnsignedVB(out, zigzagEncode(value));
}

// A 2-bit tag followed by 3 signed fields of 2, 4, 6 or 32 bits.
void writeTag2_3S32(Buffer &out, const int32_t *values)
{
  enum { BITS_2 = 0, BITS_4 = 1, BITS_6 = 2, BITS_32 = 3 };
  enum { BYTES_1 = 0, BYTES_2 = 1, BYTES_3 = 2, BYTES_4 = 3 };

  int selector = BITS_2;
  for (int x = 0; x < 3; x++) {
    if (values[x] >= 32 || values[x] < -32) {  // more than 6 bits
      selector = BITS_32;
      break;
    }
    if (values[x] >= 8 || values[x] < -8) {  // more than 4 bits
      if (selector < BITS_6) {
        selector = BITS_6;
      }
    } else if (values[x] >= 2 || values[x] < -2) {  // more than 2 bits
      if (selector < BITS_4) {
        selector = BITS_4;
      }
    }
  }

  switch (selector) {
  case BITS_2:
    out.put((uint8_t)((selector << 6) | ((values[0] & 0x03) << 4) | ((values[1] & 0x03) << 2) |
                      (values[2] & 0x03)));
    break;
  case BITS_4:
    out.put((uint8_t)((selector << 6) | (values[0] & 0x0F)));
    out.put((uint8_t)((values[1] << 4) | (values[2] & 0x0F)));
    break;
  case BITS_6:
    out.put((uint8_t)((selector << 6) | (values[0] & 0x3F)));
    out.put((uint8_t)values[1]);
    out.put((uint8_t)values[2]);
    break;
  case BITS_32: {
    // A second selector per field, assuming at least 8 bits each; encoded in
    // reverse order so the first field is in the low bits.
    int selector2 = 0;
    for (int x = 2; x >= 0; x--) {
      selector2 <<= 2;
      if (values[x] < 128 && values[x] >= -128) {
        selector2 |= BYTES_1;
      } else if (values[x] < 32768 && values[x] >= -32768) {
        selector2 |= BYTES_2;
      } else if (values[x] < 8388608 && values[x] >= -8388608) {
        selector2 |= BYTES_3;
      } else {
        selector2 |= BYTES_4;
      }
    }
    out.put((uint8_t)((selector << 6) | selector2));
    for (int x = 0; x < 3; x++, selector2 >>= 2) {
      const int bytes = (selector2 & 0x03) + 1;
      for (int b = 0; b < bytes; b++) {
        out.put((uint8_t)(values[x] >> (8 * b)));
      }
    }
    break;
  }
  }
}

// An 8-bit selector followed by four signed fields of 0, 4, 8 or 16 bits.
void writeTag8_4S16(Buffer &out, const int32_t *values)
{
  enum { FIELD_ZERO = 0, FIELD_4BIT = 1, FIELD_8BIT = 2, FIELD_16BIT = 3 };

  uint8_t selector = 0;
  // Reverse order so the first field is in the low bits
  for (int x = 3; x >= 0; x--) {
    selector <<= 2;
    if (values[x] == 0) {
      selector |= FIELD_ZERO;
    } else if (values[x] < 8 && values[x] >= -8) {
      selector |= FIELD_4BIT;
    } else if (values[x] < 128 && values[x] >= -128) {
      selector |= FIELD_8BIT;
    } else {
      selector |= FIELD_16BIT;
    }
  }
  out.put(selector);

  int nibbleIndex = 0;
  uint8_t buffer = 0;
  for (int x = 0; x < 4; x++, selector >>= 2) {
    switch (selector & 0x03) {
    case FIELD_ZERO:
      break;
    case FIELD_4BIT:
      if (nibbleIndex == 0) {
        buffer = (uint8_t)(values[x] << 4);  // high bits first
        nibbleIndex = 1;
      } else {
        out.put((uint8_t)(buffer | (values[x] & 0x0F)));
        nibbleIndex = 0;
      }
      break;
    case FIELD_8BIT:
      if (nibbleIndex == 0) {
        out.put((uint8_t)values[x]);
      } else {
        out.put((uint8_t)(buffer | ((values[x] >> 4) & 0x0F)));
        buffer = (uint8_t)(values[x] << 4);
      }
      break;
    case FIELD_16BIT:
      if (nibbleIndex == 0) {
        out.put((uint8_t)(values[x] >> 8));  // high byte first
        out.put((uint8_t)values[x]);
      } else {
        out.put((uint8_t)(buffer | ((values[x] >> 12) & 0x0F)));
        out.put((uint8_t)(values[x] >> 4));
        buffer = (uint8_t)(values[x] << 4);
      }
      break;
    }
  }
  if (nibbleIndex == 1) {
    out.put(buffer);
  }
}

// count (1..8) signed VB fields after a one-byte header marking the non-zero
// ones; a single field has no header.
void writeTag8_8SVB(Buffer &out, const int32_t *values, int count)
{
  if (count <= 0) {
    return;
  }
  if (count == 1) {
    writeSignedVB(out, values[0]);
    return;
  }
  uint8_t header = 0;
  for (int i = count - 1; i >= 0; i--) {  // first field in the low bit
    header <<= 1;
    if (values[i] != 0) {
      header |= 0x01;
    }
  }
  out.put(header);
  for (int i = 0; i < count; i++) {
    if (values[i] != 0) {
      writeSignedVB(out, values[i]);
    }
  }
}

// ---------------------------------------------------------------------------

constexpr const char *PRODUCT = "Blackbox flight data recorder by Nicholas Sherlock";

constexpr uint8_t ROW_ITERATION = 0;
constexpr uint8_t ROW_TIME = 1;

bool predictorUsesHistory(Predictor p)
{
  return p == Predictor::Previous || p == Predictor::StraightLine || p == Predictor::Average2;
}

uint8_t groupSize(Encoding e)
{
  switch (e) {
  case Encoding::Tag8_4S16:
    return 4;
  case Encoding::Tag2_3S32:
    return 3;
  default:
    return 1;
  }
}

size_t encodingMaxBytes(Encoding e)
{
  switch (e) {
  case Encoding::SignedVB:
  case Encoding::UnsignedVB:
    return 5;
  case Encoding::Neg14Bit:
    return 2;
  case Encoding::Null:
    return 0;
  default:
    return 5;  // grouped encodings are sized per group by the caller
  }
}

}  // namespace

void Buffer::putStr(const char *s)
{
  while (*s) {
    put((uint8_t)*s++);
  }
}

void Buffer::putInt(int32_t v)
{
  char tmp[12];
  int n = 0;
  uint32_t u = v < 0 ? (uint32_t)(-(int64_t)v) : (uint32_t)v;
  do {
    tmp[n++] = (char)('0' + u % 10);
    u /= 10;
  } while (u);
  if (v < 0) {
    put('-');
  }
  while (n) {
    put((uint8_t)tmp[--n]);
  }
}

void headerLine(Buffer &out, const char *name, const char *value)
{
  out.putStr("H ");
  out.putStr(name);
  out.put(':');
  out.putStr(value);
  out.put('\n');
}

void headerLineInt(Buffer &out, const char *name, int32_t value)
{
  out.putStr("H ");
  out.putStr(name);
  out.put(':');
  out.putInt(value);
  out.put('\n');
}

// ---------------------------------------------------------------------------

Encoder::Encoder(const MainField *fields, uint8_t n, int32_t *history, const SlowField *slow,
                 uint8_t ns)
  : _fields(fields), _n(n), _width((uint8_t)(n + 2)), _hist(history), _slow(slow), _ns(ns)
{
  for (uint8_t i = 0; i < _n && _fields; i++) {
    if (_fields[i].index == 0 && _fields[i].name && strcmp(_fields[i].name, "motor") == 0) {
      _motor0 = (int16_t)(i + 2);
      break;
    }
  }
  _valid = checkTable();
}

bool Encoder::checkTable() const
{
  if (_fields == nullptr || _hist == nullptr || _n == 0 || _n > MAX_FIELDS) {
    return false;
  }
  if (_ns > 0 && _slow == nullptr) {
    return false;
  }
  for (int pass = 0; pass < 2; pass++) {
    const bool intra = pass == 0;
    uint8_t i = 0;
    while (i < _n) {
      const MainField &f = _fields[i];
      const Predictor p = intra ? f.i_pred : f.p_pred;
      const Encoding e = intra ? f.i_enc : f.p_enc;
      if (f.name == nullptr || p == Predictor::Inc) {
        return false;
      }
      if (intra && predictorUsesHistory(p)) {
        return false;
      }
      if (p == Predictor::Motor0 && (_motor0 < 0 || _motor0 >= i + 2)) {
        return false;
      }
      const uint8_t g = groupSize(e);
      if (i + g > _n) {
        return false;
      }
      for (uint8_t j = 1; j < g; j++) {
        const MainField &m = _fields[i + j];
        if ((intra ? m.i_enc : m.p_enc) != e) {
          return false;
        }
      }
      i += g;
    }
  }
  for (uint8_t i = 0; i < _ns; i++) {
    const SlowField &s = _slow[i];
    if (s.name == nullptr || predictorUsesHistory(s.pred) || s.pred == Predictor::Inc ||
        s.pred == Predictor::Motor0 || groupSize(s.enc) != 1) {
      return false;
    }
  }
  return true;
}

uint16_t Encoder::headerLineCount() const
{
  // product, data version, I interval, P interval, 6 main field lines,
  // 4 slow field lines if any, minthrottle, vbatref, motorOutput
  return (uint16_t)(4 + 6 + (_ns ? 4 : 0) + 3);
}

bool Encoder::headerLine(uint16_t k, Buffer &out) const
{
  out.clear();
  if (!_valid) {
    return false;
  }
  switch (k) {
  case 0:
    bbl::headerLine(out, "Product", PRODUCT);
    return !out.overflow();
  case 1:
    bbl::headerLineInt(out, "Data version", 2);
    return !out.overflow();
  case 2:
    bbl::headerLineInt(out, "I interval", _i_interval);
    return !out.overflow();
  case 3:
    // Every call to mainFrame() is a logged iteration.
    bbl::headerLineInt(out, "P interval", 1);
    return !out.overflow();
  default:
    break;
  }

  static const char *const MAIN_NAMES[] = {"Field I name",      "Field I signed",
                                           "Field I predictor", "Field I encoding",
                                           "Field P predictor", "Field P encoding"};
  static const char *const SLOW_NAMES[] = {"Field S name", "Field S signed",
                                           "Field S predictor", "Field S encoding"};
  uint16_t j = (uint16_t)(k - 4);
  if (j < 6) {
    out.putStr("H ");
    out.putStr(MAIN_NAMES[j]);
    out.put(':');
    for (uint8_t i = 0; i < _width; i++) {
      if (i) {
        out.put(',');
      }
      if (i < 2) {
        // loopIteration and time
        static const int8_t AUTO[6][2] = {{0, 0}, {0, 0}, {0, 0}, {1, 1}, {6, 2}, {9, 0}};
        if (j == 0) {
          out.putStr(i == ROW_ITERATION ? "loopIteration" : "time");
        } else {
          out.putInt(AUTO[j][i]);
        }
        continue;
      }
      const MainField &f = _fields[i - 2];
      switch (j) {
      case 0:
        out.putStr(f.name);
        if (f.index >= 0) {
          out.put('[');
          out.putInt(f.index);
          out.put(']');
        }
        break;
      case 1:
        out.putInt(f.is_signed ? 1 : 0);
        break;
      case 2:
        out.putInt((int32_t)f.i_pred);
        break;
      case 3:
        out.putInt((int32_t)f.i_enc);
        break;
      case 4:
        out.putInt((int32_t)f.p_pred);
        break;
      default:
        out.putInt((int32_t)f.p_enc);
        break;
      }
    }
    out.put('\n');
    return !out.overflow();
  }
  j = (uint16_t)(j - 6);
  if (_ns) {
    if (j < 4) {
      out.putStr("H ");
      out.putStr(SLOW_NAMES[j]);
      out.put(':');
      for (uint8_t i = 0; i < _ns; i++) {
        if (i) {
          out.put(',');
        }
        const SlowField &s = _slow[i];
        switch (j) {
        case 0:
          out.putStr(s.name);
          if (s.index >= 0) {
            out.put('[');
            out.putInt(s.index);
            out.put(']');
          }
          break;
        case 1:
          out.putInt(s.is_signed ? 1 : 0);
          break;
        case 2:
          out.putInt((int32_t)s.pred);
          break;
        default:
          out.putInt((int32_t)s.enc);
          break;
        }
      }
      out.put('\n');
      return !out.overflow();
    }
    j = (uint16_t)(j - 4);
  }
  switch (j) {
  case 0:
    bbl::headerLineInt(out, "minthrottle", _k.minthrottle);
    break;
  case 1:
    bbl::headerLineInt(out, "vbatref", _k.vbatref);
    break;
  case 2:
    out.putStr("H motorOutput:");
    out.putInt(_k.motor_output_low);
    out.put(',');
    out.putInt(_k.motor_output_high);
    out.put('\n');
    break;
  default:
    return false;
  }
  return !out.overflow();
}

void Encoder::begin()
{
  memset(_hist, 0, sizeof(int32_t) * 3u * _width);
  _cur = 0;
  _prev1 = 1;
  _prev2 = 2;
  _iteration = 0;
  _force_intra = true;
  _pending = false;
}

int32_t Encoder::predict(uint8_t f, Predictor p, const int32_t *cur, bool intra) const
{
  switch (p) {
  case Predictor::Zero:
    return 0;
  case Predictor::MinThrottle:
    return _k.minthrottle;
  case Predictor::P1500:
    return 1500;
  case Predictor::VbatRef:
    return _k.vbatref;
  case Predictor::MinMotor:
    return _k.motor_output_low;
  case Predictor::Motor0:
    return cur[_motor0];
  case Predictor::Previous:
    return intra ? 0 : row(_prev1)[f];
  case Predictor::StraightLine:
    return intra ? 0 : (int32_t)(2 * (int64_t)row(_prev1)[f] - row(_prev2)[f]);
  case Predictor::Average2:
    return intra ? 0 : (int32_t)(((int64_t)row(_prev1)[f] + row(_prev2)[f]) / 2);
  default:
    return 0;
  }
}

void Encoder::encodeRow(const int32_t *cur, bool intra, Buffer &out) const
{
  out.put(intra ? 'I' : 'P');
  uint8_t i = 0;
  while (i < _width) {
    Predictor p;
    Encoding e;
    if (i == ROW_ITERATION) {
      p = intra ? Predictor::Zero : Predictor::Inc;
      e = intra ? Encoding::UnsignedVB : Encoding::Null;
    } else if (i == ROW_TIME) {
      p = intra ? Predictor::Zero : Predictor::StraightLine;
      e = intra ? Encoding::UnsignedVB : Encoding::SignedVB;
    } else {
      const MainField &f = _fields[i - 2];
      p = intra ? f.i_pred : f.p_pred;
      e = intra ? f.i_enc : f.p_enc;
    }
    if (p == Predictor::Inc) {  // loopIteration in P frames: nothing written
      i++;
      continue;
    }

    int32_t r[8];
    uint8_t g = groupSize(e);
    if (e == Encoding::Tag8_8SVB) {
      // The decoder groups up to 8 consecutive TAG8_8SVB fields.
      g = 1;
      while (g < 8 && i + g < _width &&
             (intra ? _fields[i + g - 2].i_enc : _fields[i + g - 2].p_enc) == e) {
        g++;
      }
    }
    for (uint8_t j = 0; j < g; j++) {
      const uint8_t fi = (uint8_t)(i + j);
      const Predictor pj = (fi < 2) ? p : (intra ? _fields[fi - 2].i_pred : _fields[fi - 2].p_pred);
      r[j] = (int32_t)((int64_t)cur[fi] - predict(fi, pj, cur, intra));
    }

    switch (e) {
    case Encoding::SignedVB:
      writeSignedVB(out, r[0]);
      break;
    case Encoding::UnsignedVB:
      writeUnsignedVB(out, (uint32_t)r[0]);
      break;
    case Encoding::Neg14Bit:
      writeUnsignedVB(out, (uint32_t)(-r[0]) & 0x3FFF);
      break;
    case Encoding::Tag8_4S16:
      writeTag8_4S16(out, r);
      break;
    case Encoding::Tag2_3S32:
      writeTag2_3S32(out, r);
      break;
    case Encoding::Tag8_8SVB:
      writeTag8_8SVB(out, r, g);
      break;
    case Encoding::Null:
      break;
    }
    i += g;
  }
}

bool Encoder::mainFrame(uint32_t time_us, const int32_t *values, Buffer &out)
{
  out.clear();
  if (!_valid) {
    return false;
  }
  if (_pending) {
    drop();  // the caller neither kept nor dropped the last frame
  }
  int32_t *cur = row(_cur);
  cur[ROW_ITERATION] = (int32_t)_iteration;
  cur[ROW_TIME] = (int32_t)time_us;
  memcpy(cur + 2, values, sizeof(int32_t) * _n);
  const bool intra = _force_intra || (_iteration % _i_interval) == 0;
  encodeRow(cur, intra, out);
  _pending = true;
  _pending_intra = intra;
  return !out.overflow();
}

void Encoder::commit()
{
  if (!_pending) {
    return;
  }
  _pending = false;
  const uint8_t kept = _cur;
  if (_pending_intra) {
    _prev1 = kept;
    _prev2 = kept;
  } else {
    _prev2 = _prev1;
    _prev1 = kept;
  }
  // The next row goes in a slot that is neither history row.
  for (uint8_t s = 0; s < 3; s++) {
    if (s != _prev1 && s != _prev2) {
      _cur = s;
      break;
    }
  }
  _iteration++;
  _force_intra = false;
}

void Encoder::drop()
{
  if (!_pending) {
    return;
  }
  _pending = false;
  _iteration++;
  _force_intra = true;
}

size_t Encoder::maxMainFrameBytes() const
{
  size_t worst = 0;
  for (int pass = 0; pass < 2; pass++) {
    const bool intra = pass == 0;
    size_t bytes = 1 + 5 + 5;  // frame type, loopIteration, time
    uint8_t i = 0;
    while (i < _n) {
      const Encoding e = intra ? _fields[i].i_enc : _fields[i].p_enc;
      if (e == Encoding::Tag8_4S16) {
        bytes += 1 + 8;
        i += 4;
      } else if (e == Encoding::Tag2_3S32) {
        bytes += 1 + 12;
        i += 3;
      } else if (e == Encoding::Tag8_8SVB) {
        uint8_t g = 1;
        while (g < 8 && i + g < _n &&
               (intra ? _fields[i + g].i_enc : _fields[i + g].p_enc) == e) {
          g++;
        }
        bytes += (g > 1 ? 1 : 0) + 5u * g;
        i += g;
      } else {
        bytes += encodingMaxBytes(e);
        i++;
      }
    }
    worst = bytes > worst ? bytes : worst;
  }
  return worst;
}

bool Encoder::slowFrame(const int32_t *values, Buffer &out)
{
  out.clear();
  if (!_valid || _ns == 0) {
    return false;
  }
  out.put('S');
  uint8_t i = 0;
  while (i < _ns) {
    const SlowField &s = _slow[i];
    int32_t r[8];
    uint8_t g = 1;
    if (s.enc == Encoding::Tag8_8SVB) {
      while (g < 8 && i + g < _ns && _slow[i + g].enc == s.enc) {
        g++;
      }
    }
    for (uint8_t j = 0; j < g; j++) {
      r[j] = (int32_t)((int64_t)values[i + j] - predict(0, _slow[i + j].pred, nullptr, true));
    }
    switch (s.enc) {
    case Encoding::SignedVB:
      writeSignedVB(out, r[0]);
      break;
    case Encoding::UnsignedVB:
      writeUnsignedVB(out, (uint32_t)r[0]);
      break;
    case Encoding::Neg14Bit:
      writeUnsignedVB(out, (uint32_t)(-r[0]) & 0x3FFF);
      break;
    case Encoding::Tag8_8SVB:
      writeTag8_8SVB(out, r, g);
      break;
    default:
      break;
    }
    i += g;
  }
  return !out.overflow();
}

// Events (Betaflight blackboxLogEvent): 'E', the event code, its payload.

void Encoder::syncBeep(uint32_t time_us, Buffer &out)
{
  out.clear();
  out.put('E');
  out.put(0);
  writeUnsignedVB(out, time_us);
}

void Encoder::disarm(uint32_t reason, Buffer &out)
{
  out.clear();
  out.put('E');
  out.put(15);
  writeUnsignedVB(out, reason);
}

void Encoder::flightMode(uint32_t flags, uint32_t last_flags, Buffer &out)
{
  out.clear();
  out.put('E');
  out.put(30);
  writeUnsignedVB(out, flags);
  writeUnsignedVB(out, last_flags);
}

void Encoder::loggingResume(uint32_t iteration, uint32_t time_us, Buffer &out)
{
  out.clear();
  out.put('E');
  out.put(14);
  writeUnsignedVB(out, iteration);
  writeUnsignedVB(out, time_us);
  _force_intra = true;  // the decoder expects an I frame next
}

void Encoder::logEnd(Buffer &out)
{
  out.clear();
  out.put('E');
  out.put(255);
  out.putStr("End of log");
  out.put(0);
}

}  // namespace bbl
