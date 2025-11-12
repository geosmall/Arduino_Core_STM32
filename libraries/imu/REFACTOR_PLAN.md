# IMU Library Facade Pattern Refactoring Plan

## Overview

This document outlines the plan to refactor the IMU library from a simple wrapper around individual device drivers to a comprehensive facade pattern that provides unified auto-detection and bus abstraction.

## Strategic Approach

Based on analysis and user decisions:
- **Replace** current IMU library with facade pattern (on new branch `imu-refactor`)
- **Use Betaflight drivers** (not TDK) - smaller footprint, battle-tested, multi-chip support
- **Keep** standalone driver libraries (ICM42688P, ICM206xx, MPU6000, MPU9250) as examples/reference
- **Gradual transition** with incremental hardware testing after each component
- **Pattern source**: `doc/nested_libs.md` - Umbrella library design

## Current State

**Existing IMU Library (`libraries/imu/`):**
- Wraps ICM42688P driver with Betaflight integration
- Single device support (ICM-42688-P only)
- Direct dependency on ICM42688P library
- Basic C++ wrapper API

**Standalone Driver Libraries:**
- `ICM42688P/` - TDK InvenSense driver (2.4MB)
- `ICM206xx/` - ICM-20601/20602/20608/20689 support (96KB)
- `MPU6000/` - MPU-6000 driver (100KB)
- `MPU9250/` - MPU-9250/9255 driver (144KB)

**Betaflight Drivers (Already in IMU Library):**
- `imu/betaflight/drivers/accgyro/accgyro_spi_icm426xx.c` - ICM426xx family (~500 lines)
- `imu/betaflight/drivers/accgyro/accgyro_mpu.c` - Shared MPU functions
- These will be ported to facade pattern (smaller, production-proven)

## Target Architecture

**New IMU Library Structure:**
```
libraries/imu/
├── library.properties      ← Updated (no external dependencies)
├── README.md               ← Rewritten with facade pattern docs
├── REFACTOR_PLAN.md        ← This file
├── src/
│   ├── IMU.h               ← Public facade API (auto-detect, unified interface)
│   ├── IMU.cpp             ← Implementation with pimpl pattern
│   │
│   ├── common/             ← Shared types and utilities
│   │   ├── Types.h         ← ImuSample struct, ImuType enum
│   │   └── Utils.h         ← Helper functions (scaling, conversions)
│   │
│   ├── bus/                ← Hardware abstraction layer
│   │   ├── Bus.h           ← Abstract bus interface
│   │   ├── I2CBus.h        ← I2C implementation
│   │   ├── I2CBus.cpp
│   │   ├── SPIBus.h        ← SPI implementation
│   │   └── SPIBus.cpp
│   │
│   └── devices/            ← Per-chip drivers (internal)
│       ├── DeviceBase.h    ← Device interface (probe, begin, read)
│       ├── ICM42688.h      ← Ported from ICM42688P library
│       ├── ICM42688.cpp
│       ├── ICM206xx.h      ← Ported from ICM206xx library
│       ├── ICM206xx.cpp
│       ├── MPU6000.h       ← Ported from MPU6000 library
│       ├── MPU6000.cpp
│       ├── MPU9250.h       ← Ported from MPU9250 library
│       └── MPU9250.cpp
│
└── examples/
    ├── AutoDetect/         ← Auto-detection demo
    │   └── AutoDetect.ino
    ├── I2C_Basic/          ← I2C usage example
    │   └── I2C_Basic.ino
    └── SPI_Basic/          ← SPI usage example
        └── SPI_Basic.ino
```

**Key Principles:**
- Single `library.properties` at top level (no nested libraries)
- Users include only `#include <IMU.h>`
- Internal code uses relative includes: `"devices/ICM42688.h"`
- Arduino recursively compiles all `.cpp` files in `src/` tree
- Standalone libraries remain untouched (examples/reference only)

## Public API Design

**Facade Interface (`src/IMU.h`):**

```cpp
enum class ImuType : uint8_t {
  Auto = 0,      // Auto-detect via WHO_AM_I probing
  ICM42688,      // ICM-42688-P
  ICM20602,      // ICM-20602 (part of ICM206xx family)
  ICM20689,      // ICM-20689 (part of ICM206xx family)
  MPU6000,       // MPU-6000
  MPU9250,       // MPU-9250/9255
};

struct ImuSample {
  float ax, ay, az;           // Accel [m/s²] or [G]
  float gx, gy, gz;           // Gyro [rad/s] or [deg/s]
  uint32_t timestamp_us;      // Microseconds
};

class IMU {
public:
  // Bus configuration (call before begin)
  void attachI2C(TwoWire& wire, uint8_t addr);
  void attachSPI(SPIClass& spi, uint8_t csPin);

  // Initialize (auto-detect if type == Auto)
  bool begin(ImuType type = ImuType::Auto);

  // Data access
  bool read(ImuSample& sample);
  bool readGyro(float& x, float& y, float& z);
  bool readAccel(float& x, float& y, float& z);

  // Device info
  ImuType type() const;

private:
  class Impl;   // Pimpl pattern hides device headers from users
  Impl* impl_ = nullptr;
};
```

**Usage Example:**
```cpp
#include <IMU.h>

IMU imu;

void setup() {
  Wire.begin();
  imu.attachI2C(Wire, 0x68);

  if (!imu.begin(ImuType::Auto)) {
    Serial.println("IMU init failed");
    while(1);
  }

  Serial.print("Detected: ");
  switch(imu.type()) {
    case ImuType::ICM42688: Serial.println("ICM42688"); break;
    case ImuType::MPU6000:  Serial.println("MPU6000"); break;
    // ... other types
  }
}

void loop() {
  ImuSample s;
  if (imu.read(s)) {
    Serial.print("Gyro: ");
    Serial.print(s.gx); Serial.print(", ");
    Serial.print(s.gy); Serial.print(", ");
    Serial.println(s.gz);
  }
  delay(10);
}
```

## Implementation Phases

### Phase 0: Prepare for Refactoring (5 minutes)

**Goal:** Clean slate before major changes

**Prerequisites:**
- Current work committed (NERO F7 SDFS/Storage support)
- All commits pushed to `origin/ardu_ci`

**Actions:**
1. Verify clean working tree: `git status`
2. Commit pending changes if any
3. Push to remote: `git push origin ardu_ci`
4. Create new branch: `git checkout -b imu-refactor`
5. Confirm ready state

**Deliverable:** New branch ready for facade pattern work

---

### Phase 1A: Bus Abstraction + Types (30 minutes)

**Goal:** Create foundation types and verify bus communication

**Tasks:**

1. **Create Common Types** (`src/common/Types.h`)
   - Define `ImuSample` struct (ax, ay, az, gx, gy, gz, timestamp_us)
   - Define `ImuType` enum (Auto, ICM42688, ICM20602, ICM20689, MPU6000, MPU9250)
   - Add WHO_AM_I constants for each chip

2. **Document Existing Bus Abstraction** (`src/bus/` - note existing `bf_bus.h`)
   - Verify `bf_bus.h` works for facade pattern (already exists!)
   - Document SPIBus class usage
   - No new code needed - bf_bus.h is already the abstraction layer

3. **Create Test Sketch** (`examples/Test_BusOnly/Test_BusOnly.ino`)
   - Simple WHO_AM_I read via bf_bus
   - Print chip ID to verify SPI communication
   - No device initialization - just bus test

**Hardware Testing:**
- Build: `./system/ci/build.sh libraries/imu/examples/Test_BusOnly --build-id`
- Flash: `./system/ci/aflash.sh libraries/imu/examples/Test_BusOnly --use-rtt`
- **Expected output**: WHO_AM_I register = 0x47 (ICM42688P)
- **Pass criteria**: SPI communication working, correct chip ID read

**Deliverable:** Types defined, bus abstraction documented, SPI verified

---

### Phase 1B: Betaflight ICM426xx Port (2-3 hours)

**Goal:** Port Betaflight driver to DeviceBase interface with hardware validation

**Tasks:**

1. **Create Device Interface** (`src/devices/DeviceBase.h`)
   - Abstract base class with pure virtual methods
   - `static bool probe(Bus& bus)` - WHO_AM_I detection
   - `virtual bool begin()` - Initialize device
   - `virtual bool read(ImuSample& s)` - Read gyro+accel
   - `virtual ImuType kind() const` - Return device type

2. **Port Betaflight ICM426xx** (`src/devices/ICM42688_BF.h/.cpp`)
   - Extract from `betaflight/drivers/accgyro/accgyro_spi_icm426xx.c`
   - Adapt callback-based init to DeviceBase::begin()
   - Implement DeviceBase::probe() using icm426xxSpiDetect()
   - Adapt mpuAccReadSPI/mpuGyroReadSPI to DeviceBase::read()
   - Port register constants and configuration sequences

3. **Handle MPU Common Functions**
   - Port needed functions from `accgyro_mpu.c` into ICM42688_BF.cpp
   - Keep only what's necessary for ICM426xx (avoid full MPU dependency)

4. **Create Direct Test Sketch** (`examples/Test_ICM42688_Direct/`)
   - Test DeviceBase interface directly (no facade yet)
   - Instantiate ICM42688_BF device
   - Call probe(), begin(), read() methods
   - Print gyro/accel data

**Hardware Testing:**
- Build: `./system/ci/build.sh libraries/imu/examples/Test_ICM42688_Direct --build-id`
- Flash: `./system/ci/aflash.sh libraries/imu/examples/Test_ICM42688_Direct --use-rtt`
- **Expected output**:
  - WHO_AM_I probe: PASS (0x47)
  - Init sequence: PASS (registers configured)
  - Gyro data: Valid (stationary drift ~0.3-0.8 deg/s)
  - Accel data: Valid (Z ≈ 9.8 m/s² or 1G)
- **Pass criteria**: All init steps succeed, data reads non-zero and reasonable

**Deliverable:** Betaflight ICM42688 driver working via DeviceBase interface

---

### Phase 1C: Facade API (1 hour)

**Goal:** Implement public IMU facade with auto-detect (single device)

**Tasks:**

1. **Implement Facade** (`src/IMU.h/.cpp`)
   - Public API with pimpl pattern (hide DeviceBase from users)
   - `attachI2C()`, `attachSPI()` bus configuration methods
   - `begin(ImuType type = Auto)` with auto-detect via tryMakeDevice()
   - `read()`, `readGyro()`, `readAccel()` delegation to device
   - `type()` returns detected ImuType

2. **Implement tryMakeDevice()** (private helper)
   - Currently only probes ICM42688_BF
   - If probe succeeds, instantiate and return device
   - Returns nullptr if no device detected

3. **Create Facade Example** (`examples/AutoDetect_Single/`)
   - Users only `#include <IMU.h>`
   - Call `imu.attachSPI(SPI, csPin)`, `imu.begin(ImuType::Auto)`
   - Print detected chip type
   - Read and print gyro/accel data

**Hardware Testing:**
- Build: `./system/ci/build.sh libraries/imu/examples/AutoDetect_Single --build-id`
- Flash: `./system/ci/aflash.sh libraries/imu/examples/AutoDetect_Single --use-rtt`
- **Expected output**:
  - Auto-detect: ICM42688
  - Gyro/Accel data streaming
- **Pass criteria**: Auto-detect works, facade API functional end-to-end

**Deliverable:** Complete facade pattern with ICM42688 support via Betaflight driver

---

### Phase 2: Port Remaining Devices with Incremental Testing (2-3 hours)

**Goal:** Add full multi-device support to facade with hardware validation after each device

**Sub-Phase 2A: Port ICM206xx** (45 min)

1. **Port Betaflight ICM206xx** (`src/devices/ICM206xx_BF.h/.cpp`)
   - Extract from `betaflight/drivers/accgyro/accgyro_spi_icm20689.c` (shares code with ICM426xx)
   - Implement DeviceBase interface
   - Add probe() for WHO_AM_I (0x12 for ICM-20602, etc.)
   - Support ICM-20601, 20602, 20608, 20689

2. **Update Auto-Detection** (`src/IMU.cpp`)
   - Add ICM206xx to tryMakeDevice()
   - Probe order: ICM42688 → ICM206xx

3. **Hardware Test**:
   - Test on NERO F7 (has ICM-20602)
   - Verify auto-detect identifies correct chip
   - Verify data reads correctly
   - **Pass criteria**: ICM-20602 detected and functioning

**Sub-Phase 2B: Port MPU6000** (45 min)

1. **Port Betaflight MPU6000** (`src/devices/MPU6000_BF.h/.cpp`)
   - Extract from `betaflight/drivers/accgyro/accgyro_spi_mpu6000.c`
   - Implement DeviceBase interface
   - Add probe() for WHO_AM_I (0x68)

2. **Update Auto-Detection**
   - Add MPU6000 to tryMakeDevice()
   - Probe order: ICM42688 → ICM206xx → MPU6000

3. **Hardware Test** (if available):
   - Test on hardware with MPU6000 (or skip if unavailable)
   - Verify compilation at minimum
   - **Pass criteria**: Compiles cleanly, previous devices still work

**Sub-Phase 2C: Port MPU9250** (45 min)

1. **Port Betaflight MPU9250** (`src/devices/MPU9250_BF.h/.cpp`)
   - Extract from `betaflight/drivers/accgyro/accgyro_spi_mpu9250.c`
   - Implement DeviceBase interface
   - Add probe() for WHO_AM_I (0x71/0x73)

2. **Update Auto-Detection**
   - Add MPU9250 to tryMakeDevice()
   - Final probe order: ICM42688 → ICM206xx → MPU6000 → MPU9250

3. **Hardware Test** (if available):
   - Test on hardware with MPU9250 (or skip if unavailable)
   - Verify compilation at minimum
   - **Pass criteria**: All devices compile, auto-detect works for available chips

**Final Phase 2 Testing:**
- Verify forced selection mode (non-Auto ImuType)
- Test that auto-detect tries devices in correct order
- Validate each available chip type on hardware

**Deliverable:** Complete multi-device support (4 IMU families) with Betaflight drivers

---

### Phase 3: Migrate Examples and Tests (1-2 hours)

**Goal:** Update existing code to use new facade API

**Tasks:**

1. **Update IMU Examples**
   - Port existing `libraries/imu/examples/` to new API
   - Remove Betaflight-specific examples (obsolete)
   - Add migration notes in comments

2. **Mark Standalone Libraries as Examples**
   - Add README.md to each standalone library:
     - "This library is kept as reference/example"
     - "Prefer using unified IMU library with auto-detect"
     - "Direct usage: #include <ICM42688P.h> (legacy)"

3. **Test Compilation**
   - Build all new examples
   - Verify on target boards:
     - NUCLEO_F411RE (ICM42688P via SPI)
     - BLACKPILL_F411CE (if applicable)
     - NERO F7 (ICM20602 via SPI)

4. **Create Migration Guide** (`libraries/imu/MIGRATION.md`)
   - Old API → New API mapping
   - Code examples showing before/after
   - Benefits of facade pattern

**Testing:**
- Run all examples with `./system/ci/build.sh`
- Flash to hardware and verify functionality

**Deliverable:** Examples working with facade, migration guide complete

---

### Phase 4: Update dRehmFlight (1 hour - DEFERRED)

**Goal:** Migrate flight controller to facade pattern

**Note:** This phase deferred until flight testing is ready.

**Tasks:**
1. Update `sketches/dRehmFlight_STM32_BETA_1.3/dRehmFlight_STM32_BETA_1.3.ino`
2. Replace `#include <ICM42688P.h>` with `#include <IMU.h>`
3. Update `IMUinit()` to use facade API
4. Update `getIMUdata()` to use facade read methods
5. Test compilation
6. Hardware validation on NUCLEO_F411RE

**Testing:**
- Defer to flight testing phase
- Requires bench testing setup (no props)

**Deliverable:** Flight controller using unified IMU API (when ready)

---

### Phase 5: Documentation and Cleanup (1 hour)

**Goal:** Finalize refactoring with comprehensive documentation

**Tasks:**

1. **Update CLAUDE.md**
   - Replace old IMU library description
   - Document facade pattern architecture
   - Update "Completed Projects" section
   - Add library organization notes

2. **Create Architecture Documentation** (`libraries/imu/ARCHITECTURE.md`)
   - Explain facade pattern design
   - Document bus abstraction layer
   - Describe device driver interface
   - Show class relationships (UML-style text diagram)

3. **Update library.properties**
   - Version bump to 2.0.0 (breaking change)
   - Update description: "Unified IMU driver with auto-detect"
   - Remove `depends=ICM42688P` (now internal)

4. **Add Deprecation Notices**
   - Update standalone driver READMEs with deprecation notes
   - Recommend migration to unified IMU library
   - Keep libraries available for reference

5. **Clean Up Obsolete Code**
   - Remove old Betaflight wrapper if fully replaced
   - Clean up any unused internal code
   - Run `./system/ci/cleanup_repo.sh`

6. **Create Release Notes** (`libraries/imu/CHANGELOG.md`)
   - Document v2.0.0 breaking changes
   - List new features (auto-detect, multi-device, bus abstraction)
   - Migration instructions

**Deliverable:** Complete documentation, clean repository

---

## Effort Estimates

| Phase | Description | Time | Status |
|-------|-------------|------|--------|
| 0 | Prepare (commit, branch) | 5 min | Not started |
| 1A | Bus abstraction + types | 30 min | Not started |
| 1B | Betaflight ICM42688 port | 2-3 hours | Not started |
| 1C | Facade API | 1 hour | Not started |
| 2A | Port ICM206xx + test | 45 min | Not started |
| 2B | Port MPU6000 + test | 45 min | Not started |
| 2C | Port MPU9250 + test | 45 min | Not started |
| 3 | Migrate examples/tests | 1-2 hours | Not started |
| 4 | Update dRehmFlight | 1 hour | Deferred |
| 5 | Documentation cleanup | 1 hour | Not started |

**Total Estimated Effort:** 9-12 hours (excluding Phase 4)
**Note:** Slightly longer due to incremental hardware testing, but catches errors early

---

## Rollback Strategy

**If Issues Arise:**
1. All work on separate branch (`imu-refactor`)
2. Can abandon branch and return to `ardu_ci`
3. Standalone libraries remain untouched as fallback
4. No breaking changes to main branch until fully tested

**Risk Mitigation:**
- Incremental commits per sub-phase
- Hardware testing after each major component (1A, 1B, 1C, 2A, 2B, 2C)
- Betaflight drivers are battle-tested (lower risk than new code)
- Keep Phase 4 (dRehmFlight) deferred until facade fully proven

---

## Key Technical Decisions

### Why Betaflight Drivers (Not TDK)?
- **Smaller footprint:** ~500 lines vs 2900 lines (6× reduction)
- **Production proven:** Battle-tested in Betaflight/iNav flight controllers
- **Multi-chip support:** ICM-42605, ICM-42688-P, IIM-42652, IIM-42653 out of box
- **Already integrated:** Existing code in `imu/betaflight/` directory
- **Optimized:** Designed for embedded systems with tight memory constraints
- **Consistent:** All chips use same Betaflight driver architecture

### Why Facade Pattern?
- **Single include:** Users only need `#include <IMU.h>`
- **Auto-detection:** No need to know which chip at compile time
- **Unified API:** Same code works with any supported IMU
- **Bus abstraction:** Easy I2C/SPI switching (bf_bus.h already exists!)
- **Extensibility:** Adding new devices is straightforward

### Why Incremental Hardware Testing?
- **Catch errors early:** Test after each component (bus → device → facade)
- **Faster debugging:** Smaller code changes between tests
- **Confidence building:** Validate each layer before proceeding
- **CI/HIL integration:** Use existing `--use-rtt --build-id` infrastructure
- **Reduced risk:** Less code to debug if something breaks

### Why Keep Standalone Libraries?
- **Reference:** Serve as examples of direct driver usage
- **Backward compatibility:** Existing code can continue using them
- **Comparison:** Demonstrate difference between direct and facade patterns
- **Learning:** Educational value for library design

### Why Not Nested Libraries?
- Arduino doesn't support multiple `library.properties` files
- Subdirectories in `src/` are for internal organization only
- Only top-level library appears in Arduino IDE library manager

### Why Pimpl Pattern?
- Hides device driver headers from users
- Reduces compilation dependencies
- Cleaner public API (no device-specific types exposed)

---

## Success Criteria

**Phase 1A Success:**
- [ ] Types.h defines ImuSample and ImuType
- [ ] bf_bus.h documented for facade usage
- [ ] Test_BusOnly example reads WHO_AM_I = 0x47
- [ ] SPI communication verified on hardware

**Phase 1B Success:**
- [ ] DeviceBase interface defined
- [ ] Betaflight ICM426xx ported to DeviceBase
- [ ] Test_ICM42688_Direct: probe(), begin(), read() all working
- [ ] Gyro/accel data validated on hardware

**Phase 1C Success:**
- [ ] IMU facade API implemented with pimpl
- [ ] Auto-detect identifies ICM42688
- [ ] AutoDetect_Single example runs end-to-end
- [ ] Public API functional and tested

**Phase 2 Success:**
- [ ] All 4 device families compile
- [ ] Auto-detect tries all devices in order
- [ ] Forced selection mode works

**Phase 3 Success:**
- [ ] All examples compile without errors
- [ ] Examples run on target hardware
- [ ] Migration guide is clear and tested

**Final Success:**
- [ ] Complete facade pattern implementation
- [ ] All devices supported with auto-detect
- [ ] Documentation comprehensive
- [ ] Hardware validated on 3 boards
- [ ] Ready for dRehmFlight integration (Phase 4)

---

## References

- **Pattern Source:** `doc/nested_libs.md` - Umbrella library design
- **Arduino Library Spec:** https://arduino.github.io/arduino-cli/library-specification/
- **Betaflight Drivers (Source for Porting):**
  - `libraries/imu/betaflight/drivers/accgyro/accgyro_spi_icm426xx.c` - ICM426xx family
  - `libraries/imu/betaflight/drivers/accgyro/accgyro_spi_icm20689.c` - ICM206xx family
  - `libraries/imu/betaflight/drivers/accgyro/accgyro_spi_mpu6000.c` - MPU6000
  - `libraries/imu/betaflight/drivers/accgyro/accgyro_spi_mpu9250.c` - MPU9250
  - `libraries/imu/betaflight/drivers/accgyro/accgyro_mpu.c` - Shared MPU functions
- **Existing Bus Abstraction:**
  - `libraries/imu/src/bf_bus.h` - Already implemented SPI bus layer

---

**Document Status:** Revised plan with Betaflight drivers and incremental testing
**Last Updated:** 2025-11-12 (Revision 2)
**Key Changes from V1:**
- Branch renamed: `imu-facade-refactor` → `imu-refactor`
- Driver choice: TDK → Betaflight (smaller, proven, multi-chip)
- Phase 1 split into 1A/1B/1C with hardware testing checkpoints
- Phase 2 split into 2A/2B/2C with per-device testing
- Total effort: 9-12 hours (includes testing overhead)

**Next Action:** Execute Phase 0 (commit current work, push, create `imu-refactor` branch)
