# STM32 Arduino Core Development Environment

A focused STM32 Arduino development environment with CI/CD automation capabilities.

## Installation via Arduino Board Manager

The easiest way to install this core:

1. Open **Arduino IDE**
2. Go to **File → Preferences**
3. Add this URL to "Additional Boards Manager URLs":
   ```
   https://github.com/geosmall/BoardManagerFiles/raw/main/package_stm32_robotics_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for **"STM32 Robotics"**
6. Click **Install**

This installs the complete STM32 Robotics Core with all robotics libraries.

For development workflows (CI/HIL testing, build scripts), see [Prerequisites](#prerequisites) below.

## Overview

This repository contains a **fork of the STM32 Arduino Core** with simplified variant selection. It's designed for developing autonomous systems such as drone flight control systems with real-time data logging, sensor and configuration data management.

## Target Hardware

- **Primary**: STM32F411 (Nucleo F411RE, BlackPill F411CE)
- **Secondary**: STM32F405, STM32F722 (common in flight controllers)
- **Advanced**: STM32H743 (high-performance flight controllers)

## Key Features

- **Unified Development Framework**: Single codebase supporting Arduino IDE and CI/HIL workflows with `ci_log.h` abstraction
- **Production HIL Testing Framework**: Complete build-to-runtime traceability with J-Link + RTT integration
- **Enhanced Build-ID Integration**: Git SHA + UTC timestamp tracking for firmware traceability
- **Universal Device Detection**: Auto-detect any STM32 via J-Link for programming
- **Sub-20ms Ready Token Detection**: Deterministic HIL test initialization (5.2ms achieved)
- **Unified Storage Systems**: LittleFS (SPI flash), SDFS (SD card), and Generic Storage abstraction with minIni configuration management
- **IMU Integration**: High-level C++ wrapper (IMU library) supporting 6-DOF and 9-DOF sensors (ICM42688P, MPU-6000, MPU-9250) with chip detection, magnetometer calibration, and manufacturer self-test
- **Betaflight Config Converter**: Python tool converting Betaflight unified targets to BoardConfig headers with PeripheralPins.c validation and ALT variant handling
- **libPrintf Integration**: Embedded printf library eliminating nanofp complexity with 20KB+ binary savings
- **AUnit Testing Framework**: Comprehensive unit testing with HIL integration (18 tests across storage systems)
- **Real-time Debugging**: SEGGER RTT v8.62 integration for printf-style debugging
- **Flight Controller Focus**: Optimized for UAV applications with deterministic testing

## Prerequisites

### Core Build Environment

#### Required (Basic Compilation)
- **Arduino CLI** v1.3.0 (locked version for build consistency)
  - Manages STM32 core and ARM GCC toolchain automatically
  - Installation: https://arduino.github.io/arduino-cli/latest/installation/
- **STM32 Robotics Core** (this repository)
  - Includes ARM GCC 12.x toolchain (xpack-arm-none-eabi-gcc-12.2.1-1.2)
  - Install via Board Manager (see [Installation](#installation-via-arduino-board-manager) above)

#### Optional (Hardware Testing & Debugging)
- **SEGGER J-Link** v8.62+ (for HIL testing with RTT debugging)
  - `JLinkExe` - Flash programming and device detection
  - `JRun` - Execute with RTT capture (HIL testing)
  - `JLinkGDBServer` - GDB server with RTT support
  - `JLinkRTTClient` - Real-time terminal client
  - Installation: https://www.segger.com/downloads/jlink/
  - Note: Requires ST-Link V2.1 reflashed to J-Link firmware for NUCLEO boards
  - STM32CubeProgrammer - for Arduino IDE ST-Link and J-Link uploads
  - Installation: https://www.st.com/en/development-tools/stm32cubeprog.html

### System Tools (Linux/macOS)

#### Build Scripts
- **bash** - CI/CD automation scripts
- **git** - Version control and build-ID generation

#### Optional (Device Detection)
- **lsusb** - USB device enumeration (Linux only)
  - Ubuntu/Debian: `sudo apt-get install usbutils`

### Installation (CLI)
```bash
# Add custom board manager URL
arduino-cli config add board_manager.additional_urls \
  https://github.com/geosmall/BoardManagerFiles/raw/main/package_stm32_robotics_index.json

# Install STM32 Robotics core
arduino-cli core update-index
arduino-cli core install STM32_Robotics:stm32

# Verify installation
./system/ci/env_check_quick.sh true
```

## Quick Start

### Build and Upload
```bash
# Compile sketch
arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE <sketch_directory>

# Upload via ST-Link
arduino-cli upload --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE <sketch_directory>

# Upload via J-Link with auto-detection (when ST-Link reflashed)
./system/ci/flash_auto.sh --quick <path_to_binary.bin>
```

### HIL Testing (Recommended)
```bash
# One-button build and test with environment validation
./system/ci/aflash.sh <sketch_directory> --env-check

# Unified development workflow (Arduino IDE + CI/HIL support)
./system/ci/build.sh <sketch_directory>                    # Arduino IDE (Serial)
./system/ci/build.sh <sketch_directory> --use-rtt          # CI/HIL (RTT)
./system/ci/aflash.sh <sketch_directory> --use-rtt --build-id --env-check  # Complete workflow

# Enhanced ready token detection with build-ID parsing (5.2ms latency)
./system/ci/await_ready.sh [log_file] [timeout] [pattern]
# Output: READY NUCLEO_F411RE 901dbd1-dirty 2025-09-09T10:07:44Z
```

### Using Makefile (HIL_RTT_Test/)
```bash
make                # Compile
make upload         # Compile and upload
make check          # Verify environment
```

## Libraries

### Storage and Configuration
- **LittleFS**: SPI flash storage for configuration and firmware
- **SDFS v1.0.0**: SD card filesystem via SPI with LittleFS-compatible configuration
- **Storage**: Generic storage abstraction providing unified interface for LittleFS and SDFS
- **minIniStorage v1.5.0**: INI file configuration management with automatic storage backend selection

### Sensors and Hardware
- **IMU v1.0.0**: High-level C++ wrapper for InvenSense IMU sensors with chip detection, multi-instance support, and 9-DOF magnetometer support (MPU-9250/9255)
- **ICM42688P v1.0.0**: Low-level 6-axis IMU library with TDK InvenSense drivers, self-test, and data acquisition
- **ICM206xx v1.0.0**: Betaflight-derived 6-axis IMU library for ICM-20601/20602/20608/20689 with auto-detection
- **MPU6000 v1.0.0**: Betaflight-derived 6-axis IMU library for MPU-6000
- **MPU9250 v1.0.0**: Betaflight-derived 9-axis IMU library for MPU-9250/9255 with magnetometer
- **invensense-imu v6.0.3**: Bolder Flight Systems library for MPU-9250/MPU-6500 (I2C and SPI)
- **xensiv-dps3xx v1.0.0**: Infineon barometric pressure sensor library for DPS310/DPS368 with I2C support, STM32H7 detection, and configurable address
- **TinyGPSPlus v1.0.3a**: NMEA GPS parser library with CI examples for HardwareSerial and SoftwareSerial
- **SerialRx v1.0.0**: RC receiver serial protocol parser (IBus, SBUS) with software idle detection and hardware validation
- **TimerPWM v1.0.0**: Hardware timer PWM for servo/ESC control with 1µs resolution, explicit timer banks, and dual-timer support
- **STM32RTC**: Real-time clock functionality

### Core Communication
- **SPI**: SPI communication library
- **Wire**: I2C communication library
- **SoftwareSerial**: Software UART implementation

### Development and Testing
- **SEGGER_RTT v8.62**: Real-time transfer debugging with HIL integration
- **CMSIS_DSP**: ARM CMSIS DSP functions
- **libPrintf v6.2.0**: Embedded printf library eliminating nanofp complexity (20KB+ binary savings)
- **AUnit v1.7.1**: Arduino unit testing framework with HIL integration (18 comprehensive tests)

## Project Structure

```
├── cores/arduino/         # STM32 Arduino core implementation
├── variants/              # Board-specific pin definitions
├── system/                # STM32Cube HAL/LL drivers and CMSIS
├── libraries/             # Core + robotics libraries
│   ├── CMSIS_DSP/         # ARM CMSIS DSP functions
│   ├── SEGGER_RTT/        # SEGGER RTT library with HIL example
│   ├── SPI/               # SPI communication library
│   ├── Wire/              # I2C communication library
│   ├── SoftwareSerial/    # Software UART implementation
│   ├── AUnit-1.7.1/       # Arduino unit testing framework with HIL integration
│   ├── ICM42688P/         # Low-level 6-axis IMU library with TDK InvenSense drivers
│   ├── imu/               # High-level C++ wrapper for InvenSense IMU sensors
│   ├── libPrintf/         # Embedded printf library (eyalroz/printf v6.2.0 wrapper)
│   ├── LittleFS/          # SPI flash filesystem (littlefs-project/littlefs)
│   ├── minIniStorage/     # Configuration management with unified storage backend
│   ├── SDFS/              # SD filesystem v1.0.0 with LittleFS-compatible API
│   ├── SerialRx/          # RC receiver protocol parser (IBus, SBUS) with idle detection
│   ├── STM32RTC/          # Real-time clock library
│   ├── Storage/           # Generic storage abstraction for LittleFS/SDFS
│   ├── TimerPWM/          # Hardware timer PWM for servo/ESC control
│   ├── TinyGPSPlus/       # NMEA GPS parser (mikalhart/TinyGPSPlus v1.0.3a)
│   └── xensiv-dps3xx/     # Infineon barometric pressure sensor (DPS310/DPS368)
├── cmake/                 # CMake build system and examples
├── extras/
│   └── betaflight_converter/  # Betaflight → BoardConfig converter with validation
├── system/
│   ├── ci/                # Build and test automation scripts
│   └── extras/            # Arduino build hooks (prebuild/postbuild)
├── tests/                 # Unit tests and integration tests
├── targets/               # Board configuration headers
└── doc/                   # Technical documentation
```

### Repository Information
- **Main Repository**: [geosmall/Arduino_Core_STM32](https://github.com/geosmall/Arduino_Core_STM32) *(stm32duino fork with enhancements)*
- **Upstream**: [stm32duino/Arduino_Core_STM32](https://github.com/stm32duino/Arduino_Core_STM32) *(original STM32 Arduino core)*

## Production Development Workflow

### Arduino IDE Development
1. **Build for Serial**: `./system/ci/build.sh libraries/ICM42688P/examples/example-selftest`
2. **Upload via Arduino IDE**: Standard Arduino workflow with Serial monitoring

### CI/HIL Testing
1. **Environment Check**: `./system/ci/env_check_quick.sh true` (~100ms validation)
2. **Device Detection**: `./system/ci/detect_device.sh` (auto-detect any STM32 via J-Link)
3. **Unified Build**: `./system/ci/build.sh <sketch> --use-rtt --build-id --env-check` (RTT mode with traceability)
4. **HIL Testing**: `./system/ci/aflash.sh <sketch> --use-rtt --build-id --env-check` (complete workflow)
5. **Traceability Verification**: `./system/ci/await_ready.sh` (enhanced parsing, 5.2ms latency achieved)
6. **Real-time Debug**: SEGGER RTT v8.62 with `JLinkRTTClient` for printf output

### Example Workflows
```bash
# IMU sensor testing with self-test validation
./system/ci/aflash.sh libraries/ICM42688P/examples/example-selftest --use-rtt --build-id

# Storage system unit testing
./system/ci/aflash.sh tests/LittleFS_IT --use-rtt --build-id
./system/ci/aflash.sh tests/SDFS_Unit_Tests --use-rtt --build-id

# Configuration management testing
./system/ci/aflash.sh tests/minIniStorage_Unit_Tests --use-rtt --build-id
```

### Build Traceability Example
```
Git: 901dbd1-dirty (2025-09-09T10:07:44Z)
READY NUCLEO_F411RE 901dbd1-dirty 2025-09-09T10:07:44Z
```

## Unified Development Framework

Single sketches work seamlessly in both Arduino IDE and CI/HIL environments:

```cpp
#include <ci_log.h>  // Single logging abstraction (in cores/arduino/)

void setup() {
  CI_LOG("Test starting\n");
  CI_BUILD_INFO();    // RTT: shows build details, Serial: no-op
  CI_READY_TOKEN();   // RTT: shows ready token, Serial: no-op
}
```

**Key Benefits:**
- No duplicate test sketches
- Automatic Serial ↔ RTT switching via `USE_RTT` compile flag
- Build traceability integration for CI/CD workflows
- Deterministic exit tokens for automated testing

## Current Development Status

- **✅ Complete**: Storage systems (LittleFS, SDFS, Storage abstraction), configuration management (minIni), build/HIL framework, libPrintf integration
- **✅ Complete**: IMU library (6-DOF and 9-DOF support: ICM42688P, MPU-6000, MPU-9250/9255 with magnetometer calibration)
- **✅ Complete**: ICM42688P library (low-level TDK drivers with self-test and data acquisition)
- **✅ Complete**: xensiv-dps3xx library (DPS310/DPS368 barometric pressure sensor with STM32H7 detection)
- **✅ Complete**: SerialRx library (IBus hardware validated, SBUS implemented, software idle detection)
- **✅ Complete**: TimerPWM library (hardware PWM for servos/ESCs, 1µs resolution, hardware validated)
- **✅ Complete**: Betaflight Config Converter (Python tool with PeripheralPins.c validation, ALT variant handling, 53 passing tests)
- **✅ Complete**: dRehmFlight STM32 port (BETA 1.3 flight controller, 100% flight logic preserved, pending bench validation)
- **📋 Future**: CRSF protocol support for SerialRx

## Documentation

See `CLAUDE.md` for detailed build instructions, architecture overview, and development guidelines.

This repository is being collaboratively developed with [Claude Code](https://claude.ai/code) for enhanced STM32 Arduino development workflows.