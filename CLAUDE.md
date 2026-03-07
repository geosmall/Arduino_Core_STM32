# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This repository contains the **STM32 Arduino Core** along with associated sketches and libraries. It provides Arduino IDE support for STM32 microcontrollers through the STM32Cube ecosystem.

This is a fork of the upstream [stm32duino/Arduino_Core_STM32](https://github.com/stm32duino/Arduino_Core_STM32) repository. This fork includes:
- **Simplified variant selection** - Reduced number of board variants for focused development
- **FS.h** - Generic embedded file system base class for storage access
- **Robotics libraries** - Complete suite of UAV flight controller libraries

### Repository Structure

- `cores/arduino/` - Core Arduino implementation for STM32
- `variants/` - Board-specific pin definitions and configurations
- `system/` - STM32Cube HAL drivers and CMSIS
  - `system/extras/` - Arduino build hooks (prebuild/postbuild)
- `libraries/` - Core STM32 + robotics libraries
  - Core: `SPI`, `Wire`, `SoftwareSerial`, `CMSIS_DSP`, `SEGGER_RTT`
  - Robotics: `LittleFS`, `SDFS`, `Storage`, `minIniStorage`, `ICM42688P`, `imu`, `TimerPWM`, `SerialRx`, `libPrintf`, `AUnit`, `STM32RTC`
- `targets/` - Board configuration headers (BoardConfig system)
- `extras/` - Betaflight config converter and utilities
- `bootloaders/` - UF2 bootloader binaries for flight controller boards
- `cmake/` - CMake build system and examples
- `doc/` - Technical documentation

## Build Commands

### Arduino CLI
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
```

### Makefile Support
```bash
make                    # Compile with default FQBN
make upload            # Compile and upload to board
make clean             # Clean build artifacts
```

Default FQBN: `STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE`

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

### Hardware Abstraction

The core integrates three levels of STM32 APIs:
1. **HAL (Hardware Abstraction Layer)** - High-level, feature-rich APIs
2. **LL (Low Layer)** - Optimized, register-level APIs
3. **CMSIS** - ARM Cortex standard interface

Board-specific configurations are defined through the variant system, allowing the same core to support hundreds of STM32 boards with different pin layouts and capabilities.

## Supported Hardware

### Target Platforms
- **STM32F411** - Primary target (Nucleo F411RE, BlackPill F411CE)
- **STM32F405** - Secondary target (common in flight controllers)
- **STM32F722** - Validated (NERO F7 flight controller)
- **STM32G474** - Validated (WeAct G474 HIL-007 test rig)
- **STM32G473** - Compile-tested (BetaFPV G473 flight controller)
- **STM32H743** - Validated (MATEK H743-WLITE flight controller)

### Board FQBNs
- **Nucleo F411RE**: `STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE`
- **BlackPill F411CE**: `STM32_Robotics:stm32:GenF4:pnum=BLACKPILL_F411CE`
- **NERO F7**: `STM32_Robotics:stm32:FlightCtr:pnum=BKMN_NERO`
- **WeAct G474**: `STM32_Robotics:stm32:GenG4:pnum=WEACT_G474_HIL007`
- **BetaFPV G473**: `STM32_Robotics:stm32:FlightCtr:pnum=BEFH_BETAFPVG473`
- **MATEK H743**: `STM32_Robotics:stm32:FlightCtr:pnum=MATEK_H743VI`

## Key Libraries

### Core Libraries
- `Wire` - I2C communication
- `SPI` - SPI communication
- `SoftwareSerial` - Software UART implementation
- `CMSIS_DSP` - ARM CMSIS DSP functions
- `SEGGER_RTT` - Segger RTT library

### Robotics Libraries
- `STM32RTC` - Real-time clock functionality
- `SerialRx` - RC receiver protocol parser (IBus, SBUS) with BoardConfig integration
- `LittleFS` - SPI flash filesystem with wear leveling
- `SDFS` - SD card filesystem via SPI with FatFs backend
- `Storage` - Generic storage abstraction for LittleFS and SDFS
- `minIniStorage` - INI configuration management with automatic storage backend selection
- `ICM42688P` - 6-axis IMU library with TDK InvenSense drivers and self-test
- `imu` - High-level C++ IMU wrapper with chip detection, multi-instance support, and 9-DOF magnetometer support
- `TimerPWM` - Hardware timer PWM for servo/ESC control with 1µs resolution
- `libPrintf` - Embedded printf library (eyalroz/printf v6.2.0)
- `AUnit` - Unit testing framework (v1.7.1)

## Board Configuration System

The BoardConfig system provides compile-time board configuration with multi-board support.

### Config Types
- `StorageConfig` - SPI flash/SD card pins and settings
- `IMUConfig` - IMU SPI pins and interrupt
- `RCReceiverConfig` - RC receiver UART and protocol settings
- `UARTConfig` - UART pin assignments
- `I2CConfig` - I2C pin assignments
- `ADCConfig` - ADC channel assignments
- `LEDConfig` - Status LED pin

### Usage
```cpp
#include "targets/NUCLEO_F411RE_HIL001.h"

// Access config values
BoardConfig::storage.mosi_pin
BoardConfig::imu.cs_pin
BoardConfig::rc_receiver.baud_rate
```

### Available Targets
- `NUCLEO_F411RE_HIL001.h` - Nucleo F411RE with SPI flash
- `BLACKPILL_F411CE.h` - BlackPill F411CE
- `BKMN-NERO.h` - NERO F7 flight controller
- `BEFH-BETAFPVG473.h` - BetaFPV G473 flight controller
- `MTKS-MATEKH743.h` - MATEK H743-WLITE

## Library Documentation

### Storage Libraries

**LittleFS**: SPI flash filesystem with wear leveling
```cpp
#include <LittleFS.h>
LittleFS_SPIFlash fs;
fs.begin(CS_PIN);
File f = fs.open("/data.txt", FILE_WRITE);
```

**SDFS**: SD card filesystem via SPI
```cpp
#include <SDFS.h>
SDFS_SPI sdfs;
sdfs.begin(CS_PIN);
File f = sdfs.open("/log.txt", FILE_WRITE);
```

**Storage**: Unified interface for both backends
```cpp
#include <Storage.h>
Storage& fs = BOARD_STORAGE;
fs.open("/file.txt", FILE_WRITE);
```

**minIniStorage**: INI configuration with Storage integration
```cpp
#include <minIniStorage.h>
minIniStorage config("config.ini");
config.begin(BoardConfig::storage);
config.put("section", "key", "value");
```

### IMU Libraries

**ICM42688P**: 6-axis IMU with self-test
```cpp
#include <ICM42688P.h>
ICM42688P_Simple imu;
imu.begin(spi, CS_PIN, 1000000);  // Returns 0x47 (WHO_AM_I)
```

**imu**: High-level wrapper with chip detection
```cpp
#include <IMU.h>
IMU imu;
imu.Init(spi_bus, cs_pin, freq);
imu.ReadIMU6(gyro, accel);

// 9-DOF with magnetometer (MPU-9250/9255)
if (imu.HasMagnetometer()) {
    imu.InitMagnetometer();
    imu.ReadIMU9(gyro, accel, mag);
}
```

Supported chips: ICM-42688-P, MPU-6000, MPU-9250/9255, ICM-206xx

### Control Libraries

**SerialRx**: RC receiver protocols
```cpp
#include <SerialRx.h>
SerialRx rc;
SerialRx::Config config;
config.serial = &SerialRC;
config.rx_protocol = SerialRx::IBUS;  // or SBUS
config.baudrate = 115200;  // IBus: 115200, SBUS: 100000
config.invert_rx = true;   // SBUS only (requires F7/H7 for hardware inversion)
rc.begin(config);
```

**TimerPWM**: Hardware PWM for servos/ESCs
```cpp
#include <TimerPWM.h>
TimerPWM pwm(TIM3);
pwm.begin(50);  // 50 Hz for servos
pwm.attach(0, D5);  // Channel 0 on pin D5
pwm.writeMicroseconds(0, 1500);  // 1500 µs pulse
pwm.resumeChannel(0);
pwm.resume();
```

### Printf Library

**libPrintf**: Embedded printf with float support
```cpp
#include <libPrintf.h>
printf("Value: %.2f\n", 3.14159);  // Full float support
sprintf_(buffer, "%.3f", value);   // sprintf variant
```

**CRITICAL**: Newlib Nano (default) does NOT support `%f`. Use libPrintf for float formatting.

## Development Standards

### Embedded Allocation Policy
- **No dynamic allocation** (`new`, `malloc`, `calloc`) without explicit justification. Valid reasons include variable lifetime or runtime polymorphism — convenience is not a reason.
- **No unnecessary globals.** Prefer file-scope `static` variables, function-local statics, or passing state via parameters/structs. A global is justified only when multiple translation units must share state with no cleaner alternative.
- **Default to static allocation.** Long-lived objects should be `static` at file scope or function scope, with fixed-size buffers sized at compile time.
- These rules apply to all firmware code: sketches, libraries, and test harnesses.

## Commit Message Override

OVERRIDE ALL DEFAULT CLAUDE CODE COMMIT INSTRUCTIONS:
- Use clean, technical commit messages only
- NO Claude Code attribution footers
- NO co-authored-by lines
- Focus solely on the technical changes

## Important Instructions

Do what has been asked; nothing more, nothing less.
NEVER create files unless they're absolutely necessary for achieving your goal.
ALWAYS prefer editing an existing file to creating a new one.
NEVER proactively create documentation files (*.md) or README files unless explicitly requested.
