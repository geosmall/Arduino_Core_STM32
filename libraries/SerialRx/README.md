# SerialRx - RC Receiver Serial Protocol Parser

Arduino library for parsing serial RC receiver protocols (IBus, SBUS, CRSF) commonly used in UAV flight controllers.

**Hardware Validated**: Tested with FlySky FS-iA6B receiver and dual-USART loopback testing.

## Supported Protocols

| Protocol | Status | Baudrate | Frame Size | Channels | Validated |
|----------|--------|----------|------------|----------|-----------|
| **IBus** (FlySky) | ✅ Implemented | 115200 | 32 bytes | 14 (of 14) | ✅ Yes |
| **SBUS** (FrSky/Futaba) | ✅ Implemented | 100000 | 25 bytes | 14 (of 16) | ✅ Yes |
| **CRSF** (TBS Crossfire) | 📋 Planned | 420000 | Variable | 16 | ⚠️ No |

## Quick Start

```cpp
#include <SerialRx.h>

HardwareSerial SerialRC(PA10, PA9);  // USART1
SerialRx rc;

void setup() {
  SerialRx::Config config;
  config.serial = &SerialRC;
  config.rx_protocol = SerialRx::IBUS;
  config.baudrate = 115200;
  config.timeout_ms = 1000;
  config.idle_threshold_us = 300;  // Optional: software idle detection

  rc.begin(config);
}

void loop() {
  rc.update();

  if (rc.isSignalLost()) {
    // Signal lost — set safe outputs
    return;
  }

  if (rc.available()) {
    RCMessage msg;
    if (rc.getMessage(&msg)) {
      uint16_t throttle = rc.channelToPWM(msg.channels[2]);  // Ch3 (0-indexed)
      uint16_t aileron = rc.channelToPWM(msg.channels[0]);    // Ch1
      // Process RC commands (1000-2000 µs range)
    }
  }
}
```

## Failsafe Detection

SerialRx implements INAV-style 4-layer failsafe detection. The primary API is a single call:

```cpp
if (rc.isSignalLost()) {
    // Signal lost — set safe outputs
    throttle = 1000;
    aileron = elevator = rudder = 1500;
}
```

### Detection Layers

| Layer | Method | Latency | Protocol |
|-------|--------|---------|----------|
| 1 | Protocol failsafe flag | Immediate | SBUS only |
| 2 | Frame timeout (no frames received) | 100ms | All |
| 3 | Range checking (pulse < 885 or > 2115 µs) | Immediate | All |
| 4 | Per-channel expiry (stale AETR values) | 300ms | All |

Any single layer triggering causes `isSignalLost()` to return `true`.

### Failsafe API

```cpp
// Primary — one-liner for most users
bool isSignalLost();

// Diagnostics — which layer triggered
SignalStatus getSignalStatus();          // Returns enum: OK, TIMEOUT, FAILSAFE_FLAG, OUT_OF_RANGE, EXPIRED
const char* getSignalStatusString();     // Returns "OK", "TIMEOUT", etc.

// Per-channel held values during failsafe
uint16_t getLastValidPWM(uint8_t ch);    // Last known good value (PWM µs)

// Link quality
uint32_t getFramesReceived();
uint32_t getFramesFailed();
float getFrameLossPercent();
```

### Configuration

Defaults match INAV settings. Override via the Config struct before `begin()`:

```cpp
SerialRx::Config config;
config.frame_timeout_ms = 100;     // Layer 2: no frames for this long = timeout
config.channel_expiry_ms = 300;    // Layer 4: stale values for this long = expired
config.valid_pulse_min = 885;      // Layer 3: below this = out of range
config.valid_pulse_max = 2115;     // Layer 3: above this = out of range
rc.begin(config);
```

| Parameter | Default | INAV Equivalent | Notes |
|-----------|---------|-----------------|-------|
| `frame_timeout_ms` | 100 | frame timeout | Increase for long-range links |
| `channel_expiry_ms` | 300 | `MAX_INVALID_RX_PULSE_TIME` | Increase for noisy environments |
| `valid_pulse_min` | 885 | `rx_min_usec` | Raise to 925 for FlySky endpoint trick |
| `valid_pulse_max` | 2115 | `rx_max_usec` | |

### Protocol-Specific Behavior

**SBUS**: Layer 1 provides immediate detection via the protocol failsafe flag. The receiver continues sending frames with the flag set — detection is instantaneous regardless of channel values.

**IBus**: No protocol-level failsafe flag exists. Detection relies on Layers 2-4. The FS-iA6B receiver continues sending valid-looking frames on signal loss, so **Tx endpoint config** is required for reliable Layer 3 detection (see TX/RX Configuration below).

### Hardware Validation

| Board | Protocol | Detection | Result |
|-------|----------|-----------|--------|
| Nucleo F411RE (HIL-005) | IBus | Layer 3 (OUT_OF_RANGE) + Layer 4 (EXPIRED) | 3895 frames, 0 failed |
| DevEBox H743 (HIL-006) | SBUS | Layer 1 (FAILSAFE_FLAG) | 3896 frames, 0 failed |
| DevEBox H743 (HIL-006) | IBus | Layer 4 (EXPIRED) | 3896 frames, 0 failed |
| Open Revo (F405) | SBUS | Layer 2 (TIMEOUT) | 2351 frames, 0 failed |

---

## TX/RX Configuration

For failsafe detection to work reliably, the transmitter and receiver must be configured correctly.

### SBUS — Recommended

SBUS has native failsafe support via the protocol flag (Layer 1). Any receiver failsafe mode works — the flag is always set on signal loss.

```cpp
config.rx_protocol = SerialRx::SBUS;
config.baudrate = 100000;
config.invert_rx = true;   // Required: SBUS uses inverted signal
```

### IBus — Endpoint Trick Required

IBus has **no protocol-level failsafe flag**. Without TX configuration, signal loss is NOT detected — the FS-iA6B continues sending valid-looking held values.

**The problem:**
```
  ~900 µs  ← FS-i6X at -100% with endpoint trick (BELOW threshold → DETECTED)
   925 µs  ← Recommended valid_pulse_min for FlySky
   988 µs  ← FS-i6X at -100% normal endpoints (ABOVE threshold → NOT detected)
  1000 µs  ← Standard RC minimum
```

**Endpoint trick procedure (FS-i6X):**
1. Set throttle low endpoint to **120%** (Endpoints menu)
2. Navigate to **System Setup > RX Setup > Failsafe**
3. Set CH3 failsafe to **ON**, stick fully down (stores ~900 µs at 120% range)
4. **Restore throttle endpoint to 100%** — failsafe value remains at ~900 µs
5. Bench-verify: power off TX, confirm throttle reads ~900 µs

```cpp
config.rx_protocol = SerialRx::IBUS;
config.baudrate = 115200;
config.valid_pulse_min = 925;  // Raise from default 885 for FlySky endpoint trick
```

**Note:** Community documentation suggests ~880 µs from the endpoint trick. In practice, the FS-i6X produces ~900 µs. Set `valid_pulse_min` to 925 for reliable detection with margin.

### FlySky Receiver Differences

| Feature | FS-IA6B | FS-A8S |
|---------|---------|--------|
| IBus/SBUS switching | TX menu (automatic, ~5 sec) | Bind button only (hold 2-3 sec) |
| TX settings packet | Responds | Does NOT respond |
| Rebind needed | No | No (but bind button required) |
| Failsafe output (IBus) | Holds last values or TX-configured | Varies by firmware |

### Verification Procedure

1. Power on with TX on — observe `[OK]` status and channel values
2. Turn off TX — observe `isSignalLost()` returns `true`
   - **SBUS**: `FAILSAFE_FLAG` (immediate)
   - **IBus with endpoint trick**: `OUT_OF_RANGE` (immediate, ~900 µs < 925 threshold)
   - **IBus without endpoint trick**: `EXPIRED` after 300ms (or **NOT detected** if values stay in range)
3. Turn on TX — observe recovery to `[OK]`

See `examples/Failsafe_Detection/` for a ready-to-run test sketch.

---

## IBus Protocol Details

### Frame Structure

```
Byte 0-1:   Header (0x20 0x40)
Byte 2-29:  14 channels × 2 bytes (little-endian uint16_t)
Byte 30-31: Checksum (little-endian uint16_t)
Total: 32 bytes, ~100 Hz frame rate
```

**Example Frame**:
```
20 40  DB 05   DC 05   54 05   DC 05   E8 03   D0 07   D2 05   E8 03   DC 05   DC 05   DC 05   DC 05   DC 05   DC 05   DA F3
│  │  └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘
│  │     Ch1     Ch2     Ch3     Ch4     Ch5     Ch6     Ch7     Ch8     Ch9    Ch10    Ch11    Ch12    Ch13    Ch14   Chksum
│  └── Header byte 1 (0x40)
└───── Header byte 0 (0x20)
```

### Checksum Validation

```cpp
// 16-bit sum checksum (0xFFFF - sum of bytes 0-29)
uint16_t checksum = 0xFFFF;
for (int i = 0; i < 30; i++) {
    checksum -= frame[i];
}
// Verify against bytes 30-31 (little-endian)
```

### Frame Synchronization

**State Machine** (5 states):
```
┌─────────────────┐
│ Idle >300µs     │ (optional) Resets parser, expect 0x20 next
└────────┬────────┘
         ↓
┌─────────────────┐  0x20   ┌─────────────────┐  0x40   ┌─────────────────┐
│ WaitForHeader0  │────────→│ ParserHasHdr0   │────────→│ ParserHasHdr1   │
└─────────────────┘         └────────┬────────┘         └────────┬────────┘
         ↑                           │                           │ 28 bytes
         │                  not 0x40 │                           ↓
         ├───────────────────────────┘                  ┌─────────────────┐
         │                                       cksum  │ ParserHasFrame  │
         │                          ┌─────────────────┐ │  (accumulate)   │
         │           valid/invalid  │ ParserHasChkSum │ └────────┬────────┘
         └──────────────────────────┤      (verify)   │←─────────┘
                                    └─────────────────┘
```

- Scans byte stream for header pattern (0x20 0x40)
- Accumulates 28 channel bytes (14 channels × 2 bytes)
- Validates checksum before accepting frame
- Auto-recovers from partial/corrupted frames

**False Positive Protection**:

Header pattern can appear in channel data (e.g., Ch=1312µs → 0x0520 adjacent to Ch=0x??40). Protection mechanisms:

1. **Checksum validation** (always enabled): Rejects frames with invalid checksums
2. **Software idle detection** (optional): After 300µs idle, next byte MUST be 0x20
   - Inter-frame gap: ~7ms (frame transmission: 2.78ms)
   - Eliminates false starts without hardware dependencies
   - Arduino-compatible (uses `micros()`)

**Configuration**:
```cpp
config.idle_threshold_us = 300;  // Enable idle detection (0 = disabled)
```

**Validation Results**:
- ✅ Loopback test: 501/501 frames, 0% loss (idle detection enabled)
- ✅ Real receiver: 15-second test with FlySky FS-iA6B, all channels correct
- ✅ Defense in depth: Idle detection + checksum validation

## Hardware Setup

### NUCLEO-F411RE Wiring

```
RC Receiver → NUCLEO-F411RE
────────────────────────────
IBus pin    → PA10 (CN10-33, D2)
GND         → GND
VCC         → 5V (if receiver needs external power)
```

**Notes**:
- RX only (PA10 = USART1 RX)
- 3.3V signal compatible
- Most receivers powered from flight controller/ESC

## Examples

### IBus_Basic
Real receiver testing with dual mode:
- **RTT mode**: 15-second timed test with `*STOP*` wildcard for CI/HIL
- **Serial mode**: Continuous display for Arduino IDE
- Hardware validated with FlySky FS-iA6B

Upload via Arduino IDE, open Serial Monitor at 115200 baud.

### IBus_Loopback_Test
Dual-USART validation (no receiver needed):
- TX: USART6 (PA11) generates 100 Hz IBus frames
- RX: USART1 (PA10) parses via SerialRx
- Result: 501/501 frames (0% loss) in 5-second test

**Jumper**: PA11 (CN10-14) → PA10 (CN10-33)

### IBus_Loopback_DMA_Test
DMA mode validation using loopback (no receiver needed):
- Same loopback setup as IBus_Loopback_Test
- RX uses DMA mode: ~100 IRQs/sec vs ~11,500 IRQs/sec in interrupt mode
- Validates DMA buffer handling and IDLE line detection

**Jumper**: PA11 (CN10-14) → PA10 (CN10-33)

### Failsafe_Detection
Demonstrates the 4-layer failsafe detection system:
- Reports signal status transitions (`SIGNAL LOST` / `SIGNAL RECOVERED`)
- Shows held channel values during failsafe
- Multi-board: Nucleo F411RE (IBus), DevEBox H743 (SBUS), Open Revo (SBUS)
- 30-second timed test with frame statistics

Upload via Arduino IDE, open Serial Monitor at 115200 baud. Turn TX off/on to observe failsafe transitions.

### SBUS_Basic
SBUS receiver testing with multi-board support:
- **RTT mode**: 15-second timed test with `*STOP*` wildcard for CI/HIL
- **Serial mode**: Continuous display for Arduino IDE
- **Hardware RXINV**: Automatic on H7/F7/G4 via `config.invert_rx = true`

**Supported Boards**:
| Board | MCU | Hardware RXINV | External Inverter |
|-------|-----|----------------|-------------------|
| DevEBox H743 | STM32H7 | ✅ Yes | Not needed |
| OpenPilot Revolution | STM32F4 | ❌ No | Required |

Upload via Arduino IDE, select the appropriate board, open Serial Monitor at 115200 baud.

## Channel Mapping

Standard AETR (Aileron, Elevator, Throttle, Rudder):

| Channel | Function | Range |
|---------|----------|-------|
| Ch1 (0) | Aileron (Roll) | 1000-2000 µs |
| Ch2 (1) | Elevator (Pitch) | 1000-2000 µs |
| Ch3 (2) | Throttle | 1000-2000 µs |
| Ch4 (3) | Rudder (Yaw) | 1000-2000 µs |
| Ch5 (4) | Aux1 (Switch) | 1000/2000 µs |
| Ch6 (5) | Aux2 (Switch) | 1000/2000 µs |

## Architecture

### Class Hierarchy

```
SerialRx (Transport Layer + Failsafe Detection)
    ├── RingBuffer (Serial buffering)
    └── ProtocolParser (Interface)
            ├── IBusParser (Implemented)
            ├── SBusParser (Implemented)
            └── CRSFParser (Future)
```

## Performance

- **Flash**: ~2 KB (parser + transport)
- **RAM**: ~100 bytes (message buffer + state)
- **CPU**: ~10 µs per byte @ 100 MHz (STM32F411)
- **Interrupt-free**: Non-blocking, compatible with time-critical tasks

## SBUS Protocol (Implemented)

**Specifications**:
- Baudrate: 100000 (8E2 framing, inverted signal)
- Frame: 25 bytes (0x0F header, 22 channel bytes, flags, 0x00 footer)
- Channels: 16 × 11-bit (0-2047 range)
- Typical range: 172-1811 (1000-2000 µs equivalent)

**Signal Inversion**:
SBUS uses an inverted UART signal (idle low instead of idle high). The library provides automatic hardware inversion support via the `invert_rx` config option:

| MCU Family | Hardware RXINV | Notes |
|------------|----------------|-------|
| STM32H7 | ✅ Yes | USART_CR2_RXINV bit - no external hardware needed |
| STM32F7 | ✅ Yes | USART_CR2_RXINV bit - no external hardware needed |
| STM32G4 | ✅ Yes | USART_CR2_RXINV bit - no external hardware needed |
| STM32F4 | ❌ No | Requires external inverter circuit |

**External Inverter Options** (for F4):
- Single NPN transistor with pull-up resistor
- 74HC04 hex inverter IC
- Dedicated SBUS inverter module

**Example** (SBUS_Basic):
```cpp
SerialRx::Config config;
config.serial = &SerialRC;
config.rx_protocol = SerialRx::SBUS;
config.baudrate = 100000;
config.invert_rx = true;  // Enable hardware RX inversion (H7/F7/G4)
                          // On F4: no effect, external inverter required
rc.begin(config);
```

## Future Protocols

### CRSF (Future)

**Specifications**:
- Baudrate: 420000
- Frame: Variable length (RC channels = 26 bytes)
- Channels: 16 × 10-bit (0-1023 range)
- Checksum: CRC8-DVB-S2

**Complexity**: Variable-length frames, CRC8, multiple frame types (RC, telemetry, GPS).

## Testing

Upload examples via Arduino IDE or arduino-cli:

```bash
# Loopback test (validates protocol implementation, no receiver needed)
arduino-cli compile --fqbn STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE libraries/SerialRx/examples/IBus_Loopback_Test
arduino-cli upload --fqbn STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE libraries/SerialRx/examples/IBus_Loopback_Test

# Real receiver test (validates hardware integration)
arduino-cli compile --fqbn STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE libraries/SerialRx/examples/IBus_Basic
arduino-cli upload --fqbn STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE libraries/SerialRx/examples/IBus_Basic
```

## References

- **IBus Protocol**: FlySky iBus specification
- **Loopback Testing**: `libraries/SerialRx/examples/IBus_Loopback_Test/README.md`
- **Technical Docs**: `doc/SERIAL.md`

---

**Production Status**: ✅ Ready for flight controller integration
