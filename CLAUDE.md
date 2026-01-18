# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This repository contains the **STM32 Arduino Core** along with associated sketches and libraries. It provides Arduino IDE support for STM32 microcontrollers through the STM32Cube ecosystem.

This is a fork of the upstream [stm32duino/Arduino_Core_STM32](https://github.com/stm32duino/Arduino_Core_STM32) repository. This fork includes:
- **Simplified variant selection** - Reduced number of board variants for focused development
- **Added FS.h** - Generic embedded file system base class for storage access (unified interface for SPI flash and SD card storage)
- **Robotics libraries** - Complete suite of UAV flight controller libraries integrated directly into the core
- **HIL testing framework** - Production-grade CI/CD workflow with build traceability

**IMPORTANT - Development Environment Symlink:**
The installed Arduino STM32 core at `~/.arduino15/packages/STM32_Robotics/hardware/stm32/1.1.0` is a **symlink** to this repository (`/home/geo/Arduino/Arduino_Core_STM32`). This means:
- There is NO library conflict between "installed" and "local" libraries - they are the same files
- When debugging library issues, don't chase phantom "library priority" problems
- All paths resolving to `.arduino15/.../stm32/1.1.0/` actually point to this repo

To set up this symlink for development, see the workspace [README.md](../README.md#local-development-with-symlink).

### Repository Structure

- `cores/arduino/` - Core Arduino implementation for STM32
- `variants/` - Board-specific pin definitions and configurations
- `system/` - STM32Cube HAL drivers and CMSIS
  - `system/extras/` - Arduino build hooks (prebuild/postbuild)
- `libraries/` - Core STM32 + robotics libraries
  - Core: `SPI`, `Wire`, `SoftwareSerial`, `CMSIS_DSP`, `SEGGER_RTT`
  - Robotics: `LittleFS`, `SDFS`, `Storage`, `minIniStorage`, `ICM42688P`, `imu`, `TimerPWM`, `SerialRx`, `libPrintf`, `AUnit`, `STM32RTC`
- `cmake/` - CMake build system and examples
- `tests/` - Unit tests, integration tests, and flight controller applications
  - Flight controllers: `dRehmFlight_STM32_BETA_1.3` (4-motor quadcopter)
- `targets/` - Board configuration headers (BoardConfig system)
- `extras/` - Betaflight config converter and utilities
- `doc/` - Technical documentation

## Build Systems and Commands

**IMPORTANT - CI Scripts Are Required (Located at Workspace Level):**
- ✅ **ALWAYS use** `./ci/build.sh` and `./ci/aflash.sh` for building and testing (from workspace root)
- ✅ **ALWAYS use** `./ci/cleanup_repo.sh` before commits
- ❌ **DO NOT use** `arduino-cli compile` directly - use `build.sh` wrapper instead
- ❌ **DO NOT use** `arduino-cli upload` directly - use `aflash.sh` wrapper instead

**Note:** CI scripts are located at the workspace level (`Arduino/ci/`), not in this submodule. Run all CI commands from the workspace root (`Arduino/`).

**Why CI scripts are mandatory:**
1. Environment validation (Arduino CLI, STM32 Robotics Core)
2. Build traceability (Git SHA + UTC timestamp)
3. Deterministic builds (cache management)
4. HIL testing integration (RTT, exit wildcard detection)
5. Device auto-detection (50+ STM32 devices)

### CI Build Scripts (Primary Method)
Enhanced build workflow with environment validation and device auto-detection:

```bash
# Run from workspace root (Arduino/)
# Standard build and HIL testing (USE THESE)
./ci/build.sh Arduino_Core_STM32/<sketch_directory> [--build-id] [--env-check] [--use-rtt]
./ci/aflash.sh Arduino_Core_STM32/<sketch_directory> [--env-check] [--use-rtt] [--build-id]

# Environment and device utilities
./ci/env_check_quick.sh         # Fast environment validation
./ci/detect_device.sh           # Auto-detect STM32 via J-Link
./ci/flash_auto.sh <binary>     # Program with auto-detected device
./ci/cleanup_repo.sh            # Clean build artifacts before commit
```

**Key Features**:
- **Environment Validation**: Arduino CLI and STM32 Robotics Core validation
- **Build Traceability**: Git SHA + UTC timestamp integration
- **Device Auto-Detection**: 50+ STM32 device IDs supported
- **Cache Management**: `--clean-cache` for deterministic builds

### Arduino CLI (Reference Only - Use CI Scripts Instead)
The CI scripts wrap arduino-cli with additional validation and features:

```bash
# Install STM32 core
arduino-cli core update-index
arduino-cli core install STM32_Robotics:stm32

# Compile sketch
arduino-cli compile --fqbn STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE <sketch_directory>

# Upload to board
arduino-cli upload --fqbn STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE <sketch_directory>

# List available/connected boards
arduino-cli board listall
arduino-cli board list

# J-Link upload (when ST-Link reflashed to J-Link)
./ci/jlink_upload.sh <path_to_binary.bin>
```

### Makefile Support
Generic Makefile provided for cross-platform builds:

```bash
make                    # Compile with default FQBN
make upload            # Compile and upload to board
make clean             # Clean build artifacts
```

Default FQBN: `STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE`

### Build Scripts
Enhanced build workflow with environment validation and device auto-detection:

```bash
# Run from workspace root (Arduino/)
# Standard build and HIL testing
./ci/build.sh Arduino_Core_STM32/<sketch_directory> [--build-id] [--env-check] [--use-rtt]
./ci/aflash.sh Arduino_Core_STM32/<sketch_directory> [FQBN] [--env-check] [--use-rtt] [--build-id]

# Environment and device utilities
./ci/env_check_quick.sh         # Fast environment validation
./ci/detect_device.sh           # Auto-detect STM32 via J-Link
./ci/flash_auto.sh <binary>     # Program with auto-detected device
./ci/cleanup_repo.sh            # Clean build artifacts before commit
```

**Key Features**:
- **Environment Validation**: Arduino CLI and STM32 Robotics Core validation
- **Build Traceability**: Git SHA + UTC timestamp integration
- **Device Auto-Detection**: 50+ STM32 device IDs supported
- **Cache Management**: `--clean-cache` for deterministic builds

**CRITICAL - Sequential Hardware Testing**:
- **NEVER run `aflash.sh` commands in parallel** - hardware tests must execute sequentially
- Each `aflash.sh` call programs and runs on physical hardware (one device at a time)
- Multiple parallel `aflash.sh` calls will fail or produce undefined behavior
- Use `build.sh` in parallel for compilation-only tasks (no hardware required)

**FQBN Specification**:
- `aflash.sh` accepts optional FQBN as second positional argument (after sketch directory)
- Default FQBN: `STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE`
- **CRITICAL**: Match FQBN to connected hardware to ensure correct pin mappings and peripherals
- Examples (run from workspace root):
  ```bash
  # BLACKPILL_F411CE (when connected)
  ./ci/aflash.sh Arduino_Core_STM32/libraries/MPU9250/examples/MPU9250_Basic STM32_Robotics:stm32:GenF4:pnum=BLACKPILL_F411CE --use-rtt --build-id

  # NUCLEO_F411RE (default)
  ./ci/aflash.sh Arduino_Core_STM32/libraries/MPU9250/examples/MPU9250_Basic --use-rtt --build-id
  ```

### J-Link and RTT Utilities

```bash
# J-Link utilities
./ci/jrun.sh <elf> [timeout] [exit_wildcard] # J-Run execution with RTT
./ci/flash.sh [--quick] <binary>             # Flash with fixed device

# Manual RTT Connection
JLinkGDBServer -Device STM32F411RE -If SWD -Speed 4000 -RTTTelnetPort 19021 &
JLinkRTTClient                                     # Connect to RTT
```

**Running Multiple Sequential Tests**:
When running multiple HIL tests in a loop (e.g., for consistency validation), add a delay between runs to allow the hardware to fully reset:
```bash
# Good: Add 0.5s delay between tests (run from workspace root)
for i in 1 2 3 4 5; do
    echo "=== Test $i/5 ==="
    ./ci/aflash.sh Arduino_Core_STM32/tests/MyTest --use-rtt --build-id 2>&1 | grep -E "(PASS|FAIL)"
    sleep 0.5
done

# Bad: No delay - may cause intermittent failures due to hardware state
for i in 1 2 3 4 5; do ./ci/aflash.sh Arduino_Core_STM32/tests/MyTest --use-rtt --build-id; done
```

### CMake Build System

```bash
# Prerequisites: CMake >=3.21, Python3 >=3.9
pip install cmake graphviz jinja2

# Setup and build
cmake/scripts/cmake_easy_setup.py -b <board> -s <sketch_folder>
cmake -S <sketch_folder> -B <build_folder> -G Ninja
cmake --build <build_folder>
```

## Architecture Overview

### Core Components

**Arduino Core** (`cores/arduino/`):
- `main.cpp` - Standard Arduino main loop with STM32 initialization
- `Arduino.h` - Main Arduino header with STM32-specific extensions
- HAL integration through `stm32/` subdirectory
- Hardware abstraction for STM32Cube HAL and LL APIs

**Board Variants** (`variants/`):
- Organized by STM32 family (F4xx, F7xx, G4xx, H7xx, etc.)
- Each variant contains:
  - `PeripheralPins.c` - Pin mapping to STM32 peripherals
  - `variant_*.h/.cpp` - Board-specific pin definitions
  - `boards_entry.txt` - Board configuration for boards.txt

**Build Configuration**:
- `boards.txt` - Arduino IDE board definitions and menus
- `platform.txt` - Toolchain and compiler settings
- Board selection uses FQBN format: `STM32_Robotics:stm32:<board_group>:pnum=<specific_board>`

## Target Hardware and Applications

### Primary Development Boards
- **Nucleo F411RE** (Primary): `STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE`
  - **HIL Setup**: On-board ST-Link V2.1 reflashed to J-Link firmware
  - **Serial Monitor**: Available via J-Link connection (connected CDC ACM serial monitor)
  - **Programming**: J-Run execution via reflashed J-Link interface
- **BlackPill F411CE** (Secondary): `STM32_Robotics:stm32:GenF4:pnum=BLACKPILL_F411CE`
  - **CRITICAL**: Must specify FQBN when using `aflash.sh` to ensure correct BoardConfig pin mappings
  - **Example**: `./ci/aflash.sh Arduino_Core_STM32/<sketch> STM32_Robotics:stm32:GenF4:pnum=BLACKPILL_F411CE --use-rtt`
- **NERO F7 Flight Controller**: `STM32_Robotics:stm32:FlightCtr:pnum=BKMN_NERO`
  - **MCU**: STM32F722RET6
  - **IMU**: ICM-20602 on SPI1 (PA7/PA6/PA5/PC4)
  - **Variant**: `variant_BKMN_NERO.h` in `variants/STM32F7xx/F722R(C-E)T_F730R8T_F732RET/`
  - **BoardConfig**: Available at `targets/BKMN-NERO.h`
  - **Example**: `./ci/aflash.sh Arduino_Core_STM32/<sketch> STM32_Robotics:stm32:FlightCtr:pnum=BKMN_NERO --use-rtt`
- **MATEK H743-WLITE**: `STM32_Robotics:stm32:FlightCtr:pnum=MATEK_H743VI`
  - **MCU**: STM32H743VIT6
  - **IMU**: ICM42688P on SPI1
  - **BoardConfig**: Available at `targets/MTKS-MATEKH743.h`
  - **Example**: `./ci/aflash.sh Arduino_Core_STM32/<sketch> STM32_Robotics:stm32:FlightCtr:pnum=MATEK_H743VI --use-rtt`

**Important**: Examples using BoardConfig system (e.g., MPU9250, LittleFS, ICM206xx) auto-detect board via `ARDUINO_*` defines. The FQBN must match the connected hardware to ensure correct pin assignments and peripheral configurations.

### Target Hardware Platforms
This repository supports **UAV flight controller boards** with the following STM32 microcontrollers:

- **STM32F411** - Primary target (Nucleo F411RE, BlackPill F411CE for development)
- **STM32F405** - Secondary target (common in flight controllers)
- **STM32F722** - Validated target (NERO F7 flight controller with ICM-20602)
- **STM32H743** - Validated target (MATEK H743-WLITE flight controller with ICM42688P)

### Example Target Applications
- **UAV Flight Controllers** - Autonomous drone flight control systems
  - `dRehmFlight_STM32_BETA_1.3` - 4-motor conventional quadcopter (port from Teensy BETA 1.3)
- **Embedded Storage Systems** - SPI flash (LittleFS) and SD card (SDFS) file systems
- **Real-time Data Logging** - Flight data, telemetry, and configuration storage
- **Sensor Data Management** - IMU, GPS, and other sensor data processing

### Key Libraries

**Core Libraries**:
- `Wire` - I2C communication
- `SPI` - SPI communication  
- `SoftwareSerial` - Software UART implementation
- `CMSIS_DSP` - ARM CMSIS DSP functions
- `SEGGER_RTT` - Segger RTT library

**STM32-Specific Libraries**:
- `STM32RTC` - Real-time clock functionality
- `SerialRx` - RC receiver protocol parser (IBus ✅, SBUS ✅, CRSF 📋) with BoardConfig integration and software idle detection
- `LittleFS` - SPI flash filesystem with wear leveling
- `SDFS` - SD card filesystem via SPI with FatFs backend
- `Storage` - Generic storage abstraction for LittleFS and SDFS
- `minIniStorage` - INI configuration management with automatic storage backend selection
- `ICM42688P` - 6-axis IMU library with TDK InvenSense drivers and self-test
- `IMU` - High-level C++ IMU wrapper with chip detection, multi-instance support, and 9-DOF magnetometer support (MPU-9250/9255)
- `TimerPWM` - Hardware timer PWM for servo/ESC control with 1µs resolution
- `libPrintf` - Embedded printf library (eyalroz/printf v6.2.0) - 20% binary reduction
- `AUnit` - Unit testing framework (v1.7.1) with RTT integration

### Hardware Abstraction

The core integrates three levels of STM32 APIs:
1. **HAL (Hardware Abstraction Layer)** - High-level, feature-rich APIs
2. **LL (Low Layer)** - Optimized, register-level APIs
3. **CMSIS** - ARM Cortex standard interface

Board-specific configurations are defined through the variant system, allowing the same core to support hundreds of STM32 boards with different pin layouts and capabilities.

## Embedded Hardware Validation Standards

**CRITICAL DISTINCTION**: Embedded systems require physical signal validation, not just software execution. This is fundamentally different from web/software development where console output often constitutes validation.

### Validation Levels

**1. Software Validation** (Necessary but NOT Sufficient):
- ✅ Code compiles without errors
- ✅ Code runs on target hardware without crashes
- ✅ RTT/Serial output shows expected calculated values
- ✅ **Proves**: Firmware executes correctly
- ❌ **Does NOT prove**: Hardware peripherals are configured/operating correctly
- ❌ **Does NOT prove**: Physical signals are being generated

**2. Hardware Validation** (Required for "Hardware Validated" Claims):
- ✅ **Measurement required**: Oscilloscope, logic analyzer, or input capture
- ✅ **Specifications verified**: Frequency, pulse width, voltage levels within tolerance
- ✅ **Physical signals confirmed**: Actual pin outputs measured, not just calculated
- ✅ **Example**: PWM_Verification uses TIM2 input capture to measure TIM3 output (49.50 Hz ±2%)

### When to Claim "Hardware Validated"

**Acceptable Claims**:
- ✅ "Hardware validated" - Only when physical signals measured and verified against specs
- ✅ "Measured with oscilloscope: 49.50 Hz ±2%" - Specific measurement data
- ✅ "Input capture verified: 1500 µs pulse width" - Measurement methodology stated
- ✅ "Software tested on NUCLEO_F411RE" - Honest about validation level

**Unacceptable Claims**:
- ❌ "Hardware validated" when only software executed successfully
- ❌ "Validated on hardware" when only RTT output shows expected values
- ❌ "Hardware tested" when code just runs without crashing
- ❌ Any validation claim without actual signal measurement

### Validation Test Requirements

Every hardware peripheral library must include measurement-based validation:

**Required Elements**:
1. **Measurement Methodology**:
   - Tool used (oscilloscope, logic analyzer, input capture timer)
   - Probe points or jumper connections
   - Measurement procedure

2. **Tolerance Specifications**:
   - Frequency: ±X% or ±X Hz
   - Pulse width: ±X µs
   - Voltage levels: Min/Max thresholds
   - Timing accuracy requirements

3. **Pass/Fail Criteria**:
   - Based on measured values, not calculated values
   - Defined tolerance bands
   - Clear success/failure determination

4. **Hardware Setup**:
   - Physical connections required (jumpers, probes)
   - Test equipment needed
   - Board configuration

**Example - Good Validation Test** (PWM_Verification):
```cpp
// Hardware: TIM2 input capture measures TIM3 PWM output
// Setup: Jumper wire D5 (TIM3_CH1) → A0 (TIM2_CH1)
// Measurement: Input capture reads actual period
// Specification: 50 Hz ±2% (49-51 Hz acceptable)
// Result: 49.50 Hz measured ✅ PASS
```

**Example - Insufficient "Validation"**:
```cpp
// ❌ BAD: No measurement
CI_LOGF("PWM set to 50 Hz\n");  // This only shows software calculated a value
// No actual signal measurement = NOT hardware validated
```

### Common Pitfalls to Avoid

1. **Confusing "runs on hardware" with "hardware validated"**:
   - Running on target ≠ Peripherals working correctly
   - RTT output ≠ Physical signal measurement

2. **Trusting register writes without verification**:
   - Setting TIM3->ARR doesn't prove timer is counting
   - Configuring GPIO doesn't prove signal is toggling

3. **Assuming correct behavior without measurement**:
   - "No errors" ≠ "Working correctly"
   - Silent failure modes exist in embedded systems

### Integration with TODO Workflow

When creating validation todos, be explicit:
- ✅ "Measure TIM3 frequency with input capture"
- ✅ "Verify ESC pulse widths with oscilloscope"
- ❌ "Run hardware validation" (too ambiguous)
- ❌ "Test on hardware" (doesn't specify measurement)

## Completed Projects

### Build Workflow ✅ **COMPLETED**

HIL testing framework with complete build-to-runtime traceability and device auto-detection.

**Key Features**:
- **Deterministic HIL Testing**: Exit wildcard methodology with "*STOP*" detection
- **Build Traceability**: Git commit SHA + UTC timestamp integration
- **Universal Device Support**: Auto-detection across 50+ STM32 device IDs
- **One-Command Workflow**: Complete build+test automation with environment validation

**Production Usage** (from workspace root):
```bash
./ci/build.sh Arduino_Core_STM32/tests/HIL_RTT_Test --build-id --env-check
./ci/aflash.sh Arduino_Core_STM32/tests/HIL_RTT_Test
```

### Unified Development Framework ✅ **COMPLETED**

Single-sketch development supporting both Arduino IDE and CI/HIL workflows via `ci_log.h` abstraction.

**Key Features**:
- **Dual-Mode Support**: Same sketch works with Serial (IDE) or RTT (HIL)
- **Build Integration**: `--use-rtt` and `--build-id` flag support
- **Single Codebase**: Eliminates duplicate test files
- **Clean Include Paths**: Arduino core integration for system-wide availability

**Integration Pattern**:
```cpp
#include <ci_log.h>
void setup() {
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif
  CI_LOG("Test starting\n");
  CI_BUILD_INFO();    // RTT: shows build details, Serial: no-op
  CI_READY_TOKEN();   // RTT: shows ready token, Serial: no-op
}
```

### Storage Libraries ✅ **COMPLETED**

**SDFS**: SPI-based SD card filesystem with FatFs backend, LittleFS-compatible API
- Complete File I/O, directory operations, runtime card detection
- Usage: `SDFS_SPI sdfs; sdfs.begin(CS_PIN); File f = sdfs.open("/log.txt", FILE_WRITE);`

**LittleFS**: SPI flash filesystem with 3 examples (ListFiles, ChipID, Usage)
- HIL integrated, 20+ chip support, 8 AUnit tests

**AUnit Integration**: v1.7.1 testing framework with `aunit_hil.h` wrapper
- 18 total tests (LittleFS: 8, SDFS: 7, framework: 3)
- Usage: `./ci/aflash.sh Arduino_Core_STM32/tests/LittleFS_IT --use-rtt --build-id`

### Board Configuration System ✅ **COMPLETED**

Compile-time board config with multi-board support (NUCLEO_F411RE, BLACKPILL_F411CE, NOXE_V3)
- Config types: StorageConfig, IMUConfig, RCReceiverConfig, UARTConfig, I2CConfig, ADCConfig, LEDConfig
- CS Mode control: Software/hardware via get_ssel_pin() helper
- Usage: `#include "targets/NUCLEO_F411RE_HIL001.h"` → `BoardConfig::storage.mosi_pin`
- **Hardware Validated**: BLACKPILL_F411CE fully validated (LED, SPI flash W25Q64, LittleFS filesystem) - see BLACKPILL_TEST_PLAN.md

### Storage Abstraction & Config ✅ **COMPLETED**

**Storage**: Unified interface for SDFS/LittleFS with automatic backend selection
- Usage: `Storage& fs = BOARD_STORAGE; fs.open("/file.txt", FILE_WRITE);`

**minIni**: INI config (v1.5) with Storage integration, 6 test suites
- Usage: `minIniStorage config("config.ini"); config.begin(BoardConfig::storage); config.put("key", "val");`

**libPrintf**: eyalroz/printf v6.2.0, ~20% binary reduction (8KB+ savings)
- Usage: `#include <libPrintf.h>` → `printf("Pi = %.6f\n", 3.14159);`
- **CRITICAL - Float Formatting on ARM Cortex-M**: Newlib Nano (default C runtime) does NOT support `%f` in sprintf/printf - calls produce empty output. minIniStorage uses libPrintf's `sprintf_()` in `StorageGlue.h` for float config storage. Any code needing float-to-string formatting must `#include <libPrintf.h>` and use `sprintf_()` directly, or standard `sprintf()` (aliased when libPrintf.h included). HIL validated: float values round-trip correctly through minIni on both LittleFS and SDFS.

### IMU Libraries ✅ **COMPLETED**

**ICM42688P**: Arduino-compatible library adapted from UVOS, 100% InvenSense factory algorithms preserved
- 4 examples: Simple (WHO_AM_I), self-test, interrupt-driven, processed AG data
- Usage: `ICM42688P_Simple imu; imu.begin(spi, PA4, 1000000);  // Returns 0x47`

**IMU (High-Level Wrapper)**: Unified C++ wrapper for InvenSense IMUs with chip detection and 9-DOF support
- Multi-instance support, ChipType enum (ICM42688_P, MPU-6000, MPU-9250, ICM-206xx)
- **6-DOF API**: Init, ReadIMU6, FSR/ODR/power config, preset API
- **9-DOF API**: InitMagnetometer, ReadIMU9, ReadMagnetometer, CalibrateMagnetometer (MPU-9250/9255 only)
- **Magnetometer**: AK8963 support via I2C master mode, figure-8 calibration, hard/soft iron correction
- **Hardware Validated**: MPU-9250 on BLACKPILL_F411CE - detection (0x71), AK8963 init (0x48), 9-DOF streaming, calibration (±1.6 µT centered)
- Usage:
  - `IMU imu; imu.Init(spi_bus, cs_pin, freq);`
  - `if (imu.HasMagnetometer()) { imu.InitMagnetometer(); imu.ReadIMU9(gyro, accel, mag); }`
- Supported: ICM-42688-P (6-DOF), MPU-6000 (6-DOF), MPU-9250/9255 (9-DOF), ICM-206xx (6-DOF)

### TimerPWM Library ✅ **COMPLETED**

Hardware timer PWM for 1µs resolution servo/ESC control
- Features: Explicit timer banks (prevents conflicts), BoardConfig integration, Arduino Servo API compatible
- Multi-channel (4/timer), dual timer support (servos + ESCs simultaneously)
- **Critical**: PWM requires both `resumeChannel(channel)` (CCxE bit) and `resume()` (counter start) per AN4013 §2.5
- Hardware validated: 49.50 Hz @ ±2%, 990.10 Hz @ ±4% with TIM2 input capture
- Examples: PWM_Verification, Servo_Verification, DualTimerPWM_Verification
- Docs: `libraries/TimerPWM/APPROACH.md`, `doc/TIMERS.md`, `doc/TIMERS_PWM_OUT.md`

### Betaflight Config Converter ✅ **COMPLETED**

Python tool (`extras/betaflight_converter/`) converting Betaflight configs to BoardConfig headers with validation
- Parser→Validator→Generator pipeline, 53 passing tests (`cd extras/betaflight_converter && pytest -v`)
- PeripheralPins.c validation, ALT variant handling (PB0_ALT1 for timer/AF conflicts), motor timer grouping
- MCUs: STM32F411/F405/F745/H743
- Usage: `python3 convert.py data/MTKS-MATEKH743.config` → `output/MTKS-MATEKH743.h`
- Generates: StorageConfig, IMUConfig, I2CConfig, UARTConfig, ADCConfig, LEDConfig, Servo/Motor namespaces
- Validated: JHEF-JHEF411 (NOXE V3), MTKS-MATEKH743 (H743-WLITE)
- **CS Mode**: Software chip select (default) for max library compatibility

### SerialRx Library ✅ **COMPLETED**

RC receiver protocol parser with BoardConfig integration, software idle detection (300µs), failsafe
- Protocols: IBus (hardware validated, FlySky FS-iA6B, 501/501 frames 0% loss), SBUS (implemented), CRSF (framework ready)
- Usage: `SerialRx rc; config.rx_protocol = SerialRx::IBUS; config.baudrate = BoardConfig::rc_receiver.baud_rate; rc.begin(config);`
- Examples: IBus_Basic, IBus_Loopback_Test, SBUS_Basic
- Docs: `libraries/SerialRx/README.md`, `doc/SERIAL.md`

## Projects In Progress

### dRehmFlight STM32 Port ✅ **PORT COMPLETE** (Pending Bench Validation)

Minimal-change port of dRehmFlight BETA 1.3 (Teensy-based UAV flight controller) to STM32F4/H7, targeting 4-motor conventional quadcopter.

**Target Hardware**:
- Development: NUCLEO_F411RE with breadboard ICM42688P + SBUS receiver
- Deployment: NOXE V3 (STM32F411), MATEK H743-WLITE (STM32H743)

**Port Strategy**: Minimal changes - hardware interface only, preserve all flight control logic
- **Libraries Replaced**: MPU6050/MPU9250 → IMU, PWM/PPM/DSM → SerialRx, bit-bang PWM → TimerPWM
- **Hardware Abstraction**: BoardConfig system (NUCLEO_F411RE_JHEF411.h)
- **Application Focus**: 4-motor quadcopter (servos commented out)

**Key Metrics**:
- Binary Size: 47.5KB (9% of 512KB flash)
- RAM Usage: 3.3KB (2% of 128KB RAM)
- Line Count: 1933 → 1640 lines (-15% reduction)
- Flight Control Logic Modified: 0 functions (100% preserved)
- Hardware Interface Modified: 6 functions (IMUinit, getIMUdata, radioSetup, updateRadioChannels, commandMotors, setup)

**Changes** (6 functions, 100% flight control preserved):
1. IMU: MPU6050/9250 → ICM42688P via IMU library (±250 DPS, ±2G, 2kHz ODR, polling-based)
2. Radio RX: PWM/PPM/DSM → SerialRx (SBUS, adapter pattern, eliminated 110 lines)
3. Motor: Bit-bang → TimerPWM OneShot125 (TIM1/TIM3, 125-250µs)
4. Pins: BoardConfig system (4 targets: NUCLEO_F411RE, BLACKPILL_F411CE, BKMN_NERO, MATEK_H743VI)
5. Build: `./ci/aflash.sh Arduino_Core_STM32/sketches/dRehmFlight_STM32_BETA_1.3 --use-rtt --build-id`

**Port Status** ✅:
- ✅ Compiles for all 4 targets (47.5KB binary on F411)
- ✅ All flight control logic preserved (100% unchanged)
- ✅ Minimal changes achieved (only 6 hardware functions modified)
- ✅ **IMU Hardware Validated**: WHO_AM_I verified (0x47), self-test passed, gyro data operational
- ✅ **IMU Data Validated**: Stationary drift readings confirmed (X≈0.38, Y≈-0.81, Z≈0.30 °/s)
- ✅ **Polling-Based IMU**: 2kHz loop matches 2kHz IMU ODR (same approach as Betaflight/iNav)
- ✅ **IMU Filters**: AAF configured (Gyro 258 Hz, Accel 170 Hz) - Betaflight-standard settings
- ✅ **Setup() Execution**: All initialization complete (IMU, filters, radio RX, motor timers)
- ✅ **Main Loop Running**: 2kHz loop timing operational with RTT/Serial logging

**Remaining Validation** 📋:
- 📋 RC receiver bench testing (SBUS on USART1) - validate SerialRx integration
- 📋 Motor control bench testing (OneShot125 via TIM1/TIM3) - validate TimerPWM output
- 📋 Flight testing with PID tuning - tune for specific airframe
- 📋 Deployment to target flight controller hardware

**Issues Resolved**:
1. **UART Conflict**: Fixed by moving RC receiver to USART1 (PB7/PB6), Serial debug on USART2 (PA2/PA3)
2. **Uninitialized Callbacks**: Added NULL initialization in HardwareSerial::init() with NULL checks in ISRs
3. **RTT Logging**: Fixed with cache clear + CI_LOG_FLOAT() for float formatting
4. **IMU Sensor Enable**: Fixed by adding EnableAccelLNMode() and EnableGyroLNMode() after ConfigureInvDevice()
   - ConfigureInvDevice() sets registers but doesn't start continuous sampling
   - Sensors were stuck in power-off state returning saturated values (-32768)
   - Fix enables continuous 2kHz data acquisition for polling-based flight loop

**Key Learning - IMU Full-Scale Range Configuration**:
Investigation revealed ~8× raw value discrepancy between dRehmFlight and imu-polled-basic example on identical hardware (both stationary, same board). Root cause: **different gyroscope FSR settings**, not a bug.

| Configuration | FSR Setting | Sensitivity (LSB/°/s) | Raw Gyro (Stationary) |
|---------------|-------------|----------------------|----------------------|
| imu-polled-basic (original) | Power-on default (±2000 °/s) | 16.4 | X=5-7, Y=-13~-15, Z=3-6 |
| dRehmFlight | Explicit ±250 °/s | 131.0 | X=46-58, Y=-100~-118, Z=31-52 |
| **Ratio** | 8× sensitivity | 131/16.4 = 8× | ~8× raw counts |

**Conclusion**: Both correct - different measurement ranges. After standardizing to ±250 °/s, values match within sensor noise. Flight controllers typically use ±250 °/s or ±500 °/s for stable flight (higher resolution). All IMU library examples now standardized to ±250 °/s.

**Original Source**: Nicholas Rehm's dRehmFlight Teensy BETA 1.3 (MIT License)
- Original: https://github.com/nickrehm/dRehmFlight
- Teensy version: `libraries/dRehmFlight/Versions/dRehmFlight_Teensy_BETA_1.3/`

## Future Projects

### New Variant Validation 📋 **FUTURE PROJECT**

Establish automated validation methodology for new STM32 board variants in custom Arduino core fork.

**Planned Features**:
- **Automated Test Suite**: Core clock accuracy measurement and validation
- **Serial Communication**: Validation patterns for `Serial.print()` functionality
- **HIL Integration**: Integration with existing framework for deterministic testing
- **Multi-Family Support**: STM32F4xx, F7xx, H7xx variant validation

# important-instruction-reminders
Do what has been asked; nothing more, nothing less.
NEVER create files unless they're absolutely necessary for achieving your goal.
ALWAYS prefer editing an existing file to creating a new one.
NEVER proactively create documentation files (*.md) or README files. Only create documentation files if explicitly requested by the User.
AVOID documentation duplication across files. Before adding content, check if it's already documented elsewhere in the project. Reference existing documentation rather than repeating content (e.g., STM32 processor targets, build system details, and CI/HIL workflows are covered in main project documentation).

## Claude Code Collaboration Notes

**Repository Attribution**: This repository's collaborative development with Claude Code is documented in README.md under the Documentation section.

### ci_log.h API Reference

**CRITICAL**: Always use the correct ci_log.h macros. Common mistakes to avoid:
- ❌ `CI_LOG_INIT()` does NOT exist - there is no init macro
- ❌ `CI_LOG()` does NOT support printf formatting - use `CI_LOGF()` instead
- ❌ **NEVER** use direct `printf()` or `Serial.print()` - use `CI_PRINTF()` or `CI_LOG()` instead
- ✅ **YOU MUST** call `Serial.begin()` when NOT using RTT mode - ci_log.h does NOT initialize Serial

**Available Macros** (`Arduino_Core_STM32/cores/arduino/ci_log.h`):
```cpp
CI_LOG(s)              // String literals only (no printf formatting)
CI_LOGF(...)           // Printf-style formatting (RTT: SEGGER_RTT_printf, Serial: Serial.printf)
                       // ⚠️ RTT LIMITATION: %f float formatting NOT supported in RTT mode
CI_PRINTF(...)         // Full printf with float support via libPrintf
                       // ⚠️ REQUIRES: #include <libPrintf.h> AND putchar_() implementation
                       // ✅ RECOMMENDED for formatted output including floats
CI_BUILD_INFO()        // Shows build SHA + timestamp (RTT only, no-op in Serial mode)
CI_READY_TOKEN()       // Shows ready token (RTT only, no-op in Serial mode)
CI_LOG_FLOAT(prefix, value, decimals)  // Float output helper (works in both modes)
```

**Correct Usage Pattern**:
```cpp
#include <ci_log.h>
#include <libPrintf.h>  // REQUIRED for CI_PRINTF - links the printf_ library

// CI_PRINTF requires putchar_() for libPrintf output routing
extern "C" void putchar_(char c) {
#ifdef USE_RTT
    SEGGER_RTT_PutChar(0, c);
#else
    Serial.write(c);
#endif
}

void setup() {
  // Initialize Serial for non-RTT mode (Arduino IDE)
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait for Serial with timeout
#endif

  CI_LOG("Starting test\n");           // String literal
  CI_LOGF("Value: %d\n", 123);         // Printf formatting (integers OK in RTT)

  // Float formatting - use CI_PRINTF for full printf support including floats
  float temp = 23.456;
  CI_PRINTF("Temp: %.2f\n", temp);     // ✅ Works in both RTT and Serial modes

  CI_BUILD_INFO();                     // Build traceability (RTT only)
  CI_READY_TOKEN();                    // Ready signal (RTT only)
}

void loop() {
  // ... test execution ...

  // CRITICAL: Always end HIL tests with *STOP* exit wildcard
  CI_LOG("*STOP*\n");  // Required for aflash.sh exit wildcard detection
  while(1);            // Halt after test completion
}
```

**Exit Wildcard Requirements for HIL Testing**:
- ✅ **ALWAYS** end tests with `CI_LOG("*STOP*\n")` before halting
- ✅ aflash.sh requires `*STOP*` for deterministic test completion
- ❌ Without `*STOP*`, aflash.sh will timeout after 60 seconds
- 💡 Can include test status before `*STOP*` (e.g., `*TEST_PASS*` then `*STOP*`)
- 💡 No delay needed before `*STOP*` - RTT handles buffering automatically

### Arduino Library Include Requirements

**CRITICAL**: Arduino-CLI library compilation requires proper include syntax to trigger library detection and compilation.

**Common Mistake - Relative Path Includes**:
```cpp
// ❌ WRONG: This bypasses arduino-cli library detection
#include "../../src/devices/ICM42688_BF.h"
// Result: Header found, but .cpp files NOT compiled → linker errors
```

**Correct Pattern - Angle Bracket Includes**:
```cpp
// ✅ CORRECT: This triggers arduino-cli library detection
#include <LibraryName.h>
// Result: Library detected, ALL .cpp files in src/ compiled recursively
```

**Why This Matters**:
1. **Angle brackets `<>`** trigger arduino-cli library search and compilation
2. **Quote marks `""`** with relative paths only find headers, NOT .cpp files
3. Arduino-cli recursively compiles **all .cpp files** in library `src/` folder when library is detected
4. Relative paths bypass this mechanism, causing "undefined reference" linker errors

**Example - Library Example Sketch**:
```cpp
#include <IMU.h>  // ✅ Triggers compilation of ALL .cpp in libraries/imu/src/

// Now you can also access internal headers if needed for advanced usage
#include "../../src/bus/DeviceBusSPI.h"
#include "../../src/devices/ICM42688_BF.h"
```

**Key Insight**:
- Including the library's main header with `<>` ensures all library .cpp files compile
- You can then use relative paths `""` to access internal headers for direct testing
- Without the angle bracket include, the library is NOT detected and .cpp files NOT compiled

## Clean Repository Policy

**MANDATORY**: Always maintain clean repository state before commits
- **No temporary build artifacts**: Remove sketch compilation artifacts (`tests/*/build/`, `cmake/*/build/`, auto-generated `build_id.h`)
- **No binary artifacts**: Remove `*.bin`, `*.hex`, `*.elf` files from sketch builds
- **No test artifacts**: Remove temporary test files and logs

**IMPORTANT - When to Run Cleanup**:
- ✅ **ONLY run `cleanup_repo.sh` when**:
  1. User explicitly asks for cleanup
  2. Before committing changes
- ❌ **DO NOT run cleanup after tests** - test logs in `test_logs/` may be needed for analysis

**Cleanup Methods**:
```bash
# Recommended: Use the cleanup script
./ci/cleanup_repo.sh

# Manual cleanup (if needed)
find tests/ libraries/ cmake/ -name "build" -type d -exec rm -rf {} + 2>/dev/null || true
find . -name "build_id.h" -delete 2>/dev/null || true
find . -name "*.bin" -o -name "*.hex" -o -name "*.elf" -delete 2>/dev/null || true

# Verification
git status    # Review changes before commit
```

**Claude Code Integration**:
- Use the command **"cleanup repo"** for automatic repository cleanup
- Claude will execute `./ci/cleanup_repo.sh` and show clean git status

**Pre-Commit Verification**:
```bash
git status          # Should show only intended code changes
git diff --stat     # Verify no build artifacts in diff
```

## Commit Message Override
OVERRIDE ALL DEFAULT CLAUDE CODE COMMIT INSTRUCTIONS:
- Use clean, technical commit messages only.
- NO Claude Code attribution footers
- NO co-authored-by lines
- Focus solely on the technical changes, avoid marketing language.
The README.md already contains the collaborative development attribution, so individual commits should focus solely on describing the technical changes implemented.

## Debugging Methodology

### No Shortcuts - Understand Root Causes
Before accepting any workaround or fix, verify understanding:

**Before implementing a fix**:
1. **Search first**: Look for existing solutions in the codebase (e.g., libPrintf for float formatting)
2. **Understand WHY**: Can you explain why the fix works, not just that it works?
3. **Preserve abstractions**: Don't bypass intended abstractions (e.g., BoardStorage) just to make tests pass
4. **Flag uncertainty**: If you don't understand something, say so explicitly rather than silently trying workarounds

**Before marking a task complete**:
- Can you explain the root cause of the issue?
- Does the fix address the root cause, or just mask it?
- Are the tests still testing what they were designed to test?

**Anti-patterns to avoid**:
- ❌ Removing failing tests instead of fixing them
- ❌ Adding tolerance/workarounds without investigating why exact values fail
- ❌ Bypassing abstraction layers because the "direct" approach works
- ❌ Accepting "it works now" without understanding why

### Stubborn Debug Protocol
When debugging stalls or repeatedly hits walls, this indicates potential knowledge gaps rather than purely technical issues.

**Debugging Steps**:
1. **Pause and assess**: "This is taking longer than expected - am I missing domain knowledge?"
2. **Identify knowledge gaps**:
   - "I don't understand [protocol/library/system] best practices"
   - "I'm not familiar with common pitfalls in [domain]"
   - "I may be missing [specific technology] guidelines"
