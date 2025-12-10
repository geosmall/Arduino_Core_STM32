# Precompiled Library Example for STM32 Targets

## Goal
Create a simple example library demonstrating Arduino's precompiled library feature (`precompiled=true`) for STM32 F4, G4, F7, and H7 targets. This demonstrates **binary-only distribution** - header + .a files only, no source code distributed. Ideal for sharing proprietary code.

## Supported Targets

| Family | MCU | FPU | Binary | Example Boards |
|--------|-----|-----|--------|----------------|
| **F4** | `cortex-m4` | `fpv4-sp-d16` hard | `cortex-m4` | NUCLEO_F411RE, BLACKPILL_F411CE, NOXE_V3 |
| **G4** | `cortex-m4` | `fpv4-sp-d16` hard | `cortex-m4` | NUCLEO_G431RB, NUCLEO_G474RE, B_G431B_ESC1 |
| **F7** | `cortex-m7` | `fpv4-sp-d16` hard | `cortex-m7` | BKMN_NERO, NUCLEO_F746ZG, NUCLEO_F722ZE |
| **H7** | `cortex-m7` | `fpv4-sp-d16` hard | `cortex-m7` | MATEK_H743VI, NUCLEO_H743ZI, NUCLEO_H753ZI |

All families use the same FPU configuration. Only **two binaries** are needed:
- `cortex-m4` binary → covers F4 + G4
- `cortex-m7` binary → covers F7 + H7

## Library Concept: CRC-16 Calculator
- **Name**: `PrecompLib`
- **Functionality**: CRC-16 (CCITT) checksum calculation
- **Rationale**: Simple API, no hardware dependencies, useful in embedded systems, small footprint (~500 bytes)

## Key Design Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Precompiled mode | `precompiled=true` | Binary-only distribution, no source fallback |
| Target architectures | cortex-m4 + cortex-m7 | Covers F4, G4, F7, and H7 families |
| FPU configuration | `fpv4-sp-d16` hard | Common to all families |
| Binary path | `src/{mcu}/fpv4-sp-d16-hard/` | Per Arduino spec |

## Directory Structure

**Distributed Library** (what users receive):
```
libraries/PrecompLib/
├── library.properties              # precompiled=true, ldflags=-lPrecompLib
├── README.md                       # Usage documentation
├── src/
│   ├── PrecompLib.h               # Public header (API declaration)
│   ├── cortex-m4/
│   │   └── fpv4-sp-d16-hard/
│   │       └── libPrecompLib.a    # Binary for F4 + G4
│   └── cortex-m7/
│       └── fpv4-sp-d16-hard/
│           └── libPrecompLib.a    # Binary for F7 + H7
└── examples/
    └── CRC16_Demo/
        └── CRC16_Demo.ino         # Example with HIL support
```

**Development Repository** (separate, not distributed):
```
PrecompLib-dev/
├── src/
│   ├── PrecompLib.h               # Header (copied to distribution)
│   └── PrecompLib.cpp             # Source (PROPRIETARY - not distributed)
└── scripts/
    └── build_archive.sh           # Generates .a files for all targets
```

## Implementation Steps

### Step 1: Create Development Directory
Create source files in a separate development location:
- `PrecompLib-dev/src/PrecompLib.h` - Public header
- `PrecompLib-dev/src/PrecompLib.cpp` - Implementation (proprietary)
- `PrecompLib-dev/scripts/build_archive.sh` - Multi-target build script

### Step 2: Implement CRC-16 Functions
```cpp
// API (in PrecompLib.h - distributed)
uint16_t crc16_calculate(const uint8_t* data, size_t length);
uint16_t crc16_update(uint16_t crc, uint8_t byte);
const char* precomplib_version(void);
```

### Step 3: Create Multi-Target Build Script
`scripts/build_archive.sh`:
```bash
#!/bin/bash
# Build precompiled libraries for F4 and H7 targets

TARGETS=(
    "cortex-m4"   # F4 + G4: F411, F405, G431, G474
    "cortex-m7"   # F7 + H7: F722, F746, H743, H753
)

for MCU in "${TARGETS[@]}"; do
    OUTDIR="output/${MCU}/fpv4-sp-d16-hard"
    mkdir -p "$OUTDIR"

    arm-none-eabi-gcc -mcpu=${MCU} -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb \
        -Os -ffunction-sections -fdata-sections -c src/PrecompLib.cpp -o PrecompLib.o

    arm-none-eabi-gcc-ar rcs "${OUTDIR}/libPrecompLib.a" PrecompLib.o
    rm PrecompLib.o

    echo "Built: ${OUTDIR}/libPrecompLib.a"
done
```

### Step 4: Create Distribution Package
Copy to `libraries/PrecompLib/`:
- `src/PrecompLib.h` (header only)
- `src/cortex-m4/fpv4-sp-d16-hard/libPrecompLib.a` (F4 binary)
- `src/cortex-m7/fpv4-sp-d16-hard/libPrecompLib.a` (H7 binary)
- `library.properties`
- `examples/CRC16_Demo/CRC16_Demo.ino`

### Step 5: Create Example Sketch
`examples/CRC16_Demo/CRC16_Demo.ino`:
- Tests standard CRC-16 CCITT vector ("123456789" → 0x29B1)
- Supports both RTT (HIL) and Serial (IDE) output via `ci_log.h`
- Includes `*STOP*` exit wildcard for `aflash.sh`

### Step 6: Verification Testing

**Test on F4 (NUCLEO_F411RE) - cortex-m4:**
```bash
./system/ci/aflash.sh libraries/PrecompLib/examples/CRC16_Demo --use-rtt --build-id
```

**Test on G4 (NUCLEO_G474RE) - cortex-m4:**
```bash
./system/ci/aflash.sh libraries/PrecompLib/examples/CRC16_Demo \
    STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_G474RE --use-rtt --build-id
```

**Test on F7 (BKMN_NERO) - cortex-m7:**
```bash
./system/ci/aflash.sh libraries/PrecompLib/examples/CRC16_Demo \
    STMicroelectronics:stm32:FlightCtr:pnum=BKMN_NERO --use-rtt --build-id
```

**Test on H7 (MATEK_H743VI) - cortex-m7:**
```bash
./system/ci/aflash.sh libraries/PrecompLib/examples/CRC16_Demo \
    STMicroelectronics:stm32:FlightCtr:pnum=MATEK_H743VI --use-rtt --build-id
```

## Critical Files

| File | Purpose |
|------|---------|
| `platform.txt:140` | Confirms `{compiler.libraries.ldflags}` in linker recipe |
| `boards.txt` | Build settings for F4/G4 (cortex-m4) and F7/H7 (cortex-m7) |
| `libraries/libPrintf/library.properties` | Pattern for library.properties |
| `libraries/AUnit-1.7.1/src/aunit_hil.h` | HIL test integration pattern |

## library.properties Content

```properties
name=PrecompLib
version=1.0.0
author=geosmall
maintainer=geosmall
sentence=Precompiled library example demonstrating CRC-16 calculation
paragraph=Demonstrates Arduino precompiled library feature for STM32 F4/G4/F7/H7 targets. Binary-only distribution.
category=Other
url=https://github.com/geosmall/Arduino_Core_STM32
architectures=stm32
precompiled=true
ldflags=-lPrecompLib
```

**Key difference**: `precompiled=true` (not `full`) means:
- No source files are compiled
- Only the precompiled .a file is linked
- Fails on unsupported architectures (no fallback)

## Verification Checklist

- [x] Library compiles and links on F4/G4 targets (cortex-m4) - JHEF_JHEF411 (STM32F411RE)
- [x] Library compiles and links on F7/H7 targets (cortex-m7) - BKMN_NERO (STM32F722RE), MATEK_H743VI (STM32H743ZI)
- [x] No source files (.cpp) are in the distributed library
- [x] HIL tests pass on F4 - JHEF_JHEF411: 3/3 tests PASS
- [x] HIL tests pass on F7 - BKMN_NERO: 3/3 tests PASS
- [x] HIL tests pass on H7 - MATEK_H743VI: 3/3 tests PASS
- [x] Serial output works for Arduino IDE users - JHEF_JHEF411: 3/3 tests PASS
- [x] build_archive.sh generates both .a files (cortex-m4: 2292 bytes, cortex-m7: 2292 bytes)

## HAL/CMSIS Dependency Considerations

**Critical**: Precompiled libraries have different requirements based on their dependencies:

### Pure Computation Libraries (like PrecompLib CRC-16)
- **No HAL/CMSIS includes** - only standard headers (`stdint.h`, `stddef.h`)
- **One binary per CPU core** is sufficient (cortex-m4, cortex-m7)
- Works across all parts in a family (F411, F405, F401, etc. all use same cortex-m4 binary)

### HAL/CMSIS-Dependent Libraries
- **Part-specific binaries required** due to compile-time defines:
  - `-DSTM32F411xE` vs `-DSTM32F405xx` select different register definitions
  - Different parts have different peripheral availability
  - Memory maps and interrupt vectors differ between parts
- Would need structure like:
  ```
  src/cortex-m4/fpv4-sp-d16-hard/STM32F411xE/libMyLib.a
  src/cortex-m4/fpv4-sp-d16-hard/STM32F405xx/libMyLib.a
  ```
- **Exponential binary count**: Each product line needs its own binary

### Why Header-Only Templates Don't Work for Proprietary Code
Header-only libraries (C++ templates) are sometimes suggested as "portable" but they:
- **Expose all source code** in the header - defeats proprietary distribution goal
- Are compiled fresh each time with correct defines - not precompiled at all
- Offer no IP protection

### Recommendation for Proprietary HAL-Dependent Code
If you need to distribute proprietary code that uses HAL/CMSIS:
1. **Minimize HAL dependencies** - isolate HAL calls to a thin wrapper layer
2. **Use callbacks/interfaces** - pass HAL functionality in at runtime
3. **Accept the binary matrix** - build for each target product line you support

## Potential Issues to Watch

1. **FPU directory naming**: If `fpv4-sp-d16-hard` doesn't work, try just `cortex-m4/` or `cortex-m7/` without FPU subdirectory
2. **Toolchain match**: Archive must be built with same xpack 12.2.1 toolchain as Arduino CLI
3. **ldflags format**: Use `-lPrecompLib` (not `-llibPrecompLib.a`)
4. **No fallback**: With `precompiled=true`, compilation fails if .a not found for target architecture
5. **Same FPU, different MCU**: All families share FPU settings but need separate binaries per CPU core
6. **HAL includes = part-specific**: Any `#include <stm32f4xx_hal.h>` makes the binary part-specific

## Binary-Only Distribution Workflow

```
┌─────────────────────────────────────────────────────────────┐
│                    DEVELOPMENT (Private)                     │
├─────────────────────────────────────────────────────────────┤
│  PrecompLib-dev/                                            │
│  ├── src/PrecompLib.cpp  ← Proprietary implementation       │
│  ├── src/PrecompLib.h    ← Public API                       │
│  └── scripts/build_archive.sh                               │
│                     │                                        │
│           ┌────────┴────────┐                               │
│           ▼                 ▼                               │
│    -mcpu=cortex-m4    -mcpu=cortex-m7                       │
│           │                 │                               │
│           ▼                 ▼                               │
│    libPrecompLib.a    libPrecompLib.a                       │
│      (for F4+G4)        (for F7+H7)                         │
└─────────────────────────────────────────────────────────────┘
                      │
                      ▼ (copy header + binaries only)
┌─────────────────────────────────────────────────────────────┐
│                   DISTRIBUTION (Public)                      │
├─────────────────────────────────────────────────────────────┤
│  libraries/PrecompLib/                                      │
│  ├── library.properties  (precompiled=true)                 │
│  ├── src/PrecompLib.h    ← Header only                      │
│  ├── src/cortex-m4/fpv4-sp-d16-hard/libPrecompLib.a (F4+G4)│
│  └── src/cortex-m7/fpv4-sp-d16-hard/libPrecompLib.a (F7+H7)│
│                                                             │
│  Users can use the library but cannot see implementation    │
└─────────────────────────────────────────────────────────────┘
```

## Future Project: Mixer Overlay System for dRehmFlight

Building on the precompiled library patterns established above, a future project will create a **mixer overlay system** enabling dRehmFlight users to develop, share, and dynamically load airframe-specific mixer configurations.

### Use Case: Shareable Mixer Code

dRehmFlight users with similar airframes and hardware can share pre-developed and verified mixers:
- **Quad X**, **Quad +**, **Hex Y6**, **Octo X** - standard configurations
- **Custom VTOL**, **Tricopter**, **Flying Wing** - specialized airframes
- Users select mixer at runtime from SD card or SPI flash
- No reflashing required to change airframe configuration

### Background

Based on Warren Gay's "Beginning STM32" overlay examples ([GitHub](https://github.com/Apress/Beg-STM32-Devel-FreeRTOS-libopencm3-GCC/tree/master/rtos)):
- **overlay0**: Overlays in internal flash, loaded to RAM on demand
- **overlay1**: Overlays in external SPI flash (W25Q32), loaded via SPI reads

### Architecture: dRehmFlight + Mixer Overlays

```
┌─────────────────────────────────────────────────────────────┐
│  dRehmFlight Application (flash)                            │
│  ├── Flight control loop (PID, filters, safety)             │
│  ├── IMU, RC receiver, motor drivers                        │
│  └── libOverlayMgr.a (precompiled, callback-based)         │
│           │                                                 │
│           ▼ loads selected mixer into RAM                  │
│  ┌─────────────────────────────────────────────────────────┐│
│  │  Mixer Overlay (RAM)                                    ││
│  │  mixer_calculate(throttle, roll, pitch, yaw, motors[]) ││
│  └─────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────┘
         │
         ▼ reads from
┌─────────────────────────────────────────────────────────────┐
│  SD Card / SPI Flash                                        │
│  ├── mixers/quad_x.bin      (standard X quadcopter)        │
│  ├── mixers/quad_plus.bin   (+ configuration)              │
│  ├── mixers/hex_y6.bin      (Y6 hexacopter)                │
│  ├── mixers/vtol_tilt.bin   (tilt-rotor VTOL)              │
│  └── mixers/custom.bin      (user's custom airframe)       │
└─────────────────────────────────────────────────────────────┘
```

### Mixer API (Standard Interface)

All mixers implement the same interface, enabling interchangeability:

```cpp
// Input from flight controller
typedef struct {
    float throttle;   // 0.0 to 1.0
    float roll;       // -1.0 to 1.0
    float pitch;      // -1.0 to 1.0
    float yaw;        // -1.0 to 1.0
} mixer_input_t;

// Output to motor drivers
typedef struct {
    float motors[8];  // Up to 8 motors, 0.0 to 1.0
    uint8_t count;    // Number of active motors
} mixer_output_t;

// Standard mixer function signature
void mixer_calculate(const mixer_input_t* in, mixer_output_t* out);

// Optional: mixer metadata
const char* mixer_name(void);      // e.g., "Quad X"
const char* mixer_author(void);    // e.g., "geosmall"
uint32_t mixer_version(void);      // e.g., 0x010000 = v1.0.0
```

### Precompiled Overlay Manager Library

A precompiled `.a` library providing overlay infrastructure using the **callback pattern**:

```
┌─────────────────────────────────────────────────────────────┐
│  Precompiled Library: libOverlayMgr.a                       │
│  (One binary per CPU core - NOT device-specific)            │
├─────────────────────────────────────────────────────────────┤
│  • Overlay loader (read from storage → RAM)                 │
│  • CRC validation before execution                          │
│  • Function pointer management                              │
│  • Overlay metadata parsing                                 │
│                                                             │
│  Requires user-provided callbacks:                          │
│  • storage_read(path, buf, len) - read from LittleFS/SDFS  │
│  • get_overlay_region() - returns RAM address for overlays  │
└─────────────────────────────────────────────────────────────┘
```

### Key Design Decisions for Future Project

| Decision | Choice | Rationale |
|----------|--------|-----------|
| HAL dependency | **Callbacks only** | Keeps library device-independent (2 binaries) |
| Storage abstraction | User-provided `storage_read()` | Works with LittleFS, SDFS, raw SPI |
| RAM region | User-provided via callback | Linker script defines per-device |
| Mixer binaries | **Device-specific** | Position-dependent code for fixed RAM address |
| Mixer API | **Standardized interface** | Enables sharing between users |

### What This Enables

1. **Shareable mixers** - users with same hardware share verified mixer configs
2. **Runtime selection** - change airframe without reflashing
3. **Community library** - collection of mixers for common airframes
4. **Custom development** - users create mixers for unique airframes
5. **Safe updates** - CRC validation before loading mixer code

### Mixer Development Workflow

```
┌─────────────────────────────────────────────────────────────┐
│  User A (Quad X developer)                                  │
├─────────────────────────────────────────────────────────────┤
│  1. Write mixer_quad_x.c implementing standard API          │
│  2. Compile with linker script for RAM overlay region       │
│  3. Test on their hardware                                  │
│  4. Share quad_x.bin with community                         │
└─────────────────────────────────────────────────────────────┘
                      │
                      ▼ shares .bin file
┌─────────────────────────────────────────────────────────────┐
│  User B (same hardware, Quad X airframe)                    │
├─────────────────────────────────────────────────────────────┤
│  1. Download quad_x.bin to SD card                          │
│  2. Configure dRehmFlight to load "mixers/quad_x.bin"       │
│  3. Fly with verified mixer - no compilation needed         │
└─────────────────────────────────────────────────────────────┘
```

### Dependencies on Current Plan

This future project depends on successful completion of the PrecompLib example:
- [ ] Validate `precompiled=true` with `ldflags` works correctly
- [ ] Confirm callback pattern keeps library device-independent
- [ ] Test binary distribution workflow on F4/G4/F7/H7 targets

### Mixer Binaries: Device-Specific but Shareable

Mixer binaries are **position-dependent** (compiled for specific RAM address), but:
- Users with **same board + same linker script** can share directly
- Standard BoardConfig targets (NUCLEO_F411RE, MATEK_H743VI, etc.) define standard overlay regions
- Community can maintain mixer collections per BoardConfig target

## References

- [Arduino Library Specification - Precompiled Libraries](https://arduino.github.io/arduino-cli/latest/library-specification/#precompiled-binaries)
- [Arduino CLI library.properties format](https://arduino.github.io/arduino-cli/latest/library-specification/#libraryproperties-file-format)
- [Beginning STM32 - Overlay Examples](https://github.com/Apress/Beg-STM32-Devel-FreeRTOS-libopencm3-GCC/tree/master/rtos) - Warren Gay's overlay0/overlay1 implementations
