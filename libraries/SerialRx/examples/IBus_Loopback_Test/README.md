# SerialRx IBus Loopback Example

Hardware validation example for the SerialRx library using USART loopback on a single MCU.

## Overview

This example demonstrates and validates the SerialRx library (IBus protocol) by creating a loopback between two USART peripherals:
- **USART6** generates IBus frames at 115200 baud (simulates RC receiver)
- **USART1** receives and parses frames via SerialRx library
- **Validates** frame reception, parsing accuracy, and timeout detection

## Hardware Setup

### Required Hardware
- **NUCLEO-F411RE** or compatible STM32F411 board
- **1x Jumper Wire** (female-to-female DuPont wire recommended)

### Connections

```
NUCLEO-F411RE Connections:
┌───────────────────────────────────┐
│  CN10 Connector (Right Side)      │
│                                   │
│  Pin 14: PA11 (USART6 TX) ●───┐   │
│                               │   │
│  Pin 33: PA10 (USART1 RX) ●◄──┘   │
│                                   │
└───────────────────────────────────┘

Jumper Wire: PA11 → PA10
```

**Pin Locations** (NUCLEO-F411RE):
| Signal | Arduino Pin | Morpho Pin | Connector | Physical Location |
|--------|-------------|------------|-----------|-------------------|
| USART6 TX | - | PA11 | CN10-14 | Right side, row 1, pin 14 |
| USART1 RX | D2 | PA10 | CN10-33 | Right side, row 3, pin 33 |

### Setup Instructions

1. Power off the board
2. Connect jumper wire: **PA11 (CN10-14) → PA10 (CN10-33)**
3. Verify connection is secure
4. Power on the board

**CAUTION**: Ensure no other connections to PA10 or PA11 during testing.

## Test Methodology

### Test Flow

1. **Initialization** (0-100ms):
   - Configure USART6 TX-only (PA11)
   - Configure USART1 RX via SerialRx library (PA10)
   - Begin test timer

2. **Frame Generation** (0-5000ms):
   - USART6 transmits IBus frames at 100 Hz (10ms interval)
   - Each frame contains 10 channels with incrementing test pattern
   - Total frames sent: ~500

3. **Frame Reception** (0-5000ms):
   - SerialRx polls USART1 for incoming bytes
   - IBusParser decodes frames and validates checksum
   - Received frames counted and validated

4. **Validation** (5000ms):
   - Frame count: Minimum 400 frames (80% success rate)
   - Frame loss: Maximum 2.0%
   - Channel values: Within expected range (1000-2999)
   - No unexpected timeouts

### Pass/Fail Criteria

**PASS** (`*TEST_PASS*`):
- ✅ Received ≥400 frames (out of ~500 sent)
- ✅ Frame loss rate ≤2.0%
- ✅ All channel values within valid range
- ✅ No unexpected timeouts

**FAIL** (`*TEST_FAIL*`):
- ❌ Received <400 frames
- ❌ Frame loss rate >2.0%
- ❌ Invalid channel values detected
- ❌ Unexpected timeout during test

## Running the Example

### Arduino IDE (Recommended for First Use)

1. Open `IBus_Loopback_Test.ino` in Arduino IDE
2. Select **Tools > Board > STM32 Boards > Nucleo-64**
3. Select **Tools > Board part number > Nucleo F411RE**
4. Upload sketch
5. Open Serial Monitor (115200 baud)
6. Connect jumper wire PA11 → PA10
7. Press RESET button
8. Observe test output in Serial Monitor

### Command Line Build

```bash
# Build with RTT and build traceability
./system/ci/build.sh libraries/SerialRx/examples/IBus_Loopback_Test --build-id --env-check --use-rtt

# Flash and run with automated exit detection
./system/ci/aflash.sh libraries/SerialRx/examples/IBus_Loopback_Test --use-rtt
```

### Expected Output

```
=== SerialRx IBus Loopback Example ===
Build SHA: eb20e9d
Build Date: 2025-10-18 11:04:57 UTC
Branch: serial-rx

Hardware Setup:
  Jumper: PA11 (USART6 TX) -> PA10 (USART1 RX)
  TX: USART6 @ 115200 baud (IBus generator)
  RX: USART1 @ 115200 baud (SerialRx library)

Test starting...
READY!
Progress: 100 frames RX, 100 frames TX
Progress: 200 frames RX, 200 frames TX
Progress: 300 frames RX, 300 frames TX
Progress: 400 frames RX, 400 frames TX

=== Test Complete ===
Frames Sent: 500
Frames Received: 498
Frame Loss: 2 (0.40%)

*TEST_PASS*
```


## Troubleshooting

### No frames received (0 frames RX)

**Cause**: Jumper wire not connected or wrong pins

**Solution**:
1. Verify jumper connects PA11 (CN10-14) to PA10 (CN10-33)
2. Check wire continuity with multimeter
3. Ensure pins are not bent or damaged

### Low frame count (50-200 frames RX)

**Cause**: Intermittent connection or electrical noise

**Solution**:
1. Use shorter jumper wire (<10cm recommended)
2. Ensure firm connection to header pins
3. Check for corrosion on header pins
4. Try different jumper wire

### Test fails with high frame loss (>2%)

**Cause**: Timing issues or buffer overflow

**Solution**:
1. Verify correct FQBN: `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE`
2. Check system clock configuration (should be 100 MHz)
3. Ensure no other high-priority interrupts interfering

### Compile errors

**Cause**: Missing SerialRx library or incorrect paths

**Solution**:
1. Verify `libraries/SerialRx/` exists in repository
2. Check library.properties is present
3. Ensure SerialRx library compiles standalone

## Technical Details

### IBus Frame Format

```
Byte 0-1:   Header (0x20 0x40)
Byte 2-29:  14 channels × 2 bytes each (little-endian)
Byte 30-31: Checksum (little-endian)
Total: 32 bytes per frame
```

**Checksum Calculation**:
```
checksum = 0xFFFF - sum(all bytes except checksum)
```

### Peripheral Configuration

**USART6 (Transmitter)**:
- Mode: TX-only (RX pin = NC)
- Baudrate: 115200
- Data bits: 8
- Parity: None
- Stop bits: 1
- Pin: PA11 (CN10-14)

**USART1 (Receiver via SerialRx)**:
- Mode: RX+TX (TX unused)
- Baudrate: 115200
- Data bits: 8
- Parity: None
- Stop bits: 1
- Pin: PA10 (CN10-33)

### Memory Usage

- Flash: 14.4 KB (2% of 512 KB)
- RAM: 2.5 KB (1.9% of 128 KB)

## Related Examples

- `libraries/SerialRx/examples/IBus_Basic/` - Basic RC receiver example with real hardware

## References

- SerialRx Library: `libraries/SerialRx/`
- IBus Protocol: FlySky iBus specification
- HIL Framework: `doc/SERIAL.md` (loopback testing section)
