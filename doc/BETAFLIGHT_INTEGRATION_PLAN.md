# Betaflight Integration Plan - Multi-Chip IMU Support

**Status:** In Progress
**Started:** 2025-11-02
**Approach:** Hybrid (Keep TDK ICM42688P + Add Betaflight MPU drivers)

---

## Executive Summary

Integrate Betaflight's production-validated IMU drivers to:
1. Fix critical AFSR bug in ICM42688P (gyro stall issue)
2. Add MPU-6000 and MPU-9250 support (currently missing)
3. Create unified multi-chip IMU interface
4. Enable chip-agnostic dRehmFlight operation

**Key Constraint:** Do NOT modify TDK InvenSense SDK code (`libraries/ICM42688P/src/Invn/`)

---

## Architecture Overview

### License Boundaries (GPL Containment)

```
✅ MIT Licensed:
- ICM42688P/ (TDK SDK + Arduino wrapper)
- imu/ (unified wrapper + bus abstraction)
- dRehmFlight/ (flight controller)

⚠️ GPL v3 Licensed:
- MPU6000/ (Betaflight-derived)
- MPU9250/ (Betaflight-derived)
- imu/betaflight/ (reference, not linked into MIT code)
```

**Strategy:** GPL libraries are separate, optional. Users can use MIT-only (ICM42688P) or add GPL libraries for MPU support.

---

## Phase 1: AFSR Fix in Arduino Wrapper (CRITICAL) - 1 day ⭐

### Problem
AFSR (Automatic Full-Scale Range) bug causes gyro output stalls on ICM-426xx family. Discovered by ArduPilot and Betaflight communities. See: https://github.com/ArduPilot/ardupilot/pull/25332

### Solution Location
**File:** `libraries/ICM42688P/src/icm42688p.c` (Arduino wrapper layer)
**NOT:** `libraries/ICM42688P/src/Invn/` (TDK SDK - untouched)

### Implementation

Apply AFSR workaround after TDK initialization:

```c
int icm42688p_init(struct inv_icm426xx *icm_driver, SPIClass *spi_bus,
                    uint8_t cs_pin, uint32_t spi_freq) {
    // ... existing TDK initialization ...

    /* AFSR workaround - must be applied after TDK init
     * Prevents gyro output stalls on ICM-426xx family
     * Reference: https://github.com/ArduPilot/ardupilot/pull/25332 */
    uint8_t intf_config1;
    rc = inv_icm426xx_read_reg(icm_driver, MPUREG_INTF_CONFIG1, 1, &intf_config1);
    if (rc != 0) return rc;

    intf_config1 &= ~0xC0;  // Clear AFSR bits [7:6]
    intf_config1 |= 0x40;   // Set AFSR_DISABLE

    rc = inv_icm426xx_write_reg(icm_driver, MPUREG_INTF_CONFIG1, 1, &intf_config1);

    return rc;
}
```

### Validation
- Test with `examples/ICM42688_Basic/`
- Verify no gyro stalls during extended operation
- Check register readback confirms 0x40 in bits [7:6]

---

## Phase 2: Bus Abstraction Foundation - 2 days

### Purpose
Create minimal Arduino-compatible abstraction for Betaflight drivers WITHOUT copying GPL code.

### Location
`libraries/imu/src/` (MIT licensed, clean-room implementation)

### New Files

#### 1. `bf_bus.h` - Bus abstraction interface
```cpp
// Arduino SPI abstraction for Betaflight drivers
// Clean-room implementation (MIT license)

#ifndef BF_BUS_H
#define BF_BUS_H

#include <Arduino.h>
#include <SPI.h>

typedef struct {
    SPIClass *spi;
    uint8_t cs_pin;
    uint32_t freq;
} extDevice_t;

void spiWriteReg(extDevice_t *dev, uint8_t reg, uint8_t data);
uint8_t spiReadRegMsk(extDevice_t *dev, uint8_t reg);
void spiSetClkDivisor(extDevice_t *dev, uint16_t divisor);
bool spiBusTransferMultiple(extDevice_t *dev, uint8_t *out, uint8_t *in, int length);

#endif // BF_BUS_H
```

#### 2. `bf_bus.cpp` - Arduino SPI implementation
```cpp
#include "bf_bus.h"

void spiWriteReg(extDevice_t *dev, uint8_t reg, uint8_t data) {
    dev->spi->beginTransaction(SPISettings(dev->freq, MSBFIRST, SPI_MODE3));
    digitalWrite(dev->cs_pin, LOW);
    dev->spi->transfer(reg & 0x7F);  // Clear read bit
    dev->spi->transfer(data);
    digitalWrite(dev->cs_pin, HIGH);
    dev->spi->endTransaction();
}

uint8_t spiReadRegMsk(extDevice_t *dev, uint8_t reg) {
    uint8_t data;
    dev->spi->beginTransaction(SPISettings(dev->freq, MSBFIRST, SPI_MODE3));
    digitalWrite(dev->cs_pin, LOW);
    dev->spi->transfer(reg | 0x80);  // Set read bit
    data = dev->spi->transfer(0xFF);
    digitalWrite(dev->cs_pin, HIGH);
    dev->spi->endTransaction();
    return data;
}

void spiSetClkDivisor(extDevice_t *dev, uint16_t divisor) {
    // Arduino handles clock via SPISettings in each transaction
    // This is a no-op for compatibility
}

bool spiBusTransferMultiple(extDevice_t *dev, uint8_t *out, uint8_t *in, int length) {
    dev->spi->beginTransaction(SPISettings(dev->freq, MSBFIRST, SPI_MODE3));
    digitalWrite(dev->cs_pin, LOW);
    dev->spi->transfer(out, length);
    if (in != nullptr) {
        memcpy(in, out, length);
    }
    digitalWrite(dev->cs_pin, HIGH);
    dev->spi->endTransaction();
    return true;
}
```

#### 3. `bf_types.h` - Device structure definitions
```cpp
// Minimal Betaflight device types for Arduino
// Clean-room implementation (MIT license)

#ifndef BF_TYPES_H
#define BF_TYPES_H

#include <stdint.h>
#include "bf_bus.h"

typedef enum {
    MPU_NONE = 0,
    MPU_60x0_SPI,
    MPU_9250_SPI,
    ICM_42688P_SPI
} mpuDetectionResult_e;

typedef struct gyroDev_s {
    extDevice_t dev;
    float scale;
    int16_t gyroADCRaw[3];
    mpuDetectionResult_e mpuDetectionResult;
    uint8_t hardware_lpf;  // Hardware low-pass filter setting
    void (*initFn)(struct gyroDev_s *gyro);
    bool (*readFn)(struct gyroDev_s *gyro);
} gyroDev_t;

typedef struct accDev_s {
    extDevice_t dev;
    float scale;
    int16_t accADC[3];
    void (*initFn)(struct accDev_s *acc);
    bool (*readFn)(struct accDev_s *acc);
} accDev_t;

// Helper macros
#define GYRO_SCALE_2000DPS  (2000.0f / 32768.0f)
#define ACC_1G_LSB          4096  // For ±8g range

#endif // BF_TYPES_H
```

### Notes
- These are **clean-room implementations** for Arduino compatibility
- NOT copied from Betaflight (avoid GPL contamination)
- Minimal functionality needed to support MPU drivers

---

## Phase 3: MPU-6000 Library (Betaflight-based, GPL v3) - 3 days

### License Decision: GPL v3 (Confirmed)

**Rationale:**
- ✅ **Production quality** - Betaflight drivers are battle-tested in thousands of flight controllers
- ✅ **Bug-free initialization** - Includes all quirks/workarounds discovered over years
- ✅ **Faster development** - Direct adaptation vs reinventing the wheel
- ✅ **GPL containment** - Separate library, optional dependency, core remains MIT
- ✅ **dRehmFlight stays MIT** - Depends on MIT `IMU.h` interface only, not GPL driver directly

**GPL Containment Strategy:**
- MPU6000 library (GPL v3) is a **separate, optional library**
- `IMU` wrapper (MIT) provides **interface abstraction** - depends on `IMU.h` interface, not implementation
- Users who don't need MPU-6000 simply don't include the library
- Clear documentation of license boundaries

### Create New Library
**Location:** `libraries/MPU6000/`
**License:** GPL v3 (Betaflight-derived code)

### File Structure
```
MPU6000/
├── src/
│   ├── MPU6000.h           # Arduino class wrapper
│   ├── MPU6000.cpp         # Implementation
│   ├── mpu6000_bf.c        # Betaflight driver (GPL v3)
│   ├── mpu6000_bf.h        # Betaflight header
│   └── mpu_common.h        # Shared MPU definitions
├── examples/
│   ├── MPU6000_Basic/
│   │   └── MPU6000_Basic.ino
│   ├── MPU6000_DLPF_Config/
│   │   └── MPU6000_DLPF_Config.ino
│   └── MPU6000_6DOF/
│       └── MPU6000_6DOF.ino
├── LICENSE                 # GPL v3
└── README.md
```

### Implementation (`MPU6000.h`)
```cpp
#ifndef MPU6000_H
#define MPU6000_H

#include <Arduino.h>
#include <SPI.h>

class MPU6000 {
public:
    MPU6000();
    ~MPU6000();

    // Initialization
    bool begin(SPIClass &spi, uint8_t cs_pin, uint32_t freq = 1000000);

    // Detection
    uint8_t whoAmI();  // Returns 0x68 for MPU-6000

    // Data reading
    bool readGyro(float &gx, float &gy, float &gz);
    bool readAccel(float &ax, float &ay, float &az);
    bool read6DOF(float &gx, float &gy, float &gz,
                  float &ax, float &ay, float &az);

    // Configuration
    void setDLPF(uint8_t dlpf_cfg);  // 0=256Hz, 1=188Hz, 2=98Hz, 3=42Hz, 4=20Hz, 5=10Hz, 6=5Hz
    void setGyroFSR(uint16_t fsr);   // 250, 500, 1000, 2000 dps
    void setAccelFSR(uint8_t fsr);   // 2, 4, 8, 16 g

private:
    void *gyro_dev;  // gyroDev_t pointer
    void *acc_dev;   // accDev_t pointer
    bool initialized;
};

#endif // MPU6000_H
```

### Key Betaflight Functions to Wrap
From `libraries/imu/betaflight/drivers/accgyro/accgyro_spi_mpu6000.c`:
- `mpu6000SpiDetect()` - WHO_AM_I detection (returns 0x68)
- `mpu6000SpiGyroInit()` - Initialize gyro with DLPF
- `mpu6000SpiAccInit()` - Initialize accelerometer
- `mpuGyroReadSPI()` - Read gyro data (shared MPU function)
- `mpuAccReadSPI()` - Read accel data (shared MPU function)
- `mpuGyroDLPF()` - Convert DLPF setting to register value

### Hardware Validation
- MPU-6000 breakout board
- SPI connection to NUCLEO_F411RE
- Test examples: Basic detection, DLPF config, 6DOF reading

---

## Phase 4: MPU-9250 & ICM-206xx Libraries (Betaflight-based, GPL v3) - 3 days

### MPU-9250 Library
**Location:** `libraries/MPU9250/`
**License:** GPL v3 (Betaflight-derived code)

### ICM-206xx Family Library
**Location:** `libraries/ICM20689/`
**License:** GPL v3 (Betaflight-derived code)
**Supported Chips:** ICM-20601, ICM-20602, ICM-20608, ICM-20689

### File Structure
```
MPU9250/
├── src/
│   ├── MPU9250.h           # Arduino class wrapper
│   ├── MPU9250.cpp         # Implementation
│   ├── mpu9250_bf.c        # Betaflight driver (GPL v3)
│   ├── mpu9250_bf.h        # Betaflight header
│   └── mpu_common.h        # Shared MPU definitions (link from MPU6000)
├── examples/
│   ├── MPU9250_Basic/
│   │   └── MPU9250_Basic.ino
│   ├── MPU9250_DLPF_Config/
│   │   └── MPU9250_DLPF_Config.ino
│   └── MPU9250_9DOF/       # With magnetometer
│       └── MPU9250_9DOF.ino
├── LICENSE                 # GPL v3
└── README.md
```

### Implementation (`MPU9250.h`)
```cpp
#ifndef MPU9250_H
#define MPU9250_H

#include <Arduino.h>
#include <SPI.h>

class MPU9250 {
public:
    MPU9250();
    ~MPU9250();

    // Initialization
    bool begin(SPIClass &spi, uint8_t cs_pin, uint32_t freq = 1000000);

    // Detection
    uint8_t whoAmI();  // Returns 0x71 for MPU-9250

    // Data reading
    bool readGyro(float &gx, float &gy, float &gz);
    bool readAccel(float &ax, float &ay, float &az);
    bool readMag(float &mx, float &my, float &mz);  // AK8963 magnetometer
    bool read9DOF(float &gx, float &gy, float &gz,
                  float &ax, float &ay, float &az,
                  float &mx, float &my, float &mz);

    // Configuration
    void setDLPF(uint8_t gyro_dlpf, uint8_t accel_dlpf);
    void setGyroFSR(uint16_t fsr);   // 250, 500, 1000, 2000 dps
    void setAccelFSR(uint8_t fsr);   // 2, 4, 8, 16 g

private:
    void *gyro_dev;  // gyroDev_t pointer
    void *acc_dev;   // accDev_t pointer
    bool initialized;
};

#endif // MPU9250_H
```

### Key Betaflight Functions to Wrap
From `libraries/imu/betaflight/drivers/accgyro/accgyro_spi_mpu9250.c`:
- `mpu9250SpiDetect()` - WHO_AM_I detection with write-verify (0x71 or 0x73)
- `mpu9250SpiGyroInit()` - Initialize with retry loop (up to 20 attempts)
- `mpu9250SpiAccInit()` - Initialize accelerometer
- Write-verify pattern for critical registers (100µs delays)
- Mag bypass mode for AK8963 access

### Hardware Validation
- MPU-9250 breakout board
- SPI connection to NUCLEO_F411RE
- Test examples: Basic detection, DLPF config, 9DOF with magnetometer

---

## Phase 5: Unified IMU Wrapper Enhancement - 2 days

### Update Files
**Location:** `libraries/imu/src/IMU.h` and `IMU.cpp`
**License:** MIT (no Betaflight code copied)

### Multi-Chip Support

#### Chip Detection Enum
```cpp
enum class ChipType {
    UNKNOWN = 0,
    ICM42688P = 0x47,   // TDK driver
    MPU6000 = 0x68,     // Betaflight driver
    MPU9250 = 0x71      // Betaflight driver (also 0x73 for MPU-9255)
};
```

#### Updated IMU Class
```cpp
class IMU {
public:
    // Initialization with auto-detection
    ChipType Init(SPIClass &spi, uint8_t cs_pin, uint32_t freq);

    // Unified data reading
    bool ReadIMU6(float *gx, float *gy, float *gz,
                  float *ax, float *ay, float *az);
    bool ReadIMU9(float *gx, float *gy, float *gz,
                  float *ax, float *ay, float *az,
                  float *mx, float *my, float *mz);  // MPU-9250 only

    // Unified filter configuration (per imu_hal.md)
    void SetFilterPreset(FilterPreset preset);  // SAFE/SMOOTH/BALANCED/ACRO

    // Chip identification
    ChipType GetChipType() const;
    const char* GetChipName() const;

private:
    ChipType chip;
    void *driver;  // Points to ICM42688P, MPU6000, or MPU9250 instance

    uint8_t readWhoAmI(SPIClass &spi, uint8_t cs_pin, uint32_t freq);
};
```

#### Auto-Detection Implementation
```cpp
ChipType IMU::Init(SPIClass &spi, uint8_t cs_pin, uint32_t freq) {
    // Read WHO_AM_I register
    uint8_t whoami = readWhoAmI(spi, cs_pin, freq);

    switch(whoami) {
        case 0x47:  // ICM-42688-P
            driver = new ICM42688P();
            chip = ChipType::ICM42688P;
            static_cast<ICM42688P*>(driver)->begin(spi, cs_pin, freq);
            break;

        case 0x68:  // MPU-6000
            driver = new MPU6000();
            chip = ChipType::MPU6000;
            static_cast<MPU6000*>(driver)->begin(spi, cs_pin, freq);
            break;

        case 0x71:  // MPU-9250
        case 0x73:  // MPU-9255
            driver = new MPU9250();
            chip = ChipType::MPU9250;
            static_cast<MPU9250*>(driver)->begin(spi, cs_pin, freq);
            break;

        default:
            chip = ChipType::UNKNOWN;
            return chip;
    }

    return chip;
}
```

#### Unified Filter Presets (per imu_hal.md)
```cpp
enum class FilterPreset {
    SAFE,      // Very tame for bring-up and unknown/noisy frames
    SMOOTH,    // Extra on-chip smoothing with moderate latency
    BALANCED,  // Strong default for 2 kHz loops (snappy but controlled)
    ACRO       // Lowest added phase; rely on strong software filtering
};

void IMU::SetFilterPreset(FilterPreset preset) {
    switch(chip) {
        case ChipType::ICM42688P:
            applyICM42688PPreset(preset);
            break;
        case ChipType::MPU6000:
            applyMPU6000Preset(preset);
            break;
        case ChipType::MPU9250:
            applyMPU9250Preset(preset);
            break;
    }
}

// ICM-42688-P presets
void IMU::applyICM42688PPreset(FilterPreset preset) {
    ICM42688P *imu = static_cast<ICM42688P*>(driver);

    switch(preset) {
        case FilterPreset::SAFE:
            // Gyro AAF: 126 Hz, Accel AAF: 84 Hz, UI order: 2nd, UI BW: code 1
            imu->SetGyroFilterHz(ICM42688P_AAF_126HZ);
            imu->SetAccelFilterHz(ICM42688P_AAF_84HZ);
            imu->SetUiFilters(1, 2, 2);  // BW=1, gyro_order=2, accel_order=2
            break;

        case FilterPreset::SMOOTH:
            // Gyro AAF: 213 Hz, Accel AAF: 126 Hz, UI order: 2nd, UI BW: code 1
            imu->SetGyroFilterHz(ICM42688P_AAF_213HZ);
            imu->SetAccelFilterHz(ICM42688P_AAF_126HZ);
            imu->SetUiFilters(1, 2, 2);
            break;

        case FilterPreset::BALANCED:
            // Gyro AAF: 258 Hz, Accel AAF: 170 Hz, UI order: 1st, UI BW: code 15
            imu->SetGyroFilterHz(ICM42688P_AAF_258HZ);
            imu->SetAccelFilterHz(ICM42688P_AAF_170HZ);
            imu->SetUiFilters(15, 1, 1);  // Low-latency
            break;

        case FilterPreset::ACRO:
            // Gyro AAF: 303 Hz @ 8kHz ODR, Accel AAF: 258 Hz, UI order: 1st, UI BW: code 15
            imu->SetGyroFilterHz(ICM42688P_AAF_303HZ);
            imu->SetAccelFilterHz(ICM42688P_AAF_258HZ);
            imu->SetUiFilters(15, 1, 1);
            // TODO: Set 8kHz gyro ODR (requires ODR API)
            break;
    }
}

// MPU-6000 presets
void IMU::applyMPU6000Preset(FilterPreset preset) {
    MPU6000 *imu = static_cast<MPU6000*>(driver);

    switch(preset) {
        case FilterPreset::SAFE:
            imu->setDLPF(2);  // 98 Hz (gyro), 94 Hz (accel)
            break;
        case FilterPreset::SMOOTH:
            imu->setDLPF(1);  // 188 Hz (gyro), 184 Hz (accel)
            break;
        case FilterPreset::BALANCED:
            imu->setDLPF(0);  // 256/260 Hz wide (8kHz internal, 4kHz host via SMPLRT_DIV=1)
            break;
        case FilterPreset::ACRO:
            imu->setDLPF(0);  // 256/260 Hz wide (8kHz internal, 8kHz host via SMPLRT_DIV=0)
            break;
    }
}

// MPU-9250 presets
void IMU::applyMPU9250Preset(FilterPreset preset) {
    MPU9250 *imu = static_cast<MPU9250*>(driver);

    switch(preset) {
        case FilterPreset::SAFE:
            imu->setDLPF(2, 2);  // Gyro: 92 Hz, Accel: 92 Hz
            break;
        case FilterPreset::SMOOTH:
            imu->setDLPF(1, 1);  // Gyro: 184 Hz, Accel: 184 Hz
            break;
        case FilterPreset::BALANCED:
            imu->setDLPF(0, 1);  // Gyro: 250 Hz wide, Accel: 184 Hz (4kHz host)
            break;
        case FilterPreset::ACRO:
            imu->setDLPF(0, 1);  // Gyro: 250 Hz wide, Accel: 184 Hz (8kHz host)
            break;
    }
}
```

### Documentation Update
Update `imu_hal.md` with:
- Implementation status for each chip
- Filter preset mapping table
- Usage examples for multi-chip detection

---

## Phase 6: dRehmFlight Multi-Chip Testing - 2 days

### Test Matrix

| Hardware | IMU | BoardConfig | Expected Result |
|----------|-----|-------------|-----------------|
| NUCLEO_F411RE | ICM42688P breadboard | `NUCLEO_F411RE_ICM42688P.h` | ✅ Already working + AFSR fix |
| NUCLEO_F411RE | MPU-6000 breadboard | `NUCLEO_F411RE_MPU6000.h` | ✅ New support |
| NUCLEO_F411RE | MPU-9250 breadboard | `NUCLEO_F411RE_MPU9250.h` | ✅ New support |

### BoardConfig Variants

#### 1. `NUCLEO_F411RE_ICM42688P.h` (existing - verify AFSR fix)
```cpp
namespace BoardConfig {
    namespace imu {
        namespace spi {
            constexpr SPIClass* bus = &SPI_1;
            constexpr uint8_t cs_pin = PA4;
            constexpr uint32_t frequency = 1000000;
        }
        constexpr ChipType expected_chip = ChipType::ICM42688P;
    }
}
```

#### 2. `NUCLEO_F411RE_MPU6000.h` (new)
```cpp
namespace BoardConfig {
    namespace imu {
        namespace spi {
            constexpr SPIClass* bus = &SPI_1;
            constexpr uint8_t cs_pin = PA4;
            constexpr uint32_t frequency = 1000000;
        }
        constexpr ChipType expected_chip = ChipType::MPU6000;
    }
}
```

#### 3. `NUCLEO_F411RE_MPU9250.h` (new)
```cpp
namespace BoardConfig {
    namespace imu {
        namespace spi {
            constexpr SPIClass* bus = &SPI_1;
            constexpr uint8_t cs_pin = PA4;
            constexpr uint32_t frequency = 1000000;
        }
        constexpr ChipType expected_chip = ChipType::MPU9250;
    }
}
```

### Validation Procedure

#### 1. Chip Detection Test
```cpp
// In dRehmFlight setup()
ChipType detected = imu.Init(*BoardConfig::imu::spi::bus,
                              BoardConfig::imu::spi::cs_pin,
                              BoardConfig::imu::spi::frequency);

if (detected != BoardConfig::imu::expected_chip) {
    CI_LOGF("ERROR: Expected chip 0x%02X, detected 0x%02X\n",
            static_cast<uint8_t>(BoardConfig::imu::expected_chip),
            static_cast<uint8_t>(detected));
    while(1);  // Halt on chip mismatch
}

CI_LOGF("IMU detected: %s\n", imu.GetChipName());
```

#### 2. Filter Preset Test
```cpp
// Apply BALANCED preset (default for 2kHz flight loop)
imu.SetFilterPreset(FilterPreset::BALANCED);

// Verify filter application (chip-specific)
switch(imu.GetChipType()) {
    case ChipType::ICM42688P:
        // Verify AAF: 258 Hz gyro, 170 Hz accel
        // Verify UI: code 15, 1st order
        break;
    case ChipType::MPU6000:
        // Verify DLPF: 256 Hz
        break;
    case ChipType::MPU9250:
        // Verify DLPF: Gyro 250 Hz, Accel 184 Hz
        break;
}
```

#### 3. Main Loop Test (2kHz)
```cpp
void loop() {
    currentTime = micros();
    dt = (currentTime - previousTime) / 1000000.0f;

    // Read IMU data (chip-agnostic)
    if (!imu.ReadIMU6(&GyroX, &GyroY, &GyroZ, &AccX, &AccY, &AccZ)) {
        CI_LOG("ERROR: IMU read failed\n");
    }

    // ... rest of flight loop ...

    loopRate(2000);  // 2kHz
}
```

#### 4. AFSR Verification (ICM42688P only)
```cpp
// Verify INTF_CONFIG1 register has AFSR_DISABLE set
// Register 0x4D should read 0x4X (bit 6 set, bit 7 clear)
// This confirms AFSR workaround is applied
```

### Success Criteria

✅ All three IMU chips detected correctly
✅ Filter presets apply appropriate chip-specific settings
✅ 2kHz main loop maintains stable timing
✅ IMU data rates match expectations (no stalls)
✅ AFSR fix confirmed on ICM42688P (no gyro stalls during extended operation)
✅ Gyro/accel data quality comparable across all chips

---

## Timeline

| Phase | Duration | Status |
|-------|----------|--------|
| Phase 1: AFSR fix | 1 day | ✅ COMPLETE (2025-11-02) |
| Phase 2: Bus abstraction | 2 days | ✅ COMPLETE (2025-11-02) |
| Phase 3: MPU-6000 library | 3 days | ✅ COMPLETE + HARDWARE VALIDATED (2025-11-03) |
| Phase 4: MPU-9250 library | 3 days | ✅ COMPLETE - Hardware validation pending |
| Phase 5: Unified wrapper | 2 days | 📋 PENDING |
| Phase 6: dRehmFlight testing | 2 days | 📋 PENDING |
| **TOTAL** | **13 days (~2.5 weeks)** | **4/6 phases complete** |

---

## Benefits

✅ **Preserves TDK SDK** - Official vendor code untouched
✅ **AFSR fix applied** - In Arduino wrapper layer (legal modification point)
✅ **Multi-chip support** - All 3 target chips (ICM42688P, MPU-6000, MPU-9250)
✅ **Production-validated** - Betaflight drivers battle-tested in thousands of FCs
✅ **GPL contained** - Separate libraries, core remains MIT
✅ **Chip-agnostic FC** - dRehmFlight works with any IMU
✅ **Unified API** - Consistent filter presets across all chips
✅ **Filter expertise** - Betaflight-validated AAF/DLPF settings

---

## References

- **ArduPilot AFSR Issue:** https://github.com/ArduPilot/ardupilot/pull/25332
- **Betaflight Source:** https://github.com/betaflight/betaflight
- **TDK InvenSense SDK:** `libraries/ICM42688P/src/Invn/`
- **Betaflight Drivers (Reference):** `libraries/imu/betaflight/drivers/accgyro/`
- **Filter Documentation:** `libraries/imu/imu_hal.md`

---

## Notes

- **DO NOT modify** `libraries/ICM42688P/src/Invn/` (TDK vendor code)
- **AFSR fix location:** `libraries/ICM42688P/src/icm42688p.c` (Arduino wrapper)
- **GPL containment strategy:** Separate MPU6000/MPU9250 libraries with GPL v3
- **Bus abstraction:** Clean-room MIT implementation (no Betaflight code copied)
- **Unified wrapper:** MIT licensed, no GPL code, delegates to chip-specific drivers

---

## Phase 1 Completion - AFSR Fix ✅

**Completed:** 2025-11-02

**Implementation:**
- Added `icm42688p_disable_afsr()` function to `libraries/ICM42688P/src/icm42688p.h`
- Integrated AFSR fix into `IMU::Init()` in `libraries/imu/src/IMU.cpp`
- Fix automatically applied for all IMU wrapper users

**Validation (HIL Testing):**
- ✅ dRehmFlight (2kHz flight controller loop)
- ✅ imu-polled-bf (Betaflight 8kHz config)
- ✅ imu-polled-mpu6000-sim (MPU-6000 4kHz/1kHz config)
- ✅ imu-raw-data-registers (Interrupt-driven 1kHz)

**Results:**
- ICM-42688-P detection successful (WHO_AM_I: 0x47)
- No gyro output stalls observed
- Continuous data streaming in all configurations
- Fix is transparent - no user code changes required

---

## Phase 2 Completion - Bus Abstraction ✅

**Completed:** 2025-11-02

**Implementation:**
- Created `libraries/imu/src/bf_types.h` (59 lines) - Device type definitions
- Created `libraries/imu/src/bf_bus.h` (49 lines) - SPI bus interface
- Created `libraries/imu/src/bf_bus.cpp` (124 lines) - Arduino SPI implementation
- Created `tests/bf_bus_test/` - Hardware validation test

**Key Features:**
- 100% clean-room MIT implementation (no Betaflight code copied)
- SPI Mode 3 support for MPU sensors (CPOL=1, CPHA=1)
- `extDevice_t` struct with SPIClass*, cs_pin, frequency
- Core API: `spiWriteReg`, `spiReadRegMsk`, `spiBusTransferMultiple`
- Convenience API: `spiReadRegBuf`, `spiWriteRegBuf`
- Minimal `gyroDev_t` and `accDev_t` structs for MPU driver support

**Validation (HIL Testing):**
- ✅ Test hardware: NUCLEO_F411RE + ICM-42688-P
- ✅ Binary size: 17,128 bytes (3% flash), 2,288 bytes RAM (1%)
- ✅ **TEST 1:** `spiReadRegMsk` - WHO_AM_I = 0x47 ✅ PASS
- ✅ **TEST 2:** `spiWriteReg` - Write/readback matches ✅ PASS
- ✅ **TEST 3:** `spiReadRegBuf` - Burst read valid (Gyro: X=37, Y=-112, Z=16) ✅ PASS
- ✅ **TEST 4:** `spiWriteRegBuf` - Executed successfully ✅ PASS

**Results:**
- All SPI bus operations validated with real hardware
- Deterministic HIL test execution with exit wildcard detection
- Ready for MPU-6000/9250 driver integration (Phase 3)

---

---

## Phase 3 Completion - MPU-6000 Library ✅

**Completed:** 2025-11-03

**Implementation:**
- Created `libraries/MPU6000/` with GPL v3 license
- Adapted Betaflight MPU-6000 driver code (`mpu6000_bf.c/h`)
- Implemented Arduino wrapper class (`MPU6000.h/cpp`)
- Created shared MPU register definitions (`mpu_common.h`)
- Implemented MPU6000_Basic example sketch

**Files Created:**
1. `LICENSE` - GNU GPL v3 with Betaflight attribution
2. `README.md` - Library documentation with GPL notice and containment strategy
3. `library.properties` - Arduino library metadata
4. `src/MPU6000.h` - Arduino class interface (GPL v3)
5. `src/MPU6000.cpp` - Arduino wrapper implementation (GPL v3)
6. `src/mpu6000_bf.h` - Betaflight driver header (GPL v3)
7. `src/mpu6000_bf.c` - Betaflight driver adaptation (GPL v3)
8. `src/mpu_common.h` - Shared MPU register definitions (GPL v3)
9. `examples/MPU6000_Basic/MPU6000_Basic.ino` - Detection and 6DOF example

**Key Features:**
- **Production-validated initialization** - Betaflight reset sequences, timing delays preserved
- **WHO_AM_I + PRODUCT_ID detection** - Validates MPU-6000 revisions C4-D10
- **Configurable DLPF** - 7 bandwidth settings (5Hz to 256Hz)
- **Configurable FSR** - Gyro: ±250/500/1000/2000 dps, Accel: ±2/4/8/16g
- **6-axis data reading** - Gyro (dps) and Accel (g) with scale conversion
- **Arduino-compatible API** - Clean interface with SPI abstraction via `bf_bus`

**GPL Containment:**
- MPU6000 library is **separate, optional** - users can exclude if not needed
- `IMU` wrapper (MIT) provides **interface abstraction** - no GPL code in interface
- dRehmFlight (MIT) depends on `IMU.h` interface only, not GPL implementation
- Clear license boundaries documented in README.md

**Hardware Validation:**
- ✅ **Board:** NUCLEO_F411RE with JHEF411 target config
- ✅ **IMU:** MPU-6000 on SPI1 (PA5=SCK, PA6=MISO, PA7=MOSI, PA4=CS)
- ✅ **WHO_AM_I:** 0x68 (MPU-6000 correctly detected)
- ✅ **Initialization:** Successful with 1 MHz SPI
- ✅ **Data acquisition:** 50 samples at 10 Hz
- ✅ **Gyro readings:** Stationary drift ~0.5 dps (expected)
- ✅ **Accel readings:** Z-axis ~0.97g (gravity)
- ✅ **HIL test:** Deterministic exit with *STOP* wildcard
- ✅ **Binary size:** 22,268 bytes (4% flash)

**API Example:**
```cpp
#include <MPU6000.h>

MPU6000 imu;
imu.begin(SPI, PA4, 1000000);        // Initialize
uint8_t id = imu.whoAmI();           // Returns 0x68
imu.read6DOF(gx, gy, gz, ax, ay, az); // Read 6-axis
imu.setDLPF(0);                       // 256 Hz bandwidth
```

**Status:** Phase 3 complete with hardware validation ✅

---

---

## Phase 4 Completion - MPU-9250 Library ✅

**Completed:** 2025-11-02

**Implementation:**
- Created `libraries/MPU9250/` with GPL v3 license
- Adapted Betaflight MPU-9250 driver code with write-verify patterns (`mpu9250_bf.c/h`)
- Implemented Arduino wrapper class (`MPU9250.h/cpp`)
- Copied shared bus abstraction (`bf_bus.*`, `bf_types.h`, `mpu_common.h`)
- Implemented MPU9250_Basic example sketch

**Files Created:**
1. `LICENSE` - GNU GPL v3 with Betaflight attribution
2. `README.md` - Library documentation with GPL notice, hardware validation noted
3. `library.properties` - Arduino library metadata
4. `src/MPU9250.h` - Arduino class interface (GPL v3)
5. `src/MPU9250.cpp` - Arduino wrapper implementation (GPL v3)
6. `src/mpu9250_bf.h` - Betaflight driver header (GPL v3)
7. `src/mpu9250_bf.c` - Betaflight driver adaptation (GPL v3)
8. `src/mpu_common.h` - Shared MPU register definitions (copied from MPU6000)
9. `src/bf_bus.h/cpp` - SPI bus abstraction (copied from MPU6000)
10. `src/bf_types.h` - Device type definitions (copied from MPU6000)
11. `examples/MPU9250_Basic/MPU9250_Basic.ino` - Detection and 6DOF example

**Key Features:**
- **Production-validated initialization** - Betaflight write-verify patterns preserved
- **Write-verify with retry** - Up to 20 attempts for critical registers (MPU-9250 specific)
- **Slow read/write operations** - 1µs delays for register reliability
- **WHO_AM_I detection with retry** - 150ms delays between attempts
- **Dual chip support** - MPU-9250 (0x71) and MPU-9255 (0x73)
- **Separate gyro/accel DLPF** - Independent filter configuration
- **Magnetometer bypass** - BYPASS_EN set for AK8963 access (future magnetometer support)
- **Configurable FSR** - Gyro: ±250/500/1000/2000 dps, Accel: ±2/4/8/16g
- **6-axis data reading** - Gyro (dps) and Accel (g) with scale conversion

**Build Results:**
```
✓ Build successful
Binary: MPU9250_Basic.ino.bin
Size: 22,836 bytes (4% of 512KB flash)
RAM: 2,188 bytes (1% of 128KB RAM)
Build time: 8 seconds
```

**Hardware Validation Target:**
- **Board:** Blackpill F411CE
- **IMU:** MPU-9250 breakout board
- **Interface:** SPI_1 (PA5=SCK, PA6=MISO, PA7=MOSI), CS=PA4
- **Status:** Ready for hardware testing ⏳

**Next Steps:**
- Hardware validation on Blackpill F411 + MPU-9250 (Phase 4 final step)
- Phase 5: Unified IMU wrapper with multi-chip auto-detection
- Phase 6: dRehmFlight testing with all 3 IMU chips

---

## Phase 4b Completion - ICM20689 Library (ICM-206xx Family) ✅

**Completed:** 2025-11-03

**Implementation:**
- Created `libraries/ICM20689/` with GPL v3 license
- Adapted Betaflight ICM-206xx driver code (`icm20689_bf.c/h`)
- Implemented Arduino wrapper class (`ICM20689.h/cpp`)
- Created shared MPU register definitions (`mpu_common.h`)
- Implemented ICM20689_Basic example sketch

**Files Created:**
1. `LICENSE` - GNU GPL v3 with Betaflight attribution
2. `README.md` - Library documentation with multi-chip support details
3. `library.properties` - Arduino library metadata
4. `src/ICM20689.h` - Arduino class interface with ChipVariant enum (GPL v3)
5. `src/ICM20689.cpp` - Arduino wrapper implementation (GPL v3)
6. `src/icm20689_bf.h` - Betaflight driver header (GPL v3)
7. `src/icm20689_bf.c` - Betaflight driver adaptation (GPL v3)
8. `src/mpu_common.h` - Shared MPU register definitions (copied from MPU6000)
9. `src/bf_bus.h/cpp` - SPI bus abstraction (copied from MPU6000)
10. `src/bf_types.h` - Device type definitions with ICM-206xx enums (copied from MPU6000)
11. `examples/ICM20689_Basic/ICM20689_Basic.ino` - Detection and 6DOF example

**Key Features:**
- **Multi-chip support** - Single driver for 4 chip variants
- **Auto-detection** - ChipVariant enum and getChipName() API
- **Production-validated initialization** - Betaflight reset sequences with timing delays preserved
- **Clock selection with settle delay** - PLL clock with 120µs settle time
- **Configurable DLPF** - 8 bandwidth settings (5Hz to 3600Hz)
- **Configurable FSR** - Gyro: ±250/500/1000/2000 dps, Accel: ±2/4/8/16g
- **6-axis data reading** - Gyro (dps) and Accel (g) with scale conversion
- **Arduino-compatible API** - Clean interface with SPI abstraction via `bf_bus`

**Supported Chips:**
| Chip | WHO_AM_I | Status |
|------|----------|--------|
| ICM-20601 | 0xAC | ✅ Supported |
| ICM-20602 | 0x12 | ⭐ Primary target |
| ICM-20608 | 0xAF | ✅ Supported |
| ICM-20689 | 0x98 | ✅ Supported |

**Build Results:**
```
✓ Build successful
Binary: ICM20689_Basic.ino.bin
Size: 22,124 bytes (4% of 512KB flash)
RAM: 2,192 bytes (1% of 128KB RAM)
Build time: 8 seconds
```

**API Example:**
```cpp
#include <ICM20689.h>

ICM20689 imu;
imu.begin(SPI, PA4, 1000000);               // Initialize
uint8_t id = imu.whoAmI();                   // Returns 0x12 (ICM-20602)
const char* name = imu.getChipName();        // Returns "ICM-20602"
imu.read6DOF(gx, gy, gz, ax, ay, az);       // Read 6-axis
imu.setDLPF(0);                              // 250 Hz bandwidth
```

**Status:** Phase 4b complete - Ready for hardware validation with ICM-20602 ⏳

---

**Last Updated:** 2025-11-03
**Status:** Phase 3 hardware validated ✅ | Phase 4a/4b ready for hardware validation ⏳
