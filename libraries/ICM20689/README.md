# ICM20689 Arduino Library

Production-validated ICM-206xx family IMU library derived from Betaflight.

## Supported Chips

This library supports multiple chips in the ICM-206xx family:
- **ICM-20601** (WHO_AM_I: 0xAC)
- **ICM-20602** (WHO_AM_I: 0x12) ⭐ Primary target
- **ICM-20608** (WHO_AM_I: 0xAF)
- **ICM-20689** (WHO_AM_I: 0x98)

All chips share the same register interface and are supported by a single driver.

## Features

- **6-axis IMU** - Gyroscope + Accelerometer
- **Production-validated initialization** - Betaflight reset sequences and timing delays preserved
- **Auto-detection** - Supports all 4 chip variants via WHO_AM_I
- **Configurable DLPF** - Digital Low-Pass Filter settings
- **Configurable FSR** - Full-Scale Range for gyro and accelerometer
- **SPI interface** - Up to 8 MHz SPI clock
- **Arduino-compatible API** - Clean interface with standard types

## License

**GPL v3** - This library is derived from Betaflight and inherits its GPL v3 license.

### GPL Containment Strategy

This library is **separate and optional**. The core Arduino_Core_STM32 remains MIT licensed.

- ✅ Use this library if you need ICM-206xx support
- ✅ Exclude this library if GPL is incompatible with your project
- ✅ The unified `IMU` wrapper (MIT) provides interface abstraction
- ✅ Your code depends on the `IMU.h` interface only, not GPL implementation

## Hardware Setup

### Typical Wiring (SPI)
```
ICM-20602 → STM32
-----------------
VCC  → 3.3V
GND  → GND
SCK  → SPI_SCK
MISO → SPI_MISO
MOSI → SPI_MOSI
CS   → GPIO (your choice)
```

### Example: NUCLEO_F411RE
```
SCK  → PA5 (SPI1_SCK)
MISO → PA6 (SPI1_MISO)
MOSI → PA7 (SPI1_MOSI)
CS   → PA4 (GPIO)
```

## API Reference

### Initialization

```cpp
#include <ICM20689.h>

ICM20689 imu;

// Initialize with SPI bus, CS pin, and frequency
bool success = imu.begin(SPI, PA4, 1000000);  // 1 MHz for detection
```

### Chip Detection

```cpp
uint8_t whoAmI = imu.whoAmI();
// Returns: 0x12 (ICM-20602), 0xAC (ICM-20601), 0xAF (ICM-20608), 0x98 (ICM-20689)

ChipVariant chip = imu.getChipVariant();
// Returns: ICM20601, ICM20602, ICM20608, or ICM20689
```

### Data Reading

```cpp
float gx, gy, gz;  // Gyroscope (degrees/second)
float ax, ay, az;  // Accelerometer (g)

// Read gyro only
bool success = imu.readGyro(gx, gy, gz);

// Read accel only
bool success = imu.readAccel(ax, ay, az);

// Read both (6-axis)
bool success = imu.read6DOF(gx, gy, gz, ax, ay, az);
```

### Configuration

```cpp
// Set Digital Low-Pass Filter
imu.setDLPF(0);  // 0=250Hz, 1=176Hz, 2=92Hz, 3=41Hz, 4=20Hz, 5=10Hz, 6=5Hz, 7=3600Hz

// Set Gyroscope Full-Scale Range
imu.setGyroFSR(2000);  // 250, 500, 1000, or 2000 dps

// Set Accelerometer Full-Scale Range
imu.setAccelFSR(16);   // 2, 4, 8, or 16 g
```

## Examples

### Basic Detection and 6DOF Reading

See `examples/ICM20689_Basic/ICM20689_Basic.ino`:

```cpp
#include <ICM20689.h>

ICM20689 imu;

void setup() {
  Serial.begin(115200);

  if (!imu.begin(SPI, PA4, 1000000)) {
    Serial.println("ICM-206xx initialization failed!");
    while (1);
  }

  Serial.print("Detected: ");
  Serial.println(imu.getChipName());
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

## Technical Details

### Chip Differences

All ICM-206xx chips use the same driver with these differences:

| Chip | WHO_AM_I | Gyro Range | Accel Range | Notes |
|------|----------|------------|-------------|-------|
| ICM-20601 | 0xAC | ±2000 dps | ±16g | Budget option |
| ICM-20602 | 0x12 | ±2000 dps | ±16g | ⭐ Popular in FCs |
| ICM-20608 | 0xAF | ±2000 dps | ±16g | Similar to 20602 |
| ICM-20689 | 0x98 | ±2000 dps | ±16g | Higher performance |

### Betaflight Driver Source

This library adapts Betaflight's `accgyro_spi_icm20689.c` driver:
- **Path:** `drivers/accgyro/accgyro_spi_icm20689.c`
- **Initialization:** Production-validated reset sequences with timing delays
- **Clock Selection:** PLL clock with 120µs settle time
- **SPI Speed:** Up to 8 MHz (tested safe limit)

### Initialization Sequence

1. **Reset Device** - PWR_MGMT_1 BIT_RESET + 100ms delay
2. **WHO_AM_I Detection** - Verify chip variant (0x12, 0xAC, 0xAF, or 0x98)
3. **Disable I2C** - USER_CTRL I2C_IF_DIS
4. **Signal Path Reset** - ACCEL_RST + TEMP_RST + 100ms delay
5. **Clock Selection** - PWR_MGMT_1 INV_CLK_PLL + 120µs settle
6. **Configure FSR** - GYRO_CONFIG (±2000 dps), ACCEL_CONFIG (±16g)
7. **Configure DLPF** - CONFIG register bandwidth setting
8. **Interrupt Setup** - INT_PIN_CFG, INT_ENABLE (data ready)

## References

- **Betaflight Source:** https://github.com/betaflight/betaflight
- **ICM-20602 Datasheet:** https://invensense.tdk.com/products/motion-tracking/6-axis/icm-20602/
- **ICM-20689 Datasheet:** https://invensense.tdk.com/products/motion-tracking/6-axis/icm-20689/
