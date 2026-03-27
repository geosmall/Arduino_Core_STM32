# STM32 Robotics Arduino Core

A focused STM32 Arduino core for robotics and flight controller development.

## Installation via Arduino Board Manager

1. Open **Arduino IDE**
2. Go to **File → Preferences**
3. Add this URL to "Additional Boards Manager URLs":
   ```
   https://github.com/geosmall/BoardManagerFiles/raw/main/package_stm32_robotics_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for **"STM32 Robotics"**
6. Click **Install**

## Overview

This is a **fork of the STM32 Arduino Core** optimized for robotics applications including drone flight controllers, real-time data logging, and sensor management.

## Supported Hardware

| Category | Boards |
|----------|--------|
| Development | Nucleo F411RE, BlackPill F411CE, WeAct G474 |
| Flight Controllers | NOXE V3 (F411), OpenPilot Revo (F405), NERO F7 (F722), BetaFPV G473, MATEK H743 |

## Key Features

- **Robotics Libraries**: IMU sensors, RC receivers, servo/ESC control, storage systems
- **UF2 Bootloader Support**: Drag-and-drop firmware upload without debugger
- **Storage Systems**: LittleFS (SPI flash), SDFS (SD card) with unified API
- **IMU Support**: ICM-42688-P, MPU-6000, MPU-9250/9255 with magnetometer calibration
- **RC Protocols**: IBus, SBUS, and CRSF with hardware validation
- **PWM Control**: Hardware timer PWM for servos/ESCs with 1µs resolution

## Quick Start

### Compile and Upload

```cpp
// Blink example
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
```

1. Select board: **Tools → Board → STM32 Robotics → Nucleo 64 → NUCLEO_F411RE**
2. Select port: **Tools → Port → [your port]**
3. Click **Upload**

## UF2 Bootloader Support

UF2 bootloaders enable drag-and-drop firmware upload without a debugger.

### Supported Boards

| Board | MCU | Bootloader |
|-------|-----|------------|
| NOXE V3 | STM32F411CE | `bootuf2-noxe_v3-v2.0.0.bin` |
| OpenPilot Revo | STM32F405RG | `bootuf2-revo_f405-v2.0.0.bin` |
| NERO F7 | STM32F722RE | `bootuf2-nero_f7-v2.0.0.bin` |
| BetaFPV G473 | STM32G473CE | `bootuf2-betafpv_g473-v2.0.0.bin` |
| MATEK H743 | STM32H743VI | `bootuf2-matek_h743-v2.0.0.bin` |

Bootloader binaries are in `bootloaders/`.

### Initial Bootloader Installation

**Via Arduino IDE (easiest):**

1. Select board: **Tools → Board → STM32 Robotics → FlightCtr → [your board]**
2. Select programmer: **Tools → Programmer → DFU** (or J-Link/ST-Link)
3. For DFU: Hold BOOT button while pressing RESET
4. Click: **Tools → Burn Bootloader**

**Via DFU command line:**

```bash
# Enter DFU mode: Hold BOOT while pressing RESET
dfu-util -a 0 -s 0x08000000:leave -D bootloaders/bootuf2-noxe_v3-v2.0.0.bin
```

### Uploading via UF2

After bootloader is installed:

1. **Double-tap reset** - LED pulses, USB drive appears (only applies to dev boards with reset buttons)
2. **Drag firmware.uf2** to the drive, or use Arduino IDE with **UF2 Bootloader** upload method

## Libraries

### Storage and Configuration
- **LittleFS** - SPI flash filesystem with wear leveling
- **SDFS** - SD card filesystem via SPI
- **Storage** - Unified storage abstraction
- **minIniStorage** - INI configuration management

### Sensors
- **IMU** - High-level wrapper for InvenSense IMUs (ICM-42688-P, MPU-6000, MPU-9250, ICM-206xx)
- **xensiv-dps3xx** - DPS310/DPS368 barometric pressure sensor
- **TinyGPSPlus** - NMEA GPS parser
- **ms4525do** - MS4525DO airspeed sensor

### Control
- **SerialRx** - RC receiver protocols (IBus, SBUS, CRSF)
- **TimerPWM** - Hardware PWM for servos/ESCs (1µs resolution)
- **Scheduler** - INav-based cooperative multitasking

### Core
- **SPI**, **Wire**, **SoftwareSerial** - Communication
- **SEGGER_RTT** - Real-time debugging
- **libPrintf** - Embedded printf (20KB+ smaller than newlib)
- **STM32RTC** - Real-time clock
- **CMSIS_DSP** - ARM DSP functions
- **EmbeddedCLI** - Command-line interface
- **DShot** - DShot digital motor protocol (150/300/600/1200)
- **AUnit** - Unit testing framework
- **PrecompLib** - CRC-16 utilities

## Project Structure

```
├── cores/arduino/         # Arduino core implementation
├── variants/              # Board-specific pin definitions
├── libraries/             # Robotics libraries (22 libraries)
├── bootloaders/           # UF2 bootloader binaries
├── targets/               # Board configuration headers
├── extras/
│   ├── betaflight_converter/  # Betaflight → BoardConfig converter
│   ├── dfu-util/              # DFU upload utility (multi-platform)
│   └── uf2conv/               # UF2 format converter
└── doc/                   # Technical documentation
```

## Development Status

- **✅ Complete**: Storage (LittleFS, SDFS), IMU library (6-DOF/9-DOF), SerialRx (IBus, SBUS, CRSF), TimerPWM, DShot
- **✅ Complete**: UF2 bootloaders for 5 flight controller boards

## Resources

- **Repository**: [geosmall/Arduino_Core_STM32](https://github.com/geosmall/Arduino_Core_STM32)
- **Upstream**: [stm32duino/Arduino_Core_STM32](https://github.com/stm32duino/Arduino_Core_STM32)
- **Board Manager**: [geosmall/BoardManagerFiles](https://github.com/geosmall/BoardManagerFiles)

---

This repository is collaboratively developed with [Claude Code](https://claude.ai/code).
