# IMU Library Migration Status

## Executive Summary

**Goal**: Eliminate TDK driver dependency from IMU.cpp/.h, use only internal Betaflight-based drivers with preset-based configuration.

**Current Phase**: Phase 1 - ICM42688_BF Extension (75% complete)

**Timeline**: 11-16 days total, ~3 days elapsed

## Completed Work

### ✅ Phase 1.1: Preset Infrastructure (COMPLETED)
- Added `ImuPreset` enum to ICM42688_BF.h (SAFE/SMOOTH/BALANCED/ACRO)
- Created `ICM42688PresetConfig` structure in ICM42688_BF.cpp
- Implemented preset LUT with 4 configurations matching imu_hal.md specification
- All presets use ±2000dps/±16g FSR per imu_hal.md lines 20-21

### ✅ Phase 1.2: Public Preset API (COMPLETED)
- Added public `applyPreset(ImuPreset preset)` method to ICM42688_BF.h
- Implemented `applyPreset()` in ICM42688_BF.cpp with proper sequencing:
  1. Set ODR (gyro + accel)
  2. Set FSR (gyro + accel)
  3. Configure AAF filters (gyro + accel, bank switching)
  4. Configure UI filters (BW codes + filter order)
  5. Update samplingRateHz_ member

### ✅ Phase 1.3: Protected Low-Level Config Methods (COMPLETED)
Implemented 8 protected configuration methods in ICM42688_BF.cpp:

1. **`setGyroODR(uint16_t odr_hz)`** - Gyro ODR with non-sequential encoding
   - 8kHz=0x03, 4kHz=0x05, 2kHz=0x06, 1kHz=0x07
   - Read-modify-write to preserve FSR bits

2. **`setAccelODR(uint16_t odr_hz)`** - Accel ODR configuration
   - Always 1kHz per imu_hal.md specification

3. **`setGyroFSR(uint16_t fsr_dps)`** - Gyro full-scale range
   - Hardcoded to ±2000dps per imu_hal.md
   - Updates gyrScale_ for correct conversion

4. **`setAccelFSR(uint16_t fsr_g)`** - Accel full-scale range
   - Hardcoded to ±16g per imu_hal.md
   - Updates accScale_ for correct conversion

5. **`setGyroAAF(const AAFConfig& config)`** - Gyro anti-alias filter
   - Bank 1 register writes (DELT, DELTSQR, BITSHIFT)

6. **`setAccelAAF(const AAFConfig& config)`** - Accel anti-alias filter
   - Bank 2 register writes (DELT, DELTSQR, BITSHIFT)

7. **`setUIFilters()`** - UI filter BW codes and order
   - BW code: 0-15 (15=wide/low-latency)
   - Order: 1st-order=3, 2nd-order=2

8. **`disableAFSR()`** - Auto-FSR workaround
   - Prevents sticky samples during range switching

### ✅ Compilation Testing (COMPLETED)
- Built AutoDetect_Single example successfully
- Binary size: 26.8KB (5% of flash)
- No compilation errors or warnings
- Preset infrastructure ready for use

### ✅ Documentation Updates (COMPLETED)

**imu_hal.md**:
- Added complete ICM-20602 configuration section
- Added ICM-20602 column to chip comparison table
- Added "Understanding DLPF_CFG=0 + divider Behavior" section
- Updated LUT implementation examples to include ICM-20602

**imu_presets.md**:
- Minor formatting improvements for consistency

**MIGRATION_PLAN.md**:
- Created comprehensive 4-phase migration plan
- Detailed timeline (11-16 days)
- User decisions documented (self-test preserve, FIFO remove, etc.)
- Complete code examples for all methods

## In Progress

### 🔄 Phase 1.4: Extract and Adapt TDK Self-Test (IN PROGRESS)

**Objective**: Port TDK InvenSense factory self-test algorithms to Betaflight driver while preserving 100% factory test logic.

**TDK Source Files Identified**:
- `libraries/ICM42688P/src/Invn/Drivers/Icm426xx/Icm426xxSelfTest.c` - Factory algorithms
- `libraries/ICM42688P/examples/example-selftest/example-selftest.c` - Reference integration

**Next Steps**:
1. Analyze TDK self-test implementation structure
2. Extract factory algorithm functions
3. Adapt to Betaflight DeviceBus abstraction
4. Create `runSelfTest()` method in ICM42688_BF
5. Add self-test example sketch
6. Validate against TDK reference output

**Estimated Time Remaining**: 3-4 days

## Pending Phases

### 📋 Phase 2: Update IMU.cpp/.h to Preset-Based API (2-3 days)

**Objective**: Replace TDK driver calls with Betaflight driver + preset API.

**Key Changes**:
- Replace TDK `inv_icm426xx_*` calls with BF driver methods
- Update `Init()` to use `applyPreset()`
- Remove TDK-specific enums, replace with numeric constants
- Update `RunSelfTest()` to call BF self-test method
- Remove `ReadDataFromFifo()` (breaking change - acceptable per user decision)

**Not Started**

### 📋 Phase 3: Add Preset Support to Other Drivers (2-3 days)

**Objective**: Extend MPU6000_BF, MPU9250_BF, ICM206xx_BF with preset support.

**Drivers to Update**:
1. **MPU6000_BF** - Classic DLPF, divider always active
2. **MPU9250_BF** - MPU-6500-class, divider only with DLPF engaged
3. **ICM206xx_BF** - Wide/bypass mode, software decimation required

**Pattern**: Apply same LUT-based preset approach as ICM42688_BF

**Not Started**

### 📋 Phase 4: Testing and Validation (2-3 days)

**Objective**: Verify migration correctness and update all examples.

**Test Plan**:
1. Compile all IMU examples
2. Hardware validation on NUCLEO_F411RE
3. Compare preset output vs TDK driver (WHO_AM_I, gyro/accel data)
4. Verify self-test pass/fail matches TDK reference
5. Update all example sketches to use preset API
6. Binary size comparison (expect ~25KB reduction)

**Not Started**

## User Decisions (Confirmed)

1. **Self-test**: ✅ Preserve - Extract and adapt TDK self-test algorithms
2. **Enum migration**: ✅ Clean break - Replace all TDK enums with numeric constants
3. **Runtime config**: ✅ Full support via preset-based API (SAFE/SMOOTH/BALANCED/ACRO)
4. **FIFO support**: ✅ Remove - Delete `ReadDataFromFifo()` API (acceptable breaking change)

## Key Architectural Decisions

### Preset-Based Configuration Philosophy
- **User Intent**: Users select preset by intent (SAFE/SMOOTH/BALANCED/ACRO), not individual registers
- **Single Source of Truth**: All register values from imu_hal.md specification (lines 42-58)
- **Consistent Across Chips**: Same intent produces equivalent behavior on all IMUs
- **Protected Low-Level Methods**: Individual config methods exist but are internal implementation details

### FSR Standardization
- **All presets**: ±2000dps gyro, ±16g accel (per imu_hal.md lines 20-21)
- **Rationale**: Flight controller standard, good resolution without clipping
- **Future**: Could add separate FSR preset enum if needed

### ODR Encoding Quirk
- **Non-sequential values**: 8kHz=0x03, 4kHz=0x05 (NOT 0x04!), 2kHz=0x06, 1kHz=0x07
- **Source**: ICM-42688-P datasheet register map
- **Handled**: Switch statement in `setGyroODR()` maps Hz to register codes

## Breaking Changes

### Removed APIs (Phase 2)
- `ReadDataFromFifo()` - FIFO support removed per user decision
- All TDK enum types replaced with numeric constants

### Migration Path for Users
**Before** (TDK driver):
```cpp
IMU imu;
imu.Init(spi, cs_pin, 1000000);
// TDK driver auto-configures with hardcoded settings
```

**After** (BF driver + presets):
```cpp
IMU imu;
imu.Init(spi, cs_pin, 1000000);
imu.ApplyPreset(ImuPreset::FILTER_BALANCED);  // Explicit preset selection
```

### Binary Size Impact
- **Expected reduction**: ~25KB (TDK driver removal)
- **Current baseline**: 26.8KB (AutoDetect_Single with TDK)
- **Validation**: Will measure in Phase 4

## Files Modified

### Phase 1 Changes
```
libraries/imu/
├── imu_hal.md                        # Added ICM-20602, DLPF divider explanation
├── imu_presets.md                    # Minor formatting improvements
├── MIGRATION_PLAN.md                 # NEW: Complete migration plan
├── MIGRATION_STATUS.md               # NEW: This status document
└── src/devices/
    ├── ICM42688_BF.h                 # Added ImuPreset enum, applyPreset(), protected methods
    └── ICM42688_BF.cpp               # Added preset LUT, applyPreset(), 8 config methods
```

### Pending Changes (Phase 2+)
```
libraries/imu/
├── src/
│   ├── IMU.h                         # Remove TDK includes, add preset API
│   ├── IMU.cpp                       # Replace TDK calls with BF driver
│   └── devices/
│       ├── MPU6000_BF.h/.cpp         # Add preset support
│       ├── MPU9250_BF.h/.cpp         # Add preset support
│       └── ICM206xx_BF.h/.cpp        # Add preset support
└── examples/
    ├── AutoDetect_Single/            # Update to use presets
    ├── AutoDetect_Multiple/          # Update to use presets
    └── [all other examples]          # Update to use presets
```

## Success Criteria

### Phase 1 (Current) ✅
- ✅ ImuPreset enum defined
- ✅ Preset LUT matches imu_hal.md specification
- ✅ applyPreset() method implemented
- ✅ 8 protected config methods implemented
- ✅ Compiles without errors
- 🔄 Self-test extraction (in progress)

### Phase 2 (Pending)
- IMU.cpp/IMU.h compile without TDK includes
- All TDK function calls replaced
- Preset API integrated into IMU class

### Phase 3 (Pending)
- MPU6000/MPU9250/ICM206xx have preset support
- All 4 drivers use consistent LUT approach

### Phase 4 (Pending)
- All examples compile and run
- Hardware validation confirms correct operation
- Binary size reduction measured (~25KB expected)
- Documentation updated

## Timeline

| Phase | Task | Duration | Status |
|-------|------|----------|--------|
| 1.1 | Preset infrastructure | 0.5 days | ✅ COMPLETED |
| 1.2 | Public preset API | 0.5 days | ✅ COMPLETED |
| 1.3 | Protected config methods | 1 day | ✅ COMPLETED |
| 1.4 | Extract/adapt self-test | 3-4 days | 🔄 IN PROGRESS |
| 2 | Update IMU.cpp/.h | 2-3 days | 📋 PENDING |
| 3 | Extend other BF drivers | 2-3 days | 📋 PENDING |
| 4 | Testing and validation | 2-3 days | 📋 PENDING |
| **Total** | | **11-16 days** | **~20% complete** |

## Next Session

**Resume Point**: Phase 1.4 - Extract and adapt TDK self-test

**Context**:
- TDK self-test source: `libraries/ICM42688P/src/Invn/Drivers/Icm426xx/Icm426xxSelfTest.c`
- Reference example: `libraries/ICM42688P/examples/example-selftest/example-selftest.c`
- Goal: Port factory algorithms to BF driver while preserving 100% test logic

**Immediate Next Steps**:
1. Read and analyze TDK self-test implementation
2. Identify factory algorithm functions to preserve
3. Design BF driver integration (DeviceBus abstraction)
4. Implement `runSelfTest()` method in ICM42688_BF
5. Create validation example sketch
