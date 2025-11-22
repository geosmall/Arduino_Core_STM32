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
- **DeviceBase abstraction**: Polymorphic device handling
- **SPI/I2C support**: Bus abstraction layer (DeviceBus)
- **Shared register maps**: DRY principle for MPU-family devices
- **Hardware validated**: Tested on NUCLEO_F411RE, BlackPill F411CE, NERO F7

## Examples

See `examples/` directory:
- **AutoDetect_Single**: Facade with auto-detection
- **Polled_FlightController**: 2kHz polling loop (dRehmFlight pattern)
- **Interrupt_DataReady**: INT1 interrupt-driven
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

## Contributing

This is a fork of Arduino_Core_STM32 with integrated robotics libraries. Contributions should maintain GPL compliance for Betaflight-derived code.
