# BLACKPILL_F411CE Target Validation Test Plan

**Date**: 2025-01-04
**Target File**: `targets/BLACKPILL_F411CE.h`
**Hardware**: WeAct Studio MiniSTM32F4x1 (STM32F411CEU6)
**Status**: Ready for execution

---

## Overview

This test plan validates the updated BLACKPILL_F411CE.h target configuration through three progressive hardware tests:

1. **LED Blink Test** - Basic GPIO and target file validation
2. **SPI Flash Test** - SPI1 peripheral and onboard W25Q flash chip
3. **MPU-9250 Test** - SPI2 peripheral and external IMU module

---

## Hardware Requirements

### Available Hardware (Confirmed)
- ✅ BLACKPILL F411CE board with onboard LED (PC13)
- ✅ Onboard SPI flash chip (W25Q64 or W25Q128)
- ✅ External MPU-9250 IMU module
- ✅ J-Link debugger (ST-Link reflashed to J-Link firmware)
- ✅ Jumper wires for MPU-9250 connection

### Software Requirements
- Arduino CLI 1.3.0
- STM32 Core 2.7.1
- J-Link tools installed
- RTT support enabled

---

## Target File Summary

### Updated Configuration (2025-01-04)
The BLACKPILL_F411CE.h file was updated to match NUCLEO_F411RE_JHEF411.h format:

**Key Changes**:
- ✅ Added missing IMUConfig frequency parameter (critical fix)
- ✅ Converted to typed configs (ADCConfig, LEDConfig)
- ✅ Standardized motor struct parameter order: `{pin, ch, min, max}`
- ✅ Updated data types to uint32_t
- ✅ Reduced SPI frequencies to 1 MHz (jumper wire safe)
- ✅ Removed redundant UART namespace
- ✅ Added comprehensive documentation header

### Pin Assignments
```cpp
// LED
LEDConfig status_leds{PC13};  // Onboard blue LED

// Storage (SPI1)
StorageConfig storage{
  LITTLEFS, PA7, PA6, PA5, PA15, 1000000  // 1 MHz
};

// IMU (SPI2)
SPIConfig imu_spi{PB15, PB14, PB13, PB12, 1000000};  // 1 MHz
IMUConfig imu{imu_spi, PB2, 1000000};  // PB2 = interrupt

// ADC
ADCConfig battery{PA1, PA4, 110, 170};

// GPS (UART1)
UARTConfig gps{PA9, PA10, 115200};

// RC Receiver (UART2)
RCReceiverConfig rc_receiver{PA3, PB10, 115200, 1000, 300};

// I2C Sensors
I2CConfig sensors{PB9, PB8, 400000};

// Motors (8 channels)
// TIM3: PB4, PB5, PB0_ALT1, PB1_ALT1
// TIM4: PB6, PB7
// TIM2: PA2, PA0
```

---

## Test Phase 1: LED Blink (1 Hz)

### Objective
Verify PC13 LED configuration and basic GPIO functionality.

### Test Sketch
**Approach**: Adapt Arduino Blink example

**Source**: Use standard Arduino Blink example
**Modification**: Set to 1 Hz (1000ms on/off), add RTT logging

**Temporary Location**: `tests/BLACKPILL_LED_Test/`

### Build & Flash Commands
```bash
# Build
./system/ci/build.sh tests/BLACKPILL_LED_Test \
  STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE \
  --build-id

# Flash with RTT
./system/ci/aflash.sh tests/BLACKPILL_LED_Test \
  STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE \
  --use-rtt
```

### Expected Results
**Visual**:
- Onboard blue LED (PC13) blinks at 1 Hz
- LED is active LOW on BLACKPILL

**RTT Output**:
```
*READY*
LED ON
LED OFF
LED ON
LED OFF
...
(10 blinks total)
*STOP*
```

### Success Criteria
- ✅ LED visibly blinking at 1 Hz rate
- ✅ RTT output confirms correct timing
- ✅ No compilation errors
- ✅ Clean exit with *STOP*

### Troubleshooting
- **No LED activity**: Check PC13 definition in variant file
- **Wrong blink rate**: Verify LED_BUILTIN maps to PC13
- **Compilation fails**: Check FQBN is correct for BLACKPILL

---

## Test Phase 2: SPI Flash (ChipID Detection)

### Objective
Verify SPI1 configuration and onboard W25Q flash chip detection.

### Test Sketch
**Location**: `libraries/LittleFS/examples/LittleFS_ChipID/LittleFS_ChipID.ino`

**Features**:
- Auto-detects flash chip via JEDEC ID
- Reports manufacturer, capacity
- Already has BLACKPILL_F411CE BoardConfig support

### Hardware Details
**Onboard Flash Chip**: W25Q64 (8MB) or W25Q128 (16MB)
**SPI Pins**: PA7 (MOSI), PA6 (MISO), PA5 (SCLK), PA15 (CS)
**Frequency**: 1 MHz (reduced from 8 MHz for safety)

**Note**: Some BLACKPILL board versions route MISO differently (PA6 vs PB4). Current target file uses PA6 (most common).

### Build & Flash Commands
```bash
# Build
./system/ci/build.sh libraries/LittleFS/examples/LittleFS_ChipID \
  STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE \
  --build-id

# Flash with RTT
./system/ci/aflash.sh libraries/LittleFS/examples/LittleFS_ChipID \
  STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE \
  --use-rtt
```

### Expected Results
**For W25Q64** (8MB):
```
Manufacturer: EF (Winbond)
JEDEC ID: 4017
Capacity: 8388608 bytes (8 MB)
Flash chip detected successfully!
*STOP*
```

**For W25Q128** (16MB):
```
Manufacturer: EF (Winbond)
JEDEC ID: 4018
Capacity: 16777216 bytes (16 MB)
Flash chip detected successfully!
*STOP*
```

### Success Criteria
- ✅ JEDEC ID detected (0xEF40XX for Winbond)
- ✅ Capacity matches installed chip
- ✅ No SPI communication errors
- ✅ Clean chip detection and mount

### Troubleshooting
- **No chip detected**: Check MISO pin routing (PA6 vs PB4 board variant)
- **Wrong JEDEC ID**: Different flash chip installed (check bottom of PCB)
- **SPI timeout**: Check CS pin (PA15) and SPI frequency
- **Mount fails**: Flash chip may be damaged or not fully soldered

---

## Test Phase 3: MPU-9250 IMU (WHO_AM_I + Data)

### Objective
Verify SPI2 configuration and MPU-9250 communication.

### Hardware Setup
**External Connections Required**:
```
MPU-9250 Module → BLACKPILL F411CE
--------------------------------
VCC  → 3.3V (NOT 5V!)
GND  → GND
SCK  → PB13 (SPI2_SCK)
MISO → PB14 (SPI2_MISO)
MOSI → PB15 (SPI2_MOSI)
CS   → PB12 (GPIO chip select)
INT  → PB2  (Optional - for interrupt example)
```

**Critical**: Use 3.3V only! 5V will damage the MPU-9250 sensor.

### Test Sketch
**Location**: `libraries/MPU9250/examples/MPU9250_Basic/MPU9250_Basic.ino`

**Features**:
- WHO_AM_I register verification (0x71 or 0x73)
- Continuous gyro + accel data output
- DLPF configuration
- Already has BLACKPILL_F411CE BoardConfig support

### Build & Flash Commands
```bash
# Build
./system/ci/build.sh libraries/MPU9250/examples/MPU9250_Basic \
  STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE \
  --build-id

# Flash with RTT
./system/ci/aflash.sh libraries/MPU9250/examples/MPU9250_Basic \
  STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE \
  --use-rtt
```

### Expected Results
```
MPU9250 Initialization...
WHO_AM_I: 0x71
MPU9250 detected successfully!

Gyro (deg/s): X=0.12 Y=-0.45 Z=0.03
Accel (g):    X=0.01 Y=0.02 Z=1.00

Gyro (deg/s): X=0.09 Y=-0.42 Z=0.01
Accel (g):    X=0.00 Y=0.03 Z=0.99
...
*STOP*
```

### Success Criteria
- ✅ WHO_AM_I returns 0x71 (MPU-9250) or 0x73 (MPU-9255)
- ✅ Gyro values near zero when stationary (< ±2 deg/s)
- ✅ Accel Z-axis approximately 1.0g when board flat
- ✅ Accel X/Y axes near 0g when board flat
- ✅ Data updates continuously without SPI errors

### Success Ranges
**Gyro (stationary board)**:
- X, Y, Z: -2.0 to +2.0 deg/s (typical: ±0.5 deg/s)

**Accel (board flat on table)**:
- X: -0.1 to +0.1 g
- Y: -0.1 to +0.1 g
- Z: +0.9 to +1.1 g (gravity)

### Troubleshooting
- **WHO_AM_I fails**: Check wiring, especially CS (PB12) and MISO (PB14)
- **Wrong WHO_AM_I**: Different IMU chip (0x68 = MPU-6000, 0x70 = MPU-6500)
- **SPI timeout**: Check SCK (PB13) and MOSI (PB15) connections
- **Data all zeros**: Sensor not powered or damaged
- **Accel Z not 1.0g**: Board not flat, or sensor orientation different

**Note**: Target file comment incorrectly says "SPI1" but pins are actually SPI2 (PB13/14/15). This is correct behavior - comment needs fixing in future.

---

## Pre-Test Checklist

### Before Starting Tests
1. ✅ Commit BLACKPILL_F411CE.h changes to git
2. ✅ Verify J-Link connection to BLACKPILL
3. ✅ Run device detection: `./system/ci/detect_device.sh`
   - Expected: "STM32F411xC/E" (Device ID: 0x431)
4. ✅ Verify FQBN available:
   ```bash
   arduino-cli board listall | grep BLACKPILL
   ```
   - Expected: "BlackPill F411CE"

### FQBN Verification
```bash
STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE
```

### Environment Check
```bash
./system/ci/env_check_quick.sh
```
Expected output:
- Arduino CLI: 1.3.0
- STM32 Core: 2.7.1

---

## Test Execution Order

### Sequential Execution (Recommended)
Execute tests in order due to dependencies:

1. **LED Test** (5 minutes)
   - Validates basic build system and GPIO
   - No external hardware needed
   - Quick visual confirmation

2. **SPI Flash Test** (10 minutes)
   - Validates SPI1 peripheral
   - Uses onboard hardware only
   - Confirms flash chip detection

3. **MPU-9250 Test** (20 minutes)
   - Validates SPI2 peripheral
   - Requires external wiring
   - Most complex test

**Total Estimated Time**: 35-45 minutes

---

## Safety Notes

### Hardware Safety
- ✅ All SPI frequencies reduced to 1 MHz (safe for jumper wires)
- ✅ PC13 LED is onboard (no external wiring needed)
- ⚠️ **Use 3.3V for MPU-9250** (NOT 5V - will damage sensor)
- ✅ J-Link connection validated before flashing

### Software Safety
- ✅ All test sketches include *STOP* exit wildcard
- ✅ RTT logging enabled for remote monitoring
- ✅ Build traceability with --build-id flag
- ✅ Environment validation before builds

---

## Known Issues & Notes

### Target File Issues (Non-Critical)
1. **Comment mismatch**: IMU config comment says "SPI1" but pins are SPI2 (PB13/14/15)
   - **Status**: Correct behavior, comment needs updating
   - **Impact**: None (code is correct)

2. **MISO routing variance**: Different BLACKPILL board versions route flash MISO differently
   - **V2.0**: MISO on PB4
   - **V2.1+**: MISO on PA6 (current target config)
   - **Impact**: If flash test fails, check board version and update target file

### Pin Collision Status
All pin collisions from previous analysis have been resolved:
- ✅ PA4: Storage CS moved to PA15 (ADC conflict fixed)
- ✅ PB6/PB7: I2C moved to PB9/PB8 (motor conflict fixed)
- ✅ UART duplicates removed (namespace cleanup)

---

## Deliverables

### After Completing All Tests
1. **Test Log**: RTT output from each test phase
2. **Hardware Confirmation**:
   - Flash chip model and capacity verified
   - IMU model (MPU-9250/9255) and WHO_AM_I confirmed
   - Board version noted (for MISO routing reference)
3. **Issue Report**: Any failures or unexpected behavior
4. **Target File Validation**: Confirm target file works as designed

### Documentation Updates
- Update BLACKPILL_F411CE.h comment (SPI1→SPI2 for IMU)
- Document board version compatibility (if MISO routing differs)
- Add test results to CLAUDE.md or similar documentation

---

## Rollback Plan

### If LED Test Fails
**Symptoms**: No LED activity or wrong blink rate
**Root Cause**: Target file LED config incorrect
**Action**:
1. Verify PC13 definition in variant file
2. Check `LEDConfig status_leds{PC13}` in target file
3. Confirm LED_BUILTIN maps to PC13 for BLACKPILL

### If Flash Test Fails
**Symptoms**: No chip detected, wrong JEDEC ID, or SPI timeout
**Root Cause**: SPI1 pin mismatch or flash chip variant
**Action**:
1. Check BLACKPILL board version (V2.0 vs V2.1+)
2. Verify MISO routing: PA6 (V2.1+) or PB4 (V2.0)
3. Update target file if needed:
   ```cpp
   // For V2.0 boards:
   StorageConfig storage{LITTLEFS, PA7, PB4, PA5, PA15, 1000000};
   ```
4. Physically inspect flash chip on bottom of PCB (read part number)

### If IMU Test Fails
**Symptoms**: WHO_AM_I wrong or data errors
**Root Cause**: SPI2 pin mismatch, wiring error, or wrong IMU chip
**Action**:
1. Verify wiring: PB13 (SCK), PB14 (MISO), PB15 (MOSI), PB12 (CS)
2. Check 3.3V power supply to IMU
3. Verify IMU chip marking (MPU-9250 vs MPU-6000/6500)
4. If MPU-6000: Use `libraries/MPU6000/examples/MPU6000_Basic` instead
   - Expected WHO_AM_I: 0x68

---

## Success Criteria Summary

### Overall Validation Complete When:
- ✅ All 3 test phases pass
- ✅ No compilation errors
- ✅ No SPI communication failures
- ✅ All chip detection successful (flash + IMU)
- ✅ Visual and RTT output matches expectations

### Target File Validated:
- ✅ LED configuration correct (PC13)
- ✅ SPI1 configuration correct (storage pins)
- ✅ SPI2 configuration correct (IMU pins)
- ✅ All typed configs working (ADCConfig, LEDConfig, IMUConfig)
- ✅ Build system integration successful (FQBN correct)

---

## References

### File Locations
- **Target File**: `targets/BLACKPILL_F411CE.h`
- **Variant**: `variants/STM32F4xx/F411C(C-E)(U-Y)/variant_BLACKPILL_F411CE.h`
- **PeripheralPins**: `variants/STM32F4xx/F411C(C-E)(U-Y)/PeripheralPins_BLACKPILL_F411CE.c`
- **Boards.txt**: `boards.txt` (lines 568-575)

### Build Scripts
- **Build**: `./system/ci/build.sh`
- **Flash**: `./system/ci/aflash.sh`
- **Device Detect**: `./system/ci/detect_device.sh`
- **Env Check**: `./system/ci/env_check_quick.sh`

### Hardware Documentation
- **BLACKPILL Schematic**: https://github.com/WeActStudio/WeActStudio.MiniSTM32F4x1/blob/master/HDK/MiniF4x1Cx_V31.pdf
- **STM32F411 Datasheet**: STMicroelectronics DS9716
- **MPU-9250 Datasheet**: InvenSense/TDK PS-MPU-9250A-01

---

## Revision History

**2025-01-04**: Initial test plan created
- Hardware confirmed: Onboard flash, external MPU-9250
- Test approach: Use existing Arduino Blink example
- All 3 test phases defined
- Pre-test checklist completed
- Known issues documented
