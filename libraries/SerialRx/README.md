# SerialRx - RC Receiver Serial Protocol Parser

Arduino library for parsing serial RC receiver protocols commonly used in UAV flight controllers. Interrupt-free, non-blocking design compatible with time-critical control loops.

## Supported Protocols

| Protocol | Baudrate | Frame Size | Channels | Checksum | Validated |
|----------|----------|------------|----------|----------|-----------|
| **IBus** (FlySky) | 115200 | 32 bytes | 14 (of 14) | 16-bit sum | Yes |
| **SBUS** (FrSky/Futaba) | 100000 | 25 bytes | 14 (of 16) | Header/footer | Yes |
| **CRSF** (Crossfire / ELRS) | 420000 | Variable | 14 (of 16) | CRC8-DVB-S2 | Yes |

All protocols deliver 14 channels via `RCMessage::channels[]`. SBUS and CRSF transmit 16 channels in 22 bytes of 11-bit packed data; only the first 14 are unpacked.

## Quick Start

```cpp
#include <SerialRx.h>

HardwareSerial SerialRC(PA10, PA9);  // USART1
SerialRx rc;

void setup() {
  SerialRx::Config config;
  config.serial = &SerialRC;
  config.rx_protocol = SerialRx::IBUS;  // or SBUS, CRSF
  config.baudrate = 115200;             // IBus: 115200, SBUS: 100000, CRSF: 420000
  config.timeout_ms = 1000;
  rc.begin(config);
}

void loop() {
  rc.update();  // Must call frequently — polls serial buffer

  if (rc.isSignalLost()) {
    // Failsafe — set safe outputs
    return;
  }

  if (rc.available()) {
    RCMessage msg;
    if (rc.getMessage(&msg)) {
      uint16_t throttle = rc.channelToPWM(msg.channels[2]);  // 1000-2000 µs
      uint16_t aileron  = rc.channelToPWM(msg.channels[0]);
    }
  }
}
```

## Protocol Configuration

### IBus

```cpp
config.rx_protocol = SerialRx::IBUS;
config.baudrate = 115200;
config.idle_threshold_us = 300;  // Optional: software idle detection
```

- Standard 8N1 UART, not inverted
- No protocol-level failsafe flag — see [IBus Endpoint Trick](#ibus-endpoint-trick) for workaround
- `channelToPWM()` is a passthrough (IBus values are already in PWM µs)

### SBUS

```cpp
config.rx_protocol = SerialRx::SBUS;
config.baudrate = 100000;
config.invert_rx = true;  // Required — SBUS uses inverted signal
```

- 8E2 framing (even parity, 2 stop bits), inverted signal
- Hardware RXINV on F7/H7/G4 (automatic via `invert_rx`). F4 requires an external inverter — see [SBUS Signal Inversion](#sbus-signal-inversion)
- Has protocol failsafe flag (Layer 1 — immediate detection)
- `channelToPWM()` converts 11-bit raw (172-1811) to PWM µs using INAV formula

### CRSF

```cpp
config.rx_protocol = SerialRx::CRSF;
config.baudrate = 420000;
config.invert_rx = false;
```

- Standard 8N1 UART, not inverted
- Variable-length frames with CRC8-DVB-S2 validation
- Parses Link Statistics frames (type 0x14) for uplink LQ — LQ=0 triggers Layer 1 failsafe
- Same 11-bit channel packing and PWM conversion as SBUS

## Failsafe

SerialRx implements INAV-style 4-layer failsafe detection:

| Layer | Method | Latency | Protocol |
|-------|--------|---------|----------|
| 1 | Protocol failsafe flag | Immediate | SBUS, CRSF |
| 2 | Frame timeout (no frames received) | 100ms | All |
| 3 | Range checking (pulse < 885 or > 2115 µs) | Immediate | All |
| 4 | Per-channel expiry (stale AETR values) | 300ms | All |

Any single layer triggers `isSignalLost()`.

**SBUS** detects immediately via the protocol failsafe flag in every frame.

**CRSF** detects via Link Statistics uplink LQ. When LQ=0 (no RF packets), the next RC channels frame is flagged. This catches degraded links where the receiver still forwards stale channel data — a scenario Layers 2-4 alone miss.

**IBus** has no protocol flag. Detection relies on Layers 2-4. For reliable detection, the FlySky endpoint trick is required (see [reference](#ibus-endpoint-trick)).

### API

```cpp
bool isSignalLost();                    // Primary — true if any layer triggered
SignalStatus getSignalStatus();         // OK, TIMEOUT, FAILSAFE_FLAG, OUT_OF_RANGE, EXPIRED
const char* getSignalStatusString();    // Human-readable status
uint16_t getLastValidPWM(uint8_t ch);   // Held value during failsafe (AETR, ch 0-3)
uint32_t getFramesReceived();
uint32_t getFramesFailed();
float getFrameLossPercent();
```

## Architecture & Performance

```
SerialRx (Transport Layer + Failsafe Detection)
    ├── RingBuffer<RCMessage, 16> (message queue)
    └── ProtocolParser (abstract interface)
            ├── IBusParser (5-state machine, 16-bit checksum)
            ├── SBusParser (4-state machine, header/footer validation)
            └── CRSFParser (3-state machine, CRC8-DVB-S2, Link Stats LQ tracking)
```

- **Flash**: ~2 KB (parser + transport)
- **RAM**: ~100 bytes (message buffer + state)
- **CPU**: ~10 µs per byte @ 100 MHz (STM32F411)
- **Non-blocking**: No interrupts, no DMA required — polls `Serial.available()`

---

## Reference

### Failsafe Tuning

Defaults match INAV. Override via `Config` before `begin()`:

| Parameter | Default | INAV Equivalent | Notes |
|-----------|---------|-----------------|-------|
| `frame_timeout_ms` | 100 | frame timeout | Increase for long-range links |
| `channel_expiry_ms` | 300 | `MAX_INVALID_RX_PULSE_TIME` | Increase for noisy environments |
| `valid_pulse_min` | 885 | `rx_min_usec` | Raise to 925 for FlySky endpoint trick |
| `valid_pulse_max` | 2115 | `rx_max_usec` | |

### SBUS Signal Inversion

SBUS uses an inverted UART signal. The library handles this via `config.invert_rx = true`:

| MCU Family | Hardware RXINV | Notes |
|------------|----------------|-------|
| STM32F7/H7/G4 | Yes | USART_CR2_RXINV bit — no external hardware |
| STM32F4 | No | Requires external inverter (NPN transistor, 74HC04, or dedicated module) |

### IBus Endpoint Trick

IBus has no protocol failsafe flag. The FS-iA6B continues sending valid-looking held values on signal loss. The workaround is to configure a failsafe value outside the normal pulse range:

```
  ~900 µs  ← FS-i6X with endpoint trick (DETECTED — below 925 threshold)
   925 µs  ← Recommended valid_pulse_min
   988 µs  ← FS-i6X normal endpoints (NOT detected — above threshold)
```

**Procedure (FS-i6X):**
1. Set throttle low endpoint to **120%** (Endpoints menu)
2. **System Setup > RX Setup > Failsafe** — set CH3 to ON, stick fully down (~900 µs at 120%)
3. **Restore endpoint to 100%** — failsafe value stays at ~900 µs
4. Set `config.valid_pulse_min = 925`

### IBus Frame Format

```
Byte 0-1:   Header (0x20 0x40)
Byte 2-29:  14 channels × 2 bytes (little-endian uint16_t)
Byte 30-31: Checksum (little-endian uint16_t, 0xFFFF - sum of bytes 0-29)
Total: 32 bytes, ~100 Hz
```

Software idle detection (`config.idle_threshold_us = 300`) prevents false header matches from channel data. After a 300 µs idle gap, the parser requires the next byte to be a valid header.

### CRSF Frame Format

```
Byte 0:       Sync (0xC8)
Byte 1:       Length (type + payload + CRC byte count)
Byte 2:       Type
Byte 3-N:     Payload (variable)
Byte N+1:     CRC8-DVB-S2 (over type + payload)
Max: 64 bytes
```

**Handled frame types:**
- **0x16** RC Channels Packed (22-byte payload) — pushed to message queue
- **0x14** Link Statistics (10-byte payload) — uplink LQ extracted for failsafe
- All others — CRC-validated and discarded

### Hardware Validation

| Board | Protocol | Detection | Result |
|-------|----------|-----------|--------|
| Nucleo F411RE (HIL-005) | IBus | Layer 3 + Layer 4 | 3895 frames, 0 failed |
| DevEBox H743 (HIL-006) | SBUS | Layer 1 (FAILSAFE_FLAG) | 3896 frames, 0 failed |
| DevEBox H743 (HIL-006) | IBus | Layer 4 (EXPIRED) | 3896 frames, 0 failed |
| Open Revo (F405) | SBUS | Layer 2 (TIMEOUT) | 2351 frames, 0 failed |
| Nucleo F411RE (loopback) | CRSF | Layer 1 (LQ=0), CRC8 rejection | 436 frames, 1 failed (intentional) |

### Testing

Loopback tests validate protocol parsing without a receiver. Jumper PA11 (USART6 TX) to PA10 (USART1 RX):

```bash
# IBus loopback
arduino-cli compile --fqbn STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE libraries/SerialRx/examples/IBus_Loopback_Test

# CRSF loopback
arduino-cli compile --fqbn STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE libraries/SerialRx/examples/CRSF_Loopback_Test
```
