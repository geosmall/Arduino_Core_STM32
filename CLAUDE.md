# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This repository contains the **STM32 Arduino Core** along with associated sketches and libraries. It provides Arduino IDE support for STM32 microcontrollers through the STM32Cube ecosystem.

This is a fork of the upstream [stm32duino/Arduino_Core_STM32](https://github.com/stm32duino/Arduino_Core_STM32) repository. This fork includes:
- **Simplified variant selection** - Reduced number of board variants for focused development
- **Added FS.h** - Generic embedded file system base class for storage access (unified interface for SPI flash and SD card storage)
- **Robotics libraries** - Complete suite of UAV flight controller libraries integrated directly into the core
- **HIL testing framework** - Production-grade CI/CD workflow with build traceability

### Repository Structure

- `cores/arduino/` - Core Arduino implementation for STM32
- `variants/` - Board-specific pin definitions and configurations
- `system/` - STM32Cube HAL drivers and CMSIS
  - `system/ci/` - Build and test automation scripts
  - `system/extras/` - Arduino build hooks (prebuild/postbuild)
- `libraries/` - Core STM32 + robotics libraries
  - Core: `SPI`, `Wire`, `SoftwareSerial`, `CMSIS_DSP`, `SEGGER_RTT`
  - Robotics: `LittleFS`, `SDFS`, `Storage`, `minIniStorage`, `ICM42688P`, `imu`, `TimerPWM`, `SerialRx`, `libPrintf`, `AUnit`, `STM32RTC`
- `cmake/` - CMake build system and examples
- `tests/` - Unit tests and integration tests
- `targets/` - Board configuration headers (BoardConfig system)
- `extras/` - Betaflight config converter and utilities
- `doc/` - Technical documentation

## Build Systems and Commands

### Arduino CLI (Recommended)
The primary build system uses arduino-cli with STM32 boards:

```bash
# Install STM32 core
arduino-cli core update-index
arduino-cli core install STMicroelectronics:stm32

# Compile sketch
arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE <sketch_directory>

# Upload to board
arduino-cli upload --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE <sketch_directory>

# List available/connected boards
arduino-cli board listall
arduino-cli board list

# J-Link upload (when ST-Link reflashed to J-Link)
./system/ci/jlink_upload.sh <path_to_binary.bin>
```

### Makefile Support
Generic Makefile provided for cross-platform builds:

```bash
make                    # Compile with default FQBN
make upload            # Compile and upload to board
make clean             # Clean build artifacts
```

Default FQBN: `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE`

### Build Scripts
Enhanced build workflow with environment validation and device auto-detection:

```bash
# Standard build and HIL testing
./system/ci/build.sh <sketch_directory> [--build-id] [--env-check] [--use-rtt]
./system/ci/aflash.sh <sketch_directory> [--env-check] [--use-rtt] [--build-id]

# Environment and device utilities
./system/ci/env_check_quick.sh         # Fast environment validation
./system/ci/detect_device.sh           # Auto-detect STM32 via J-Link
./system/ci/flash_auto.sh <binary>     # Program with auto-detected device
./system/ci/cleanup_repo.sh            # Clean build artifacts before commit
```

**Key Features**:
- **Environment Validation**: Arduino CLI (1.3.0) and STM32 Core (2.7.1) validation
- **Build Traceability**: Git SHA + UTC timestamp integration
- **Device Auto-Detection**: 50+ STM32 device IDs supported
- **Cache Management**: `--clean-cache` for deterministic builds

### J-Link and RTT Utilities

```bash
# J-Link utilities
./system/ci/jrun.sh <elf> [timeout] [exit_wildcard] # J-Run execution with RTT
./system/ci/flash.sh [--quick] <binary>             # Flash with fixed device

# Manual RTT Connection
JLinkGDBServer -Device STM32F411RE -If SWD -Speed 4000 -RTTTelnetPort 19021 &
JLinkRTTClient                                     # Connect to RTT
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
- Board selection uses FQBN format: `STMicroelectronics:stm32:<board_group>:pnum=<specific_board>`

## Target Hardware and Applications

### Primary Development Boards
- **Nucleo F411RE** (Primary): `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE`
  - **HIL Setup**: On-board ST-Link V2.1 reflashed to J-Link firmware
  - **Serial Monitor**: Available via J-Link connection (connected CDC ACM serial monitor)
  - **Programming**: J-Run execution via reflashed J-Link interface
- **BlackPill F411CE** (Secondary): `STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE`
- **Nucleo H753ZI** (High-Performance): `STMicroelectronics:stm32:Nucleo_144:pnum=NUCLEO_H753ZI`

### Target Hardware Platforms
This repository supports **UAV flight controller boards** with the following STM32 microcontrollers:

- **STM32F411** - Primary target (Nucleo F411RE, BlackPill F411CE for development)
- **STM32F405** - Secondary target (common in flight controllers) 
- **STM32H743** - Future target (high-performance flight controllers)

### Example Target Applications
- **UAV Flight Controllers** - Autonomous drone flight control systems
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
- `IMU` - High-level C++ IMU wrapper with chip detection and multi-instance support
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

**Production Usage**:
```bash
./system/ci/build.sh HIL_RTT_Test --build-id --env-check
./system/ci/aflash.sh HIL_RTT_Test
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

### SDFS Implementation ✅ **COMPLETED**

SPI-based SD card filesystem with FatFs backend providing LittleFS-compatible API for seamless storage switching.

**Key Features**:
- **Complete File I/O**: All operations (create, read, write, delete, seek, truncate)
- **Directory Operations**: Full enumeration, creation, and traversal
- **Configuration System**: SDFSConfig.h with configurable limits
- **Runtime Detection**: Dynamic sector size and card capacity detection
- **Error Handling**: SDFSERR enum with mount protection and diagnostics

**Production Example**:
```cpp
#include <SDFS.h>
SDFS_SPI sdfs;

void setup() {
  if (sdfs.begin(CS_PIN)) {  // Auto-detects card capacity
    File file = sdfs.open("/flight.log", FILE_WRITE_BEGIN);
    file.printf("Card: %llu MB\n", sdfs.totalSize() / (1024*1024));
    file.close();
  }
}
```

### LittleFS Example Integration ✅ **COMPLETED**

Complete integration of all LittleFS examples with unified CI/HIL framework for SPI flash testing.

**Key Features**:
- **3 Examples**: ListFiles, LittleFS_ChipID, LittleFS_Usage
- **HIL Integration**: Full ci_log.h integration with deterministic testing
- **Hardware Support**: 20+ SPI flash chips from multiple manufacturers
- **Interactive Removal**: Eliminated waitforInput() calls for automation

### AUnit Testing Framework Integration ✅ **COMPLETED**

AUnit v1.7.1 unit testing framework integrated with HIL CI/CD workflow for comprehensive storage library testing.

**Key Features**:
- **Complete Integration**: `aunit_hil.h` wrapper with RTT/Serial abstraction
- **18 Total AUnit Tests**: LittleFS (8 tests), SDFS (7 tests), AUnit framework validation (3 tests)
- **Hardware Validation**: Real SPI flash and SD card testing on STM32F411RE
- **Dual-Mode Support**: Same tests work with RTT (HIL) and Serial (IDE)
- **Type Safety**: Established AUnit assertion patterns for embedded types

**Production Usage**:
```bash
./system/ci/aflash.sh tests/LittleFS_Unit_Tests --use-rtt --build-id
./system/ci/aflash.sh tests/SDFS_Unit_Tests --use-rtt --build-id
./system/ci/aflash.sh tests/AUnit_Pilot_Test --use-rtt --build-id
```

### Board Configuration System ✅ **COMPLETED**

Compile-time board configuration system with multi-board support, flexible peripheral config, and hardware/software chip select control.

**Key Features**:
- **Multi-Board Support**: NUCLEO_F411RE, BLACKPILL_F411CE, NOXE_V3 with automatic `ARDUINO_*` detection
- **Config Types**: StorageConfig, IMUConfig, RCReceiverConfig, UARTConfig, I2CConfig, ADCConfig, LEDConfig
- **CS Mode Control**: Software/hardware chip select via CS_Mode enum with get_ssel_pin() helper
- **Composable Architecture**: SPIConfig building blocks for storage, IMU, and RC receiver
- **Frequency Optimization**: 1MHz (jumper wire), 8MHz (hardwired)

**Usage**:
```cpp
#include "targets/NUCLEO_F411RE_LITTLEFS.h"
SPIClass spi(BoardConfig::storage.mosi_pin, BoardConfig::storage.miso_pin,
             BoardConfig::storage.sclk_pin, BoardConfig::storage.get_ssel_pin());
// See targets/*.h and targets/HW_CONFIG.md for complete usage
```

### Generic Storage Abstraction ✅ **COMPLETED**

Unified storage interface for SDFS and LittleFS with automatic backend selection via BoardConfig.

**Usage**: `#include <Storage.h>` → `Storage& fs = BOARD_STORAGE;` → `fs.open("/file.txt", FILE_WRITE);`

### minIni Configuration Management ✅ **COMPLETED**

INI file configuration (v1.5) integrated with Storage abstraction for LittleFS/SDFS.

**Features**: String/int/float/bool support, section/key enumeration, HIL validated (6 test suites)

**Usage**: `minIniStorage config("config.ini");` → `config.begin(BoardConfig::storage);` → `config.put("key", "val");`

### libPrintf ✅ **COMPLETED**

Embedded printf library (eyalroz/printf v6.2.0) eliminating nanofp confusion with ~20% binary reduction (8KB+ savings).

**Usage**: `#include <libPrintf.h>` → `printf("Pi = %.6f\n", 3.14159);  // Float formatting works`

### ICM-42688-P IMU Library Integration ✅ **COMPLETED**

Complete Arduino-compatible ICM42688P library with manufacturer-grade reliability and performance. Successfully adapted from UVOS framework while preserving 100% of InvenSense factory algorithms.

**Target Hardware**: ICM-42688-P 6-axis IMU sensor via SPI with PC4 interrupt (EXTI4)

**Key Features**:
- **Factory Code Preservation**: Zero modifications to InvenSense sensor algorithms
- **Multiple Usage Modes**: Basic SPI, self-test, interrupt-driven data, processed AG data
- **BoardConfig Integration**: Dynamic pin/frequency configuration support
- **HIL Testing**: Automated validation with RTT and build traceability
- **Arduino Ecosystem**: Full compatibility with Arduino IDE and CLI

**Available Examples**:
1. **ICM42688P_Simple**: Basic SPI communication and device identification
2. **example-selftest**: Manufacturer self-test with bias calculation
3. **example-raw-data-registers**: Interrupt-driven raw sensor data acquisition
4. **example-raw-ag**: Processed accelerometer/gyroscope data with clock calibration

**Usage**:
```cpp
#include <ICM42688P_Simple.h>
SPIClass spi(PA7, PA6, PA5);
ICM42688P_Simple imu;
imu.begin(spi, PA4, 1000000);  // Returns 0x47 device ID
// See libraries/ICM42688P/examples/ for complete examples
```

### High-Level IMU Library ✅ **COMPLETED**

Unified C++ wrapper library for InvenSense IMU sensors, starting with ICM-42688-P and designed for easy extension to MPU-6000, MPU-9250, and other InvenSense parts.

**Key Features**:
- **Context-Based Design**: Multi-instance support via `serif->context` pointer
- **Chip Detection**: ChipType enum with GetChipType() for ICM42688_P, MPU-6000, MPU-9250
- **Interrupt Support**: EnableDataReadyInt1() and DisableDataReadyInt1() methods
- **Full API**: Init, Reset, RunSelfTest, ReadIMU6, sensor configuration (FSR, ODR, power modes)
- **BoardConfig Integration**: Works with multi-board configuration system
- **HIL Validated**: Both examples tested on NUCLEO_F411RE hardware

**Available Examples**:
1. **imu-selftest**: Manufacturer self-test with bias calculation and chip detection
2. **imu-raw-data-registers**: Interrupt-driven raw sensor data acquisition at 1kHz

**Usage**:
```cpp
#include <IMU.h>
SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin, BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin, BoardConfig::imu.spi.get_ssel_pin());
IMU imu;
imu.Init(spi_bus, BoardConfig::imu.spi.cs_pin, BoardConfig::imu.spi.freq_hz);
IMU::ChipType chip = imu.GetChipType();  // Returns ICM42688_P (0x47)
// See libraries/imu/examples/ for complete examples
```

**Notes**: Currently supports ICM-42688-P only. Chip detection framework ready for future MPU-6000/MPU-9250 support.

### TimerPWM Library ✅ **COMPLETED**

Hardware timer-based PWM library for high-resolution (1µs) servo and ESC control on UAV flight controllers.

**Key Features**:
- 1 MHz timer resolution (1µs pulse width control)
- Explicit timer bank configuration (prevents frequency conflicts)
- BoardConfig hardware abstraction
- Arduino Servo compatible API (Write method supports 0-180° or µs)
- Multi-channel support (up to 4 channels per timer)
- Dual timer support (servos + ESCs simultaneously)
- **Hardware validated**: PWM channel enable (resumeChannel) correctly implemented per AN4013 Section 2.5

**Implementation Notes**:
- PWMOutputBank correctly enables channel output via `resumeChannel()` after configuration
- Per STM32 timer documentation (AN4013 Section 2.5), PWM requires both `resumeChannel(channel)` (CCxE bit) and `resume()` (counter start)
- See `libraries/TimerPWM/APPROACH.md` for complete implementation details and validation results

### Betaflight Config Converter ✅ **COMPLETED**

Python tool that converts Betaflight unified target configurations into Arduino STM32 BoardConfig headers with comprehensive validation.

**Location**: `extras/betaflight_converter/`

**Key Features**:
- **Parser→Validator→Generator Pipeline**: Clean architecture with 53 passing tests
- **PeripheralPins.c Validation**: Cross-validates all pins against Arduino Core STM32 variants
- **ALT Variant Handling**: Automatically detects and generates ALT pin variants (e.g., PB0_ALT1) for timer/AF conflicts
- **Multi-Variant MCU Support**: Automatically finds correct variant for different chip packages
- **Motor Timer Grouping**: Groups motors by timer banks for TimerPWM integration
- **Cross-Platform**: Works on Windows, macOS, Linux with Python 3.7+

**Testing**:
```bash
# Install pytest (one-time) - Recommended
pipx install pytest

# Run tests from converter directory
cd extras/betaflight_converter && pytest -v
# Expected: 53 tests passing
```

**Supported MCUs**:
- STM32F411 (F411CE - BlackPill, JHEF411 - NOXE V3)
- STM32F405 (F405RG - common in flight controllers)
- STM32F745 (F7 series)
- STM32H743 (H743VIH6 - Matek H743-WLITE, H743ZIT6 - Nucleo boards)

**Usage**:
```bash
cd extras/betaflight_converter
python3 convert.py data/MTKS-MATEKH743.config  # Generates output/MTKS-MATEKH743.h
```

**Generated Configs Include**:
- Storage (SPI flash/SD card) → `StorageConfig`
- IMU (gyro + interrupt) → `IMUConfig`
- I2C sensors → `I2CConfig`
- UARTs → `UARTConfig`
- ADC battery monitoring → `ADCConfig`
- Status LEDs → `LEDConfig`
- Servos (50 Hz PWM) → `Servo` namespace
- Motors (DSHOT/OneShot) → `Motor` namespace

**Validated Targets**:
- ✅ JHEF-JHEF411 (NOXE V3) - 5 motors, SPI flash, dual SPI buses
  - Generated config: `output/JHEF-JHEF411.h` (8MHz SPI for hardwired boards)
  - HIL test config: `targets/NUCLEO_F411RE_JHEF411.h` (1MHz SPI for jumper wire testing)
- ✅ MTKS-MATEKH743 (H743-WLITE) - 8 motors, 2 servos, dual gyros, 7 UARTs, SD card → `output/MTKS-MATEKH743.h`

**Naming Convention**: Follows madflight - output filename matches config filename (e.g., `JHEF-JHEF411.config` → `JHEF-JHEF411.h`)

**CS Mode Selection**: Generator uses software chip select (default) for maximum library compatibility. Hardware CS mode removed in commit fixing ICM42688P library integration.

**Documentation**: See `extras/betaflight_converter/README.md` for quick start

**Converter Examples**:
- **example-icm42688p-minimal**: Basic ICM42688P WHO_AM_I verification using generated config
  - Tests SPI communication and chip detection
  - Validates generated IMU config (SPI pins, CS, frequency)
  - Expected: 5 continuous WHO_AM_I reads returning 0x47
- **motor_pwm_verification**: PWMOutputBank with TIM2 input capture measurement
  - Uses PWMOutputBank to generate PWM on Motor1 (PA8/TIM1) and Motor4 (PB0/TIM3)
  - Uses TIM2 input capture on PA0/PA1 to measure actual frequencies
  - Demonstrates timer bank grouping from Betaflight converter
  - Hardware validation following embedded validation standards

**Usage Example**:
```cpp
#include <PWMOutputBank.h>
#include "targets/NUCLEO_F411RE_LITTLEFS.h"

// Servo control at 50 Hz
PWMOutputBank servo_pwm;
auto& servo_ch = BoardConfig::Servo::pwm_output;
servo_pwm.Init(BoardConfig::Servo::timer, BoardConfig::Servo::frequency_hz);
servo_pwm.AttachChannel(servo_ch.ch, servo_ch.pin, servo_ch.min_us, servo_ch.max_us);
servo_pwm.SetPulseWidth(servo_ch.ch, 1500);  // 1500 µs center position
servo_pwm.Start();

// ESC control at 1 kHz (OneShot125)
PWMOutputBank esc_pwm;
auto& esc_ch = BoardConfig::ESC::esc1;
esc_pwm.Init(BoardConfig::ESC::timer, BoardConfig::ESC::frequency_hz);
esc_pwm.AttachChannel(esc_ch.ch, esc_ch.pin, esc_ch.min_us, esc_ch.max_us);
esc_pwm.SetPulseWidth(esc_ch.ch, 187);  // 187 µs midpoint
esc_pwm.Start();
```

**Examples** (All use consistent BoardConfig pin assignments):
- **PWM_Verification**: Single timer validation with input capture
  - PWM: PB4/D5 (TIM3_CH1) @ 50 Hz → Capture: PA0/A0 (TIM2_CH1)
  - Hardware measurement: 49.50 Hz ✅ PASS (±2% tolerance)
  - Demonstrates timeout/fail behavior for missing jumpers
  - Deterministic HIL testing with exit wildcard

- **Servo_Verification**: Servo PWM validation using JHEF411 config
  - PWM: PB0/A3 (TIM3_CH3 - repurposed Motor4) @ 50 Hz → Capture: PB10/D6 (TIM2_CH3)
  - Hardware measurement: 49.50 Hz ✅ PASS (±2% tolerance)
  - Validates 1500 µs pulse width (center position)
  - Uses same jumper setup as motor_pwm_verification Motor4 test

- **DualTimerPWM**: Demo of simultaneous servo and ESC control
  - Servo: PB4/D5 (TIM3) @ 50 Hz (1000-2000 µs pulses)
  - ESC1: PB6/D10 (TIM4) @ 1 kHz (125-250 µs OneShot125 pulses)
  - ESC2: PB7/CN7-21 (TIM4) @ 1 kHz
  - Shows practical dual timer operation for flight controllers

- **DualTimerPWM_Verification**: Dual timer hardware validation
  - Servo PWM: PB4/D5 (TIM3) → Capture: PA0/A0 (TIM2_CH1)
  - ESC PWM: PB6/D10 (TIM4) → Capture: PB10/D6 (TIM2_CH3)
  - Hardware measurements:
    - Servo: 49.50 Hz ✅ PASS (49-51 Hz tolerance)
    - ESC: 990.10 Hz ✅ PASS (980-1020 Hz tolerance)
  - Proves independent timer operation without crosstalk
  - All pins configured via BoardConfig for consistent test rig setup

**Hardware Validation Results**:
- ✅ **Single Timer**: PWM_Verification - 49.50 Hz measured (±2% spec)
- ✅ **Dual Timer**: DualTimerPWM_Verification - 49.50 Hz servo + 990.10 Hz ESC measured simultaneously
- **Methodology**: TIM2 input capture with jumper wires (no oscilloscope required)
- **Test Features**: 15-second timeout for missing jumpers, helpful error messages

**Documentation**:
- `libraries/TimerPWM/APPROACH.md` - Design rationale, technical decisions, and channel enable fix
- `doc/TIMERS.md` - Comprehensive STM32 timer architecture and API reference
- `doc/TIMERS_PWM_OUT.md` - Practical servo/ESC PWM configuration guide with correct channel enable steps
- `targets/NUCLEO_F411RE_LITTLEFS.h` - Hardware configuration with standardized pin assignments

**Important**: All timer PWM examples correctly demonstrate the required `resumeChannel()` call per STM32 documentation. Manual PWM configuration requires both channel enable (`resumeChannel()`) and counter start (`resume()`).

### SerialRx Library ✅ **COMPLETED**

RC receiver protocol parser with hardware-validated IBus implementation, SBUS support, and BoardConfig integration.

**Key Features**:
- **Dual Protocol Support**: IBus (hardware validated) and SBUS (implemented)
- **BoardConfig Integration**: RCReceiverConfig for consistent pin/protocol configuration across target boards
- **Software Idle Detection**: Optional timestamp-based frame synchronization (300µs threshold)
- **Ring Buffer Management**: Efficient circular buffer for serial data
- **Failsafe Detection**: Configurable timeout monitoring
- **HIL Integration**: Full ci_log.h support with deterministic testing

**Supported Protocols**:
- ✅ **IBus** (FlySky): 32-byte frames, 115200 baud, 14 channels
- ✅ **SBUS** (FrSky/Futaba): 25-byte frames, 100000 baud, 16 channels × 11-bit (requires inverted signal)
- 📋 **CRSF** (TBS Crossfire): Framework ready

**Hardware Validation**:
- **IBus Loopback**: 501/501 frames (0% loss) with dual-USART testing
- **IBus Real Receiver**: FlySky FS-iA6B validated (15s continuous, 10 channels, 1000-2000µs range)
- **SBUS**: Implemented but not hardware validated

**Production Usage with BoardConfig**:
```cpp
#include <SerialRx.h>
#include "targets/NUCLEO_F411RE_LITTLEFS.h"

HardwareSerial SerialRC(BoardConfig::rc_receiver.rx_pin,
                        BoardConfig::rc_receiver.tx_pin);
SerialRx rc;

void setup() {
  SerialRx::Config config;
  config.serial = &SerialRC;
  config.rx_protocol = SerialRx::IBUS;  // or SerialRx::SBUS
  config.baudrate = BoardConfig::rc_receiver.baud_rate;
  config.timeout_ms = BoardConfig::rc_receiver.timeout_ms;
  config.idle_threshold_us = BoardConfig::rc_receiver.idle_threshold_us;
  rc.begin(config);
}

void loop() {
  rc.update();
  if (rc.available()) {
    RCMessage msg;
    if (rc.getMessage(&msg)) {
      uint16_t throttle = msg.channels[2];  // Channel 3
    }
  }
}
```

**Examples**:
- **IBus_Basic**: Real receiver validation (FlySky FS-iA6B, 15s HIL test, BoardConfig integrated)
- **IBus_Loopback_Test**: Dual-USART validation (501/501 frames, deterministic exit)
- **SBUS_Basic**: SBUS testing (requires inverted signal, timeout detection verified)

**Documentation**:
- `libraries/SerialRx/README.md` - Protocol docs, state machine, validation results
- `doc/SERIAL.md` - Technical implementation details

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
- ✅ **YOU MUST** call `Serial.begin()` when NOT using RTT mode - ci_log.h does NOT initialize Serial

**Available Macros** (`Arduino_Core_STM32/cores/arduino/ci_log.h`):
```cpp
CI_LOG(s)              // String literals only (no printf formatting)
CI_LOGF(...)           // Printf-style formatting (RTT: SEGGER_RTT_printf, Serial: Serial.printf)
CI_BUILD_INFO()        // Shows build SHA + timestamp (RTT only, no-op in Serial mode)
CI_READY_TOKEN()       // Shows ready token (RTT only, no-op in Serial mode)
CI_LOG_FLOAT(prefix, value, decimals)  // Float output helper (works in both modes)
```

**Correct Usage Pattern**:
```cpp
#include <ci_log.h>

void setup() {
  // Initialize Serial for non-RTT mode (Arduino IDE)
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait for Serial with timeout
#endif

  CI_LOG("Starting test\n");           // String literal
  CI_LOGF("Value: %d\n", 123);         // Printf formatting
  CI_LOG_FLOAT("Temp: ", 23.5, 2);    // Float output
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

## Clean Repository Policy

**MANDATORY**: Always maintain clean repository state before commits
- **No temporary build artifacts**: Remove sketch compilation artifacts (`tests/*/build/`, `cmake/*/build/`, auto-generated `build_id.h`)
- **No binary artifacts**: Remove `*.bin`, `*.hex`, `*.elf` files from sketch builds
- **No test artifacts**: Remove temporary test files and logs

**Cleanup Methods**:
```bash
# Recommended: Use the cleanup script
./system/ci/cleanup_repo.sh

# Manual cleanup (if needed)
find tests/ libraries/ cmake/ -name "build" -type d -exec rm -rf {} + 2>/dev/null || true
find . -name "build_id.h" -delete 2>/dev/null || true
find . -name "*.bin" -o -name "*.hex" -o -name "*.elf" -delete 2>/dev/null || true

# Verification
git status    # Review changes before commit
```

**Claude Code Integration**:
- Use the command **"cleanup repo"** for automatic repository cleanup
- Claude will execute `./system/ci/cleanup_repo.sh` and show clean git status

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

### Stubborn Debug Protocol
When debugging stalls or repeatedly hits walls, this indicates potential knowledge gaps rather than purely technical issues.

**Debugging Steps**:
1. **Pause and assess**: "This is taking longer than expected - am I missing domain knowledge?"
2. **Identify knowledge gaps**:
   - "I don't understand [protocol/library/system] best practices"
   - "I'm not familiar with common pitfalls in [domain]"
   - "I may be missing [specific technology] guidelines"
