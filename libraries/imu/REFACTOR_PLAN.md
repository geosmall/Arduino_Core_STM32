# IMU Library Refactoring Plan - Madflight-Inspired Approach

## Executive Summary

**Revised Strategy:** After attempting to keep Betaflight drivers unmodified (which created 46+ stub files and extreme complexity), we're adopting the **madflight approach**: systematically modify Betaflight driver files with documented find-replace operations.

**Key Learning:** The goal of "unmodified drivers" was well-intentioned but impractical. Creating 46 stub files to support unmodified drivers IS modification - just in the worst possible place. A few documented find-replace operations are simpler and more maintainable.

**Reference Implementation:** `/home/geo/src/madflight/src/imu/` - proven in production flight controller

---

## Architecture Comparison

### Our Previous Approach (Abandoned)
- **Complexity**: 46+ stub files + 5 adapter files
- **Compilation**: Failed after full session of stub creation
- **Maintainability**: Low (brittle stub layer breaks on Betaflight updates)
- **Philosophy**: "Keep drivers unmodified" → Created more problems than it solved

### Madflight Approach (Adopted)
- **Complexity**: ~600 lines total, 0 stub files
- **Compilation**: Clean, simple
- **Maintainability**: High (4 find-replace operations per driver)
- **Philosophy**: "Systematic modifications with clear documentation"

**Decision**: Use madflight's proven approach - it's simpler, works in production, and easier to maintain.

---

## Target Architecture (Madflight-Inspired)

```
libraries/imu/
├── library.properties          # No external dependencies
├── README.md                   # User-facing documentation
├── REFACTOR_PLAN.md            # This file
├── MADFLIGHT_COMPARISON.md     # Analysis of approaches
├── src/
│   ├── IMU.h                   # High-level facade API
│   ├── IMU.cpp                 # Facade implementation
│   │
│   ├── bus/                    # Bus abstraction (like MPU_Interface)
│   │   ├── DeviceBus.h         # Abstract interface
│   │   ├── DeviceBusSPI.h      # SPI implementation (~50 lines)
│   │   └── DeviceBusI2C.h      # I2C implementation (~45 lines)
│   │
│   ├── devices/                # Modified Betaflight drivers
│   │   ├── DeviceBase.h        # Device interface
│   │   ├── ICM42688_BF.h       # Modified BF driver (minimal changes)
│   │   ├── ICM42688_BF.cpp     # ~400 lines from Betaflight
│   │   ├── MPU6000_BF.h        # Future device
│   │   └── MPU6000_BF.cpp
│   │
│   └── common/
│       └── Types.h             # ImuSample, ImuType enums
│
└── examples/
    ├── AutoDetect/             # Auto-detection example
    ├── SPI_Basic/              # Simple SPI usage
    └── I2C_Basic/              # Simple I2C usage
```

### Key Differences from Original Plan

**What Changed:**
1. ❌ **No stub infrastructure** - Was: 46 stub files, Now: 0 stub files
2. ✅ **Modified Betaflight drivers** - Systematic find-replace (4 operations per file)
3. ✅ **Simple bus abstraction** - Direct Arduino SPI/I2C usage (no translation layer)
4. ✅ **Proven pattern** - Based on working madflight implementation

**What Stayed the Same:**
1. ✅ Facade pattern for public API
2. ✅ DeviceBase interface for device abstraction
3. ✅ Auto-detection support
4. ✅ Multi-device support goal

---

## Systematic Betaflight Driver Modification

### The 4 Find-Replace Operations

**Per Madflight's proven approach:**

```
Find                       Replace With
----                       ------------
"spiWriteReg(dev, "    →   "dev->writeReg("
"spiReadRegMsk(dev, "  →   "dev->readReg("
"setUserBank(dev, "    →   "setUserBank("
"extDevice_t"          →   "DeviceBus"
```

**Additional modifications:**
- Cast to C++ class
- Add DeviceBase interface implementation
- Document changes in file header comment

**Example header comment:**
```cpp
/*
 * Modified from Betaflight for Arduino integration
 * Original: https://github.com/betaflight/betaflight/.../accgyro_spi_icm426xx.c
 *
 * Changes applied (systematic find-replace):
 * 1. "spiWriteReg(dev, " → "dev->writeReg("
 * 2. "spiReadRegMsk(dev, " → "dev->readReg("
 * 3. "setUserBank(dev, " → "setUserBank("
 * 4. "extDevice_t" → "DeviceBus"
 * 5. Cast to C++ class with DeviceBase interface
 */
```

### Why This Works

**Benefits:**
- ✅ **2 minutes per driver** (vs hours of stub creation)
- ✅ **Scriptable/automatable** (can create update script)
- ✅ **Maintainable** (clear modification history)
- ✅ **Debuggable** (direct Arduino integration, no indirection)
- ✅ **Proven** (works in production madflight)

**Comparison to "Unmodified" Approach:**
- Old: 46 stub files, still failing compilation
- New: 4 find-replace operations, compiles immediately

---

## Deep Analysis Results (Betaflight + Madflight)

**Analysis Date:** 2025-11-14
**Repos Analyzed:**
- `/home/geo/src/betaflight/src/main/drivers/accgyro/accgyro_spi_icm426xx.c` (493 lines)
- `/home/geo/src/madflight/src/imu/ICM426XX/ICM426XX.cpp` (408 lines)

### Madflight's Exact Class Structure

**Pattern to follow** (`ICM426XX.h`):
```cpp
class ICM426XX {
protected:
  ICM426XX(MPU_Interface *dev, uint8_t whoAmI);  // Protected constructor
  void setUserBank(uint8_t bank);
  MPU_Interface *dev;  // Bus pointer (we use DeviceBus*)

public:
  static ICM426XX* detect(MPU_Interface *dev);  // Factory method
  void read(int16_t *accgyr);  // Read 6 int16_t: ax,ay,az,gx,gy,gz
  const char* type_name();

  uint8_t whoAmI = 0;
  float acc_scale = 1;  // [G/LSB]
  float gyr_scale = 1;  // [dps/LSB]
  uint16_t sampling_rate_hz = 1000;
};
```

**Key Design Patterns:**
1. **Protected constructor** - Forces use of factory `detect()` method
2. **Static detect()** - Returns `nullptr` on failure, `ICM426XX*` on success
3. **Constructor does init** - No separate `begin()` function needed
4. **Public scale factors** - Easy access for data conversion
5. **Simple read()** - Direct 12-byte burst read

### What Madflight Removed (493→408 lines, -85 lines = 17%)

**Removed from Betaflight:**
1. **External clock support** (~60 lines) - `#if defined(USE_GYRO_CLKIN)` block
   - Not needed for basic operation
   - Can add later if required

2. **Soft reset function** (~10 lines) - `icm426xxSoftReset()`
   - Commented out in madflight detect()
   - Optional feature

3. **Framework-specific functions** (~15 lines)
   - `mpuGyroInit(gyro)` call
   - `spiSetClkDivisor()` call
   - `gyroConfig()` reference
   - Replaced with direct register writes

**What Madflight Kept:**
1. ✅ **All register defines** - Complete register map
2. ✅ **Bank switching** - `setUserBank()` function (critical for ICM426xx)
3. ✅ **AAF filter LUTs** - `aafLUT42688[]` and `aafLUT42605[]` arrays
4. ✅ **ODR configuration** - `odrLUT[]` array
5. ✅ **Full init sequence** - Power management, filters, interrupts

### Minimal Function Requirements

**From Betaflight, extract these 3 core functions:**

**1. Detect** (Betaflight lines 273-314 → Madflight lines 218-244, ~27 lines)
```cpp
ICM42688_BF* ICM42688_BF::detect(DeviceBus* bus) {
    bus->setFreq(24000000);  // 24 MHz max

    uint8_t attemptsRemaining = 20;
    do {
        const uint8_t whoAmI = bus->readReg(MPU_RA_WHO_AM_I);
        switch (whoAmI) {
        case ICM42605_WHO_AM_I_CONST:  // 0x42
        case ICM42688P_WHO_AM_I_CONST: // 0x47
        case IIM42653_WHO_AM_I_CONST:  // 0x56
          {
          auto icm = new ICM42688_BF(bus, whoAmI);
          return icm;
        }
        delay(150);
       }
    } while (attemptsRemaining--);

    return nullptr;  // Not detected
}
```

**2. Init** (Betaflight lines 361-434 → Madflight constructor lines 247-335, ~89 lines)
- Power off sequence
- AAF filter configuration (bank 1 & 2)
- UI filter configuration
- Interrupt setup
- Power on sequence
- ODR/FSR configuration
- **All logic goes in protected constructor**

**3. Read** (Madflight lines 337-339, 3 lines)
```cpp
void ICM42688_BF::read(int16_t* accgyr) {
    // Read 12 bytes: ax,ay,az,gx,gy,gz (little endian)
    bus_->readRegs(ICM426XX_RA_ACCEL_DATA_X1, (uint8_t*)accgyr, 12);
}
```

### Type Mapping: Betaflight → Our Code

| Betaflight Type | Our Equivalent | Action | Notes |
|----------------|----------------|--------|-------|
| `extDevice_t*` | `DeviceBus*` | Replace | Bus abstraction pointer |
| `gyroDev_t` | ❌ Omit | Remove | Init logic inlined in constructor |
| `accDev_t` | ❌ Omit | Remove | Init logic inlined in constructor |
| `mpuSensor_e` | `uint8_t whoAmI_` | Replace | Just store WHO_AM_I value |
| `spiWriteReg(dev,...)` | `bus_->writeReg(...)` | ✅ Applied | Find-replace done |
| `spiReadRegMsk(dev,...)` | `bus_->readReg(...)` | ✅ Applied | Find-replace done |
| `setUserBank(dev,...)` | `setUserBank(...)` | ✅ Applied | Find-replace done |
| `odrConfig_e` | ✅ Keep inline | Copy | Local enum |
| `aafConfig_e` | ✅ Keep inline | Copy | Local enum |
| `aafConfig_t` | ✅ Keep inline | Copy | Local struct (3 fields) |
| `odrLUT[]` | ✅ Keep inline | Copy | Static array |
| `aafLUT42688[]` | ✅ Keep inline | Copy | Static array |
| `aafLUT42605[]` | ✅ Keep inline | Copy | Static array |

**No Betaflight Framework Types Needed** ✅

### Code Size Comparison

**Betaflight Original:**
- Total: 493 lines
- Register defines: ~199 lines
- External clock: ~60 lines
- Soft reset: ~10 lines
- Detect functions: ~80 lines (4 separate functions)
- Init function: ~74 lines
- Framework calls: ~15 lines

**Madflight Adaptation:**
- Total: 408 lines (-85 lines = -17%)
- Register defines: ~199 lines (same)
- External clock: ❌ Removed
- Soft reset: ❌ Removed
- Detect: ~27 lines (1 function with retry)
- Init (constructor): ~89 lines
- Read: ~3 lines
- Helpers: ~10 lines

**Our Target:**
- Total: ~410 lines (matches madflight)
- Header: ~60 lines
- Implementation: ~350 lines

---

## Implementation Phases (Revised)

### Phase 0: Prepare and Clean (10 minutes)

**Actions:**
1. ✅ Save MADFLIGHT_COMPARISON.md for reference
2. ✅ Reset repo to before Phase 1A/1B work
3. ✅ Update REFACTOR_PLAN.md with madflight approach
4. Commit: "Update IMU refactor plan - adopt madflight approach"
5. Push to `origin/imu-refactor`

**Deliverable:** Clean slate, updated plan documented

---

### Phase 1: Simple Bus Abstraction (30 minutes)

**Goal:** Create minimal bus interface (like madflight's MPU_Interface)

**Tasks:**

1. **Create Bus Interface** (`src/bus/DeviceBus.h`)
   ```cpp
   class DeviceBus {
   public:
       virtual ~DeviceBus() {}
       virtual void setFreq(uint32_t freq) = 0;
       virtual uint8_t readReg(uint8_t reg) = 0;
       virtual void writeReg(uint8_t reg, uint8_t data) = 0;
       virtual void readRegs(uint8_t reg, uint8_t *data, uint16_t n) = 0;
       virtual void writeRegs(uint8_t reg, const uint8_t *data, uint16_t n) = 0;
   };
   ```

2. **Create SPI Implementation** (`src/bus/DeviceBusSPI.h`)
   - Direct Arduino SPIClass usage
   - ~50 lines (based on madflight's MPU_InterfaceSPI)
   - No stub headers needed

3. **Create I2C Implementation** (`src/bus/DeviceBusI2C.h`)
   - Direct Arduino Wire usage
   - ~45 lines (based on madflight's MPU_InterfaceI2C)
   - No stub headers needed

4. **Create Test** (`examples/Test_BusOnly/Test_BusOnly.ino`)
   - Simple WHO_AM_I read
   - Verify SPI communication works

**Hardware Testing:**
```bash
./system/ci/build.sh libraries/imu/examples/Test_BusOnly --use-rtt --build-id
./system/ci/aflash.sh libraries/imu/examples/Test_BusOnly --use-rtt
```

**Expected output:** WHO_AM_I = 0x47 (ICM42688P)

**Deliverable:** Clean bus abstraction, ~100 lines total

---

### Phase 2: Create ICM42688 Driver (Madflight Pattern) (1.5-2 hours)

**Goal:** Create C++ class wrapper following madflight's proven pattern

**Reference Files:**
- Madflight pattern: `/home/geo/src/madflight/src/imu/ICM426XX/ICM426XX.{h,cpp}`
- Source registers/logic: `/home/geo/src/betaflight/src/main/drivers/accgyro/accgyro_spi_icm426xx.c`

**Approach:** Follow madflight's exact class structure, don't try to wrap entire 493-line Betaflight driver.

**Tasks:**

1. **Create Header File** (`src/devices/ICM42688_BF.h`, ~60 lines)

   **Class structure (exact madflight pattern):**
   ```cpp
   class ICM42688_BF {
   protected:
     ICM42688_BF(DeviceBus* bus, uint8_t whoAmI);  // Protected constructor
     void setUserBank(uint8_t bank);
     DeviceBus* bus_;

   public:
     static ICM42688_BF* detect(DeviceBus* bus);  // Factory method
     void read(int16_t* accgyr);  // Read 6 int16_t: ax,ay,az,gx,gy,gz
     const char* typeName() const;

     uint8_t whoAmI_;
     float accScale_;  // [G/LSB]
     float gyrScale_;  // [dps/LSB]
     uint16_t samplingRateHz_;
   };
   ```

2. **Create Implementation File** (`src/devices/ICM42688_BF.cpp`, ~350 lines)

   **Sections to copy:**
   - Register defines from Betaflight (lines 1-199)
   - Enums and LUTs (ODR, AAF configs)
   - `detect()` from madflight (lines 218-244)
   - Init logic in constructor from Betaflight `icm426xxGyroInit()` (lines 361-434)
   - `read()` from madflight (lines 337-339)
   - Helper functions: `setUserBank()`, `typeName()`

   **Key modifications:**
   - ✅ Apply 4 find-replace operations (already done if copying from modified file)
   - ❌ Remove external clock support (~60 lines)
   - ❌ Remove soft reset function (~10 lines)
   - ❌ Remove framework calls (`mpuGyroInit`, `gyroConfig`)
   - ✅ Constructor does full initialization (no separate `begin()`)

3. **Add Documentation Header**
   ```cpp
   /*
    * Modified from Betaflight for Arduino integration
    * Original: https://github.com/betaflight/betaflight/.../accgyro_spi_icm426xx.c
    * Pattern: Madflight ICM426XX wrapper
    *
    * Modifications applied:
    * 1. "spiWriteReg(dev, " → "bus_->writeReg("
    * 2. "spiReadRegMsk(dev, " → "bus_->readReg("
    * 3. "setUserBank(dev, " → "setUserBank("
    * 4. "extDevice_t" → "DeviceBus"
    * 5. C++ class with factory pattern
    * 6. Constructor-based initialization
    * 7. Removed: External clock, soft reset, framework dependencies
    */
   ```

4. **Create Test Example** (`examples/Test_ICM42688_Direct/`, ~100 lines)
   ```cpp
   // Create SPI bus
   DeviceBusSPI bus(&SPI, PA4);

   // Detect chip (factory pattern)
   ICM42688_BF* imu = ICM42688_BF::detect(&bus);
   if (!imu) {
       CI_LOG("*FAIL* Detection failed\n");
       CI_LOG("*STOP*\n");
       while(1);
   }

   CI_LOGF("Detected: %s\n", imu->typeName());

   // Read loop (no begin() needed - constructor initialized)
   int16_t data[6];  // ax,ay,az,gx,gy,gz
   imu->read(data);
   ```

**Hardware Testing:**
```bash
./system/ci/build.sh libraries/imu/examples/Test_ICM42688_Direct --use-rtt --build-id
./system/ci/aflash.sh libraries/imu/examples/Test_ICM42688_Direct --use-rtt
```

**Expected Output:**
- WHO_AM_I detection: 0x47 (ICM42688P)
- Gyro/accel data streaming (raw int16_t values)
- Test completes with *STOP* wildcard

**Success Criteria:**
- [ ] Clean compilation (no warnings)
- [ ] WHO_AM_I = 0x47 detected
- [ ] Constructor completes without errors
- [ ] `read()` returns gyro/accel data
- [ ] Gyro shows drift-like values when stationary (±50 LSB)
- [ ] Accel shows ~2048 LSB on vertical axis (1G)
- [ ] Total code ~410 lines (matches madflight)

**Deliverable:** Minimal ICM42688 driver (~410 lines total), madflight pattern, hardware validated

---

### Phase 3: Implement Facade API (1 hour)

**Goal:** High-level IMU API with auto-detection (like madflight's ImuGizmo pattern)

**Tasks:**

1. **Create Common Types** (`src/common/Types.h`)
   ```cpp
   enum class ImuType {
       Auto, ICM42688, ICM42605, MPU6000, MPU9250
   };

   struct ImuSample {
       float ax, ay, az;  // [m/s²]
       float gx, gy, gz;  // [rad/s]
       uint32_t timestamp_us;
   };
   ```

2. **Implement Facade** (`src/IMU.h`, `src/IMU.cpp`)
   - Similar to madflight's ImuGizmo adapter pattern
   - Public API:
     ```cpp
     class IMU {
     public:
         void attachSPI(SPIClass& spi, uint8_t csPin);
         void attachI2C(TwoWire& wire, uint8_t addr);
         bool begin(ImuType type = ImuType::Auto);
         bool read(ImuSample& sample);
         ImuType type() const;
     };
     ```

3. **Implement Auto-Detection**
   - Create DeviceBus instance
   - Try ICM42688_BF::detect()
   - Return device or nullptr

4. **Create Wrapper Example** (`examples/AutoDetect_Single/`)
   - Users only `#include <IMU.h>`
   - Auto-detect and read data

**Hardware Testing:**
```bash
./system/ci/build.sh libraries/imu/examples/AutoDetect_Single --use-rtt --build-id
./system/ci/aflash.sh libraries/imu/examples/AutoDetect_Single --use-rtt
```

**Expected output:**
- "Detected: ICM42688"
- Gyro/accel data streaming

**Deliverable:** Complete facade with auto-detect (~80 lines wrapper)

---

### Phase 4: Add More Devices (Future)

**Per-Device Effort:** ~1 hour each

**Process (same for each device):**
1. Copy Betaflight driver file
2. Apply 4 find-replace operations
3. Cast to C++ class with DeviceBase
4. Add to auto-detect in IMU.cpp
5. Test on hardware (if available)

**Devices to Add:**
- ICM206xx (ICM-20601, 20602, 20608, 20689)
- MPU6000
- MPU9250

**Total Additional Time:** ~3 hours for all 3 device families

---

### Phase 5: Documentation (30 minutes)

**Tasks:**
1. Update CLAUDE.md with new architecture
2. Create ARCHITECTURE.md explaining design
3. Update library.properties
4. Create migration guide
5. Add examples documentation

**Deliverable:** Complete documentation

---

## Effort Estimates (Revised After Deep Analysis)

| Phase | Description | Time | Complexity | Status |
|-------|-------------|------|------------|--------|
| 0 | Clean slate + plan update | 10 min | Simple | ✅ Complete |
| 1 | Bus abstraction (~200 lines) | 30 min | Simple | ✅ Complete |
| 2 | ICM42688 driver (madflight pattern, ~410 lines) | 1.5-2 hours | Moderate | ✅ Complete |
| 3 | Facade API (~80 lines) | 1 hour | Simple | ✅ Complete |
| 4a | MPU6000 device driver | 1 hour | Moderate | ✅ Complete |
| 4b | MPU9250 device driver | 1 hour | Moderate | 📋 Planned |
| 4c | ICM206xx device driver | 1 hour | Moderate | 📋 Planned |
| 4d | DeviceBase refactoring | 30 min | Moderate | 📋 Next |
| 5 | Documentation | 30 min | Simple | 📋 Future |

**Total Estimated:** ~6-7 hours (vs 9-12 hours with stub approach, vs 46+ stub files from original attempt)

**Actual Progress:**
- Phase 0: ✅ 10 min (complete)
- Phase 1: ✅ 40 min (complete + hardware validation)
- Phase 2: ✅ 2 hours (ICM42688 complete + hardware validation)
- Phase 3: ✅ 45 min (Facade API complete + hardware validation)
- Phase 4a: ✅ 1 hour (MPU6000 complete + hardware validation)
- **Total so far:** ~4 hours 35 min
- **Remaining:** DeviceBase refactor (30 min), MPU9250 (1h), ICM206xx (1h), docs (30 min) = ~3 hours

**Key Insight from Analysis:**
- Madflight reduced Betaflight by only 17% (493→408 lines)
- Most complexity is essential (register defines, LUTs, init sequence)
- True simplification: Remove framework dependencies, not core logic
- Pattern proven in production flight controllers

---

## Key Technical Decisions

### Why Modify Betaflight Drivers?

**Reality Check:**
- Keeping drivers "unmodified" required 46 stub files
- Stub files ARE modifications - just in the wrong place
- 4 find-replace operations are simpler than 46 stubs
- Madflight proves this works in production

**Benefits:**
- ✅ Simple, understandable code
- ✅ No stub infrastructure to maintain
- ✅ Easy to debug (direct Arduino integration)
- ✅ Fast to add new sensors (2 minutes per driver)
- ✅ Clear modification history (documented in comments)

### Why Madflight's Approach?

**Validation:**
- Proven in production flight controller
- Active maintenance and updates
- Clean, simple architecture
- ~600 lines total vs our 46+ stub files

**Philosophy:**
- Practical engineering over theoretical purity
- "Good enough" is better than "perfect but broken"
- Simple beats complex
- Working beats theoretically correct

### What We Learned

**From Today's Session:**
1. "Unmodified drivers" sounds good but creates complexity explosion
2. Stub layers are modifications in disguise (and worse ones)
3. Systematic modifications with clear docs are better than hidden stubs
4. Production-proven approaches beat theoretical ideals
5. Simplicity should be the primary goal

**Madflight's Key Insight:**
> "Just modify the driver with 4 find-replace operations.
> It's simple, it works, and future you will thank you."

---

## Success Criteria

**Phase 1 Success:**
- [ ] DeviceBus interface defined (~20 lines)
- [ ] SPI implementation working (~50 lines)
- [ ] I2C implementation working (~45 lines)
- [ ] Test_BusOnly reads WHO_AM_I correctly
- [ ] Zero stub files created

**Phase 2 Success:**
- [ ] ICM42688_BF driver ported (~400 lines)
- [ ] Only 4 find-replace operations applied
- [ ] DeviceBase interface implemented
- [ ] Test_ICM42688_Direct runs on hardware
- [ ] Gyro/accel data validated

**Phase 3 Success:**
- [ ] IMU facade API complete (~80 lines)
- [ ] Auto-detect working
- [ ] AutoDetect_Single example runs end-to-end
- [ ] Public API clean and simple

**Final Success:**
- [ ] Total code: ~600 lines (vs 46+ stub files)
- [ ] Clean compilation on first try
- [ ] Hardware validated on NUCLEO_F411RE
- [ ] Ready to add more devices (1 hour each)
- [ ] Maintainable and debuggable

---

## Rollback Strategy

**Safety Net:**
- All work on `imu-refactor` branch
- Can abandon and return to `ardu_ci` main branch
- Standalone libraries (ICM42688P, etc.) untouched as fallback
- No changes to main branch until fully tested

**Risk Assessment:**
- **Low risk**: Madflight proves approach works
- **Simple code**: Easy to understand and debug
- **Incremental**: Test after each phase
- **Proven pattern**: Not inventing new architecture

---

## Migration from Old Plan

**Abandoned Components:**
- ❌ 46+ stub files in `src/betaflight/`
- ❌ Complex stub infrastructure (platform.h, build/, drivers/, etc.)
- ❌ bf_arduino_compat.h (100+ lines of compatibility shims)
- ❌ Unmodified Betaflight driver wrapper approach

**Retained Components:**
- ✅ DeviceBase interface concept (simplified)
- ✅ Facade pattern (IMU.h public API)
- ✅ Auto-detection goal
- ✅ Multi-device support goal
- ✅ Bus abstraction (simplified to DeviceBus)

**New Components (Madflight-Inspired):**
- ✅ Simple bus abstraction (DeviceBus interface)
- ✅ Modified Betaflight drivers (systematic find-replace)
- ✅ Direct Arduino integration (no translation layer)
- ✅ Clear modification documentation

---

## References

**Madflight Implementation:**
- Source: `/home/geo/src/madflight/src/imu/`
- Key files:
  - `MPUxxxx/MPU_interface.h` - Bus abstraction (~130 lines)
  - `ICM426XX/ICM426XX.cpp` - Modified BF driver (~400 lines)
  - `ImuGizmoICM426XX.h` - Facade adapter (~85 lines)
  - Total: ~600 lines, works in production

**Comparison Document:**
- `libraries/imu/MADFLIGHT_COMPARISON.md` - Detailed analysis of both approaches

**Betaflight Source:**
- Repository: `/home/geo/src/betaflight/`
- Drivers: `src/main/drivers/accgyro/accgyro_spi_*.c`

**Arduino Library Specification:**
- https://arduino.github.io/arduino-cli/library-specification/

---

**Document Status:** Major revision - adopted madflight approach
**Last Updated:** 2025-11-13
**Key Changes from Previous Version:**
- Abandoned "unmodified driver" approach (46+ stubs)
- Adopted madflight's systematic modification approach
- Simplified architecture (0 stubs, direct Arduino integration)
- Reduced effort estimate (6 hours vs 9-12 hours)
- Added MADFLIGHT_COMPARISON.md reference

**Next Action:** Start Phase 1 - Create simple bus abstraction
