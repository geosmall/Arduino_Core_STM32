# IMU Library TDK Driver Migration Plan

**Status**: In Progress
**Last Updated**: 2025-11-22
**Tracking**: Phase 3.5 Complete (API cleanup + ICM-42688-P AAF fix) → Full Chip Validation Next

---

## Executive Summary

**Goal**: Eliminate TDK driver dependency from IMU.cpp/.h, using only internal Betaflight-based drivers with **preset-based configuration** aligned with imu_hal.md philosophy.

**Key Decisions**:
- ✅ Use **intent-based presets** (SAFE, SMOOTH, BALANCED, ACRO) instead of individual parameter APIs
- ✅ Implement **single-source-of-truth LUT** from imu_hal.md specification
- ✅ Keep low-level config methods **protected/internal** for preset implementation
- ✅ Expose high-level `applyPreset()` API to users
- ✅ **Clean break for enums** (replace TDK constants with numeric values)
- ✅ **Remove FIFO support** (breaking change, acceptable since ReadIMU6 doesn't use it)
- ❌ **Self-test NOT included** - see "Self-Test Decision" section below

**Timeline**: 7-10 days total (reduced from 11-16 days)

**Binary Size Impact**: ~25KB savings (TDK driver removal)

---

## Self-Test Decision

**Decision**: Self-test functionality is **NOT included** in this migration.

**Research Findings** (2025-11-21):
All major flight controller firmware stacks (Betaflight, iNav, ArduPilot, PX4) **do not run IMU self-test at startup**:

| Firmware | Self-Test at Boot? | Validation Method |
|----------|-------------------|-------------------|
| Betaflight | No | WHO_AM_I only |
| iNav | No | Gyro bias recording |
| ArduPilot | No | Sensor detection + calibration |
| PX4 | No | EKF alignment + health monitoring |

**Why self-test is skipped in production:**
1. **Boot latency**: Adds ~200ms startup time
2. **Motion sensitivity**: Fails if device isn't perfectly still/level at boot
4. **Manufacturing focus**: Self-test is designed for factory QC, not runtime validation

**Conclusion**: Self-test adds complexity (~570 lines of TDK code to port) for a feature that professional firmware intentionally skips. Focus effort on preset-based configuration instead.

---

## Background

### Current Architecture

**IMU.cpp/.h Dependencies**:
- TDK driver: `#include "icm42688p.h"` (ICM42688P library)
- TDK structures: `struct inv_icm426xx driver_`, `struct inv_icm426xx_serif serif_`
- TDK functions: 21 unique function calls (init, config, self-test, FIFO, interrupts)
- TDK enums: All AccelFS/GyroFS/AccelODR/GyroODR use `ICM426XX_*` constants

**Problems**:
1. **External dependency**: Requires ICM42688P library (TDK driver)
2. **Binary bloat**: ~30KB TDK driver for single-chip support
3. **Architecture mismatch**: TDK transport callbacks vs. DeviceBus abstraction
4. **Individual parameter tuning**: No preset-based configuration

### Target Architecture

**Preset-Based Configuration** (per imu_hal.md):
- Single `applyPreset(ImuPreset preset)` API
- Four intent-based presets: SAFE, SMOOTH, BALANCED, ACRO
- Single-source-of-truth LUT mapping preset → chip-specific configuration
- Low-level config methods protected/internal
- Common preset enum across all IMU chips (ICM-42688-P, MPU-6000, MPU-9250, ICM-20602)

**Benefits**:
1. ✅ **Zero external dependencies**: Only internal Betaflight drivers
2. ✅ **Binary size reduction**: ~25KB savings
3. ✅ **Unified configuration**: Same preset API for all chips
4. ✅ **Flight-tested settings**: Presets from imu_hal.md (Betaflight-validated)
5. ✅ **Simpler user code**: One preset call vs. manual parameter tuning

---

## Migration Phases

### Phase 1: Extend ICM42688 Driver with Preset-Based Configuration (3-4 days)

#### 1.1 Add Preset Infrastructure (1-2 days)

**Create preset types and LUT** (align with imu_hal.md lines 311-354):

```cpp
// Add to ICM42688.h
enum class ImuPreset : uint8_t {
    FILTER_SAFE,      // Bring-up, very noisy frames (1kHz, tight filtering)
    FILTER_SMOOTH,    // Extra on-chip smoothing (4kHz, moderate filtering)
    FILTER_BALANCED,  // Default for 2kHz PID (4kHz, balanced filtering)
    FILTER_ACRO       // Minimum phase lag (8kHz, wide filtering)
};

struct ICM42688PresetConfig {
    // ODR Configuration
    uint16_t gyro_odr_hz;      // 1k, 4k, or 8k
    uint16_t accel_odr_hz;     // Always 1k

    // FSR (always same per imu_hal.md)
    uint16_t gyro_fsr_dps;     // Always 2000
    uint8_t  accel_fsr_g;      // Always 16

    // AAF Configuration (register values)
    uint8_t  gyro_aaf_delt;
    uint8_t  gyro_aaf_bitshift;
    uint16_t gyro_aaf_deltsqr;
    uint8_t  accel_aaf_delt;
    uint8_t  accel_aaf_bitshift;
    uint16_t accel_aaf_deltsqr;

    // UI Filter Configuration
    uint8_t  ui_bw_code_gyro;   // 0-15 (15 = wide/low-latency)
    uint8_t  ui_bw_code_accel;  // 0-15
    uint8_t  ui_order_gyro;     // 1 or 2
    uint8_t  ui_order_accel;    // 1 or 2
};

// Preset LUT (from imu_hal.md)
static const ICM42688PresetConfig ICM42688_PRESETS[] = {
    // SAFE: 1kHz ODR, AAF 126/84Hz, UI code 1, 2nd-order
    {1000, 1000, 2000, 16, 4,12,0x0010, 3,12,0x0009, 1,1, 2,2},

    // SMOOTH: 4kHz ODR, AAF 213/126Hz, UI code 1, 2nd-order
    {4000, 1000, 2000, 16, 5,11,0x0019, 4,12,0x0010, 1,1, 2,2},

    // BALANCED: 4kHz ODR, AAF 258/170Hz, UI code 15, 1st-order
    {4000, 1000, 2000, 16, 6,10,0x0024, 4,11,0x0010, 15,15, 1,1},

    // ACRO: 8kHz ODR, AAF 303/170Hz, UI code 15, 1st-order
    {8000, 1000, 2000, 16, 7,10,0x0031, 4,11,0x0010, 15,15, 1,1}
};
```

**Reference**: imu_hal.md lines 42-47 (preset table), lines 49-58 (AAF register values)

#### 1.2 Add Public Preset API (1 day)

**Add to ICM42688.h public methods**:
```cpp
class ICM42688 : public DeviceBase {
public:
    // Preset-based configuration (imu_hal.md approach)
    void applyPreset(ImuPreset preset);

    // Basic control (keep simple)
    void enableAccelLNMode();
    void enableGyroLNMode();
    void disableAccel();
    void disableGyro();
    void reset();

protected:
    // Low-level config (used internally by applyPreset)
    void setAccelFSR(uint8_t fsr);
    void setGyroFSR(uint8_t fsr);
    void setAccelODR(uint8_t odr);
    void setGyroODR(uint8_t odr);
    void setGyroAAF(const AAFConfig& config);
    void setAccelAAF(const AAFConfig& config);
    void setUIFilters(uint8_t gyro_bw, uint8_t accel_bw, uint8_t gyro_order, uint8_t accel_order);
    void disableAFSR();  // Auto-FSR workaround
};
```

**Implement applyPreset()** in ICM42688.cpp:
```cpp
void ICM42688::applyPreset(ImuPreset preset) {
    const ICM42688PresetConfig& cfg = ICM42688_PRESETS[static_cast<uint8_t>(preset)];

    // 1. Set ODR (order matters - do this first)
    setGyroODR(cfg.gyro_odr_hz);
    setAccelODR(cfg.accel_odr_hz);

    // 2. Set FSR (always ±2000dps/±16g per imu_hal.md)
    setGyroFSR(cfg.gyro_fsr_dps);
    setAccelFSR(cfg.accel_fsr_g);

    // 3. Configure AAF filters (bank switching required)
    AAFConfig gyro_aaf = {cfg.gyro_aaf_delt, cfg.gyro_aaf_deltsqr, cfg.gyro_aaf_bitshift};
    AAFConfig accel_aaf = {cfg.accel_aaf_delt, cfg.accel_aaf_deltsqr, cfg.accel_aaf_bitshift};
    setGyroAAF(gyro_aaf);
    setAccelAAF(accel_aaf);

    // 4. Configure UI filters
    setUIFilters(cfg.ui_bw_code_gyro, cfg.ui_bw_code_accel,
                 cfg.ui_order_gyro, cfg.ui_order_accel);
}
```

#### 1.3 Implement Protected Low-Level Config Methods (1-2 days)

**Add protected implementation methods**:

**ODR Configuration** (per imu_hal.md lines 60-63):
```cpp
void ICM42688::setGyroODR(uint16_t odr_hz) {
    uint8_t odr_code;
    switch(odr_hz) {
        case 8000: odr_code = 0x03; break;  // 8kHz
        case 4000: odr_code = 0x05; break;  // 4kHz (NOT 0x04!)
        case 2000: odr_code = 0x06; break;  // 2kHz
        case 1000: odr_code = 0x07; break;  // 1kHz (NOT 0x06!)
        default: return;  // Invalid ODR
    }
    // Read-modify-write GYRO_CONFIG0 (preserve FSR bits[5:3])
    uint8_t reg_val = bus_->readReg(ICM426XX_RA_GYRO_CONFIG0);
    reg_val = (reg_val & 0xF0) | (odr_code & 0x0F);
    bus_->writeReg(ICM426XX_RA_GYRO_CONFIG0, reg_val);
}

void ICM42688::setAccelODR(uint16_t odr_hz) {
    uint8_t odr_code;
    switch(odr_hz) {
        case 8000: odr_code = 0x03; break;  // 8kHz
        case 4000: odr_code = 0x05; break;  // 4kHz
        case 2000: odr_code = 0x06; break;  // 2kHz
        case 1000: odr_code = 0x07; break;  // 1kHz
        default: return;  // Invalid ODR
    }
    // Read-modify-write ACCEL_CONFIG0 (preserve FSR bits[5:3])
    uint8_t reg_val = bus_->readReg(ICM426XX_RA_ACCEL_CONFIG0);
    reg_val = (reg_val & 0xF0) | (odr_code & 0x0F);
    bus_->writeReg(ICM426XX_RA_ACCEL_CONFIG0, reg_val);
}
```

**FSR Configuration**:
```cpp
void ICM42688::setGyroFSR(uint16_t fsr_dps) {
    uint8_t fsr_code;
    float scale;
    switch(fsr_dps) {
        case 2000: fsr_code = 0; scale = 1.0f / 16.4f; break;     // ±2000 dps
        case 1000: fsr_code = 1; scale = 1.0f / 32.8f; break;     // ±1000 dps
        case 500:  fsr_code = 2; scale = 1.0f / 65.5f; break;     // ±500 dps
        case 250:  fsr_code = 3; scale = 1.0f / 131.0f; break;    // ±250 dps
        default: return;  // Invalid FSR
    }
    // Read-modify-write GYRO_CONFIG0 (preserve ODR bits[3:0])
    uint8_t reg_val = bus_->readReg(ICM426XX_RA_GYRO_CONFIG0);
    reg_val = (reg_val & 0x0F) | (fsr_code << 5);
    bus_->writeReg(ICM426XX_RA_GYRO_CONFIG0, reg_val);
    gyrScale_ = scale;  // Update scale factor for read() method
}

void ICM42688::setAccelFSR(uint8_t fsr_g) {
    uint8_t fsr_code;
    float scale;
    switch(fsr_g) {
        case 16: fsr_code = 0; scale = 1.0f / 2048.0f; break;   // ±16g
        case 8:  fsr_code = 1; scale = 1.0f / 4096.0f; break;   // ±8g
        case 4:  fsr_code = 2; scale = 1.0f / 8192.0f; break;   // ±4g
        case 2:  fsr_code = 3; scale = 1.0f / 16384.0f; break;  // ±2g
        default: return;  // Invalid FSR
    }
    // Read-modify-write ACCEL_CONFIG0 (preserve ODR bits[3:0])
    uint8_t reg_val = bus_->readReg(ICM426XX_RA_ACCEL_CONFIG0);
    reg_val = (reg_val & 0x0F) | (fsr_code << 5);
    bus_->writeReg(ICM426XX_RA_ACCEL_CONFIG0, reg_val);
    accScale_ = scale;  // Update scale factor for read() method
}
```

**AAF Filter Configuration** (per imu_hal.md lines 64-67):
```cpp
void ICM42688::setGyroAAF(const AAFConfig& config) {
    setUserBank(1);  // Switch to Bank 1
    bus_->writeReg(0x0B, 0x01);  // Enable AAF
    bus_->writeReg(0x0C, config.delt);
    bus_->writeReg(0x0D, config.deltsqr & 0xFF);  // DELTSQR[7:0]
    bus_->writeReg(0x0E, (config.bitshift << 4) | ((config.deltsqr >> 8) & 0x0F));  // BITSHIFT + DELTSQR[11:8]
    setUserBank(0);  // Return to Bank 0
}

void ICM42688::setAccelAAF(const AAFConfig& config) {
    setUserBank(2);  // Switch to Bank 2
    bus_->writeReg(0x03, (config.delt << 1) | 0x01);  // DELT[6:1] + enable bit[0]
    bus_->writeReg(0x04, config.deltsqr & 0xFF);  // DELTSQR[7:0]
    bus_->writeReg(0x05, (config.bitshift << 4) | ((config.deltsqr >> 8) & 0x0F));  // BITSHIFT + DELTSQR[11:8]
    setUserBank(0);  // Return to Bank 0
}
```

**UI Filter Configuration** (per imu_hal.md lines 68-71):
```cpp
void ICM42688::setUIFilters(uint8_t gyro_bw, uint8_t accel_bw, uint8_t gyro_order, uint8_t accel_order) {
    // BW codes: 0-15 (15 = wide/low-latency)
    // Order: 1 = 1st-order, 2 = 2nd-order (encoded as 0, 1 in register per TDK datasheet)

    // GYRO_ACCEL_CONFIG0 (0x52): gyro BW[3:0], accel BW[7:4]
    bus_->writeReg(0x52, (accel_bw << 4) | gyro_bw);

    // GYRO_CONFIG1 (0x51): gyro order bits[3:2] (0=1st, 1=2nd, 2=3rd)
    uint8_t gyro_order_code = (gyro_order == 1) ? 0 : 1;  // 1st-order=0, 2nd-order=1
    uint8_t reg_val = bus_->readReg(0x51);
    reg_val = (reg_val & ~0x0C) | (gyro_order_code << 2);
    bus_->writeReg(0x51, reg_val);

    // ACCEL_CONFIG1 (0x53): accel order bits[4:3] (0=1st, 1=2nd, 2=3rd)
    uint8_t accel_order_code = (accel_order == 1) ? 0 : 1;  // 1st-order=0, 2nd-order=1
    reg_val = bus_->readReg(0x53);
    reg_val = (reg_val & ~0x18) | (accel_order_code << 3);
    bus_->writeReg(0x53, reg_val);
}
```

**Device Reset**:
```cpp
void ICM42688::reset() {
    bus_->writeReg(ICM426XX_RA_PWR_MGMT0, 0x00);  // Power off
    delay(10);
    bus_->writeReg(ICM426XX_RA_DEVICE_CONFIG, 0x01);  // Soft reset
    delay(100);  // Wait for reset to complete
}
```

---

### Phase 2: Update IMU.cpp/.h to Preset-Based API (2-3 days) ✅ COMPLETE

**Completed 2025-11-21** (Commit: `0b2c58e13`)
- Removed TDK driver dependency from IMU.cpp/.h
- Added preset-based API: `ApplyPreset(SAFE|SMOOTH|BALANCED|ACRO)`
- Retained fine-grained filter APIs for advanced users: `SetGyroAAF()`, `SetAccelAAF()`, `SetUIFilters()`
- Updated Polled_FlightController example for preset API
- Removed obsolete SelfTest example
- Binary reduction: ~546 lines removed

#### 2.1 Remove TDK Driver Dependencies (1 day)

**Remove from IMU.h**:
```cpp
// DELETE these lines
#include "icm42688p.h"                           // TDK driver + filter API
struct inv_icm426xx driver_;                     // TDK driver instance
struct inv_icm426xx_serif serif_;                // TDK transport callbacks
void SetSensorEventCallback(...);                // TDK event callback
bool ReadDataFromFifo(...);                      // FIFO API (unused)
```

**Add to IMU.h**:
```cpp
// ADD these lines
#include "devices/ICM42688.h"                 // Internal driver
ICM42688* driver_;                // driver instance (for ICM-42688-P)
DeviceBusSPI* bus_;                              // Bus abstraction
```

#### 2.2 Update Enum Values (1 day)

**Replace TDK constants** in IMU.h enums:

```cpp
// BEFORE (TDK-based, lines 66-96)
enum AccelFS : uint8_t {
    gpm16 = ICM426XX_ACCEL_CONFIG0_FS_SEL_16g,   // TDK constant
    gpm8  = ICM426XX_ACCEL_CONFIG0_FS_SEL_8g,
    gpm4  = ICM426XX_ACCEL_CONFIG0_FS_SEL_4g,
    gpm2  = ICM426XX_ACCEL_CONFIG0_FS_SEL_2g
};

// AFTER (Numeric register encoding)
enum AccelFS : uint8_t {
    gpm16 = 0,  // ±16g (ACCEL_CONFIG0 bits[5:3] = 0)
    gpm8  = 1,  // ±8g
    gpm4  = 2,  // ±4g
    gpm2  = 3   // ±2g
};

enum GyroFS : uint8_t {
    dps2000 = 0,  // ±2000 dps (GYRO_CONFIG0 bits[5:3] = 0)
    dps1000 = 1,  // ±1000 dps
    dps500  = 2,  // ±500 dps
    dps250  = 3   // ±250 dps
};

enum AccelODR : uint8_t {
    odr8k  = 3,  // 8 kHz (ACCEL_CONFIG0 bits[3:0] = 3)
    odr4k  = 5,  // 4 kHz (NOT 4!)
    odr2k  = 6,  // 2 kHz
    odr1k  = 7   // 1 kHz (NOT 6!)
};

enum GyroODR : uint8_t {
    odr8k  = 3,  // 8 kHz (GYRO_CONFIG0 bits[3:0] = 3)
    odr4k  = 5,  // 4 kHz (NOT 4!)
    odr2k  = 6,  // 2 kHz
    odr1k  = 7   // 1 kHz (NOT 6!)
};
```

**CRITICAL**: ODR encoding is non-sequential (3=8kHz, 5=4kHz, 6=2kHz, 7=1kHz). See imu_hal.md line 62.

#### 2.3 Add Preset API to IMU.h (1 day)

**Add public preset methods to IMU.h**:
```cpp
class IMU {
public:
    // Preset-based configuration (imu_hal.md approach)
    enum class Preset : uint8_t {
        SAFE,      // Bring-up, very noisy frames
        SMOOTH,    // Extra on-chip smoothing
        BALANCED,  // Default for 2kHz PID
        ACRO       // Minimum phase lag
    };

    bool ApplyPreset(Preset preset);

    // Legacy individual config (keep for backward compatibility, but discourage)
    // NOTE: Using ApplyPreset() is recommended over individual setters
    bool SetAccelFS(AccelFS fs);
    bool SetGyroFS(GyroFS fs);
    bool SetAccelODR(AccelODR odr);
    bool SetGyroODR(GyroODR odr);

    // ... rest of existing API
};
```

#### 2.4 Migrate Function Implementations (1 day)

**Update IMU.cpp Init()**:
```cpp
bool IMU::Init(...) {
    // Create bus abstraction
    bus_ = new DeviceBusSPI(spi, cs_pin, freq);

    // Auto-detect IMU chip (existing logic)
    if (type == ImuType::ICM42688P || type == ImuType::Auto) {
        driver_ = ICM42688::detect(bus_);
        if (driver_) {
            chip_type_ = ChipType::ICM42688_P;
            // Apply default BALANCED preset
            driver_->applyPreset(ImuPreset::FILTER_BALANCED);
            return true;
        }
    }

    // ... MPU6000/MPU9250/ICM206xx detection logic unchanged
}
```

**Implement ApplyPreset()**:
```cpp
bool IMU::ApplyPreset(Preset preset) {
    // Map IMU::Preset to driver-specific preset enum
    ImuPreset driver_preset = static_cast<ImuPreset>(preset);

    switch(chip_type_) {
        case ChipType::ICM42688_P:
            driver_->applyPreset(driver_preset);
            return true;

        case ChipType::MPU6000:
            driver_->applyPreset(driver_preset);
            return true;

        case ChipType::MPU9250:
        case ChipType::MPU9255:
            driver_->applyPreset(driver_preset);
            return true;

        case ChipType::ICM20601:
        case ChipType::ICM20602:
        case ChipType::ICM20689:
            driver_->applyPreset(driver_preset);
            return true;

        default:
            return false;
    }
}
```

**Remove self-test API**:
- Delete `RunSelfTest()` method from IMU.h/cpp (self-test not included per decision above)

**Update ReadIMU6()** - No changes needed (already uses DeviceBase::read())

---

### Phase 2.5: ICM-42688-P Hardware Validation (0.5 days)

**Goal**: Validate Phase 2 migration on hardware before proceeding to other chips.

#### 2.5.1 Hardware Test on NUCLEO_F411RE

**Test Command**:
```bash
./system/ci/aflash.sh libraries/imu/examples/Polled_FlightController --use-rtt --build-id
```

**Validation Criteria**:
- [ ] WHO_AM_I returns 0x47 (ICM-42688-P detected)
- [ ] BALANCED preset applied successfully
- [ ] 2kHz polling loop runs without errors
- [ ] Gyro/Accel data within expected range (stationary: gyro ~0 dps, accel ~1g on Z)
- [ ] 20 samples printed over ~10 seconds
- [ ] Clean exit with "*STOP*" token

#### 2.5.2 Binary Size Verification

**Expected**:
- Polled_FlightController: ~31KB (vs ~56KB with TDK driver)
- Savings: ~25KB from TDK driver removal

#### 2.5.3 Sign-off

Once hardware validation passes:
- [ ] Push Phase 2 commit to origin/dev
- [ ] Proceed to Phase 3 (other chip drivers)

---

### Phase 3: Add Preset Support to MPU6000/MPU9250/ICM206xx Drivers (2-3 days)

#### 3.1 MPU6000 Preset Implementation (1 day)

**Add preset LUT** (from imu_hal.md lines 90-96):
```cpp
// Add to MPU6000.cpp
struct MPU6000PresetConfig {
    uint8_t  dlpf_cfg;      // CONFIG.DLPF_CFG (0-6)
    uint8_t  smplrt_div;    // SMPLRT_DIV register (0-255)
    uint16_t gyro_odr_hz;   // Effective gyro ODR
};

static const MPU6000PresetConfig MPU6000_PRESETS[] = {
    {2, 0, 1000},  // SAFE: DLPF=2 (98/94Hz), DIV=0 → 1kHz
    {1, 0, 1000},  // SMOOTH: DLPF=1 (188/184Hz), DIV=0 → 1kHz
    {0, 1, 4000},  // BALANCED: DLPF=0 (256Hz), DIV=1 → 4kHz
    {0, 0, 8000}   // ACRO: DLPF=0 (256Hz), DIV=0 → 8kHz
};
```

**Implement applyPreset()**:
```cpp
void MPU6000::applyPreset(ImuPreset preset) {
    const MPU6000PresetConfig& cfg = MPU6000_PRESETS[static_cast<uint8_t>(preset)];

    bus_->writeReg(MPU_RA_CONFIG, cfg.dlpf_cfg);
    delayMicroseconds(1);

    bus_->writeReg(MPU_RA_SMPLRT_DIV, cfg.smplrt_div);
    delayMicroseconds(1);

    samplingRateHz_ = cfg.gyro_odr_hz;
}
```

#### 3.2 MPU9250 Preset Implementation (1 day)

**Add preset LUT** (from imu_hal.md lines 140-145):
```cpp
// Add to MPU9250.cpp
struct MPU9250PresetConfig {
    uint8_t  dlpf_cfg_gyro;   // CONFIG.DLPF_CFG (0-6)
    uint8_t  dlpf_cfg_accel;  // ACCEL_CONFIG2.A_DLPF_CFG
    uint8_t  smplrt_div;      // SMPLRT_DIV (only for DLPF_CFG=1-6)
    uint16_t gyro_odr_hz;     // On-sensor rate
    uint16_t effective_rate;  // Effective rate (may require SW decimation)
};

static const MPU9250PresetConfig MPU9250_PRESETS[] = {
    {2, 2, 0, 1000, 1000},  // SAFE: DLPF=2 (92Hz), DIV=0 → 1kHz
    {1, 1, 0, 1000, 1000},  // SMOOTH: DLPF=1 (184Hz), DIV=0 → 1kHz
    {0, 1, 0, 8000, 4000},  // BALANCED: DLPF=0 (wide), 8kHz → SW decimate to 4kHz
    {0, 1, 0, 8000, 8000}   // ACRO: DLPF=0 (wide), 8kHz
};
```

**Implement applyPreset()**:
```cpp
void MPU9250::applyPreset(ImuPreset preset) {
    const MPU9250PresetConfig& cfg = MPU9250_PRESETS[static_cast<uint8_t>(preset)];

    // Gyro DLPF
    bus_->writeReg(MPU_RA_CONFIG, cfg.dlpf_cfg_gyro);
    delayMicroseconds(1);

    // Accel DLPF
    bus_->writeReg(MPU_RA_ACCEL_CONFIG2, cfg.dlpf_cfg_accel);
    delayMicroseconds(1);

    // Sample rate divider (NOTE: Only effective when DLPF_CFG=1-6)
    bus_->writeReg(MPU_RA_SMPLRT_DIV, cfg.smplrt_div);
    delayMicroseconds(1);

    samplingRateHz_ = cfg.effective_rate;  // Store effective rate for user reference
}
```

**Note on Software Decimation**: For BALANCED/ACRO presets (DLPF_CFG=0), the on-sensor rate is 8kHz but effective rate is 4kHz/8kHz. This requires software decimation in the application code. See imu_hal.md lines 282-296 for decimation example.

#### 3.3 ICM206xx Preset Implementation (1 day)

**Add preset LUT** (from imu_hal.md lines 218-223):
```cpp
// Add to ICM206xx.cpp (same structure as MPU9250, 6500-class)
static const ICM20602PresetConfig ICM20602_PRESETS[] = {
    {2, 2, 0, 1000, 1000},  // SAFE: DLPF=2 (~92Hz), DIV=0 → 1kHz
    {1, 1, 0, 1000, 1000},  // SMOOTH: DLPF=1 (~176/184Hz), DIV=0 → 1kHz
    {0, 1, 0, 8000, 4000},  // BALANCED: DLPF=0 (wide), 8kHz → SW decimate to 4kHz
    {0, 1, 0, 8000, 8000}   // ACRO: DLPF=0 (wide), 8kHz
};
```

**Implement applyPreset()**: Same implementation as MPU9250 (6500-class architecture).

---

### Phase 4: Testing and Validation (1-2 days)

#### 4.1 Preset Validation (1 day)

**Test each preset on ICM-42688-P** (NUCLEO_F411RE):

Create test sketch: `PresetValidation.ino`
```cpp
#include <IMU_Driver.h>
#include <SPI.h>

SPIClass spi_bus(MOSI, MISO, SCK);
IMU_Driver imu;

void verifyPreset(IMU::Preset preset, const char* name) {
    CI_LOGF("Testing %s preset...\n", name);
    imu.ApplyPreset(preset);

    // TODO: Add register read-back verification
    // - Bank 0: GYRO_CONFIG0, ACCEL_CONFIG0 (verify ODR codes)
    // - Bank 1: Gyro AAF registers (0x0B-0x0E)
    // - Bank 2: Accel AAF registers (0x03-0x05)
    // - Bank 0: UI filter registers (0x51-0x53)
}

void setup() {
    spi_bus.begin();
    imu.attachSPI(spi_bus, CS_PIN, SPI_FREQ);
    imu.begin(ImuType::Auto);

    verifyPreset(IMU::Preset::SAFE, "SAFE");
    verifyPreset(IMU::Preset::SMOOTH, "SMOOTH");
    verifyPreset(IMU::Preset::BALANCED, "BALANCED");
    verifyPreset(IMU::Preset::ACRO, "ACRO");

    CI_LOG("*STOP*\n");
    while(1);
}
```

**Validation criteria**:
- SAFE: ODR=1kHz (0x07), AAF 126/84Hz, UI code 1, 2nd-order
- SMOOTH: ODR=4kHz (0x05), AAF 213/126Hz, UI code 1, 2nd-order
- BALANCED: ODR=4kHz (0x05), AAF 258/170Hz, UI code 15, 1st-order
- ACRO: ODR=8kHz (0x03), AAF 303/170Hz, UI code 15, 1st-order

#### 4.2 Integration Testing (1 day)

**Test dRehmFlight** with preset API:
```cpp
// In dRehmFlight setup()
imu.Init(...);
imu.ApplyPreset(IMU::Preset::BALANCED);  // Default for 2kHz PID loop

// Flight test: Verify stable flight at 2kHz loop rate
```

**Test all IMU examples**:
- **AutoDetect_Single**: Verify multi-chip detection still works, apply BALANCED preset
- **Polled_FlightController**: Use BALANCED preset (4kHz gyro, 1kHz accel)
- **Interrupt_DataReady**: Use ACRO preset (8kHz gyro)

**Regression test**: Verify all 4 chips (ICM-42688-P, MPU-6000, MPU-9250, ICM-20602) auto-detect and run correctly with presets.

**Binary size validation**:
- Compile dRehmFlight before migration (with TDK driver)
- Compile dRehmFlight after migration (drivers only)
- Expected savings: ~25KB

---

## Breaking Changes

### 1. Removed APIs

**ReadDataFromFifo()** - REMOVED
- Rationale: FIFO support eliminated. ReadIMU6() already bypasses TDK FIFO, using direct register reads.
- Migration: No action needed (unused in practice)

**SetSensorEventCallback()** - REMOVED
- Rationale: TDK-specific event callback removed with TDK driver
- Migration: Remove callback registration code

### 2. Changed Enums (Values Only, Names Unchanged)

**Impact**: Application code using enums does **NOT** need changes. Only internal enum values changed.

```cpp
// OLD (TDK constants)
AccelFS::gpm16 = ICM426XX_ACCEL_CONFIG0_FS_SEL_16g  // TDK constant value
GyroFS::dps2000 = ICM426XX_GYRO_CONFIG0_FS_SEL_2000dps

// NEW (Register encoding)
AccelFS::gpm16 = 0  // Register encoding for ±16g
GyroFS::dps2000 = 0  // Register encoding for ±2000 dps

// Application code unchanged:
imu.SetGyroFS(IMU::GyroFS::dps2000);  // Same call, different internal value
```

### 3. New Recommended API

**RECOMMENDED**: Use preset-based configuration
```cpp
imu.ApplyPreset(IMU::Preset::BALANCED);  // All config in one call
```

**LEGACY**: Individual setters still work but discouraged
```cpp
imu.SetGyroFS(IMU::GyroFS::dps2000);
imu.SetGyroODR(IMU::GyroODR::odr4k);
// ... manual configuration (not recommended)
```

---

## Migration Guide for Users

### For dRehmFlight / Flight Controller Applications

**Before** (TDK-based, individual config):
```cpp
imu.Init(...);
imu.SetGyroFS(IMU::GyroFS::dps2000);
imu.SetAccelFS(IMU::AccelFS::gpm16);
imu.SetGyroODR(IMU::GyroODR::odr4k);
// ... manual AAF/UI filter configuration
```

**After** (Preset-based, imu_hal.md approach):
```cpp
imu.Init(...);  // Automatically applies BALANCED preset
// OR explicitly:
imu.ApplyPreset(IMU::Preset::BALANCED);  // All config in one call
```

### Preset Selection Guide

| Preset | Use Case | Flight Characteristics |
|--------|----------|----------------------|
| **SAFE** | Initial bring-up, very noisy frames | Heavily filtered (1kHz), sluggish response |
| **SMOOTH** | Smooth video, GPS hold | Moderate filtering (4kHz), balanced |
| **BALANCED** | General flight, racing (2kHz PID) | **Recommended default** (4kHz, optimized for 2kHz loop) |
| **ACRO** | Aggressive acro, freestyle | Minimal filtering (8kHz), lowest latency |

**Reference**: imu_hal.md lines 12-17 (preset philosophy)

---

## Timeline

| Phase | Duration | Key Deliverables | Status |
|-------|----------|------------------|--------|
| **Phase 1** | 3-4 days | ICM42688 with preset API | ✅ Complete |
| **Phase 2** | 2-3 days | IMU.cpp/.h migrated to preset-based API | ✅ Complete |
| **Phase 2.5** | 0.5 days | ICM-42688-P hardware validation | ✅ Complete |
| **Phase 3** | 1-2 days | MPU6000/MPU9250/ICM206xx preset support | ✅ Complete |
| **Phase 3.5** | 0.5 days | API cleanup + ICM-42688-P AAF fix | ✅ Complete |
| **Phase 4** | 1-2 days | Final integration testing (all chips) | 📋 Pending |
| **TOTAL** | **7-11 days** | Complete TDK driver elimination | |

**Reduced from original 11-16 days** due to:
- Self-test removed (saves 3-4 days - not used by major FC stacks)
- Simpler preset-based API vs. individual runtime config methods
- Clearer validation criteria (preset register values from imu_hal.md)

---

## Success Criteria

✅ Zero TDK driver includes in IMU.cpp/.h
✅ Preset-based API working: ApplyPreset(SAFE/SMOOTH/BALANCED/ACRO)
✅ All preset register values match imu_hal.md specification
✅ dRehmFlight compiles and flies with BALANCED preset
✅ Binary size reduced by ~25KB
✅ All 4 drivers support common ImuPreset enum
✅ Individual config APIs preserved (protected) for backward compatibility

---

## Alignment with imu_hal.md Philosophy

✅ **Intent-based presets**: SAFE/SMOOTH/BALANCED/ACRO per imu_hal.md lines 12-17
✅ **Single-source-of-truth LUT**: Preset configs from imu_hal.md lines 340-354
✅ **Hardcoded FSR**: ±2000dps/±16g per imu_hal.md lines 20-21
✅ **Chip abstraction**: Common preset enum across all IMU chips
✅ **Register-level implementation**: Low-level methods protected, high-level preset public
❌ **Self-test omitted**: Professional FC stacks (Betaflight, iNav, ArduPilot, PX4) skip self-test - validates via WHO_AM_I + gyro bias instead

---

## ICM-42688-P SAFE Preset Gyro Bias Issue (2025-11-22)

**Problem**: ICM-42688-P SAFE preset exhibited large gyro DC offset (~12-30 DPS) while BALANCED preset worked correctly.

**Root Cause Analysis**:
Investigation isolated the issue through systematic testing:

| Configuration | AAF | UI Code | Gyro DPS | Result |
|--------------|-----|---------|----------|--------|
| Original SAFE | 126Hz | 1 | 12.12, -28.84, 1.51 | BIAS |
| AAF 126Hz + UI=15 | 126Hz | 15 | 12.12, -28.84, 1.49 | BIAS |
| AAF 258Hz + UI=1 | 258Hz | 1 | 0.32, -0.84, 0.23 | OK |
| Current Fix | 258Hz | 15 | 0.32, -0.84, 0.21 | OK |

**True Root Cause**: The original SAFE preset had **incorrect AAF register values**.

Per ICM-42688-P datasheet Table 15, the AAF register values are:
| Bandwidth | DELT | DELTSQR | BITSHIFT |
|-----------|------|---------|----------|
| 126 Hz | 3 | 9 | 12 |
| 170 Hz | 4 | 16 | 11 |
| 258 Hz | 6 | 36 | 10 |

Original SAFE preset used: `delt=4, deltsqr=16, bitshift=12` (commented as "126Hz")
- This is **an invalid configuration**: DELT=4/DELTSQR=16 are for 170Hz, but BITSHIFT=12 is for 126Hz
- The mismatch caused undefined behavior in the chip's internal DSP, producing DC offset

**Key Learning**: A low-pass filter cannot physically introduce DC offset (0 Hz passes through).
When large DC offsets appear, look for register misconfiguration, not filter bandwidth limits.

**Solution Applied**:
All presets now use AAF 258Hz (Betaflight default) with verified register values:
- Gyro AAF 258Hz: `delt=6, deltsqr=0x0024 (36), bitshift=10`
- This matches Betaflight's proven configuration

**References**:
- Betaflight PR #12444: Fix ICM426XX AA filter
- Betaflight issue #12970: Artefacts in ICM-42688-P gyro output
- ArduPilot issue #25025: ICM42688 "stuck" gyro values

---

## References

- **imu_hal.md**: Filter presets and register configuration specification
- **imu_presets.md**: Betaflight-oriented preset guide (reference only)
- **TDK ICM-42688-P Datasheet**: DS-000347 Rev 1.7 (register map)
- **Betaflight Source**: Original driver reference (accgyro_spi_icm426xx.c)
- **CLAUDE.md**: Embedded hardware validation standards
