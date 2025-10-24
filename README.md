# STM32 Arduino Core Development Environment

A focused STM32 Arduino development environment with CI/CD automation capabilities.

## Overview

This repository contains a **fork of the STM32 Arduino Core** with simplified variant selection. It's designed for developing autonomous systems such as drone flight control systems with real-time data logging, sensor and configuration data management.

## Target Hardware

- **Primary**: STM32F411 (Nucleo F411RE, BlackPill F411CE)
- **Secondary**: STM32F405 (common in flight controllers)
- **Future**: STM32H743 (high-performance flight controllers)

## Key Features

- **Unified Development Framework**: Single codebase supporting Arduino IDE and CI/HIL workflows with `ci_log.h` abstraction
- **Production HIL Testing Framework**: Complete build-to-runtime traceability with J-Link + RTT integration
- **Enhanced Build-ID Integration**: Git SHA + UTC timestamp tracking for firmware traceability
- **Universal Device Detection**: Auto-detect any STM32 via J-Link for programming
- **Sub-20ms Ready Token Detection**: Deterministic HIL test initialization (5.2ms achieved)
- **Unified Storage Systems**: LittleFS (SPI flash), SDFS (SD card), and Generic Storage abstraction with minIni configuration management
- **IMU Integration**: High-level C++ wrapper (IMU library) and low-level TDK drivers (ICM42688P) with chip detection and manufacturer self-test
- **Betaflight Config Converter**: Python tool converting Betaflight unified targets to BoardConfig headers with PeripheralPins.c validation and ALT variant handling
- **libPrintf Integration**: Embedded printf library eliminating nanofp complexity with 20KB+ binary savings
- **AUnit Testing Framework**: Comprehensive unit testing with HIL integration (22 tests across storage systems)
- **Real-time Debugging**: SEGGER RTT v8.62 integration for printf-style debugging
- **Flight Controller Focus**: Optimized for UAV applications with deterministic testing

## Dependencies

### Required Tools
- **Arduino CLI** v1.3.0 (locked version)
- **STM32 Core** v2.7.1 (STMicroelectronics:stm32)
- **J-Link** v8.62+ (for HIL testing and RTT debugging)

### Installation
```bash
# Install STM32 core
arduino-cli core update-index
arduino-cli core install STMicroelectronics:stm32

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
- **IMU v1.0.0**: High-level C++ wrapper for InvenSense IMU sensors with chip detection and multi-instance support
- **ICM42688P v1.0.0**: Low-level 6-axis IMU library with TDK InvenSense drivers, self-test, and data acquisition
- **SerialRx v1.0.0**: RC receiver serial protocol parser (IBus, SBUS) with software idle detection and hardware validation
- **STM32RTC**: Real-time clock functionality

### Development and Testing
- **libPrintf v6.2.0**: Embedded printf library eliminating nanofp complexity (20KB+ binary savings)
- **AUnit v1.7.1**: Arduino unit testing framework with HIL integration (22 comprehensive tests)

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
│   └── TimerPWM/          # Hardware timer PWM for servo/ESC control
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
./system/ci/aflash.sh tests/LittleFS_Unit_Tests --use-rtt --build-id
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
- **✅ Complete**: IMU library (high-level wrapper with chip detection, context-based design, interrupt support)
- **✅ Complete**: ICM42688P library (low-level TDK drivers with self-test and data acquisition)
- **✅ Complete**: SerialRx library (IBus hardware validated, SBUS implemented, software idle detection)
- **✅ Complete**: Betaflight Config Converter (Python tool with PeripheralPins.c validation, ALT variant handling, 53 passing tests)
- **📋 Future**: Additional IMU sensor support (MPU-6000, MPU-9250), CRSF protocol support

## Documentation

See `CLAUDE.md` for detailed build instructions, architecture overview, and development guidelines.

This repository is being collaboratively developed with [Claude Code](https://claude.ai/code) for enhanced STM32 Arduino development workflows.