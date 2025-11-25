# IMU Library - Magnetometer Support Implementation Plan

## Status: Core Implementation Complete ✅ | Examples & Docs Pending 📋

---

## Objective
Add 9-DOF magnetometer support to the IMU library wrapper for MPU-9250/9255 compatibility, enabling magnetometer-based yaw estimation in dRehmFlight port.

---

## Implementation Summary

### ✅ COMPLETED - Core Implementation (2025-01-25)

All core magnetometer functionality has been implemented and **compilation tested successfully**.

#### 1. DeviceBase.h - Virtual Magnetometer API ✅
**File**: `libraries/imu/src/devices/DeviceBase.h`

**Added Methods**:
- `virtual bool hasMagnetometer() const` - Device capability detection
- `virtual bool initMagnetometer()` - AK8963 initialization
- `virtual bool readMagnetometer(float* mag)` - Read mag data (µT)
- `virtual bool read9DOF(int16_t* accgyr, float* mag)` - Combined 9-axis read
- `virtual bool calibrateMagnetometer()` - Figure-8 calibration
- `virtual void setMagCalibration(...)` - Apply calibration values
- `virtual void getMagCalibration(...) const` - Retrieve calibration

**Default Behavior**: All methods return false or no-op for chips without magnetometer (graceful degradation).

---

#### 2. IMU.h - Public Magnetometer API ✅
**File**: `libraries/imu/src/devices/IMU.h`

**Added Public Methods**:
```cpp
bool HasMagnetometer() const;
Result InitMagnetometer();
Result ReadMagnetometer(float& mx, float& my, float& mz);
Result ReadIMU9(std::array<int16_t, 3>& gyro_buf,
                std::array<int16_t, 3>& accel_buf,
                std::array<float, 3>& mag_buf);
int getMotion9(int16_t* ax, int16_t* ay, int16_t* az,
               int16_t* gx, int16_t* gy, int16_t* gz,
               float* mx, float* my, float* mz);
Result CalibrateMagnetometer();
void SetMagCalibration(float bias_x, float bias_y, float bias_z,
                       float scale_x, float scale_y, float scale_z);
void GetMagCalibration(float& bias_x, float& bias_y, float& bias_z,
                       float& scale_x, float& scale_y, float& scale_z) const;
```

**Usage Pattern**:
```cpp
IMU imu;
if (imu.HasMagnetometer()) {
    imu.InitMagnetometer();
    // Use 9-DOF fusion
} else {
    // Fall back to 6-DOF fusion
}
```

---

#### 3. IMU.cpp - Wrapper Implementation ✅
**File**: `libraries/imu/src/IMU.cpp`

**All methods implemented** with:
- Null/initialization checks
- Chip capability detection via `driver_->hasMagnetometer()`
- Delegation to device driver
- Graceful degradation (return ERR for chips without mag)

---

#### 4. MPU9250.h - Driver Header Updates ✅
**File**: `libraries/imu/src/devices/MPU9250.h`

**Added**:
- `hasMagnetometer() const override { return true; }` - Capability flag
- Override declarations for all magnetometer virtual methods
- Private member variables:
  - `bool mag_initialized_`
  - `float mag_scale_x/y/z_` - ASA factory calibration
  - `float mag_bias_x/y/z_` - Hard iron correction
  - `float mag_scale_factor_x/y/z_` - Soft iron correction
- Private helper methods:
  - `bool writeAK8963Register(uint8_t reg, uint8_t value)`
  - `bool readAK8963Registers(uint8_t reg, uint8_t count, uint8_t *dest)`
  - `uint8_t whoAmIAK8963()`

---

#### 5. MPU9250.cpp - Full AK8963 Implementation ✅
**File**: `libraries/imu/src/devices/MPU9250.cpp`

**Constructor Updated**:
- Initialize all magnetometer member variables
- Changed from bypass mode to I2C master mode preparation

**Key Implementations**:

##### `initMagnetometer()` - I2C Master Mode Initialization
1. Disable I2C bypass mode
2. Enable I2C master mode (`BIT_I2C_MST_EN`)
3. Configure I2C master clock (400 kHz)
4. Power down magnetometer
5. Soft reset magnetometer
6. Verify AK8963 WHO_AM_I (0x48)
7. Read ASA calibration values from fuse ROM
8. Calculate sensitivity scale factors
9. Set continuous measurement mode 2 (100 Hz, 16-bit)
10. Configure auto-read (7 bytes into EXT_SENS_DATA registers)

##### `readMagnetometer()` - Efficient Data Reading
- Read 7 bytes from EXT_SENS_DATA_00 to _06 (auto-populated by I2C master)
- Check ST2 overflow flag
- Extract LSB-first raw data (HXL, HXH, HYL, HYH, HZL, HZH)
- Apply ASA scale factors
- Convert to µT (0.15 µT/LSB for 16-bit mode)
- Apply hard iron (bias) and soft iron (scale) calibration

##### `read9DOF()` - Combined Read
- Call `read()` for 6-axis data
- Call `readMagnetometer()` for mag data
- Single efficient read operation

##### `calibrateMagnetometer()` - Figure-8 Calibration
- Collect 1500 samples over 15 seconds (100 Hz)
- Track min/max for each axis
- Calculate hard iron bias (sphere center)
- Calculate soft iron scale factors (normalize to average radius)
- User performs figure-8 motion during calibration

##### Calibration Management
- `setMagCalibration()` - Store calibration values
- `getMagCalibration()` - Retrieve calibration values

---

#### 6. MPU_Common.h - I2C Master Register Definitions ✅
**File**: `libraries/imu/src/devices/MPU_Common.h`

**Added Registers**:
```c
#define MPU_RA_I2C_MST_CTRL     0x24    // I2C Master Control
#define MPU_RA_I2C_SLV0_ADDR    0x25    // I2C Slave 0 Address
#define MPU_RA_I2C_SLV0_REG     0x26    // I2C Slave 0 Register
#define MPU_RA_I2C_SLV0_CTRL    0x27    // I2C Slave 0 Control
#define MPU_RA_I2C_SLV0_DO      0x63    // I2C Slave 0 Data Out
#define MPU_RA_EXT_SENS_DATA_00 0x49    // External Sensor Data

#define BIT_I2C_MST_EN          0x20    // Enable I2C master mode
#define BIT_BYPASS_EN           0x02    // Enable I2C bypass mode
```

---

#### 7. ak8963.h - AK8963 Register Definitions ✅
**File**: `libraries/imu/src/devices/ak8963.h` (copied from libraries/MPU9250)

**Includes**:
- Register addresses (WHO_AM_I, CNTL1, CNTL2, HXL-HZH, ST1, ST2, ASAX-ASAZ)
- Mode definitions (power down, continuous 100Hz, fuse ROM access)
- Status flags (data ready, overflow)
- I2C master control flags
- Sensitivity scale factor (0.15 µT/LSB for 16-bit mode)

---

#### 8. Compilation Test ✅
**Test Command**: `./system/ci/build.sh libraries/imu/examples/Interrupt_DataReady --env-check`

**Result**: ✅ **SUCCESS**
- Binary size: 37724 bytes (7% of 512KB flash)
- No compilation errors
- No warnings
- Ready for hardware testing

---

## 📋 REMAINING TASKS

### 1. Create Example Sketches

#### A. imu-9dof-basic Example
**Location**: `libraries/imu/examples/IMU_9DOF_Basic/IMU_9DOF_Basic.ino`

**Purpose**: Demonstrate basic 9-axis data reading with chip detection

**Key Features**:
- Auto-detect chip type (MPU-9250 vs others)
- If MPU-9250: Initialize magnetometer and read 9-DOF data
- If other: Fall back to 6-DOF data
- Display gyro, accel, and mag data
- BoardConfig integration
- CI/HIL integration with RTT

**Structure**:
```cpp
#include <IMU.h>
#include <ci_log.h>
#include <libPrintf.h>

void setup() {
  // Init IMU
  if (imu.HasMagnetometer()) {
    imu.InitMagnetometer();
    CI_LOG("9-DOF mode (with magnetometer)\n");
  } else {
    CI_LOG("6-DOF mode (no magnetometer)\n");
  }
}

void loop() {
  if (imu.HasMagnetometer()) {
    // Read 9-DOF data
    imu.ReadIMU9(gyro, accel, mag);
    // Display all axes
  } else {
    // Read 6-DOF data
    imu.ReadIMU6(data);
    // Display gyro + accel only
  }
}
```

---

#### B. imu-mag-calibration Example
**Location**: `libraries/imu/examples/IMU_Mag_Calibration/IMU_Mag_Calibration.ino`

**Purpose**: Interactive magnetometer calibration workflow

**Key Features**:
- Check for magnetometer support
- Guide user through figure-8 calibration (15 seconds)
- Display calibration values (bias + scale factors)
- Show corrected vs uncorrected mag readings
- Option to save calibration values (display for manual storage)

**Workflow**:
1. Detect MPU-9250
2. Initialize magnetometer
3. Prompt user to start calibration
4. Collect samples during figure-8 motion
5. Display calibration results
6. Demonstrate corrected readings

---

### 2. Update Documentation

#### A. libraries/imu/README.md
**Updates Needed**:
- Add "Magnetometer Support (MPU-9250/9255)" section
- Document 9-DOF API methods
- Add usage examples
- Note chip compatibility (MPU-9250: yes, others: no)
- Document calibration workflow

**Sections to Add**:
```markdown
## Magnetometer Support (9-DOF)

The IMU library supports 9-axis operation for MPU-9250/9255 chips with built-in AK8963 magnetometer.

### Chip Compatibility
- ✅ MPU-9250 (WHO_AM_I: 0x71) - 9-DOF support
- ✅ MPU-9255 (WHO_AM_I: 0x73) - 9-DOF support
- ❌ ICM-42688-P, MPU-6000, ICM-206xx - 6-DOF only

### API Usage
```cpp
IMU imu;
imu.Init(SPI, cs_pin, freq);

if (imu.HasMagnetometer()) {
    imu.InitMagnetometer();

    // Read 9-axis data
    std::array<int16_t, 3> gyro, accel;
    std::array<float, 3> mag;
    imu.ReadIMU9(gyro, accel, mag);

    // Calibration
    imu.CalibrateMagnetometer();  // Figure-8 motion
}
```

### Magnetometer Calibration
Required for accurate compass/heading:
1. Call `CalibrateMagnetometer()`
2. Perform figure-8 motion for 15 seconds
3. Store calibration values
4. Apply with `SetMagCalibration()` on next power-up
```
```

---

#### B. CLAUDE.md
**Updates Needed**:
- Update "IMU Libraries" section with magnetometer status
- Add to "Completed Projects" list
- Document 9-DOF vs 6-DOF capability per chip

**Example Update**:
```markdown
### IMU Libraries ✅ **COMPLETED**

**IMU (High-Level Wrapper)**: Unified C++ wrapper for InvenSense IMUs with chip detection
- Multi-instance support, ChipType enum (ICM42688_P, MPU-6000, MPU-9250)
- **9-DOF Support**: MPU-9250/9255 with AK8963 magnetometer
- **6-DOF Support**: ICM-42688-P, MPU-6000, ICM-206xx
- Full API: Init, Reset, ReadIMU6, ReadIMU9, ReadMagnetometer
- Magnetometer calibration (figure-8 method)
- Usage: `IMU imu; if (imu.HasMagnetometer()) { imu.InitMagnetometer(); imu.ReadIMU9(...); }`
```

---

### 3. Hardware Validation with MPU-9250

**Prerequisites**:
- MPU-9250 or MPU-9255 breakout board
- NUCLEO_F411RE or BLACKPILL_F411CE
- SPI connections (MOSI, MISO, SCK, CS)

**Validation Steps**:

#### Step 1: WHO_AM_I Detection
```bash
./system/ci/aflash.sh libraries/imu/examples/AutoDetect_Single --use-rtt --build-id
```
**Expected**: WHO_AM_I: 0x71 (MPU-9250) or 0x73 (MPU-9255)

#### Step 2: AK8963 Detection
**Expected**: `initMagnetometer()` returns true, AK8963 WHO_AM_I: 0x48

#### Step 3: Raw Magnetometer Reading
**Test**: Read raw mag data (before calibration)
**Expected**: Values in range ±4912 µT, non-zero readings

#### Step 4: Magnetometer Calibration
**Test**: Run calibration example, perform figure-8 motion
**Expected**:
- Bias values calculated (hard iron)
- Scale factors calculated (soft iron)
- Corrected readings show spherical distribution

#### Step 5: 9-DOF Data Streaming
**Test**: Run 9-DOF basic example
**Expected**: Simultaneous gyro, accel, mag data at configured rate

#### Step 6: dRehmFlight Integration
**Test**: Port dRehmFlight to use IMU library with magnetometer
**Expected**: Madgwick9DOF filter receives mag data for yaw estimation

---

## Technical Details

### I2C Master Mode vs Bypass Mode

**Implemented Approach: I2C Master Mode** (matches libraries/MPU9250 and dRehmFlight Teensy)

**Advantages**:
- Single SPI transaction reads all 9 axes (efficient)
- Automatic magnetometer data population (no per-read overhead)
- Proven approach from Betaflight/dRehmFlight
- Magnetometer updates at gyro sample rate

**How It Works**:
1. MPU-9250 acts as I2C master to AK8963
2. Configure slave 0 for auto-read (7 bytes from AK8963)
3. MPU-9250 automatically reads mag data and stores in EXT_SENS_DATA registers
4. Host reads all data via single SPI burst read

**Rejected Approach: Bypass Mode**
- Would require separate I2C bus from STM32 to AK8963
- More complex wiring
- Slower (separate transactions)
- Not used by production flight controllers

---

### Magnetometer Data Format

**AK8963 Raw Data**:
- 16-bit mode: ±4912 µT range, 0.15 µT/LSB
- LSB-first byte order (HXL, HXH, HYL, HYH, HZL, HZH)
- ST2 register: Overflow flag (bit 3)

**Processing Pipeline**:
1. Read raw 16-bit values
2. Apply ASA factory sensitivity adjustment
3. Convert to µT (multiply by 0.15)
4. Apply hard iron correction (subtract bias)
5. Apply soft iron correction (multiply by scale factor)

**Calibration Values**:
- Hard iron bias: Sphere center offset (µT)
- Soft iron scale: Axis-specific scale factors (normalize ellipsoid to sphere)

---

### Integration with dRehmFlight

**Current dRehmFlight Port Status** (from earlier investigation):
- Uses ICM42688P (6-DOF) via IMU library
- Madgwick6DOF filter for attitude estimation
- **No magnetometer support** (yaw drifts over time)

**With MPU-9250 + Magnetometer**:
```cpp
if (imu.HasMagnetometer()) {
    imu.InitMagnetometer();
    // Load stored calibration
    imu.SetMagCalibration(bias_x, bias_y, bias_z, scale_x, scale_y, scale_z);

    // In main loop:
    imu.ReadIMU9(gyro, accel, mag);
    Madgwick9DOF(gyro[0], gyro[1], gyro[2],
                 accel[0], accel[1], accel[2],
                 mag[0], mag[1], mag[2], dt);
} else {
    // 6-DOF fallback (current behavior)
    imu.ReadIMU6(data);
    Madgwick6DOF(...);
}
```

---

## Files Modified

**Modified Files** (7 total):
1. `libraries/imu/src/IMU.h` - Public API declarations
2. `libraries/imu/src/IMU.cpp` - Wrapper implementations
3. `libraries/imu/src/devices/DeviceBase.h` - Virtual method declarations
4. `libraries/imu/src/devices/MPU9250.h` - Driver declarations + member variables
5. `libraries/imu/src/devices/MPU9250.cpp` - Full AK8963 implementation
6. `libraries/imu/src/devices/MPU_Common.h` - I2C master register definitions
7. `libraries/imu/src/devices/ak8963.h` - AK8963 register definitions (new file)

**Compilation Status**: ✅ All files compile successfully

---

## Next Steps for Continuation

1. **Create IMU_9DOF_Basic example** - Copy structure from Polled_FlightController, add magnetometer reading
2. **Create IMU_Mag_Calibration example** - Interactive calibration workflow
3. **Update README.md** - Document magnetometer API and usage
4. **Update CLAUDE.md** - Add to completed projects
5. **Hardware validation** - Test with actual MPU-9250 hardware
6. **Commit and push** - Preserve implementation

---

## References

- **dRehmFlight Teensy**: `sketches/dRehmFlight_Teensy_BETA_1.3/src/MPU9250/` - Bolder Flight Systems implementation
- **libraries/MPU9250**: Betaflight-derived standalone library with full AK8963 support
- **AK8963 Datasheet**: TDK InvenSense AK8963 3-axis magnetometer specifications
- **MPU-9250 Datasheet**: InvenSense 9-DOF IMU with I2C master interface

---

**Implementation Date**: 2025-01-25
**Status**: Core complete, examples and docs pending
**Token Usage**: ~100K tokens used, implementation preserved in this plan
