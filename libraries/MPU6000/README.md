# MPU6000 - Arduino Library for InvenSense MPU-6000 IMU

**License:** GPL v3 (Betaflight-derived)
**IMU Chip:** MPU-6000 (6-axis gyroscope + accelerometer)
**Interface:** SPI
**WHO_AM_I:** 0x68

## Overview

Production-validated Arduino library for the InvenSense MPU-6000 6-axis IMU. This is a direct adaptation of Betaflight's MPU-6000 driver, preserving all the hard-won initialization sequences and register configurations.

## Features

- **SPI Interface** - Hardware SPI communication
- **6-axis Data** - 3-axis gyroscope + 3-axis accelerometer
- **Configurable DLPF** - 7 bandwidth settings (5Hz to 256Hz)
- **Configurable FSR** - Gyro: ±250/500/1000/2000 °/s, Accel: ±2/4/8/16g
- **Production-validated** - Betaflight initialization sequences preserved
- **WHO_AM_I detection** - With PRODUCT_ID verification

## Hardware Requirements

- MPU-6000 IMU breakout board or flight controller
- SPI connection (MOSI, MISO, SCK, CS)
- 3.3V power supply

## Installation

This library is included with the Arduino_Core_STM32. No separate installation needed.

## Quick Start

```cpp
#include <MPU6000.h>

MPU6000 imu;

void setup() {
  Serial.begin(115200);

  if (!imu.begin(SPI, PA4, 1000000)) {
    Serial.println("MPU-6000 initialization failed!");
    while(1);
  }

  Serial.print("MPU-6000 detected, WHO_AM_I: 0x");
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
- `bool begin(SPIClass &spi, uint8_t cs_pin, uint32_t freq = 1000000)` - Initialize MPU-6000
- `uint8_t whoAmI()` - Read WHO_AM_I register (returns 0x68)

### Data Reading
- `bool readGyro(float &gx, float &gy, float &gz)` - Read gyroscope (°/s)
- `bool readAccel(float &ax, float &ay, float &az)` - Read accelerometer (g)
- `bool read6DOF(float &gx, float &gy, float &gz, float &ax, float &ay, float &az)` - Read both

### Configuration
- `void setDLPF(uint8_t dlpf_cfg)` - Set digital low-pass filter (0-7)
- `void setSampleRateDivider(uint8_t divider)` - Set sample rate divider (0-255)
- `void setGyroFSR(uint16_t fsr)` - Set gyro full-scale range (250, 500, 1000, 2000)
- `void setAccelFSR(uint8_t fsr)` - Set accel full-scale range (2, 4, 8, 16)

### DLPF Settings

The DLPF (Digital Low-Pass Filter) controls both **filter bandwidth** and **internal sample rate**:

| Code | Gyro BW | Accel BW | Delay | Internal Rate |
|------|---------|----------|-------|---------------|
| 0    | 256 Hz  | 260 Hz   | 0.98 ms | **8 kHz** |
| 1    | 188 Hz  | 184 Hz   | 1.9 ms | 1 kHz |
| 2    | 98 Hz   | 94 Hz    | 2.8 ms | 1 kHz |
| 3    | 42 Hz   | 44 Hz    | 4.8 ms | 1 kHz |
| 4    | 20 Hz   | 21 Hz    | 8.3 ms | 1 kHz |
| 5    | 10 Hz   | 10 Hz    | 13.4 ms | 1 kHz |
| 6    | 5 Hz    | 5 Hz     | 18.6 ms | 1 kHz |
| 7    | No filter | No filter | 0.17 ms | **8 kHz** |

### Sample Rate Configuration

The output sample rate is determined by the DLPF and sample rate divider:

```
Output Rate = Internal_Sample_Rate / (1 + divider)
```

**When DLPF enabled (dlpf_cfg 0-6):**
- Internal Rate = 1 kHz (except DLPF 0 = 8 kHz)
- `divider=0` → 1000 Hz
- `divider=1` → 500 Hz
- `divider=3` → 250 Hz
- `divider=9` → 100 Hz

**When DLPF disabled (dlpf_cfg 7):**
- Internal Rate = 8 kHz
- `divider=0` → 8000 Hz
- `divider=7` → 1000 Hz
- `divider=15` → 500 Hz

**Common Configurations:**

```cpp
// Flight controller - Betaflight style (1 kHz with minimal hardware filtering)
imu.setDLPF(0);                // 256 Hz bandwidth, 8 kHz internal
imu.setSampleRateDivider(7);   // 8 kHz / (1+7) = 1000 Hz output

// Flight controller - More hardware filtering (1 kHz with 98 Hz filter)
imu.setDLPF(2);                // 98 Hz bandwidth, 1 kHz internal
imu.setSampleRateDivider(0);   // 1 kHz / (1+0) = 1000 Hz output

// High-speed logging (8 kHz raw data)
imu.setDLPF(7);                // No filter, 8 kHz internal
imu.setSampleRateDivider(0);   // 8 kHz / (1+0) = 8000 Hz output

// Low-speed data logging (100 Hz with 42 Hz filter)
imu.setDLPF(3);                // 42 Hz bandwidth, 1 kHz internal
imu.setSampleRateDivider(9);   // 1 kHz / (1+9) = 100 Hz output
```

## Examples

### MPU6000_Basic
Basic polling-based example demonstrating:
- MPU-6000 detection via WHO_AM_I (0x68)
- 6-axis data reading at 10 Hz
- DLPF and FSR configuration

### MPU6000_Interrupt
Interrupt-driven example demonstrating:
- Data-ready interrupt configuration (1 kHz)
- High-speed continuous streaming
- BoardConfig integration for pin configuration
- Sample rate statistics and monitoring
- DLPF + divider configuration for precise 1 kHz output

**Hardware Setup:**
```
MPU-6000 → NUCLEO_F411RE
-------------------------
VCC  → 3.3V
GND  → GND
SCK  → PA5 (SPI1_SCK)
MISO → PA6 (SPI1_MISO)
MOSI → PA7 (SPI1_MOSI)
CS   → PA4 (GPIO)
INT  → PB3 (EXTI3)  ← Data-ready interrupt
```

## Integration with IMU Wrapper

This library integrates with the unified `IMU` wrapper for multi-chip support:

```cpp
#include <IMU.h>

IMU imu;
IMU::ChipType chip = imu.Init(SPI, PA4, 1000000);  // Auto-detects MPU-6000
```

## Betaflight Heritage

This code is directly adapted from:
- **Betaflight Project:** https://github.com/betaflight/betaflight
- **Source Files:** `src/main/drivers/accgyro/accgyro_spi_mpu6000.c`
- **Original Authors:** Dominic Clifton, John Ihlein, and Betaflight contributors

Key Betaflight features preserved:
- Reset and signal path reset sequences with proper delays
- Clock source configuration (PLL with Z-axis gyro reference)
- I2C interface disable for SPI-only operation
- Sample rate divider configuration
- Register write timing (15µs delays)
- WHO_AM_I + PRODUCT_ID validation

## References

- **MPU-6000 Datasheet:** [InvenSense/TDK Documentation](https://invensense.tdk.com/)
- **Register Map:** MPU-6000 Register Map and Descriptions Rev 4.2
- **Betaflight:** https://github.com/betaflight/betaflight
