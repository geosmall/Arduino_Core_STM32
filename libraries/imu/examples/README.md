# IMU Library Examples

This folder contains examples demonstrating the IMU_Driver facade library for STM32 Arduino.

## User-Facing Examples

### 1. AutoDetect_Single - Facade with Auto-Detection
**Use Case**: High-level IMU abstraction for multi-device support

**Features**:
- Auto-detects IMU chip type (ICM42688P, MPU6000, MPU9250, ICM206xx families)
- Unified API across all supported IMUs
- BoardConfig integration for multi-board support (NUCLEO_F411RE, BlackPill F411CE, NERO F7)
- Streaming data output with performance metrics
- Simple facade pattern - single include, automatic device detection

**Supported IMUs**:
- ICM-42688-P (0x47), ICM-42605 (0x42), IIM-42653 (0x56)
- MPU-6000 (0x68), MPU-9250 (0x71), MPU-9255 (0x73)
- ICM-20601 (0xAC), ICM-20602 (0x12), ICM-20689 (0x98)

**When to Use**:
- Flight controller applications
- Robotics projects requiring portable IMU access
- Any application that may use different IMU hardware
- When you want simple, automatic IMU detection

**Build Commands**:
```bash
# NUCLEO_F411RE (default)
./system/ci/aflash.sh libraries/imu/examples/AutoDetect_Single --use-rtt --build-id

# BlackPill F411CE
./system/ci/aflash.sh libraries/imu/examples/AutoDetect_Single \
  STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE --use-rtt --build-id

# NERO F7 Flight Controller
./system/ci/aflash.sh libraries/imu/examples/AutoDetect_Single \
  STMicroelectronics:stm32:FlightCtr:pnum=BKMN_NERO --use-rtt --build-id
```

---

### 2. Polled_FlightController - Polled Loop Example
**Use Case**: Fixed-rate control loops like dRehmFlight (2kHz polling)

**Features**:
- Demonstrates polling-based IMU data acquisition
- Simulates MPU-6000 DLPF behavior on ICM-42688-P hardware
- Configuration: Gyro ±250 DPS @ 4kHz, Accel ±2G @ 1kHz
- AAF: Gyro 258 Hz, Accel 170 Hz (MPU-6000 DLPF parity)
- No interrupt pin required
- Matches dRehmFlight flight controller pattern

**When to Use**:
- Fixed-rate control loops (e.g., 2kHz flight controller)
- When loop rate matches or is close to IMU ODR
- Simpler setup for deterministic timing
- When interrupt pin is not available
- Learning how dRehmFlight uses the IMU

**Polling Methodology**:
- Loop rate: 2kHz continuous polling (common flight controller rate)
- Higher ODRs (4k) provide oversampling opportunity
- Matches dRehmFlight's approach to IMU data acquisition

**Build Commands**:
```bash
./system/ci/aflash.sh libraries/imu/examples/Polled_FlightController --use-rtt --build-id
```

---

### 3. Interrupt_DataReady - Interrupt-Driven Example
**Use Case**: Event-driven data acquisition, CPU-efficient for variable-rate loops

**Features**:
- Uses hardware interrupt pin (INT1) for data-ready signaling
- Reads data only when new samples are available
- Most CPU-efficient approach
- Requires interrupt pin connection

**When to Use**:
- Variable loop rate applications
- Power-sensitive applications
- When you want to read data only when available
- When interrupt pin is available

**Build Commands**:
```bash
./system/ci/aflash.sh libraries/imu/examples/Interrupt_DataReady --use-rtt --build-id
```

---

### 4. SelfTest - Self-Test Example
**Use Case**: IMU hardware validation using manufacturer self-test

**Features**:
- Demonstrates manufacturer self-test integration
- Uses TDK InvenSense factory algorithms (100% preserved)
- Validates IMU hardware functionality
- Reports gyro/accel self-test results and bias values

**Supported IMUs**:
- ✅ ICM-42688-P (fully supported)
- ❌ MPU-6000, MPU-9250, ICM-206xx (not yet implemented)

**When to Use**:
- Initial hardware validation (ICM-42688-P only)
- Production testing
- Troubleshooting IMU hardware issues
- Learning how self-test works

**Build Commands**:
```bash
./system/ci/aflash.sh libraries/imu/examples/SelfTest --use-rtt --build-id
```

**Note**: Currently only ICM-42688-P is supported. The example will detect other IMU types but exit with an error message.

---

## Development Examples

Low-level driver tests and legacy examples are located in the `dev/` subfolder:

### Driver Hardware Validation Tests
- **Test_ICM42688_Direct** - ICM42688 Betaflight driver validation (NUCLEO_F411RE)
- **Test_MPU6000_Direct** - MPU6000 Betaflight driver validation (NUCLEO_F411RE)
- **Test_MPU9250_Direct** - MPU9250 Betaflight driver validation (BlackPill F411CE)
- **Test_ICM206xx_Direct** - ICM206xx Betaflight driver validation (NUCLEO_F411RE, NERO F7)
- **Test_BusOnly** - Low-level SPI bus testing

### Legacy Examples
- **imu-polled-bf** - Legacy Betaflight-style config example (wide FSR, 8kHz ODR)

These are primarily for driver development and hardware validation. Most users should use the top-level examples instead.

---

## Performance Comparison

### Facade Performance (AutoDetect_Single)
Hardware-validated read rates on actual hardware:

| IMU | Board | CPU | Read Rate |
|-----|-------|-----|-----------|
| ICM-42688-P | NUCLEO F411RE | 100 MHz | 34,939 Hz |
| MPU-6000 | NUCLEO F411RE | 100 MHz | 30,532 Hz |
| MPU-9250 | BlackPill F411CE | 100 MHz | 30,941 Hz |
| ICM-20602 | NERO F7 | 216 MHz | 28,231 Hz |

### Polling vs Interrupt-Driven (ICM-42688-P @ 1kHz ODR)

Both methods were tested on NUCLEO_F411RE and show equivalent data quality:

**✅ Functionally Equivalent Performance**:
- Both methods show identical settling behavior (~10-15 samples)
- Data quality is equivalent (within ±1-2 counts for gyro)
- Same noise characteristics for stationary sensor

**✅ Settling Time**:
- Samples 1-10: Gyro saturated at -32768 (IMU power-on settling)
- Sample 15: First valid gyro data appears
- Samples 20+: Fully stable readings

**✅ Steady-State Performance** (Samples 20+):
- Gyro drift: ±0.04 to ±0.11 deg/sec (both methods)
- Accelerometer: Z-axis ≈8500 counts ≈ 1g (gravity, as expected)

---

## Polling Considerations for Flight Controllers

### When Polling Works Best

**Ideal Scenario** (dRehmFlight-style):
- Main loop: 2000 Hz (500µs period)
- IMU ODR: 2000 Hz (or 4000 Hz for oversampling)
- Result: Nearly 1:1 correspondence, minimal duplicate/missed reads

**Acceptable Scenarios**:
- Loop rate slightly faster than ODR: Some duplicate reads (filtered by control loop)
- Loop rate slightly slower than ODR: Some missed samples (acceptable for control stability)

### Timing Budget Example (2kHz Loop)

```
Total period: 500µs
├─ SPI Read (1MHz, 12 bytes): ~100µs
├─ Data processing: ~50µs
├─ Madgwick filter: ~100µs
├─ PID control: ~100µs
├─ Motor commands: ~50µs
└─ Margin: ~100µs
```

---

## Hardware Setup

### Required Hardware
- Development board: NUCLEO_F411RE, BLACKPILL_F411CE, or NERO F7
- IMU breakout board (any supported chip)
- Breadboard and jumper wires (for dev boards)

### Wiring (BoardConfig Automatic)

The examples use BoardConfig for automatic pin assignment:

**NUCLEO_F411RE** (JHEF411 config):
```
IMU          →  NUCLEO_F411RE
-----------     --------------
VIN          →  3.3V
GND          →  GND
SCL          →  PA5 (SPI1_SCK)
MOSI         →  PA7 (SPI1_MOSI)
MISO         →  PA6 (SPI1_MISO)
CS           →  PA4 (Software CS)
INT1         →  PC4 (interrupt example only)
```

**BLACKPILL_F411CE**:
```
IMU          →  BLACKPILL_F411CE
-----------     ----------------
VIN          →  3.3V
GND          →  GND
SCL          →  PB13 (SPI2_SCK)
MOSI         →  PB15 (SPI2_MOSI)
MISO         →  PB14 (SPI2_MISO)
CS           →  PB12 (Software CS)
```

**NERO F7** (integrated):
- ICM-20602 on SPI1 (PA7/PA6/PA5/PC4)
- No external wiring required

**Note**: Interrupt pin (INT1) is only required for `Interrupt_DataReady` example.

---

## Example Code Patterns

### Facade Pattern (AutoDetect_Single)
```cpp
#include <IMU_Driver.h>
#include <SPI.h>

SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin,
                 BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin,
                 BoardConfig::imu.spi.get_ssel_pin());
IMU_Driver imu;

void setup() {
  spi_bus.begin();
  imu.attachSPI(spi_bus, BoardConfig::imu.spi.cs_pin,
                BoardConfig::imu.spi.freq_hz);

  if (!imu.begin(ImuType::Auto)) {
    CI_LOG("*FAIL* IMU detection failed\n");
    while(1);
  }

  CI_LOGF("Detected: %s\n", imu.typeName());
}

void loop() {
  ImuSample sample;
  if (imu.read(sample)) {
    // sample.ax, sample.ay, sample.az (m/s²)
    // sample.gx, sample.gy, sample.gz (rad/s)
  }
}
```

### Polling Pattern (Polled_FlightController)
```cpp
#include <IMU.h>
#include <SPI.h>

SPIClass spi_bus(...);
IMU imu;

void setup() {
  spi_bus.begin();
  imu.Init(spi_bus, cs_pin, spi_freq);

  // Configure for MPU-6000 parity
  imu.SetGyroFsr(IMU::GFS_250DPS);
  imu.SetAccelFsr(IMU::AFS_2G);
  imu.SetGyroODR(IMU::GODR_4000Hz);
  imu.SetAccelODR(IMU::AODR_1000Hz);
}

void loop() {
  static uint32_t last_read = micros();

  // 2kHz polling loop (500µs period)
  if (micros() - last_read >= 500) {
    int16_t data[6];
    imu.ReadIMU6(data);
    last_read = micros();

    // Process data...
  }
}
```

### Interrupt Pattern (Interrupt_DataReady)
```cpp
#include <IMU.h>
#include <SPI.h>

volatile bool data_ready = false;

void imu_data_ready_handler() {
  data_ready = true;
}

void setup() {
  // ... IMU init ...

  // Configure INT1 for data-ready
  pinMode(interrupt_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interrupt_pin),
                  imu_data_ready_handler, RISING);
}

void loop() {
  if (data_ready) {
    data_ready = false;
    int16_t data[6];
    imu.ReadIMU6(data);

    // Process data...
  }
}
```

---

## References

- **IMU_Driver Library**: `libraries/imu/src/IMU_Driver.h` (facade)
- **IMU Library**: `libraries/imu/src/IMU.h` (wrapper)
- **Device Drivers**: `libraries/imu/src/devices/` (Betaflight drivers)
- **Board Configurations**: `targets/` directory
- **Refactor Status**: `libraries/imu/REFACTOR_STATUS.md`
- **Flight Controller Example**: `sketches/dRehmFlight_STM32_BETA_1.3/` (uses polling)

---

## License

Same as Arduino_Core_STM32 (BSD-style, see repository root)
