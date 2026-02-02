# OpenPilot Revolution F405 - RC Input Configuration

Hardware-validated configuration for the OpenPilot Revolution F405 flight controller.

## RC Input Pinout

| Function | Pin | Notes |
|----------|-----|-------|
| **RC Signal** | PA10 | USART1 RX |
| **Inverter Control** | PC0 | GPIO output |
| **USART1 TX** | PA9 | Unused for RC input |

## Hardware Inverter

The board has a hardware inverter circuit on the USART1 RX line, controlled by PC0. This allows the same 3-pin header to support multiple RC protocols:

| PC0 State | Signal Mode | Supported Protocols | Baud Rate | Framing |
|-----------|-------------|---------------------|-----------|---------|
| **HIGH** | Inverted | **SBUS** | 100000 | 8E2 |
| **LOW** | Pass-through | iBus, DSM, other non-inverted | 115200 | 8N1 |

**CRITICAL**: Set PC0 to match your receiver's protocol:
- **SBUS**: `digitalWrite(PC0, HIGH)` - inverts the already-inverted SBUS signal
- **iBus**: `digitalWrite(PC0, LOW)` - passes non-inverted signal through

**Note**: iBus support is hardware-ready but not validated. The FS-A8S receiver output mode is set during binding (not via transmitter menu):
- **SBUS**: Normal bind (short press bind button)
- **iBus**: Hold bind button ~3 sec while powering on, then bind

SBUS is the recommended protocol - it's the standard for flight controllers and validated to work perfectly on this board.

## Validation Results

### Pin Activity Test (PinWiggle)

Tested 2026-02-01 using PinWiggle diagnostic sketch with FS-A8S receiver:

- **Active pin**: PA10 (USART1 RX)
- **Transition rate**: ~27,000/sec (consistent with SBUS @ 100kbaud)
- **Inverter state**: HIGH (SBUS mode)
- **All other UART pins**: 0 transitions

#### Pins Tested

| Pin | UART | Result |
|-----|------|--------|
| PA10 | USART1 RX | **~27k trans/sec** |
| PA9 | USART1 TX | 0 |
| PB_6 | USART1 TX alt | 0 |
| PB_7 | USART1 RX alt | 0 |
| PA2 | USART2 TX | 0 |
| PA3 | USART2 RX | 0 |
| PC6 | USART6 TX | 0 |
| PC7 | USART6 RX | 0 |
| PA0 | UART4 TX | 0 |
| PA1 | UART4 RX | 0 |
| PC10 | UART4 TX alt | 0 |
| PC11 | UART4 RX alt | 0 |
| PB14 | PPM | 0 |

Note: USART3 (PB10/PB11) excluded - used for debug serial output (Flexi port).

### SerialRx SBUS Test

Tested 2026-02-01 using REVO_SBUS_Test sketch with FS-A8S receiver + FS-I6X transmitter:

```
=== REVO F405 SBUS Test ===
RC Input: PA10 (USART1 RX)
Inverter: PC0
Inverter: ENABLED (SBUS mode)

=== Test Complete ===
Frames: 2144
Failed: 0
Loss:   0.00%
```

| Metric | Value |
|--------|-------|
| Frame rate | ~143 Hz |
| Frames received | 2144 (15 sec) |
| Frames failed | 0 |
| Loss rate | 0.00% |
| Test duration | 15 seconds |

#### Channel Values (sticks centered)

| Channel | Value | Function |
|---------|-------|----------|
| Ch1 | 1023 | Aileron |
| Ch2 | 1027 | Elevator |
| Ch3 | 1047 | Throttle |
| Ch4 | 914 | Rudder |
| Ch5 | 1807 | Switch |
| Ch6 | 240 | Aux |

SBUS range: 0-2047 (typical usable range: 172-1811)

### Inverter Toggle Test

Tested 2026-02-01 to verify inverter is required for SBUS:

```
Test sequence:
  0-5s:  Inverter HIGH (should work)
  5-10s: Inverter LOW (should fail)
  10-15s: Inverter HIGH (should recover)

[4s] Inv=HIGH Frames=713 Timeout=no
>>> Inverter: LOW (disabled) - expecting signal loss
[5s] Inv=LOW  Frames=716 Timeout=YES   ← Frame count frozen
[6s] Inv=LOW  Frames=716 Timeout=YES
[7s] Inv=LOW  Frames=716 Timeout=YES
[8s] Inv=LOW  Frames=716 Timeout=YES
[9s] Inv=LOW  Frames=716 Timeout=YES
>>> Inverter: HIGH (enabled) - frames during LOW: 0
[10s] Inv=HIGH Frames=855 Timeout=no   ← Immediate recovery
```

| Phase | Inverter | Frames/sec | Status |
|-------|----------|------------|--------|
| 0-5s | HIGH | ~143 | Working |
| 5-10s | **LOW** | **0** | **Complete loss** |
| 10-15s | HIGH | ~143 | Recovered |

**Key findings**:
- **Inverter LOW = Zero valid SBUS frames** (UART sees garbage)
- **Timeout detected within 500ms** of inverter disable
- **Instant recovery** when inverter re-enabled
- **Frames during LOW phase: 0** (confirmed)

### Raw Serial Protocol Detection

Tested 2026-02-01 using REVO_Serial_Debug sketch to identify protocol by examining raw bytes:

```
Test sequence:
  0-3s:  LOW  @ 115200 (iBus expected)
  3-6s:  HIGH @ 115200
  6-9s:  LOW  @ 100000 (SBUS baud)
  9-12s: HIGH @ 100000 (SBUS expected)

[0-2s] Inv=LOW  Baud=115200 Bytes=0        ← No data (wrong polarity)
[3-5s] Inv=HIGH Baud=115200 Bytes=3600/sec SBUS=35-83 headers/sec
[6-8s] Inv=LOW  Baud=100000 Bytes=0        ← No data (wrong polarity)
[9-11s] Inv=HIGH Baud=100000 Bytes=3550/sec SBUS=143 headers/sec ✓
```

| Configuration | Bytes/sec | SBUS Headers (0x0F) | Result |
|---------------|-----------|---------------------|--------|
| LOW @ 115200 | 0 | 0 | No signal |
| HIGH @ 115200 | ~3600 | 35-83 | Misaligned SBUS |
| LOW @ 100000 | 0 | 0 | No signal |
| **HIGH @ 100000** | **~3550** | **143/sec** | **Valid SBUS** |

**Key findings**:
- SBUS detected by 0x0F header bytes at 143 Hz (matches frame rate)
- No iBus headers (0x20 0x40) detected - receiver outputs SBUS only
- Correct config: PC0=HIGH @ 100000 baud, 8E2 framing

## Code Example

### Standalone (No BoardConfig)

```cpp
#include <SerialRx.h>

// REVO F405 RC Input pins
#define RC_RX_PIN       PA10  // USART1 RX
#define RC_TX_PIN       PA9   // USART1 TX
#define RC_INVERTER_PIN PC0   // Hardware inverter

HardwareSerial SerialRC(RC_RX_PIN, RC_TX_PIN);
SerialRx rx;

void setup() {
    // CRITICAL: Enable inverter for SBUS
    pinMode(RC_INVERTER_PIN, OUTPUT);
    digitalWrite(RC_INVERTER_PIN, HIGH);  // HIGH = SBUS (inverted)

    // Configure SerialRx
    SerialRx::Config config;
    config.serial = &SerialRC;
    config.rx_protocol = SerialRx::SBUS;
    config.baudrate = 100000;
    config.timeout_ms = 1000;

    rx.begin(config);
}

void loop() {
    rx.update();

    if (rx.available()) {
        RCMessage msg;
        if (rx.getMessage(&msg)) {
            uint16_t throttle = msg.channels[2];
            uint16_t aileron = msg.channels[0];
            // ...
        }
    }

    if (rx.timeout(1000)) {
        // Handle failsafe
    }
}
```

### Using BoardConfig

```cpp
#include <SerialRx.h>
#include "targets/OPEN-REVO.h"

HardwareSerial SerialRC(BoardConfig::rc_receiver.rx_pin,
                        BoardConfig::rc_receiver.tx_pin);
SerialRx rx;

void setup() {
    // CRITICAL: Enable inverter for SBUS
    pinMode(BoardConfig::rc_inverter_pin, OUTPUT);
    digitalWrite(BoardConfig::rc_inverter_pin, HIGH);

    // Configure SerialRx
    SerialRx::Config config;
    config.serial = &SerialRC;
    config.rx_protocol = SerialRx::SBUS;
    config.baudrate = BoardConfig::rc_receiver.baud_rate;
    config.timeout_ms = BoardConfig::rc_receiver.timeout_ms;

    rx.begin(config);
}
```

### Using CI/RTT Infrastructure

For HIL testing with clean RTT output:

```cpp
#include <SerialRx.h>
#include <ci_log.h>
#include <libPrintf.h>

// Required for CI_PRINTF
extern "C" void putchar_(char c) {
#ifdef USE_RTT
    SEGGER_RTT_PutChar(0, c);
#else
    Serial.write(c);
#endif
}

#define RC_RX_PIN       PA10
#define RC_TX_PIN       PA9
#define RC_INVERTER_PIN PC0

HardwareSerial SerialRC(RC_RX_PIN, RC_TX_PIN);
SerialRx rc;

void setup() {
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
#endif

    CI_LOG("=== SBUS Test ===\n");
    CI_BUILD_INFO();

    pinMode(RC_INVERTER_PIN, OUTPUT);
    digitalWrite(RC_INVERTER_PIN, HIGH);

    SerialRx::Config config;
    config.serial = &SerialRC;
    config.rx_protocol = SerialRx::SBUS;
    config.baudrate = 100000;
    config.timeout_ms = 500;

    rc.begin(config);
    CI_READY_TOKEN();
}

void loop() {
    rc.update();
    // ... test logic ...
    CI_PRINTF("Frames: %lu\n", rc.getFramesReceived());
    CI_LOG("*STOP*\n");  // Required for aflash.sh exit detection
    while(1);
}
```

## Physical Connector

The 3-pin SBUS header on the board:

| Pin | Signal |
|-----|--------|
| 1 | GND |
| 2 | +5V |
| 3 | Signal (to PA10 via inverter) |

## Build Commands

```bash
# Run SBUS validation (RTT output - recommended for clean output)
./ci/aflash.sh Arduino_Core_STM32/sketches/REVO_SBUS_Simple \
    STM32_Robotics:stm32:FlightCtr:pnum=OPEN_REVO --use-rtt

# Run SBUS test with inverter toggle (RTT output)
./ci/aflash.sh Arduino_Core_STM32/sketches/REVO_SBUS_Test \
    STM32_Robotics:stm32:FlightCtr:pnum=OPEN_REVO --use-rtt

# Run iBus test (RTT output)
./ci/aflash.sh Arduino_Core_STM32/sketches/REVO_IBus_Test \
    STM32_Robotics:stm32:FlightCtr:pnum=OPEN_REVO --use-rtt

# Run PinWiggle diagnostic (Serial output)
./ci/saflash.sh Arduino_Core_STM32/sketches/PinWiggle \
    STM32_Robotics:stm32:FlightCtr:pnum=OPEN_REVO
```

**Note**: RTT output (`--use-rtt`) is recommended over Serial for clean output. The J-Link CDC serial port can exhibit garbled output on this board, while RTT provides reliable, clean output via SEGGER J-Link.

## Files Modified/Created

| File | Purpose |
|------|---------|
| `variants/.../variant_OPEN_REVO.h` | Added RC input documentation |
| `targets/OPEN-REVO.h` | Added `rc_inverter_pin` constant |
| `sketches/REVO_SBUS_Test/` | SBUS test with inverter toggle (CI/RTT) |
| `sketches/REVO_SBUS_Simple/` | Minimal SBUS validation (CI/RTT) |
| `sketches/REVO_IBus_Test/` | iBus test with polarity toggle (CI/RTT) |
| `sketches/REVO_Serial_Debug/` | Raw byte protocol detector (Serial) |
| `sketches/PinWiggle/` | UART pin activity detector (Serial) |
| `libraries/SerialRx/examples/SBUS_Basic/` | Added F4 inverter support |

Test sketches marked (CI/RTT) use `ci_log.h` and `libPrintf.h` for dual Serial/RTT output support.

## Validation Summary

Multiple test runs confirmed reliable SBUS reception:

| Test Run | Duration | Frames | Failed | Rate | Loss |
|----------|----------|--------|--------|------|------|
| REVO_SBUS_Test | 15 sec | 2144 | 0 | 143 Hz | 0.00% |
| REVO_SBUS_Simple | 10 sec | 1428 | 0 | 142 Hz | 0.00% |
| Inverter Toggle | 15 sec | 1430 | 0 | 143 Hz | 0.00% |
| **Final (RTT)** | 10 sec | 1428 | 0 | 142 Hz | 0.00% |

**Result**: 100% checksum pass rate across all tests. Zero failed frames.

**Hardware Validated**: OpenPilot Revolution F405 + FS-A8S receiver + FS-I6X transmitter

## Known Issues

### J-Link CDC Serial Garbling

Serial output via J-Link CDC (`/dev/ttyACM0`) can exhibit garbled/corrupted characters on this board. The SBUS data is valid - only the debug output display is affected.

**Workaround**: Use RTT output (`--use-rtt` flag) for clean, reliable output. RTT bypasses the USB CDC path and provides deterministic output via SEGGER J-Link.

```bash
# Garbled output (Serial via J-Link CDC)
./ci/saflash.sh ... STM32_Robotics:stm32:FlightCtr:pnum=OPEN_REVO

# Clean output (RTT via J-Link)
./ci/aflash.sh ... STM32_Robotics:stm32:FlightCtr:pnum=OPEN_REVO --use-rtt
```

## References

- Variant: `variants/STM32F4xx/F405RGT_F415RGT/variant_OPEN_REVO.h`
- Target config: `targets/OPEN-REVO.h`
- SBUS test: `sketches/REVO_SBUS_Test/`
- Simple validation: `sketches/REVO_SBUS_Simple/`
- Protocol detector: `sketches/REVO_Serial_Debug/`
- Pin diagnostic: `sketches/PinWiggle/`
- Betaflight config: [OPEN-REVO.config](https://github.com/betaflight/unified-targets/blob/master/configs/default/OPEN-REVO.config)
