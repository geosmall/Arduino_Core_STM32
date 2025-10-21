# SerialRx - RC Receiver Serial Protocol Parser

Arduino library for parsing serial RC receiver protocols (IBus, SBUS, CRSF) commonly used in UAV flight controllers.

**Hardware Validated**: Tested with FlySky FS-iA6B receiver and dual-USART loopback testing.

## Supported Protocols

| Protocol | Status | Baudrate | Frame Size | Channels | Validated |
|----------|--------|----------|------------|----------|-----------|
| **IBus** (FlySky) | ✅ Implemented | 115200 | 32 bytes | 14 | ✅ Yes |
| **SBUS** (FrSky/Futaba) | ✅ Implemented | 100000 | 25 bytes | 16 | ⚠️ No |
| **CRSF** (TBS Crossfire) | 📋 Framework Ready | 420000 | Variable | 16 | ⚠️ No |

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

  if (rc.available()) {
    RCMessage msg;
    if (rc.getMessage(&msg)) {
      uint16_t throttle = msg.channels[2];  // Ch3 (0-indexed)
      uint16_t aileron = msg.channels[0];   // Ch1
      // Process RC commands (1000-2000 µs range)
    }
  }

  if (rc.timeout(1000)) {
    // Signal lost - activate failsafe
  }
}
```

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

**Usage**:
```bash
# CI/HIL testing
./scripts/aflash.sh libraries/SerialRx/examples/IBus_Basic --use-rtt --build-id

# Arduino IDE
# Upload via IDE, open Serial Monitor at 115200 baud
```

### IBus_Loopback_Test
Dual-USART validation:
- TX: USART6 (PA11) generates 100 Hz IBus frames
- RX: USART1 (PA10) parses via SerialRx
- Result: 501/501 frames (0% loss) in 5-second test

**Jumper**: PA11 (CN10-14) → PA10 (CN10-33)

### SBUS_Basic
SBUS receiver testing (⚠️ not hardware validated yet):
- **RTT mode**: 15-second timed test with `*STOP*` wildcard for CI/HIL
- **Serial mode**: Continuous display for Arduino IDE
- **IMPORTANT**: Requires inverted signal (hardware or software)

**Usage**:
```bash
# CI/HIL testing
./scripts/aflash.sh libraries/SerialRx/examples/SBUS_Basic --use-rtt --build-id

# Arduino IDE
# Upload via IDE, open Serial Monitor at 115200 baud
```

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
SerialRx (Transport Layer)
    ├── RingBuffer (Serial buffering)
    └── ProtocolParser (Interface)
            ├── IBusParser (Implemented)
            ├── SBusParser (Future)
            └── CRSFParser (Future)
```

## Performance

- **Flash**: ~2 KB (parser + transport)
- **RAM**: ~100 bytes (message buffer + state)
- **CPU**: ~10 µs per byte @ 100 MHz (STM32F411)
- **Interrupt-free**: Non-blocking, compatible with time-critical tasks

## SBUS Protocol (Implemented)

**Specifications**:
- Baudrate: 100000 (inverted signal - requires hardware inverter or GPIO config)
- Frame: 25 bytes (0x0F header, 22 channel bytes, flags, 0x00 footer)
- Channels: 16 × 11-bit (0-2047 range)
- Typical range: 172-1811 (1000-2000 µs equivalent)

**Signal Inversion**:
SBUS uses inverted UART signal. Options:
1. STM32: Enable USART RX inversion (RXINV bit in USART_CR2)
2. External inverter (transistor, 74HC04, etc.)

**Example** (SBUS_Basic):
```cpp
SerialRx::Config config;
config.rx_protocol = SerialRx::SBUS;
config.baudrate = 100000;
// Note: Requires signal inversion hardware/software
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

```bash
# Loopback test (validates protocol implementation)
./scripts/aflash.sh libraries/SerialRx/examples/IBus_Loopback_Test --use-rtt

# Real receiver test (validates hardware integration)
./scripts/aflash.sh libraries/SerialRx/examples/IBus_Basic --use-rtt
```

## References

- **IBus Protocol**: FlySky iBus specification
- **Loopback Testing**: `libraries/SerialRx/examples/IBus_Loopback_Test/README.md`
- **Technical Docs**: `doc/SERIAL.md`

---

**Production Status**: ✅ Ready for flight controller integration
