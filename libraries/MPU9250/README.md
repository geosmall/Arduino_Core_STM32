# MPU9250 - Arduino Library for InvenSense MPU-9250 IMU

**License:** GPL v3 (Betaflight-derived)
**IMU Chip:** MPU-9250 / MPU-9255 (9-axis gyroscope + accelerometer + magnetometer)
**Interface:** SPI
**WHO_AM_I:** 0x71 (MPU-9250) or 0x73 (MPU-9255)

## ⚠️ License Notice

This library is licensed under **GNU General Public License v3.0** because it is derived from production-validated Betaflight flight controller code.

**What this means:**
- ✅ You can use this library freely in your projects
- ✅ You can modify and distribute this library
- ⚠️ If you distribute code that uses this library, you must also provide source code
- ⚠️ Your project that links to this library becomes GPL v3

**Alternatives:**
- If you need MIT licensing, consider using the ICM-42688-P library instead (MIT licensed)
- Or implement your own MPU-9250 driver from the datasheet

**Why GPL?**
- Production-quality code battle-tested in thousands of flight controllers worldwide
- All initialization quirks, write-verify patterns, and timing workarounds included
- Faster development vs clean-room implementation

## Overview

Production-validated Arduino library for the InvenSense MPU-9250 9-axis IMU. This is a direct adaptation of Betaflight's MPU-9250 driver, preserving all the hard-won initialization sequences, write-verify patterns, and register configurations.

**Hardware Validated:** Blackpill F411CE + MPU-9250 breakout board

## Features

- **SPI Interface** - Hardware SPI communication
- **9-axis Data** - 3-axis gyroscope + 3-axis accelerometer + 3-axis magnetometer (AK8963)
- **Write-Verify Pattern** - Robust initialization with automatic retry (up to 20 attempts)
- **Configurable DLPF** - 7 gyro + 8 accel bandwidth settings
- **Configurable FSR** - Gyro: ±250/500/1000/2000 °/s, Accel: ±2/4/8/16g
- **Production-validated** - Betaflight initialization sequences preserved
- **WHO_AM_I detection** - Supports both MPU-9250 (0x71) and MPU-9255 (0x73)

## Hardware Requirements

- MPU-9250 or MPU-9255 IMU breakout board or flight controller
- SPI connection (MOSI, MISO, SCK, CS)
- 3.3V power supply

## Installation

This library is included with the Arduino_Core_STM32. No separate installation needed.

## Quick Start

```cpp
#include <MPU9250.h>

MPU9250 imu;

void setup() {
  Serial.begin(115200);

  if (!imu.begin(SPI, PA4, 1000000)) {
    Serial.println("MPU-9250 initialization failed!");
    while(1);
  }

  Serial.print("MPU-9250 detected, WHO_AM_I: 0x");
  Serial.println(imu.whoAmI(), HEX);
}

void loop() {
  float gx, gy, gz, ax, ay, az;

  if (imu.read6DOF(gx, gy, gz, ax, ay, az)) {
    Serial.print("Gyro: ");
    Serial.print(gx); Serial.print(", ");
    Serial.print(gy); Serial.print(", ");
    Serial.println(gz);

    Serial.print("Accel: ");
    Serial.print(ax); Serial.print(", ");
    Serial.print(ay); Serial.print(", ");
    Serial.println(az);
  }

  delay(100);
}
```

## API Reference

### Initialization
- `bool begin(SPIClass &spi, uint8_t cs_pin, uint32_t freq = 1000000)` - Initialize MPU-9250
- `uint8_t whoAmI()` - Read WHO_AM_I register (returns 0x71 or 0x73)

### Data Reading
- `bool readGyro(float &gx, float &gy, float &gz)` - Read gyroscope (°/s)
- `bool readAccel(float &ax, float &ay, float &az)` - Read accelerometer (g)
- `bool read6DOF(float &gx, float &gy, float &gz, float &ax, float &ay, float &az)` - Read gyro + accel
- `bool readMag(float &mx, float &my, float &mz)` - Read magnetometer (µT) - **Future**

### Configuration
- `void setDLPF(uint8_t gyro_dlpf, uint8_t accel_dlpf)` - Set digital low-pass filters
- `void setGyroFSR(uint16_t fsr)` - Set gyro full-scale range (250, 500, 1000, 2000)
- `void setAccelFSR(uint8_t fsr)` - Set accel full-scale range (2, 4, 8, 16)

### DLPF Settings

**Gyro DLPF:**

| Code | Bandwidth | Delay | Sample Rate |
|------|-----------|-------|-------------|
| 0    | 250 Hz    | 0.97 ms | 8 kHz |
| 1    | 184 Hz    | 2.9 ms | 1 kHz |
| 2    | 92 Hz     | 3.9 ms | 1 kHz |
| 3    | 41 Hz     | 5.9 ms | 1 kHz |
| 4    | 20 Hz     | 9.9 ms | 1 kHz |
| 5    | 10 Hz     | 17.85 ms | 1 kHz |
| 6    | 5 Hz      | 33.48 ms | 1 kHz |
| 7    | 3600 Hz   | 0.17 ms | 8 kHz |

**Accel DLPF:**

| Code | Bandwidth | Delay |
|------|-----------|-------|
| 0    | 460 Hz    | 1.94 ms |
| 1    | 184 Hz    | 5.80 ms |
| 2    | 92 Hz     | 7.80 ms |
| 3    | 41 Hz     | 11.80 ms |
| 4    | 20 Hz     | 19.80 ms |
| 5    | 10 Hz     | 35.70 ms |
| 6    | 5 Hz      | 66.96 ms |
| 7    | 460 Hz    | 1.94 ms |

## Examples

- **MPU9250_Basic** - WHO_AM_I detection and 6-axis data reading

## Integration with IMU Wrapper

This library integrates with the unified `IMU` wrapper for multi-chip support:

```cpp
#include <IMU.h>

IMU imu;
IMU::ChipType chip = imu.Init(SPI, PA4, 1000000);  // Auto-detects MPU-9250
```

## Betaflight Heritage

This code is directly adapted from:
- **Betaflight Project:** https://github.com/betaflight/betaflight
- **Source Files:** `src/main/drivers/accgyro/accgyro_spi_mpu9250.c`
- **Original Authors:** Dominic Clifton, John Ihlein, Kalyn Doerr (RS2K), and Betaflight contributors

Key Betaflight features preserved:
- Write-verify pattern with retry loop (up to 20 attempts)
- Slow read/write with 1µs delays for register reliability
- WHO_AM_I detection with retry and 150ms delays
- Clock source configuration (PLL with internal oscillator)
- Sample rate divider configuration
- Magnetometer bypass enable for AK8963 access
- Supports both MPU-9250 (0x71) and MPU-9255 (0x73)

## References

- **MPU-9250 Datasheet:** [InvenSense/TDK Documentation](https://invensense.tdk.com/)
- **Register Map:** MPU-9250 Register Map and Descriptions
- **Betaflight:** https://github.com/betaflight/betaflight

## License

GNU General Public License v3.0 - See LICENSE file for full details.

This is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
