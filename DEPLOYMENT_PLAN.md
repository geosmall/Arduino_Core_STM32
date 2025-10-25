# Repository Consolidation and Board Manager Deployment Plan

> **⚠️ HISTORICAL DOCUMENT**: This is the original migration plan. For **current status**, see [DEPLOYMENT_STATUS.md](DEPLOYMENT_STATUS.md).
>
> **Quick Status**: Phases 1-3 complete ✅ | v1.0.0 live on Board Manager | Testing and automation remaining

## Overview

Consolidate all development work into `Arduino_Core_STM32` repository and create separate `BoardManagerFiles` repository for Arduino IDE Board Manager distribution.

## Current State

**Main Repository** (`geosmall/Arduino_dev`):
- Robotics libraries (SerialRx, IMU, ICM42688P, LittleFS, SDFS, Storage, etc.)
- CI/CD scripts and test infrastructure
- Board configuration system (targets/)
- Betaflight converter tool (extras/)
- Documentation (CLAUDE.md, README.md)

**Core Submodule** (`geosmall/Arduino_Core_STM32`):
- STM32 Arduino core (fork of stm32duino)
- Custom features: ci_log.h, FS.h, SEGGER_RTT
- Core libraries: SPI, Wire, SoftwareSerial
- Minimal variants for F4/H7 families

## Target State

**Development Repository** (`geosmall/Arduino_Core_STM32`):
- Complete Arduino core with all robotics libraries
- CI/CD scripts and test infrastructure
- Board configuration system
- All development tools and documentation
- Single repository for all development work

**Distribution Repository** (`geosmall/BoardManagerFiles`):
- Package index JSON for Board Manager
- Release archives hosted via GitHub Releases
- Installation documentation
- Minimal repo - distribution only

**Deprecated** (`geosmall/Arduino_dev`):
- Archive with pointer to Arduino_Core_STM32
- Or delete after successful migration

## Pre-Consolidation Completed ✅

The following has already been completed:
- ✅ HIL_Validation example created in `Arduino_Core_STM32/libraries/SEGGER_RTT/examples/HIL_Validation/`
- ✅ HIL_RTT_Test/ directory deleted from parent repo
- ✅ Makefile preserved as `doc/Makefile.example`
- ✅ Submodule committed and pushed (commit: 4e1cfa517)
- ✅ Parent repo updated with new submodule pointer (commit: 3c027c0)

## Implementation Plan

### Phase 1: Consolidate into Arduino_Core_STM32

#### Step 1.1: Move Robotics Libraries
Move from `Arduino_dev/libraries/` to `Arduino_Core_STM32/libraries/`:
- SerialRx
- IMU
- ICM42688P
- LittleFS
- SDFS
- Storage
- minIniStorage
- TimerPWM
- libPrintf
- AUnit-1.7.1
- STM32RTC (verify if already present)

#### Step 1.2: Move CI/CD Infrastructure
Move from `Arduino_dev/` to `Arduino_Core_STM32/`:
- `scripts/` → `Arduino_Core_STM32/system/ci/`
- `tests/` → `Arduino_Core_STM32/tests/`
- `targets/` → `Arduino_Core_STM32/targets/`
- `extras/` → `Arduino_Core_STM32/extras/`
- `doc/` → `Arduino_Core_STM32/doc/`
- `cmake/` → `Arduino_Core_STM32/cmake/` (if needed)

**Note**: `HIL_RTT_Test/` is NOT moved - functionality replaced by `Arduino_Core_STM32/libraries/SEGGER_RTT/examples/HIL_Validation/`

#### Step 1.3: Consolidate Documentation
- Move `CLAUDE.md` to `Arduino_Core_STM32/CLAUDE.md`
- Merge `Arduino_dev/README.md` content into `Arduino_Core_STM32/README.md`
- Update all internal documentation references

#### Step 1.4: Update Internal Paths
- Update example sketches with correct include paths
- Update script references to new directory structure
- Update CLAUDE.md with new repository structure
- Verify all relative paths in tests/examples

#### Step 1.5: Commit and Push
- Commit all changes to `Arduino_Core_STM32/ardu_ci` branch
- Push to `origin/ardu_ci`
- Tag as preparation for v1.0.0 release

### Phase 2: Create Board Manager Repository

#### Step 2.1: Create BoardManagerFiles Repository
- Create new GitHub repo: `geosmall/BoardManagerFiles`
- Initialize with README.md
- Add `.gitignore` for temporary files

#### Step 2.2: Create Package Index JSON
Create `package_stm32_core_index.json`:

```json
{
  "packages": [{
    "name": "STM32_Robotics",
    "maintainer": "geosmall",
    "websiteURL": "https://github.com/geosmall/Arduino_Core_STM32",
    "email": "",
    "platforms": [{
      "name": "STM32 Robotics Core",
      "architecture": "stm32",
      "version": "1.0.0",
      "category": "Contributed",
      "url": "https://github.com/geosmall/Arduino_Core_STM32/releases/download/v1.0.0/stm32-robotics-1.0.0.tar.gz",
      "archiveFileName": "stm32-robotics-1.0.0.tar.gz",
      "checksum": "SHA-256:xxxxx",
      "size": "50000000",
      "boards": [
        {"name": "NUCLEO_F411RE"},
        {"name": "BLACKPILL_F411CE"},
        {"name": "JHEF411"}
      ],
      "toolsDependencies": []
    }]
  }]
}
```

#### Step 2.3: Create Installation Documentation
Create `BoardManagerFiles/README.md`:

```markdown
# STM32 Robotics Core - Board Manager Installation

## Quick Install

1. Open Arduino IDE
2. Go to **File → Preferences**
3. Add this URL to "Additional Boards Manager URLs":
   ```
   https://raw.githubusercontent.com/geosmall/BoardManagerFiles/main/package_stm32_core_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for "STM32 Robotics"
6. Click **Install**

## Included Libraries

- **SerialRx** - RC receiver protocol parser (IBus, SBUS)
- **IMU** - High-level InvenSense IMU wrapper
- **ICM42688P** - Low-level 6-axis IMU driver
- **LittleFS** - SPI flash filesystem
- **SDFS** - SD card filesystem
- **Storage** - Unified storage abstraction
- **minIniStorage** - INI configuration management
- **TimerPWM** - Hardware timer PWM library
- **libPrintf** - Embedded printf library
- **STM32RTC** - Real-time clock

## Supported Boards

- NUCLEO-F411RE
- BlackPill F411CE
- NOXE V3 (JHEF411)

## Development Repository

https://github.com/geosmall/Arduino_Core_STM32
```

#### Step 2.4: Create Release Packaging Script
Create `BoardManagerFiles/system/ci/create_release.sh`:

```bash
#!/bin/bash
# Create Board Manager release archive from Arduino_Core_STM32

VERSION=$1
CORE_REPO="../Arduino_Core_STM32"
ARCHIVE_NAME="stm32-robotics-${VERSION}.tar.gz"

if [ -z "$VERSION" ]; then
  echo "Usage: $0 <version>"
  exit 1
fi

# Create archive from core repo
tar -czf "${ARCHIVE_NAME}" -C "${CORE_REPO}/.." Arduino_Core_STM32/

# Calculate checksum and size
SHA256=$(shasum -a 256 "${ARCHIVE_NAME}" | awk '{print $1}')
SIZE=$(stat -f%z "${ARCHIVE_NAME}" 2>/dev/null || stat -c%s "${ARCHIVE_NAME}")

echo "Archive: ${ARCHIVE_NAME}"
echo "SHA-256: ${SHA256}"
echo "Size: ${SIZE} bytes"
echo ""
echo "Update package_stm32_core_index.json with these values"
```

### Phase 3: Create Initial Release

#### Step 3.1: Tag Arduino_Core_STM32
```bash
cd Arduino_Core_STM32
git tag -a v1.0.0 -m "Initial Board Manager release"
git push origin v1.0.0
```

#### Step 3.2: Create Release Archive
```bash
cd BoardManagerFiles
./system/ci/create_release.sh 1.0.0
```

#### Step 3.3: Upload to GitHub Release
- Create GitHub release v1.0.0 in `Arduino_Core_STM32` repo
- Upload `stm32-robotics-1.0.0.tar.gz`
- Copy release URL

#### Step 3.4: Update Package Index
- Update `package_stm32_core_index.json` with:
  - Correct download URL from GitHub release
  - SHA-256 checksum
  - File size
- Commit and push to `BoardManagerFiles` repo

### Phase 4: Testing

#### Step 4.1: Test Board Manager Installation
1. Add URL to Arduino IDE preferences
2. Open Board Manager
3. Search for "STM32 Robotics"
4. Install package
5. Verify installation location: `~/Arduino15/packages/STM32_Robotics/`

#### Step 4.2: Test Example Compilation
1. Select board: **Tools → Board → STM32 Robotics Core → NUCLEO-F411RE**
2. Open example: **File → Examples → SerialRx → IBus_Basic**
3. Compile sketch
4. Verify libraries are found
5. Test with other examples (LittleFS, IMU, etc.)

#### Step 4.3: Test CI/CD Scripts
1. Clone `Arduino_Core_STM32` repo directly
2. Run build scripts: `./system/ci/build.sh tests/LittleFS_Unit_Tests --use-rtt`
3. Run HIL tests: `./system/ci/aflash.sh tests/SDFS_Unit_Tests --use-rtt --build-id`
4. Verify all scripts work with new structure

### Phase 5: Cleanup

#### Step 5.1: Archive Arduino_dev Repository
Option A - Add deprecation notice:
- Update `Arduino_dev/README.md` with redirect to `Arduino_Core_STM32`
- Add clear notice: "This repository is deprecated. All development moved to..."

Option B - Delete repository:
- Export any issues/discussions if needed
- Delete `geosmall/Arduino_dev` repository

#### Step 5.2: Update External References
- Update any documentation pointing to old repo
- Update links in GitHub profile/pinned repos
- Update any external references (forum posts, etc.)

## Final Repository Structure

### Arduino_Core_STM32 (Development)
```
Arduino_Core_STM32/
├── boards.txt                  # Board definitions
├── platform.txt                # Platform configuration
├── cores/arduino/              # Core Arduino implementation
│   └── ci_log.h               # CI/HIL logging abstraction
├── variants/                   # Board variants (F4, H7)
├── libraries/                  # ALL libraries
│   ├── SPI/                   # Core library
│   ├── Wire/                  # Core library
│   ├── SEGGER_RTT/            # RTT debugging
│   │   └── examples/
│   │       ├── BasicRTT/      # Simple RTT demo
│   │       └── HIL_Validation/ # HIL test suite (replaces HIL_RTT_Test/)
│   ├── SerialRx/              # RC receiver
│   ├── IMU/                   # High-level IMU
│   ├── ICM42688P/             # Low-level IMU
│   ├── LittleFS/              # SPI flash FS
│   ├── SDFS/                  # SD card FS
│   ├── Storage/               # Storage abstraction
│   ├── minIniStorage/         # INI config
│   ├── TimerPWM/              # PWM library
│   ├── libPrintf/             # Printf library
│   └── STM32RTC/              # RTC library
├── system/
│   └── ci/                     # CI/CD build scripts
├── tests/                      # Unit tests
├── targets/                    # Board configurations
├── extras/                     # Betaflight converter
├── doc/                        # Documentation
├── cmake/                      # CMake support
├── CLAUDE.md                   # Development guide
└── README.md                   # Project documentation
```

### BoardManagerFiles (Distribution)
```
BoardManagerFiles/
├── package_stm32_core_index.json  # Board Manager JSON
├── system/
│   └── ci/
│       └── create_release.sh      # Release packaging script
└── README.md                       # Installation instructions
```

## User Workflows

### Beginner Workflow (Arduino IDE)
1. Add Board Manager URL to Arduino IDE preferences
2. Install "STM32 Robotics Core" via Board Manager
3. Select board from **Tools → Board** menu
4. Open examples from **File → Examples** menu
5. Compile and upload sketches

### Developer Workflow (CI/CD)
1. Clone `Arduino_Core_STM32` repository
2. Use build scripts: `./system/ci/build.sh <sketch>`
3. Use HIL testing: `./system/ci/aflash.sh <sketch> --use-rtt --build-id`
4. Develop libraries in `libraries/` directory
5. Run unit tests in `tests/` directory

### Release Workflow (Maintainers)
1. Develop in `Arduino_Core_STM32` repository
2. Tag release: `git tag v1.x.x`
3. Run `create_release.sh` to build archive
4. Create GitHub release with archive
5. Update `package_stm32_core_index.json` with new version
6. Users get update notification in Board Manager

## Board Manager URL

**Installation URL:**
```
https://raw.githubusercontent.com/geosmall/BoardManagerFiles/main/package_stm32_core_index.json
```

## Migration Checklist

- [ ] **Pre-Consolidation** (✅ COMPLETED)
  - [x] Create HIL_Validation example in SEGGER_RTT library
  - [x] Delete HIL_RTT_Test/ directory
  - [x] Move Makefile to doc/Makefile.example
  - [x] Commit and push submodule changes
  - [x] Update parent repo with new submodule pointer

- [x] Phase 1: Consolidate into Arduino_Core_STM32 ✅ COMPLETE
  - [x] Move libraries (SerialRx, IMU, ICM42688P, LittleFS, SDFS, Storage, minIniStorage, TimerPWM, libPrintf, AUnit, STM32RTC)
  - [x] Move system/ci/tests/targets/extras/doc
  - [x] Consolidate documentation (CLAUDE.md, README.md)
  - [x] Update internal paths (example includes, script references)
  - [x] **Run cleanup_repo.sh in submodule** (manually clean build artifacts)
  - [x] Commit and push to ardu_ci branch

- [x] Phase 2: Create BoardManagerFiles ✅ COMPLETE
  - [x] Create GitHub repository: geosmall/BoardManagerFiles
  - [x] Create package_stm32_robotics_index.json with minimal structure
  - [x] Create README.md with installation instructions
  - [ ] Create system/ci/create_release.sh packaging script ❌ SKIPPED (v1.0.0 done manually)

- [x] Phase 3: Create initial release ✅ COMPLETE
  - [x] Tag Arduino_Core_STM32 as v1.0.0
  - [x] Create release archive (STM32-Robotics-1.0.0.tar.bz2)
  - [x] Upload to GitHub release in Arduino_Core_STM32 repo
  - [x] Update package_stm32_robotics_index.json with checksum/size
  - [x] Commit and push BoardManagerFiles

- [ ] Phase 4: Testing
  - [ ] Test Board Manager installation (add URL, install package)
  - [ ] Test example compilation (SerialRx, LittleFS, SEGGER_RTT examples)
  - [ ] Test CI/CD scripts (aflash.sh with HIL_Validation, unit tests)
  - [ ] Verify libraries auto-discovered in Arduino IDE

- [ ] Phase 5: Cleanup
  - [ ] Archive Arduino_dev with deprecation notice OR delete repository
  - [ ] Update external references (GitHub profile, documentation links)

## Notes

- **No toolchain dependencies**: Uses existing STMicroelectronics:stm32 toolchain installed via official Board Manager
- **Library auto-discovery**: Arduino IDE automatically finds libraries in `libraries/` folder
- **Version management**: Semantic versioning (1.0.0, 1.1.0, etc.)
- **Update notifications**: Board Manager shows updates when new versions published
- **Backward compatibility**: CI/CD scripts continue working with consolidated repository

## Lessons Learned

### Submodule Cleanup
- Parent repo `cleanup_repo.sh` does NOT automatically clean submodule build artifacts
- Must manually clean submodule before committing:
  ```bash
  cd Arduino_Core_STM32
  rm -rf libraries/*/examples/*/build/
  rm -f libraries/*/examples/*/build_id.h
  ```
- Or add cleanup script to submodule for consistency

### Git Workflow for Submodules
1. **Always commit submodule first**, then parent repo
2. Parent repo tracks specific submodule commit SHA
3. Pushing parent before submodule creates broken reference
4. Correct order:
   ```bash
   cd Arduino_Core_STM32
   git add <files>
   git commit -m "..."
   git push origin ardu_ci
   cd ..
   git add Arduino_Core_STM32  # Updates pointer
   git commit -m "..."
   git push origin master
   ```

### Build Artifacts to Exclude
When staging commits, exclude:
- `build/` directories (contains .bin, .elf, .hex, .map files)
- `build_id.h` (auto-generated by build scripts)
- Any IDE-specific files (.vscode/, .idea/)

### HIL Testing
- Library examples can use direct SEGGER_RTT API (appropriate for RTT library)
- Application sketches should use ci_log.h for dual-mode support (RTT/Serial)
- Both patterns work with aflash.sh - no requirement to use ci_log.h

## Future Enhancements

- GitHub Actions workflow to automate release creation
- Automated testing of Board Manager installation
- Multiple board package versions (stable, beta)
- Library-specific versioning within core package
- Add cleanup_repo.sh to Arduino_Core_STM32 submodule for consistency
