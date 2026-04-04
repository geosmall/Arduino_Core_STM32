# XENSIV™ DPS3xx Digital Pressure Sensor Library

Arduino library for Infineon's XENSIV™ Digital Pressure Sensors (DPS310, DPS368).

> **Note**: This library is derived from [Infineon/arduino-xensiv-dps3xx](https://github.com/Infineon/arduino-xensiv-dps3xx) v1.0.2 by Infineon Technologies AG (MIT License).

## Supported Sensors

| Sensor | Product ID | Status |
|--------|------------|--------|
| DPS310 | 0x00 | Deprecated (still supported) |
| DPS368 | 0x01 | Active |

## Hardware Setup

### Typical I2C Wiring

| Sensor Pin | STM32 Pin | Description |
|------------|-----------|-------------|
| VIN/VDD | 3.3V | Power supply |
| GND | GND | Ground |
| SDA | D14 (PB9) | I2C Data |
| SCL | D15 (PB8) | I2C Clock |

**Default I2C Address**: `0x77` (can be changed to `0x76` via SDO pin)

### Pin Configuration (NUCLEO_F411RE)

The default Wire instance uses Arduino pins D14/D15 which map to:
- **D14** → PB9 (SDA)
- **D15** → PB8 (SCL)

As defined in `cores/arduino/pins_arduino.h`:
```cpp
#define PIN_WIRE_SDA  14  // D14
#define PIN_WIRE_SCL  15  // D15
```

## Basic Usage

### Using Default Wire

```cpp
#include <Dps3xx.h>

Dps3xx sensor;

void setup() {
    Serial.begin(115200);
    sensor.begin(Wire);  // Uses default I2C pins

    // Read temperature and pressure
    float temp, pressure;
    sensor.measureTempOnce(temp, 7);      // Oversampling = 7
    sensor.measurePressureOnce(pressure, 7);

    Serial.printf("Temp: %.2f C, Pressure: %.2f Pa\n", temp, pressure);
}
```

### Using Custom TwoWire Instance

For boards using BoardConfig or custom pin assignments:

```cpp
#include <Dps3xx.h>
#include <Wire.h>

// Custom I2C using BoardConfig pins
TwoWire baroWire(BoardConfig::sensors.sda_pin, BoardConfig::sensors.scl_pin);
Dps3xx sensor;

void setup() {
    baroWire.begin();
    baroWire.setClock(400000);  // 400 kHz
    sensor.begin(baroWire);
}
```

## Register 0x0D: Product and Revision ID

**Critical Information**: Understanding register 0x0D is essential for proper device detection.

### Register Structure

| Bits | Field | Description |
|------|-------|-------------|
| 7:4 | REV_ID | Revision ID (upper nibble) |
| 3:0 | PROD_ID | Product ID (lower nibble) |

### Expected Values

| Value | Meaning |
|-------|---------|
| Raw register = `0x10` | REV_ID=0x01, PROD_ID=0x00 (DPS310) |
| Raw register = `0x11` | REV_ID=0x01, PROD_ID=0x01 (DPS368) |

### Important: PROD_ID = 0x00 is CORRECT for DPS310

The DPS310 Product ID is **defined as 0x00** in the Infineon datasheet (page 36). This is NOT an error condition.

**Common Mistake**: Assuming `productId == 0` means "device not found". This is incorrect!

**Correct Detection Logic**:
```cpp
uint8_t productId = sensor.getProductId();   // Returns 0x00 for DPS310
uint8_t revisionId = sensor.getRevisionId(); // Returns 0x01 typically

// WRONG: if (productId == 0) { error(); }
// CORRECT:
if (revisionId == 0 && productId == 0) {
    // Both IDs zero = no device or communication failure
    Serial.println("ERROR: Device not detected");
} else {
    // Valid device (PROD_ID=0 is expected for DPS310)
    Serial.printf("DPS3xx detected: PROD_ID=0x%02X, REV_ID=0x%02X\n",
                  productId, revisionId);
}
```

### Library Bitfield Extraction

The library extracts PROD_ID and REV_ID from register 0x0D using bitfield masks defined in `dps3xx_config.h`:

```cpp
// Register 0x0D bitfield definitions
PROD_ID: {0x0D, 0x0F, 0}  // Mask 0x0F, shift 0 (bits 3:0)
REV_ID:  {0x0D, 0xF0, 4}  // Mask 0xF0, shift 4 (bits 7:4)
```

When raw register reads `0x10`:
- `PROD_ID = (0x10 & 0x0F) >> 0 = 0x00`
- `REV_ID = (0x10 & 0xF0) >> 4 = 0x01`

## I2C Debugging

### Quick I2C Scanner

To verify sensor presence before initialization:

```cpp
Wire.begin();
Wire.beginTransmission(0x77);
uint8_t error = Wire.endTransmission();
if (error == 0) {
    Serial.println("DPS3xx found at 0x77");
} else {
    Serial.println("No device at 0x77");
}
```

### Direct Register Read

To read raw register 0x0D without library:

```cpp
Wire.beginTransmission(0x77);
Wire.write(0x0D);  // PROD_ID register
Wire.endTransmission(false);  // Repeated start
Wire.requestFrom((uint8_t)0x77, (uint8_t)1);
if (Wire.available()) {
    uint8_t rawValue = Wire.read();
    Serial.printf("Raw register 0x0D: 0x%02X\n", rawValue);
    Serial.printf("  PROD_ID: 0x%02X\n", rawValue & 0x0F);
    Serial.printf("  REV_ID: 0x%02X\n", (rawValue >> 4) & 0x0F);
}
```

## Examples

| Example | Description |
|---------|-------------|
| `i2c_command` | Basic I2C usage with continuous measurement |
| `i2c_command_CI` | CI/HIL test with BoardConfig and validation |
| `i2c_interrupt` | Interrupt-driven measurement |
| `i2c_background` | Background measurement mode |
| `spi_command` | SPI interface usage |

### CI Test Example

The `i2c_command_CI` example provides comprehensive hardware validation:

```bash
# Build and run on NUCLEO_F411RE
./system/ci/aflash.sh libraries/xensiv-dps3xx/examples/i2c_command_CI --use-rtt --build-id
```

**Test Output**:
```
=== DPS3xx i2c_command_CI ===
READY NUCLEO_F411RE 56108c5f2 2025-12-04T19:55:24Z

Board Configuration:
  Target: NUCLEO_F411RE
  Baro I2C SDA: 0x000E
  Baro I2C SCL: 0x000F

Verifying I2C device at 0x77...
  I2C probe result: 0 (0=found)
Reading PROD_ID register (0x0D) directly...
  Raw PROD_ID: 0x10
Library getProductId(): 0x00 (expected 0x00 for DPS3xx)
Library getRevisionId(): 0x01
PASS: DPS3xx detected (Product ID: 0x00, Revision: 0x01)

=== Measurements ===
Temperature: 20.19 C
PASS: Temperature in valid range
Pressure: 100193.29 Pa (1001.93 hPa)
PASS: Pressure in valid range

=== Summary ===
Tests passed: 4
Tests failed: 0

*TEST_PASS*
```

## Measurement Ranges

| Parameter | Min | Max | Unit |
|-----------|-----|-----|------|
| Temperature | -40 | 85 | °C |
| Pressure | 30,000 | 110,000 | Pa |

## BoardConfig Integration

For targets using the BoardConfig system:

```cpp
// Include target-specific config
#if defined(ARDUINO_NUCLEO_F411RE)
  #include "targets/NUCLEO_F411RE.h"
  #define BARO_SDA BoardConfig::sensors.sda_pin
  #define BARO_SCL BoardConfig::sensors.scl_pin
#endif

TwoWire baroWire(BARO_SDA, BARO_SCL);
```

## References

- [Infineon DPS310 Datasheet](https://www.infineon.com/dgdl/Infineon-DPS310-DataSheet-v01_02-EN.pdf) - Register 0x0D on page 36
- [Infineon DPS368 Datasheet](https://www.infineon.com/dgdl/Infineon-DPS368-DataSheet-v01_00-EN.pdf)
- [Original Arduino Library](https://github.com/Infineon/arduino-xensiv-dps3xx)

## License

See [LICENSE.md](LICENSE.md) for details.
