# IMU Library Migration Status

## Executive Summary

**Goal**: Eliminate TDK driver dependency from IMU.cpp/.h, use only internal Betaflight-based drivers with preset-based configuration.

**Current Status**: COMPLETE

**Timeline**: 7-11 days total

**Completion**: 100%

---

## Completed Work

### Phase 1: ICM42688 Preset Infrastructure

**Status**: COMPLETED (2025-11-21)

- Added `ImuPreset` enum to ICM42688.h (SAFE/SMOOTH/BALANCED/ACRO)
- Created `ICM42688PresetConfig` structure with LUT
- Implemented preset configurations matching imu_hal.md specification
- All presets use 2000dps/16g FSR per imu_hal.md
- Protected low-level config methods (ODR, FSR, AAF, UI filters)
- Compilation verified on AutoDetect_Single example

### Phase 2: IMU.cpp/.h Migration to Preset-Based API

**Status**: COMPLETED (2025-11-21, Commit: `0b2c58e13`)

- Removed TDK driver dependency from IMU.cpp/.h
- Added preset-based API: `ApplyPreset(SAFE|SMOOTH|BALANCED|ACRO)`
- Retained fine-grained filter APIs for advanced users
- Updated Polled_FlightController example for preset API
- Removed obsolete SelfTest example
- Binary reduction: ~546 lines removed

### Phase 2.5: ICM-42688-P Hardware Validation

**Status**: COMPLETED (2025-11-21)

- WHO_AM_I verified (0x47)
- BALANCED preset applied successfully
- Gyro/Accel data within expected range
- Hardware validation on NUCLEO_F411RE passed

### Phase 3: Multi-Driver Preset Support

**Status**: COMPLETED (2025-11-22, Commit: `d015d9863`)

- MPU6000, MPU9250, ICM206xx drivers updated with preset support
- Common `ImuPreset` enum across all drivers
- LUT-based configuration matching imu_hal.md specification
- All 4 drivers now support `applyPreset()` API

### Phase 3.5: 3-Tier API Cleanup + AAF Fix

**Status**: COMPLETED (2025-11-22, Commits: `753df4fc4`, `fdeba6de7`)

**3-Tier API Architecture**:
- **Tier 1 (Core)**: `Init()`, `ReadIMU6()`, `ApplyPreset()` - Primary user API
- **Tier 2 (Extended)**: `SetGyroFSR_Ex()`, `SetAccelFSR_Ex()` - FSR configuration
- **Tier 3 (Direct)**: `ReadReg_Ex()`, `WriteReg_Ex()`, `WriteRegVerify_Ex()` - Power users

**ICM-42688-P SAFE Preset AAF Fix**:
- Root cause: Invalid AAF register configuration (mismatched DELT/DELTSQR/BITSHIFT)
- Original SAFE preset used DELT=4/DELTSQR=16 (170Hz values) with BITSHIFT=12 (126Hz value)
- This caused undefined behavior in chip DSP, producing ~12-30 DPS gyro offset
- Fix: All presets now use AAF 258Hz (Betaflight default) with verified register values
- Gyro AAF 258Hz: `delt=6, deltsqr=0x0024 (36), bitshift=10`

**File Renames** (Commit: `c037199e1`):
- Removed `_BF` suffix from device files
- `ICM42688_BF.h/cpp` → ICM42688.h/cpp
- `MPU6000_BF.h/cpp` → MPU6000.h/cpp
- `MPU9250_BF.h/cpp` → MPU9250.h/cpp
- `ICM206xx_BF.h/cpp` → ICM206xx.h/cpp

### Phase 4: Final Integration Testing

**Status**: COMPLETED (2025-11-23, Commit: `0e1690006`)

**All Examples Tested**:
- AutoDetect_Single ✅
- ICM42688P_Advanced ✅
- ICM20602_Advanced ✅
- Interrupt_DataReady ✅
- Polled_FlightController ✅
- dev/imu-polled-bf ✅

**Hardware Validation - All 4 IMU Chips**:

| IMU Chip | Board | WHO_AM_I | Read Rate | Accel Z | Gyro Bias | Status |
|----------|-------|----------|-----------|---------|-----------|--------|
| ICM-42688-P | NUCLEO_F411RE | 0x47 | 34.5 kHz | +1.035g | <1 dps | ✅ Pass |
| ICM-20602 | BKMN_NERO (F722) | 0x12 | 8 kHz | +0.98g | <1 dps | ✅ Pass |
| MPU-6000 | NUCLEO_F411RE | 0x68 | 30.6 kHz | +1.04g | <2 dps | ✅ Pass |
| MPU-9250 | BLACKPILL_F411CE | 0x71 | 30.9 kHz | -0.98g* | <1 dps | ✅ Pass |

*MPU-9250 mounted upside-down, negative Z is correct

**Binary Sizes** (RTT mode):
- AutoDetect_Single: 31,952 bytes Flash, 2,544 bytes RAM
- Polled_FlightController: 35,552 bytes Flash, 2,568 bytes RAM
- ICM42688P_Advanced: 37,596 bytes Flash, 2,560 bytes RAM
- ICM20602_Advanced: 39,100 bytes Flash, 2,744 bytes RAM
- Average: ~36 KB Flash, ~2.6 KB RAM

---

## Self-Test Decision

**Decision**: Self-test functionality NOT included in migration.

**Rationale** (2025-11-21):
Major flight controller firmware stacks (Betaflight, iNav, ArduPilot, PX4) do NOT run IMU self-test at startup:

| Firmware | Self-Test at Boot? | Validation Method |
|----------|-------------------|-------------------|
| Betaflight | No | WHO_AM_I only |
| iNav | No | Gyro bias recording |
| ArduPilot | No | Sensor detection + calibration |
| PX4 | No | EKF alignment + health monitoring |

**Why self-test is skipped in production:**
1. Boot latency: Adds ~200ms startup time
2. Motion sensitivity: Fails if device isn't perfectly still/level at boot
3. Manufacturing focus: Self-test is designed for factory QC, not runtime validation

**Conclusion**: Self-test adds complexity (~570 lines of TDK code) for a feature that professional firmware intentionally skips.

---

## User Decisions (Confirmed)

1. **Self-test**: NOT included (professional FC stacks skip self-test)
2. **Enum migration**: Clean break - TDK enums replaced with numeric constants
3. **Runtime config**: Full support via preset-based API (SAFE/SMOOTH/BALANCED/ACRO)
4. **FIFO support**: Removed - `ReadDataFromFifo()` API deleted (acceptable breaking change)
5. **3-tier API**: Implemented - Core/Extended/Direct access levels

---

## Key Architectural Decisions

### Preset-Based Configuration Philosophy
- **User Intent**: Users select preset by intent (SAFE/SMOOTH/BALANCED/ACRO), not individual registers
- **Single Source of Truth**: All register values from imu_hal.md specification
- **Consistent Across Chips**: Same intent produces equivalent behavior on all IMUs
- **Protected Low-Level Methods**: Individual config methods internal implementation details

### FSR Standardization
- **All presets**: 2000dps gyro, 16g accel (per imu_hal.md)
- **Rationale**: Flight controller standard, good resolution without clipping
- **Extended API**: `SetGyroFSR_Ex()` / `SetAccelFSR_Ex()` available for custom FSR

### 3-Tier API Design
- **Tier 1**: Core flight controller API (Init, ReadIMU6, ApplyPreset)
- **Tier 2**: Extended configuration (FSR changes, filter tuning)
- **Tier 3**: Direct register access for debugging and advanced use

---

## Breaking Changes

### Removed APIs
- `ReadDataFromFifo()` - FIFO support eliminated
- `SetSensorEventCallback()` - TDK-specific callback removed
- `RunSelfTest()` - Self-test not included (see decision above)

### Migration Path for Users
**Before** (TDK driver):
```cpp
IMU imu;
imu.Init(spi, cs_pin, 1000000);
// TDK driver auto-configures with hardcoded settings
```

**After** (BF drivers + presets):
```cpp
IMU imu;
imu.Init(spi, cs_pin, 1000000);
imu.ApplyPreset(IMU::Preset::BALANCED);  // Explicit preset selection
```

---

## Files Modified

### Completed Changes
```
libraries/imu/
src/
    IMU.h                        # 3-tier API, preset support, TDK removed
    IMU.cpp                      # BF driver integration, preset dispatch
    devices/
        DeviceBase.h             # ImuPreset enum, GyroFSR/AccelFSR enums
        ICM42688.h/.cpp          # Preset LUT, AAF fix, config methods
        MPU6000.h/.cpp           # Preset support added
        MPU9250.h/.cpp           # Preset support added
        ICM206xx.h/.cpp          # Preset support added
examples/
    AutoDetect_Single/           # Updated for new API
    Polled_FlightController/     # Updated for preset API
    ICM42688P_Advanced/          # Advanced configuration example
    ICM20602_Advanced/           # ICM-20602 specific example
    Interrupt_DataReady/         # Data ready interrupt example
    dev/imu-polled-bf/           # Betaflight config example
imu_hal.md                       # Filter specification (reference)
imu_presets.md                   # Preset guide (reference)
MIGRATION_PLAN.md                # Migration plan
MIGRATION_STATUS.md              # Status tracking (this file)
```

---

## Success Criteria

### All Completed ✅
- [x] Zero TDK driver includes in IMU.cpp/.h
- [x] Preset-based API working: ApplyPreset(SAFE/SMOOTH/BALANCED/ACRO)
- [x] All preset register values match imu_hal.md specification
- [x] All 4 drivers support common ImuPreset enum
- [x] 3-tier API implemented (Core/Extended/Direct)
- [x] AAF register misconfiguration fixed
- [x] File naming cleanup (_BF suffix removed)
- [x] All examples compile and run
- [x] Hardware validation on all 4 supported chips (ICM-42688-P, ICM-20602, MPU-6000, MPU-9250)
- [x] Binary sizes measured (~32-39 KB Flash)

---

## Timeline

| Phase | Task | Duration | Status |
|-------|------|----------|--------|
| 1 | ICM42688 preset infrastructure | 2 days | COMPLETED |
| 2 | IMU.cpp/.h migration | 2 days | COMPLETED |
| 2.5 | ICM-42688-P hardware validation | 0.5 days | COMPLETED |
| 3 | Multi-driver preset support | 1.5 days | COMPLETED |
| 3.5 | 3-tier API cleanup + AAF fix | 0.5 days | COMPLETED |
| 4 | Final integration testing | 1 day | COMPLETED |
| **Total** | | **~8 days** | **100% complete** |

---

## References

- **imu_hal.md**: Filter presets and register configuration specification
- **imu_presets.md**: Betaflight-oriented preset guide
- **MIGRATION_PLAN.md**: Complete migration plan with code examples
- **TDK ICM-42688-P Datasheet**: DS-000347 Rev 1.7 (register map)
- **Betaflight Source**: Original driver reference (accgyro_spi_icm426xx.c)
