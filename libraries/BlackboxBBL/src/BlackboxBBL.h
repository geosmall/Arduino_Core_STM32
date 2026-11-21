#pragma once

/*
 * BlackboxBBL.h — encoder for Betaflight's blackbox log format, data version 2
 * (PROVISIONAL INTERFACE).
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Derived from Betaflight's blackbox encoder (src/main/blackbox/
 * blackbox_encoding.c and blackbox.c, Betaflight commit f97bdbcf5, which
 * derive from Cleanflight); the frame layout follows the reference decoder,
 * blackbox-tools src/parser.c (commit 57c7695). Cleanflight and Betaflight
 * are free software under the GNU General Public License, version 3 or (at
 * your option) any later version; so is this library. See LICENSE.
 *
 * What it does. The caller describes its main-frame fields in a table (name,
 * sign, and a predictor and encoding for I frames and for P frames); the
 * encoder prepends loopIteration and time, writes the header lines that
 * describe the table, and encodes each frame into a caller-supplied Buffer.
 * It knows no storage: the caller pushes the bytes into its log (FlashLog, an
 * SD writer, a UART).
 *
 * Frames.
 *  - mainFrame(): an I frame every `I interval` frames and after any frame
 *    the caller did not keep, a P frame otherwise. Every call is one logged
 *    iteration ("P interval" 1/1); log at a lower rate by calling less often.
 *  - The caller keeps or drops each encoded main frame: commit() after the
 *    bytes were stored, drop() if they were not (the next frame is then an I
 *    frame, since the decoder's history no longer matches the encoder's).
 *  - slowFrame(): an S frame from a second table (no history).
 *  - Events: sync beep, disarm, flight mode, logging resume, log end, laid
 *    out as Betaflight writes them. blackbox_decode (57c7695) parses only
 *    sync beep, logging resume and log end of these; after a disarm or
 *    flight-mode event it loses main frames up to the next I frame, as it
 *    does on Betaflight's own logs.
 *
 * Table rules, checked by valid():
 *  - Grouped encodings take whole groups, exactly as the decoder reads them:
 *    TAG8_4S16 four consecutive fields, TAG2_3S32 three; TAG8_8SVB groups up
 *    to eight consecutive TAG8_8SVB fields.
 *  - I-frame predictors use no history (Zero, MinThrottle, Motor0, 1500,
 *    VbatRef, MinMotor). P-frame predictors may also use Previous,
 *    StraightLine and Average2.
 *  - Motor0 needs a field named "motor" with index 0 earlier in the table.
 *  - Inc is reserved for loopIteration, which the encoder writes itself.
 *  - TAG8_4S16 carries 16-bit residuals and NEG_14BIT 14-bit values; larger
 *    values are truncated, as in Betaflight.
 *
 * Static allocation only: history is three rows of int32 per field, owned by
 * BlackboxBBLStatic<N> or passed in.
 */

#include <stddef.h>
#include <stdint.h>

namespace bbl {

enum class Predictor : uint8_t {
  Zero = 0,
  Previous = 1,
  StraightLine = 2,
  Average2 = 3,
  MinThrottle = 4,
  Motor0 = 5,
  Inc = 6,
  P1500 = 8,
  VbatRef = 9,
  MinMotor = 11,
};

enum class Encoding : uint8_t {
  SignedVB = 0,
  UnsignedVB = 1,
  Neg14Bit = 3,
  Tag8_8SVB = 6,
  Tag2_3S32 = 7,
  Tag8_4S16 = 8,
  Null = 9,
};

struct MainField {
  const char *name;
  int8_t index;        // printed as name[index]; -1 for none
  bool is_signed;
  Predictor i_pred;
  Encoding i_enc;
  Predictor p_pred;
  Encoding p_enc;
};

struct SlowField {
  const char *name;
  int8_t index;
  bool is_signed;
  Predictor pred;      // Zero, P1500, VbatRef, MinThrottle or MinMotor
  Encoding enc;
};

// Values of the header constants some predictors refer to.
struct Constants {
  int32_t minthrottle = 0;        // "minthrottle", for MinThrottle
  int32_t vbatref = 0;            // "vbatref", for VbatRef
  int32_t motor_output_low = 0;   // "motorOutput" low, for MinMotor
  int32_t motor_output_high = 0;  // "motorOutput" high
};

// Byte sink over a caller array. Writing past the end sets overflow and
// drops the byte; the frame is then unusable.
class Buffer {
public:
  Buffer(uint8_t *data, size_t capacity) : _data(data), _cap(capacity) {}
  void clear() { _len = 0; _overflow = false; }
  void put(uint8_t b)
  {
    if (_len < _cap) {
      _data[_len++] = b;
    } else {
      _overflow = true;
    }
  }
  void putStr(const char *s);
  void putInt(int32_t v);
  const uint8_t *data() const { return _data; }
  size_t size() const { return _len; }
  bool overflow() const { return _overflow; }

private:
  uint8_t *_data;
  size_t _cap;
  size_t _len = 0;
  bool _overflow = false;
};

// Writes "H <name>:<value>\n".
void headerLine(Buffer &out, const char *name, const char *value);
void headerLineInt(Buffer &out, const char *name, int32_t value);

class Encoder {
public:
  static constexpr uint8_t MAX_FIELDS = 126;  // decoder limit 128, minus loopIteration and time

  // history: 3 * (n + 2) int32 values. slow may be null with ns == 0.
  Encoder(const MainField *fields, uint8_t n, int32_t *history,
          const SlowField *slow = nullptr, uint8_t ns = 0);

  // Table check against the rules in the file comment. Encoding with an
  // invalid table writes nothing.
  bool valid() const { return _valid; }

  void setIInterval(uint16_t frames) { _i_interval = frames ? frames : 1; }
  void setConstants(const Constants &c) { _k = c; }

  // Header: headerLineCount() lines, each written by headerLine(k, out);
  // write them in order, then any caller lines (firmware type, looptime ...)
  // with bbl::headerLine(), before the first frame.
  uint16_t headerLineCount() const;
  bool headerLine(uint16_t k, Buffer &out) const;

  // Starts a log: iteration 0, history cleared, next frame an I frame.
  void begin();

  // Encodes one main frame of n values (in table order) into out, which is
  // cleared first. Returns false if the table is invalid or out overflowed.
  // Follow with commit() or drop().
  bool mainFrame(uint32_t time_us, const int32_t *values, Buffer &out);
  void commit();
  void drop();

  // Largest main frame this table can produce, for sizing the buffer.
  size_t maxMainFrameBytes() const;

  bool slowFrame(const int32_t *values, Buffer &out);

  void syncBeep(uint32_t time_us, Buffer &out);
  void disarm(uint32_t reason, Buffer &out);
  void flightMode(uint32_t flags, uint32_t last_flags, Buffer &out);
  void loggingResume(uint32_t iteration, uint32_t time_us, Buffer &out);
  void logEnd(Buffer &out);

  uint32_t iteration() const { return _iteration; }

private:
  bool checkTable() const;
  int32_t predict(uint8_t f, Predictor p, const int32_t *cur, bool intra) const;
  void encodeRow(const int32_t *cur, bool intra, Buffer &out) const;
  int32_t *row(uint8_t slot) const { return _hist + uint32_t(slot) * _width; }

  const MainField *_fields;
  uint8_t _n;
  uint8_t _width;          // n + 2
  int32_t *_hist;
  const SlowField *_slow;
  uint8_t _ns;
  Constants _k;
  uint16_t _i_interval = 32;
  int16_t _motor0 = -1;    // row index of motor[0], or -1
  bool _valid = false;

  // History slots: _cur is being encoded, _prev1 the last kept frame,
  // _prev2 the one before.
  uint8_t _cur = 0, _prev1 = 1, _prev2 = 2;
  uint32_t _iteration = 0;
  bool _force_intra = true;
  bool _pending = false;
  bool _pending_intra = false;
};

template <uint8_t N>
class BlackboxBBLStatic : public Encoder {
public:
  explicit BlackboxBBLStatic(const MainField *fields, const SlowField *slow = nullptr,
                             uint8_t ns = 0)
    : Encoder(fields, N, &_h[0][0], slow, ns) {}

private:
  int32_t _h[3][N + 2];
};

}  // namespace bbl
