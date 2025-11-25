# IMU Library for Arduino_Core_STM32

Multi-device IMU library with Betaflight-quality drivers and automatic device detection.

## Supported IMUs

- **ICM42688 family**: ICM-42688-P (0x47), ICM-42605 (0x42), IIM-42653 (0x56)
- **MPU6000**: MPU-6000 (0x68)
- **MPU9250 family**: MPU-9250 (0x71), MPU-9255 (0x73)
- **ICM206xx family**: ICM-20601 (0xAC), ICM-20602 (0x12), ICM-20689 (0x98)

## Features

- **Auto-detection**: Automatic IMU chip identification
- **Betaflight drivers**: Flight-tested, high-quality IMU drivers
- **9-DOF support**: Magnetometer support for MPU-9250/9255 (AK8963)
- **DeviceBase abstraction**: Polymorphic device handling
- **SPI/I2C support**: Bus abstraction layer (DeviceBus)
- **Shared register maps**: DRY principle for MPU-family devices
- **Hardware validated**: Tested on NUCLEO_F411RE, BlackPill F411CE, NERO F7

## Examples

See `examples/` directory:
- **AutoDetect_Single**: Facade with auto-detection
- **Polled_FlightController**: 2kHz polling loop (dRehmFlight pattern)
- **Interrupt_DataReady**: INT1 interrupt-driven
- **IMU_9DOF_Basic**: 9-axis data reading with automatic 6-DOF/9-DOF detection
- **IMU_Mag_Calibration**: Interactive magnetometer calibration (figure-8 method)
- **SelfTest**: Manufacturer self-test (ICM-42688-P only)

## Documentation

- **REFACTOR_STATUS.md**: Complete implementation details and validation results
- **MADFLIGHT_COMPARISON.md**: Architectural comparison with madflight
- **examples/README.md**: Comprehensive example documentation

## License

**GNU General Public License v3 (GPLv3)**

This library contains code derived from [Betaflight](https://github.com/betaflight/betaflight), which is licensed under GPLv3. The IMU drivers are based on Betaflight's flight-tested accgyro drivers and therefore inherit the GPL license.

**Key Points**:
- This IMU library is GPL-licensed due to Betaflight derivation
- The Arduino_Core_STM32 core itself remains LGPL v2.1
- Applications using this IMU library must comply with GPLv3 terms
- See `LICENSE` file for full license text

**Attribution**:
- Original code: Betaflight contributors (GPLv3)
- Integration pattern: Madflight (MIT) - factory pattern approach
- Arduino port: geosmall (2024)

## Usage

```cpp
#include <IMU_Driver.h>
#include <SPI.h>

SPIClass spi_bus(MOSI, MISO, SCK);
IMU_Driver imu;

void setup() {
  spi_bus.begin();
  imu.attachSPI(spi_bus, CS_PIN, SPI_FREQ);

  if (!imu.begin(ImuType::Auto)) {
    Serial.println("IMU detection failed!");
    while(1);
  }

  Serial.print("Detected: ");
  Serial.println(imu.typeName());
}

void loop() {
  ImuSample sample;
  if (imu.read(sample)) {
    // Use sample.ax, sample.ay, sample.az (m/s²)
    // Use sample.gx, sample.gy, sample.gz (rad/s)
  }
}
```

## Magnetometer Support (9-DOF)

The IMU library supports 9-axis operation for MPU-9250/9255 chips with built-in AK8963 magnetometer.

### Chip Compatibility

| Chip | WHO_AM_I | Gyro+Accel | Magnetometer |
|------|----------|------------|--------------|
| MPU-9250 | 0x71 | ✅ 6-DOF | ✅ 9-DOF (AK8963) |
| MPU-9255 | 0x73 | ✅ 6-DOF | ✅ 9-DOF (AK8963) |
| ICM-42688-P | 0x47 | ✅ 6-DOF | ❌ No magnetometer |
| MPU-6000 | 0x68 | ✅ 6-DOF | ❌ No magnetometer |
| ICM-206xx | Various | ✅ 6-DOF | ❌ No magnetometer |

### API Usage

```cpp
#include <IMU.h>
#include <SPI.h>

IMU imu;

void setup() {
  // Initialize IMU
  imu.Init(SPI, cs_pin, spi_freq);

  // Check if magnetometer is available
  if (imu.HasMagnetometer()) {
    // Initialize magnetometer (MPU-9250/9255 only)
    imu.InitMagnetometer();

    // Optional: Apply stored calibration
    imu.SetMagCalibration(bias_x, bias_y, bias_z,
                          scale_x, scale_y, scale_z);
  }
}

void loop() {
  if (imu.HasMagnetometer()) {
    // Read 9-axis data
    std::array<int16_t, 3> gyro, accel;
    std::array<float, 3> mag;  // µT (microtesla)
    imu.ReadIMU9(gyro, accel, mag);

    // Use for 9-DOF sensor fusion (e.g., Madgwick9DOF)
  } else {
    // Fall back to 6-DOF
    std::array<int16_t, 6> data;
    imu.ReadIMU6(data);
  }
}
```

### Magnetometer Calibration

Magnetometer calibration is required for accurate compass/heading measurements.

**Method**: Figure-8 calibration
- Collects 1500 samples over 15 seconds
- Calculates hard iron bias (sphere center offset)
- Calculates soft iron scale factors (normalize ellipsoid to sphere)

**Procedure**:
```cpp
// Run calibration
imu.CalibrateMagnetometer();  // User performs figure-8 motion

// Retrieve calibration values
float bias_x, bias_y, bias_z;
float scale_x, scale_y, scale_z;
imu.GetMagCalibration(bias_x, bias_y, bias_z,
                      scale_x, scale_y, scale_z);

// Store values (EEPROM/Flash) for next power-up
// Apply on startup:
imu.SetMagCalibration(bias_x, bias_y, bias_z,
                      scale_x, scale_y, scale_z);
```

**Examples**:
- `IMU_9DOF_Basic`: Demonstrates automatic 6-DOF/9-DOF detection
- `IMU_Mag_Calibration`: Interactive calibration workflow

### Technical Details

**AK8963 Magnetometer** (MPU-9250/9255):
- 3-axis magnetometer
- ±4912 µT range
- 16-bit resolution (0.15 µT/LSB)
- 100 Hz continuous measurement mode
- I2C master mode (efficient auto-read)

**Data Flow**:
1. MPU-9250 I2C master reads AK8963 automatically
2. Mag data appears in EXT_SENS_DATA registers
3. Single SPI burst read gets all 9 axes efficiently

## Contributing

This is a fork of Arduino_Core_STM32 with integrated robotics libraries. Contributions should maintain GPL compliance for Betaflight-derived code.
