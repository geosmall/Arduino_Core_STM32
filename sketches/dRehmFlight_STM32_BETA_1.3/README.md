# dRehmFlight STM32 Port - BETA 1.3

Minimal-change port of [dRehmFlight](https://github.com/nickrehm/dRehmFlight) BETA 1.3 from Teensy 4.0 to STM32 for 4-motor conventional quadcopter.

## Overview

This port preserves 100% of Nicholas Rehm's flight control logic while adapting only the hardware interface layer for STM32F4/F7/H7 microcontrollers.

**Supported Boards** (5 boards):

| Board | MCU | IMU | Notes |
|-------|-----|-----|-------|
| NUCLEO_F411RE | STM32F411RE | ICM-42688-P | Development board (breadboard IMU) |
| BLACKPILL_F411CE | STM32F411CE | MPU-9250 | Compact dev board (9-DOF) |
| OPEN_REVO | STM32F405RGT6 | MPU-6000 | OpenPilot Revolution FC |
| BKMN_NERO | STM32F722RET6 | ICM-20602 | NERO F7 flight controller |
| MATEK_H743VI | STM32H743VIT6 | ICM-42688-P | MATEK H743-WLITE FC |

## Upstream Links

- **Original dRehmFlight**: https://github.com/nickrehm/dRehmFlight
- **Teensy reference**: `sketches/dRehmFlight_Teensy_BETA_1.3/`

**Diff Reports** (`diff_reports/`, side-by-side Teensy → STM32):
- `diff.html` - Combined view (both files with navigation)
- `diff_main.html` - Main sketch only (1735 vs 1622 lines)
- `diff_radioComm.html` - Radio communication only (198 vs 112 lines)

**Updating Diff Reports**:
- **Automatic**: A git pre-commit hook regenerates diffs when `.ino` files are committed
- **Manual** (from `Arduino_Core_STM32/`):
  ```bash
  ../ci/tools/winmerge_diff.py \
    sketches/dRehmFlight_Teensy_BETA_1.3/dRehmFlight_Teensy_BETA_1.3.ino \
    sketches/dRehmFlight_STM32_BETA_1.3/dRehmFlight_STM32_BETA_1.3.ino \
    sketches/dRehmFlight_STM32_BETA_1.3/diff_reports/diff_main.html

  ../ci/tools/winmerge_diff.py \
    sketches/dRehmFlight_Teensy_BETA_1.3/radioComm.ino \
    sketches/dRehmFlight_STM32_BETA_1.3/radioComm.ino \
    sketches/dRehmFlight_STM32_BETA_1.3/diff_reports/diff_radioComm.html
  ```

## What Changed

**Hardware Interface Layer Only** (flight control logic untouched):

### 1. IMU Integration - IMU Library
- `IMUinit()`: Uses IMU library with auto-detection (ICM-42688-P, MPU-6000, MPU-9250)
- `getIMUdata()`: Library-based reads with configurable FSR
- **Preserved**: Error correction, low-pass filtering, all math

### 2. Radio RX - SerialRx Library
- `radioSetup()`: SerialRx initialization (IBus default, SBUS optional)
- `updateRadioChannels()`: Adapter pattern (SerialRx → channel_X_raw)
- **Optional**: DMA mode for reduced interrupt overhead

### 3. Motor Control - MotorManager (TimerPWM)
- `commandMotors()`: Hardware timers via BoardConfig abstraction
- `armMotors()`: ESC arming sequence
- **Protocol**: OneShot125 (125-250µs pulses)

### 4. Servo Control - ServoManager (TimerPWM)
- `commandServos()`: Standard PWM via BoardConfig abstraction
- **Protocol**: 50 Hz PWM (1000-2000µs)

### 5. Pin Configuration - BoardConfig System
- **Auto-detection**: Board type from Arduino IDE selection
- **Abstraction**: Motors, servos, IMU, RC all via BoardConfig
- **Multi-board**: Single codebase supports all 5 boards

## What Did NOT Change

**100% Preserved Flight Control**:
- ✅ PID Controllers (`controlANGLE()`, `controlRATE()`)
- ✅ Control Mixer (`controlMixer()`)
- ✅ Madgwick Filter (`Madgwick6DOF()`)
- ✅ Command Scaling (`scaleCommands()`)
- ✅ Failsafe Logic (`failSafe()`)
- ✅ Arming Logic (`armedStatus()`)
- ✅ Loop Timing (2kHz)
- ✅ All PID Tuning Parameters

## Metrics (NUCLEO_F411RE)

| Metric | Value |
|--------|-------|
| Binary Size | 49KB (9% of 512KB flash) |
| RAM Usage | 3.7KB (2% of 128KB RAM) |
| Line Count | 1933 → 1734 (-10%) |
| Flight Logic Modified | 0 functions |

## Configuration Options

Enable features in the main `.ino` file:

```cpp
// RC Receiver (required - choose one protocol)
#define USE_SERIAL_RX
#define USE_IBUS_RX    // IBus protocol (FlySky) - default
// #define USE_SBUS_RX // SBUS protocol (FrSky, etc.)
// #define USE_RC_DMA  // Optional: UART DMA for reduced IRQ overhead

// IMU (required - choose one)
#define USE_ICM42688P   // ICM-42688-P, ICM-20602, MPU-6000
// #define USE_MPU9250_SPI // MPU-9250/9255 (enables magnetometer)

// Gyro Full Scale Range
#define GYRO_250DPS     // Default - highest resolution
// #define GYRO_500DPS
// #define GYRO_1000DPS
// #define GYRO_2000DPS  // Widest range for aerobatics

// Accelerometer Full Scale Range
#define ACCEL_2G        // Default
// #define ACCEL_4G
// #define ACCEL_8G
// #define ACCEL_16G
```

## STM32 Libraries Used

- **IMU** - High-level IMU wrapper with auto-detection (ICM-42688-P, MPU-6000, MPU-9250)
- **SerialRx** - IBus/SBUS protocol parser with optional DMA
- **PWMOutputBank** - MotorManager (OneShot125) and ServoManager (50Hz PWM)
- **BoardConfig** - Multi-board pin abstraction (targets/*.h)

All libraries available in [Arduino_Core_STM32](https://github.com/geosmall/Arduino_Core_STM32).

## Build

```bash
# From workspace root (Arduino/)
./ci/build.sh Arduino_Core_STM32/sketches/dRehmFlight_STM32_BETA_1.3

# Or with arduino-cli directly
arduino-cli compile --fqbn STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE \
  Arduino_Core_STM32/sketches/dRehmFlight_STM32_BETA_1.3

# Build for other boards
arduino-cli compile --fqbn STM32_Robotics:stm32:FlightCtr:pnum=OPEN_REVO \
  Arduino_Core_STM32/sketches/dRehmFlight_STM32_BETA_1.3

arduino-cli compile --fqbn STM32_Robotics:stm32:FlightCtr:pnum=BKMN_NERO \
  Arduino_Core_STM32/sketches/dRehmFlight_STM32_BETA_1.3

arduino-cli compile --fqbn STM32_Robotics:stm32:FlightCtr:pnum=MATEK_H743VI \
  Arduino_Core_STM32/sketches/dRehmFlight_STM32_BETA_1.3
```

**Requirements**:
- arduino-cli 1.0.0+
- STM32_Robotics core installed

## Debugging

Uses standard `Serial.print()` like the original Teensy version:

```cpp
Serial.begin(115200);  // Initialized in setup()
Serial.println("Status message");
```

**Example Output**:
```
dRehmFlight STM32 BETA 1.3
Radio RX initialized (interrupt mode)
IMU initialized successfully
Gyro X:0.35 Y:-0.81 Z:0.30
```

## Current Status

**Port Status: ✅ Complete - Ready for Hardware Testing**

| Component | Status |
|-----------|--------|
| IMU initialization | ✅ Working (auto-detection) |
| IMU data reading | ✅ Working (validated values) |
| Radio RX (IBus/SBUS) | ✅ Working (interrupt + DMA modes) |
| Motor control | ✅ Working (OneShot125 via MotorManager) |
| Servo control | ✅ Working (50Hz via ServoManager) |
| 2kHz loop timing | ✅ Working |
| Serial debugging | ✅ Working |
| Multi-board support | ✅ Working (5 boards) |

**Hardware Validation**:
- ✅ IMU communication verified (WHO_AM_I responses)
- ✅ IMU self-test passed
- ✅ Gyro readings validated (stationary drift as expected)
- 📋 RC receiver bench testing pending
- 📋 Motor control bench testing pending
- 📋 Flight testing pending

## Next Steps

### Phase 1: IMU Data Validation ✅ COMPLETE

FSR (Full Scale Range) configuration verified:
- ±250 DPS: 131 LSB/°/s sensitivity (default, highest resolution)
- ±2000 DPS: 16.4 LSB/°/s sensitivity (widest range)

Both configurations produce correct physical values when properly scaled.

### Phase 2: Hardware Bench Testing

**RC Receiver**:
- Connect FlySky FS-iA6B (IBus) or FrSky (SBUS) receiver
- Verify channel mapping (throttle, roll, pitch, yaw)
- Test failsafe behavior
- Validate arming/disarming logic

**Motor Control**:
- Connect ESCs to motor outputs
- Test OneShot125 pulse generation (125-250µs)
- Verify motor response to stick inputs
- Confirm failsafe stops motors

### Phase 3: Flight Testing

- Props-on motor response testing
- PID tuning on bench
- Initial hover attempts
- Progressive flight envelope expansion

### Phase 4: Flight Controller Deployment

- Deploy to target flight controller (OPEN_REVO, NERO F7, MATEK H743)
- Verify all peripherals
- Production flight testing

## License

MIT License - Same as original dRehmFlight

## Attribution

Original work by Nicholas Rehm: https://github.com/nickrehm/dRehmFlight

STM32 port maintains the educational focus and clean code style of the original while demonstrating minimal-change hardware abstraction patterns.
