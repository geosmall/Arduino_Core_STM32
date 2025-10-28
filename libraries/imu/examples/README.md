# IMU Library Examples

This folder contains examples demonstrating different approaches to reading data from the ICM-42688-P IMU sensor.

## Available Examples

### 1. imu-raw-data-registers (Interrupt-Driven)
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

### 2. imu-polled-basic (Polling-Based)
**Use Case**: Fixed-rate control loops like flight controllers

**Features**:
- Simple polling without interrupt pin
- Reads data at fixed rate in main loop
- No ISR overhead
- No hardware interrupt pin required

**When to Use**:
- Fixed-rate control loops (e.g., 2kHz flight controller)
- When loop rate matches or is close to IMU ODR
- Simpler setup for deterministic timing
- When interrupt pin is not available

## Performance Comparison

### Hardware Validation Results

Both examples were tested on NUCLEO_F411RE with ICM-42688-P at 1kHz ODR.

**Sample Data Comparison (First 35 Samples)**:

```
Sample  | INTERRUPT-DRIVEN              | POLLED                        | Analysis
--------|-------------------------------|-------------------------------|----------
5       | Accel[-29,225,8703]          | Accel[-26,172,8627]          |
        | Gyro[-32768,-32768,-32768]    | Gyro[-32768,-32768,-32768]   | Settling
--------|-------------------------------|-------------------------------|----------
10      | Accel[-41,186,8532]          | Accel[-54,182,8556]          |
        | Gyro[-32768,-32768,-32768]    | Gyro[-32768,-32768,-32768]   | Settling
--------|-------------------------------|-------------------------------|----------
15      | Accel[-56,176,8518]          | Accel[-36,178,8533]          |
        | Gyro[1,-13,33]                | Gyro[-3,-26,-21]             | 1st Valid
--------|-------------------------------|-------------------------------|----------
20      | Accel[-42,193,8515]          | Accel[-43,181,8502]          |
        | Gyro[5,-14,4]                 | Gyro[6,-14,6]                | Stable
--------|-------------------------------|-------------------------------|----------
25      | Accel[-40,192,8500]          | Accel[-48,189,8496]          |
        | Gyro[5,-14,2]                 | Gyro[4,-14,4]                | Stable
--------|-------------------------------|-------------------------------|----------
30      | Accel[-45,185,8492]          | Accel[-46,186,8499]          |
        | Gyro[6,-15,4]                 | Gyro[5,-13,4]                | Stable
--------|-------------------------------|-------------------------------|----------
35      | Accel[-46,191,8496]          | Accel[-50,174,8483]          |
        | Gyro[6,-13,4]                 | Gyro[6,-13,4]                | Stable
```

### Key Findings

**✅ Functionally Equivalent Performance**:
- Both methods show identical settling behavior (~10-15 samples)
- Data quality is equivalent (within ±1-2 counts for gyro)
- Independent X/Y/Z axis values confirmed
- Same noise characteristics for stationary sensor

**✅ Settling Time**:
- Samples 1-10: Gyro saturated at -32768 (IMU power-on settling)
- Sample 15: First valid gyro data appears
- Samples 20+: Fully stable readings

**✅ Steady-State Performance** (Samples 20-35):
- Gyro drift: ±0.04 to ±0.11 deg/sec (both methods)
- Accelerometer: Z-axis ≈8500 counts ≈ 1g (gravity, as expected)
- Noise levels: Comparable between both methods

## Polling Considerations for Flight Controllers

### When Polling Works Best

**Ideal Scenario** (dRehmFlight-style):
- Main loop: 2000 Hz (500µs period)
- IMU ODR: 2000 Hz
- Result: Nearly 1:1 correspondence, minimal duplicate/missed reads

**Acceptable Scenarios**:
- Loop rate slightly faster than ODR: Some duplicate reads (filtered out by control loop)
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

## Example Output

Both examples produce identical output format for easy comparison:

```
=== IMU Library - [Mode] Data Example ===
Build: a1951c086-dirty (2025-10-28T16:46:50Z)
Pin Configuration (BoardConfig):
  CS: 194, MOSI: 198, MISO: 199, SCLK: 207
  SPI Speed: 1000000 Hz

Initializing IMU...
✓ IMU initialized successfully
Detected chip: ICM-42688-P (0x47)

Configuring IMU...
✓ IMU configured for [interrupt-driven/polled] operation
  Accel ODR: 1kHz, Gyro ODR: 1kHz

Collecting 100 samples...

Sample 5: Accel[   -44,   207,  8654] Gyro[-32768,-32768,-32768]
Sample 10: Accel[   -32,   177,  8558] Gyro[-32768,-32768,-32768]
Sample 15: Accel[   -41,   186,  8525] Gyro[     0,   -27,   -19]
Sample 20: Accel[   -39,   181,  8510] Gyro[     7,   -13,     6]
...
Sample 100: Accel[   -34,   204,  8510] Gyro[     6,   -13,     4]

✓ Data collection complete
```

## Building and Testing

### Arduino IDE
```bash
# Open either example in Arduino IDE
# Select: Tools > Board > STM32 Boards > Nucleo-64
# Select: Tools > Board part number > Nucleo F411RE
# Upload to board
```

### Command Line (arduino-cli)
```bash
# Interrupt-driven example
./system/ci/build.sh libraries/imu/examples/imu-raw-data-registers --use-rtt --build-id
./system/ci/aflash.sh libraries/imu/examples/imu-raw-data-registers --use-rtt

# Polled example
./system/ci/build.sh libraries/imu/examples/imu-polled-basic --use-rtt --build-id
./system/ci/aflash.sh libraries/imu/examples/imu-polled-basic --use-rtt
```

## Hardware Setup

### Required Hardware
- NUCLEO_F411RE or BLACKPILL_F411CE development board
- ICM-42688-P breakout board (e.g., Adafruit 4264)
- Breadboard and jumper wires

### Wiring (BoardConfig Automatic)

The examples use BoardConfig for automatic pin assignment:

**NUCLEO_F411RE** (using JHEF411 config):
```
ICM-42688-P  →  NUCLEO_F411RE
-----------     --------------
VIN          →  3.3V
GND          →  GND
SCL          →  PA5 (SPI1_SCK)
SDA (MOSI)   →  PA7 (SPI1_MOSI)
SDO (MISO)   →  PA6 (SPI1_MISO)
CS           →  PA4 (Software CS)
INT1         →  PC4 (interrupt example only)
```

**Note**: Interrupt pin (INT1) is only required for `imu-raw-data-registers` example.

## References

- **ICM-42688-P Datasheet**: `doc/ds-000347_icm-42688-p-datasheet.pdf`
- **Application Note**: TDK InvenSense AN-000157 (IMU Configuration)
- **Board Configuration**: `targets/NUCLEO_F411RE_JHEF411.h`
- **Flight Controller Example**: `sketches/dRehmFlight_STM32_BETA_1.3/` (uses polling)

## License

Same as Arduino_Core_STM32 (BSD-style, see repository root)
