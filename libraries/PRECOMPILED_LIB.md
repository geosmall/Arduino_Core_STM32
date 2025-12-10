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

- [ ] Library compiles and links on F4/G4 targets (cortex-m4)
- [ ] Library compiles and links on F7/H7 targets (cortex-m7)
- [ ] No source files (.cpp) are in the distributed library
- [ ] HIL tests pass on NUCLEO_F411RE (F4)
- [ ] HIL tests pass on BKMN_NERO or MATEK_H743VI (F7/H7)
- [ ] Serial output works for Arduino IDE users
- [ ] build_archive.sh generates both .a files

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

## Future Project: Code Overlay Infrastructure Library

Building on the precompiled library patterns established above, a future project will explore creating a **precompiled overlay infrastructure library** for dynamically loading code from external storage (SPI flash, SD card) into RAM at runtime.

### Background

Based on Warren Gay's "Beginning STM32" overlay examples ([GitHub](https://github.com/Apress/Beg-STM32-Devel-FreeRTOS-libopencm3-GCC/tree/master/rtos)):
- **overlay0**: Overlays in internal flash, loaded to RAM on demand
- **overlay1**: Overlays in external SPI flash (W25Q32), loaded via SPI reads

### Concept: Precompiled Overlay Manager

A precompiled `.a` library providing overlay infrastructure using the **callback pattern** to remain device-independent:

```
┌─────────────────────────────────────────────────────────────┐
│  Precompiled Library: libOverlayMgr.a                       │
│  (One binary per CPU core - NOT device-specific)            │
├─────────────────────────────────────────────────────────────┤
│  • Overlay table management                                 │
│  • Overlay caching (track currently loaded overlay)         │
│  • Validation (CRC check before execution)                  │
│  • Generic loader (memcpy to RAM region)                    │
│                                                             │
│  Requires user-provided callbacks:                          │
│  • storage_read(addr, buf, len) - read from SPI/SD/etc      │
│  • get_overlay_region() - returns RAM address for overlays  │
└─────────────────────────────────────────────────────────────┘
         │
         ▼ Links with
┌─────────────────────────────────────────────────────────────┐
│  User Application (compiled per-device)                     │
│  • storage_impl.c - LittleFS/SDFS/raw SPI driver            │
│  • overlay_config.c - RAM region, overlay table             │
│  • Linker script defines overlay RAM region                 │
└─────────────────────────────────────────────────────────────┘
         │
         ▼ Loads from
┌─────────────────────────────────────────────────────────────┐
│  External Storage                                           │
│  • overlays.bin - position-dependent code for RAM region    │
│  • overlay_index.bin - table of overlay offsets/sizes       │
└─────────────────────────────────────────────────────────────┘
```

### Key Design Decisions for Future Project

| Decision | Choice | Rationale |
|----------|--------|-----------|
| HAL dependency | **Callbacks only** | Keeps library device-independent (2 binaries) |
| Storage abstraction | User-provided `storage_read()` | Works with LittleFS, SDFS, raw SPI |
| RAM region | User-provided via callback | Linker script defines per-device |
| Overlay binaries | **Always device-specific** | Position-dependent code for fixed RAM address |

### What This Enables

1. **Proprietary overlay infrastructure** - distribute loader as binary
2. **User-created overlays** - users compile their own overlays for their RAM region
3. **Extendable applications** - load new functionality without reflashing main app
4. **Memory efficiency** - share limited RAM among multiple code modules

### Dependencies on Current Plan

This future project depends on successful completion of the PrecompLib example:
- [ ] Validate `precompiled=true` with `ldflags` works correctly
- [ ] Confirm callback pattern keeps library device-independent
- [ ] Test binary distribution workflow on F4/G4/F7/H7 targets

### Overlay Binaries Are Always Device-Specific

Unlike the infrastructure library, **overlay binaries cannot be made portable**:
- Compiled for specific RAM execution address (position-dependent)
- RAM addresses vary by device and linker script
- Users must compile overlays for their target configuration

## References

- [Arduino Library Specification - Precompiled Libraries](https://arduino.github.io/arduino-cli/latest/library-specification/#precompiled-binaries)
- [Arduino CLI library.properties format](https://arduino.github.io/arduino-cli/latest/library-specification/#libraryproperties-file-format)
- [Beginning STM32 - Overlay Examples](https://github.com/Apress/Beg-STM32-Devel-FreeRTOS-libopencm3-GCC/tree/master/rtos) - Warren Gay's overlay0/overlay1 implementations
