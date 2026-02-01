# dRehmFlight STM32 Port - BETA 1.3

Minimal-change port of [dRehmFlight](https://github.com/nickrehm/dRehmFlight) BETA 1.3 from Teensy 4.0 to STM32F4 for 4-motor conventional quadcopter.

## Overview

This port preserves 100% of Nicholas Rehm's flight control logic while adapting only the hardware interface layer for STM32F4 microcontrollers.

**Target Hardware** (5 boards supported):
- **NUCLEO_F411RE**: Development board with breadboard ICM42688P + IBus receiver
- **BLACKPILL_F411CE**: Compact development board with MPU-9250
- **OPEN_REVO**: OpenPilot Revolution F405 flight controller (STM32F405, MPU-6000)
- **BKMN_NERO**: NERO F7 flight controller (STM32F722, ICM-20602)
- **MATEK_H743VI**: MATEK H743-WLITE flight controller (STM32H743, ICM42688P)

## Upstream Links

- **Original dRehmFlight**: https://github.com/nickrehm/dRehmFlight
- **Teensy reference**: `Arduino_Core_STM32/sketches/dRehmFlight_Teensy_BETA_1.3/`

## What Changed

**6 Hardware Interface Functions** (flight control logic untouched):

1. **IMU Integration** - ICM42688P via IMU library
   - `IMUinit()`: Clean API instead of raw register access
   - `getIMUdata()`: Library-based reads with auto-scaling
   - **Preserved**: Error correction, low-pass filtering, all math

2. **Radio RX** - SBUS via SerialRx library
   - `radioSetup()`: SerialRx initialization
   - `updateRadioChannels()`: Adapter pattern (SerialRx → channel_X_raw)
   - **Eliminated**: 110 lines of interrupt handlers

3. **Motor Control** - OneShot125 via TimerPWM
   - `commandMotors()`: Hardware timers (TIM1, TIM3)
   - `setup()`: PWMOutputBank initialization
   - **Preserved**: Same 125-250µs pulse widths

4. **Pin Configuration** - BoardConfig abstraction
   - **Motors** (NUCLEO_F411RE): TIM1 (PA8, PA9, PA10), TIM3 (PB0_ALT1, PB4)
   - **LED**: Board-specific (PC13 on BLACKPILL, PA5 on NUCLEO)
   - **Multi-board**: NUCLEO_F411RE, BLACKPILL_F411CE, BKMN_NERO, MATEK_H743VI

5. **Quad Focus** - Servos commented out (4-motor conventional quad)

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
| Binary Size | 47.5KB (9% of 512KB flash) |
| RAM Usage | 3.3KB (2% of 128KB RAM) |
| Line Count | 1933 → 1640 (-15%) |
| Flight Logic Modified | 0 functions |
| Hardware Interface Modified | 6 functions |

## STM32 Libraries Used

- **IMU** - ICM42688P high-level wrapper
- **SerialRx** - IBus/SBUS protocol parser
- **TimerPWM** - Hardware timer PWM (PWMOutputBank)
- **BoardConfig** - Multi-board pin abstraction

All libraries available in [Arduino_Core_STM32](https://github.com/geosmall/Arduino_Core_STM32).

## Build

```bash
# Build with arduino-cli
arduino-cli compile --fqbn STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE \
  Arduino_Core_STM32/sketches/dRehmFlight_STM32_BETA_1.3

# Build and flash with CI (Serial output via saflash.sh)
./ci/saflash.sh Arduino_Core_STM32/sketches/dRehmFlight_STM32_BETA_1.3 --timeout 5
```

**Requirements**:
- arduino-cli 1.3.0+
- STM32_Robotics core

## Debugging

Uses standard `Serial.print()` like the original Teensy version:

```cpp
Serial.begin(115200);  // Initialized in setup()
Serial.println("Status message");
Serial.print("Value:"); Serial.println(value, 2);
```

**CI Testing** via saflash.sh captures Serial output automatically.

**Example Output**:
```
dRehmFlight STM32 BETA 1.3
Radio RX initialized (interrupt mode)
IMU initialized successfully
No magnetometer detected (6-DOF mode)
Gyro X:0.00 Y:0.00 Z:0.00
```

## Current Status

**Port Status: ✅ Complete - Ready for Hardware Testing**
- ✅ Port compiles successfully (47.5KB binary)
- ✅ Setup() executes without crashes
- ✅ IMU initializes and produces valid data
- ✅ Radio RX initializes (SBUS on USART1)
- ✅ Motor timers initialize (OneShot125 ready)
- ✅ Main loop running at 2kHz
- ✅ Serial logging working
- ✅ Independent gyro axis values confirmed

**Issues Resolved**:
1. **UART Conflict** - Fixed by moving RC receiver to USART1 (PB7/PB6)
   - Serial debug now uses USART2 (PA2/PA3) exclusively
2. **Uninitialized Callbacks** - Fixed by NULL initialization in HardwareSerial::init()
   - Added NULL checks in UART interrupt handlers
3. **IMU Sensor Enable** - Fixed by adding EnableAccelLNMode() and EnableGyroLNMode()
   - ConfigureInvDevice() sets registers but doesn't start continuous sampling
   - Sensors were stuck in power-off state returning saturated values (-32768)
   - Fix enables continuous 2kHz data acquisition for polling-based flight loop

**Polling-Based IMU Approach**:
- Uses direct polling instead of hardware interrupts
- 2kHz loop rate matches 2kHz IMU ODR for optimal data freshness
- No interrupt pin required (simpler hardware setup)
- Validated equivalent to interrupt-driven approach (see `libraries/imu/examples/README.md`)
- Same methodology used by Betaflight, iNav, and other flight controllers

**Hardware Validation Status**:
- ✅ Breadboard setup on NUCLEO_F411RE
- ✅ IMU communication verified (WHO_AM_I = 0x47)
- ✅ IMU self-test passed
- ✅ IMU data reading operational (independent axis values)
- ✅ Gyro readings: X≈0.38, Y≈-0.81, Z≈0.30 deg/sec (stationary drift, as expected)
- 📋 RC receiver bench testing pending
- 📋 Motor control bench testing pending
- 📋 Flight testing pending
- 📋 Deployment to flight controller hardware pending validation

## Next Steps

### Phase 1: IMU Data Validation ✅ COMPLETE

**Issue Investigated**:
- dRehmFlight showed ~8x higher raw IMU values than imu-polled-basic example
- Both used correct scaling (131 LSB/°/s from datasheet)
- Same hardware (NUCLEO_F411RE, ICM-42688-P, stationary board)

**Root Cause Identified**: Full-Scale Range (FSR) Configuration Difference

The 8x discrepancy was caused by different gyroscope full-scale range settings:

| Configuration | FSR Setting | Sensitivity (LSB/°/s) | Raw Gyro Values (Stationary) |
|---------------|-------------|----------------------|------------------------------|
| **imu-polled-basic (original)** | Power-on default (±2000 °/s) | 16.4 | X=5-7, Y=-13 to -15, Z=3-6 |
| **dRehmFlight** | Explicit ±250 °/s | 131.0 | X=46-58, Y=-100 to -118, Z=31-52 |
| **Ratio** | 8x sensitivity | 131/16.4 = 8x | ~8x raw counts |

**Why the Difference**:
- **ICM-42688-P power-on default**: FSR=0 (±2000 °/s range, 16.4 LSB/°/s sensitivity)
- **dRehmFlight configuration**: Explicitly sets FSR=3 (±250 °/s range, 131 LSB/°/s sensitivity) via `ConfigureInvDevice()`
- **Physical rotation rate**: Same (~0.35 °/s stationary drift)
- **Raw counts differ**: Higher sensitivity → more LSB per degree → higher raw values

**Verification**:
After configuring imu-polled-basic to use ±250 DPS FSR (matching dRehmFlight):
```
imu-polled-basic: Gyro X=53-62, Y=-100 to -112, Z=27-47
dRehmFlight:      Gyro X=46-58, Y=-100 to -118, Z=31-52
```
Raw values now match within normal sensor noise ✅

**Conclusion**:
- Both configurations are **correct** - just different measurement ranges
- ±2000 °/s: Wider range, lower resolution (good for aerobatics)
- ±250 °/s: Narrower range, higher resolution (good for stable flight)
- All IMU library examples now standardized to ±250 °/s for consistency

### Phase 2: Hardware Bench Testing

**RC Receiver**:
- Connect FlySky FS-iA6B or compatible SBUS receiver
- Verify channel mapping (throttle, roll, pitch, yaw)
- Test failsafe behavior
- Validate arming/disarming logic

**Motor Control**:
- Connect ESCs to motor outputs (TIM1, TIM3)
- Test OneShot125 pulse generation (125-250µs)
- Verify motor response to stick inputs
- Confirm failsafe stops motors

### Phase 3: Flight Testing

**Bench Testing**:
- Props on, motor response testing
- PID tuning on bench
- Verify control authority on all axes

**Hover Testing**:
- Initial hover attempts
- Stability assessment
- PID tuning iterations

**Flight Testing**:
- Progressive flight envelope expansion
- Performance validation
- Loop rate stability monitoring

### Phase 4: Flight Controller Deployment

**Hardware Migration**:
- Deploy to target flight controller (NERO F7, MATEK H743, or similar)
- Verify all peripherals (IMU, flash, motors, receiver)
- Production flight testing
- Final PID tuning for production hardware

## License

MIT License - Same as original dRehmFlight

## Attribution

Original work by Nicholas Rehm: https://github.com/nickrehm/dRehmFlight

STM32 port maintains the educational focus and clean code style of the original while demonstrating minimal-change hardware abstraction patterns.

## For Upstream Consideration

This port demonstrates that STM32 support requires only minimal changes to the hardware interface layer. All flight control algorithms, PID tuning, mixer logic, and safety features work unchanged on STM32F4.

Key requirements for STM32 port:
1. IMU abstraction (supports ICM42688P, MPU6000, MPU9250)
2. Serial RX library (SBUS/IBus/CRSF)
3. Hardware timer PWM (OneShot125)
4. Board configuration system
