# Script Testing Guide

Testing instructions for Build Workflow scripts implementing **STM32 Arduino development with J-Run + RTT + Exit Wildcard Detection**.

## Exit Wildcard Methodology

Tests use **`"*STOP*"` detection** for deterministic completion (no timeouts). J-Run monitors RTT output and exits automatically when wildcard detected.

**Test Pattern**:
```cpp
if (test_complete) {
    SEGGER_RTT_WriteString(0, "*STOP*\r\n");  // Triggers J-Run exit
    while(1) { /* halt */ }
}
```

## Scripts by Functional Groups

### Core Essential Scripts (8)
| Script | Purpose | Dependencies |
|--------|---------|-------------|
| **Environment & Validation** |
| `env_probe.sh` | Comprehensive environment diagnostics | arduino-cli |
| `env_check_quick.sh` | **Fast environment validation (~100ms)** | **arduino-cli** |
| **Build & Test Orchestration** |
| `build.sh` | Arduino CLI compile with optional --env-check, --build-id, --use-rtt, --clean-cache | arduino-cli |
| `aflash.sh` | **One-button build-jrun-test with optional --env-check, --build-id, --use-rtt, --clean-cache** | **J-Run ELF execution** |
| **Device Detection & Programming** |
| `detect_device.sh` | **Universal STM32 device auto-detection** | **J-Link** |
| `jrun.sh` | **J-Run execution with RTT + exit wildcard detection** | **JRun (primary)** |
| **Build-ID & Ready Token** |
| `generate_build_id.sh` | **Generate build_id.h with git SHA + UTC timestamp** | **git** |
| `await_ready.sh` | **Ready token detection with sub-20ms latency stats** | **bc** |

### Programming Scripts (2)
| Script | Purpose | Dependencies |
|--------|---------|-------------|
| `flash.sh` | J-Link flash with --quick/full modes (fixed F411RE) | JLinkExe |
| `flash_auto.sh` | **J-Link flash with auto-detected device** | **J-Link + detect_device.sh** |

### Legacy/Support Scripts (1)
| Script | Purpose | Dependencies |
|--------|---------|-------------|
| `rtt_cat.sh` | RTT logging with timestamps (⚠️ **deprecated - use jrun.sh**) | JLinkGDBServer, JLinkRTTClient |

## Prerequisites

- **Hardware**: STM32 Nucleo F411RE with J-Link firmware
- **Software**: arduino-cli 1.3.0+, STM32 core 2.7.1+, J-Link v8.62+
- **Test Sketch**: HIL_RTT_Test/

**Quick Check**: `ls -la scripts/` (all should be executable)

## Individual Script Testing

### 1. env_probe.sh - Environment Probe
Captures environment state and validates toolchain.

```bash
./scripts/env_probe.sh
cat test_logs/env/latest_probe.txt
```
**Success**: FQBN valid, log created, no errors

### 2. detect_device.sh - Device Auto-Detection
Auto-detect any STM32 via J-Link DBGMCU_IDCODE register.

```bash
./scripts/detect_device.sh
eval $(./scripts/detect_device.sh | grep STM32_DEVICE_ID)
```
**Success**: Device identified, STM32_DEVICE_ID exported

### 3. env_check_quick.sh - Fast Environment Validation
Lightning-fast environment validation (~100ms) for build workflows.

```bash
./scripts/env_check_quick.sh        # Silent (exit code)
./scripts/env_check_quick.sh true   # Verbose output
```
**Success**: Exit code 0, all components validated, <200ms

### 4. jrun.sh - J-Run Execution (PRIMARY)
J-Run-based ELF execution with integrated RTT capture. Primary HIL test runner.

```bash
ELF_PATH=$(find /home/geo/.cache/arduino/sketches -name "HIL_RTT_Test.ino.elf" | head -1)
./scripts/jrun.sh "$ELF_PATH"
./scripts/jrun.sh "$ELF_PATH" STM32F411RE 60 test_run "*STOP*"
```
**Success**: ELF loads, RTT connects, exit wildcard detection works

### 5. flash_auto.sh - Auto-Detection Flash
Enhanced flash tool with universal STM32 device auto-detection.

```bash
arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE HIL_RTT_Test --export-binaries
./scripts/flash_auto.sh --quick HIL_RTT_Test/build/STMicroelectronics.stm32.Nucleo_64/HIL_RTT_Test.ino.bin
./scripts/flash_auto.sh HIL_RTT_Test/build/STMicroelectronics.stm32.Nucleo_64/HIL_RTT_Test.ino.bin
```
**Success**: Device auto-detected, optimal J-Link device used, programming completes

### 6. flash.sh - Fixed Device Flash
J-Link flash tool with dual modes (fixed F411RE device).

```bash
./scripts/flash.sh --quick HIL_RTT_Test/build/STMicroelectronics.stm32.Nucleo_64/HIL_RTT_Test.ino.bin
./scripts/flash.sh HIL_RTT_Test/build/STMicroelectronics.stm32.Nucleo_64/HIL_RTT_Test.ino.bin
```
**Success**: Quick mode <2s, full mode ~9s, no prompts

### 7. build.sh - Arduino Build
Compile sketches with build caching, ELF preservation, optional environment validation, build-ID generation, unified RTT support, and cache management.

```bash
./scripts/build.sh HIL_RTT_Test
./scripts/build.sh HIL_RTT_Test --env-check
./scripts/build.sh HIL_RTT_Test --env-check --build-id
./scripts/build.sh HIL_RTT_Test --use-rtt --build-id --env-check  # Unified framework
./scripts/build.sh HIL_RTT_Test --clean-cache --use-rtt --build-id  # Cache management
./scripts/build.sh libraries/LittleFS/examples/ListFiles --use-rtt    # LittleFS with RTT
./scripts/build.sh HIL_RTT_Test STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE --env-check
```
**Success**: Compilation succeeds, binary + ELF created, build time displayed, optional build_id.h generated, RTT mode enabled, cache cleared when requested

### 8. rtt_cat.sh - RTT Logger (⚠️ DEPRECATED)
Legacy RTT capture with timestamps. Use jrun.sh instead.

```bash
./scripts/rtt_cat.sh 10 test_run
```
**Success**: RTT connection, timestamped output, log created

### 9. generate_build_id.sh - Build-ID Header Generation (Phase 5)
Generate build_id.h with git SHA + UTC timestamp for deterministic builds.

```bash
./scripts/generate_build_id.sh HIL_RTT_Test
./scripts/generate_build_id.sh .
ls -la HIL_RTT_Test/build_id.h
```
**Success**: build_id.h created with BUILD_GIT_SHA, BUILD_UTC_TIME macros, git info displayed

### 10. await_ready.sh - Enhanced Ready Token Detection (Phase 5)
Enhanced ready token detection with build-ID parsing and sub-20ms latency measurement.

```bash
./scripts/await_ready.sh test_logs/rtt/latest_jrun.txt
./scripts/await_ready.sh test_logs/rtt/latest_jrun.txt 30 "READY"
./scripts/await_ready.sh --help
```
**Success**: Enhanced ready token detected with build-ID parsing:
- Pattern: `READY NUCLEO_F411RE 901dbd1-dirty 2025-09-09T10:07:44Z`
- Parsed: Board=NUCLEO_F411RE, Git=901dbd1-dirty, Time=2025-09-09T10:07:44Z
- Latency: Sub-20ms (5.2ms achieved), duration measured

### 11. aflash.sh - One-Button Workflow (MAIN)
Complete build-jrun-test workflow with optional environment validation, build-ID traceability, unified RTT framework support, and cache management.

```bash
./scripts/aflash.sh HIL_RTT_Test
./scripts/aflash.sh HIL_RTT_Test --env-check
./scripts/aflash.sh HIL_RTT_Test --use-rtt --build-id --env-check     # Complete unified workflow
./scripts/aflash.sh HIL_RTT_Test --clean-cache --use-rtt --build-id   # With cache management
./scripts/aflash.sh libraries/LittleFS/examples/ListFiles --use-rtt      # LittleFS unified test
./scripts/aflash.sh HIL_RTT_Test STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE 60 "*STOP*" --env-check
```
**Success**: Build completes, J-Run executes with RTT, exit wildcard detected, build traceability included, cache cleared when requested

## Integration Testing

### Cache Management Testing
Test Arduino CLI cache management for deterministic builds:

```bash
# Normal incremental build (fast)
./scripts/build.sh HIL_RTT_Test --use-rtt --build-id

# Clean cache build (deterministic, after file moves)
./scripts/build.sh HIL_RTT_Test --clean-cache --use-rtt --build-id

# Full HIL workflow with cache management
./scripts/aflash.sh tests/AUnit_Pilot_Test --clean-cache --use-rtt --build-id
```
**Success**: Cache cleared when requested, fresh compilation, correct file paths in output, deterministic builds

### Unified Development Framework Test
Test the single-codebase approach supporting both Arduino IDE and CI/HIL workflows:

```bash
# Arduino IDE mode (Serial output)
./scripts/build.sh libraries/LittleFS/examples/ListFiles

# CI/HIL mode (RTT output with build traceability)
./scripts/aflash.sh libraries/LittleFS/examples/ListFiles --use-rtt --build-id --env-check
```
**Success**: Same sketch works in both modes, appropriate output format, deterministic completion

### Complete Workflow Test
```bash
./scripts/env_probe.sh
./scripts/aflash.sh HIL_RTT_Test --env-check
./scripts/await_ready.sh test_logs/rtt/latest_jrun.txt
ls -la test_logs/env/ test_logs/rtt/
```

### Performance Benchmarks
```bash
time ./scripts/aflash.sh HIL_RTT_Test
```
**Expected**: Build ~2s, J-Run ~3s, total ~5s + test duration

## Automation Details

**J-Run (Primary)**: ELF-based execution with integrated RTT, automatic PC/SP init
**JLinkExe (Fallback)**: Binary flash with command files, no prompts

**Performance**: Build ~2s, J-Run ~3s, complete workflow ~5s + test duration

## Troubleshooting

```bash
# J-Link connection issues
lsusb | grep -i segger
pkill -f JLinkGDBServer

# Build failures
./scripts/env_probe.sh
arduino-cli core list

# Permission errors
chmod +x scripts/*.sh
```

## Log Files

- **Environment**: `test_logs/env/latest_probe.txt`
- **J-Run**: `test_logs/rtt/latest_jrun.txt` 
- **RTT**: `test_logs/rtt/latest_rtt.txt`

## Validation Status Matrix

Current validation status of all script functionality:

### Core Essential Scripts
| Script | Basic Run | Error Handling | Performance | Logging |
|--------|-----------|----------------|-------------|---------|
| env_probe.sh | ✅ | ✅ | ✅ | ✅ |
| **env_check_quick.sh** | **✅** | **✅** | **✅** | **N/A** |
| **detect_device.sh** | **✅** | **✅** | **✅** | **N/A** |
| build.sh | ✅ | ✅ | ✅ | N/A |
| build.sh (--env-check) | ✅ | ✅ | ✅ | N/A |
| build.sh (--build-id) | ✅ | ✅ | ✅ | N/A |
| build.sh (--use-rtt) | ✅ | ✅ | ✅ | N/A |
| build.sh (--clean-cache) | ✅ | ✅ | ✅ | N/A |
| **jrun.sh (primary)** | **✅** | **✅** | **✅** | **✅** |
| **aflash.sh (complete workflow)** | **✅** | **✅** | **✅** | **✅** |
| **aflash.sh (--env-check)** | **✅** | **✅** | **✅** | **✅** |
| **aflash.sh (--build-id)** | **✅** | **✅** | **✅** | **✅** |
| **aflash.sh (--use-rtt)** | **✅** | **✅** | **✅** | **✅** |
| **aflash.sh (--clean-cache)** | **✅** | **✅** | **✅** | **✅** |

### Enhanced Build-ID Scripts
| Script | Basic Run | Error Handling | Performance | Logging |
|--------|-----------|----------------|-------------|---------|
| **generate_build_id.sh** | **✅** | **✅** | **✅** | **N/A** |
| **await_ready.sh** | **✅** | **✅** | **✅** | **✅** |

### Programming Scripts
| Script | Basic Run | Error Handling | Performance | Logging |
|--------|-----------|----------------|-------------|---------|
| flash.sh (--quick) | ✅ | ✅ | ✅ | N/A |
| flash.sh (full) | ✅ | ✅ | ✅ | N/A |
| **flash_auto.sh (--quick)** | **✅** | **✅** | **✅** | **N/A** |
| **flash_auto.sh (full)** | **✅** | **✅** | **✅** | **N/A** |

### Legacy/Support Scripts
| Script | Basic Run | Error Handling | Performance | Logging |
|--------|-----------|----------------|-------------|---------|
| rtt_cat.sh (⚠️ deprecated) | ✅ | ✅ | ❌ | ✅ |

### Integration Testing Status
- **Device Auto-Detection Integration**: ✅ **VALIDATED** (50+ STM32 device IDs, optimal J-Link mapping)
- **Environment Validation Integration**: ✅ **VALIDATED** (~100ms performance, comprehensive checks)
- **Build-ID Traceability Integration**: ✅ **VALIDATED** (git SHA + UTC timestamp, 5.2ms ready token detection)
- **Unified Development Framework**: ✅ **VALIDATED** (Arduino IDE ↔ CI/HIL seamless switching)
- **AUnit Testing Integration**: ✅ **VALIDATED** (15 tests across LittleFS/SDFS/Framework, 100% pass rate)
- **Full System Integration**: ✅ **VALIDATED** (Complete build→test→deploy pipeline)

### Hardware Validation
- **STM32F411RE Nucleo**: ✅ **PRODUCTION READY**
- **J-Link RTT Integration**: ✅ **PRODUCTION READY**
- **SPI Flash (W25Q128JV)**: ✅ **PRODUCTION READY**
- **SD Card (SDFS)**: ✅ **PRODUCTION READY**

All core workflow components have been comprehensively validated for production use.

## Enhanced Build-ID Integration ✅ COMPLETED

**Enhanced Ready Token System**:
- **Previous**: `HIL_READY F411RE Sep  9 2025`
- **Enhanced**: `READY NUCLEO_F411RE 901dbd1-dirty 2025-09-09T10:07:44Z`

**Production Validation Results**:
- **Build-ID Integration**: ✅ Working (git SHA + UTC timestamp + dirty state)
- **Enhanced Ready Tokens**: ✅ Operational with automatic parsing
- **Ready Token Latency**: ✅ 5.2ms achieved (sub-20ms target)
- **Complete Traceability**: ✅ Git commit → runtime firmware linkage confirmed
- **HIL Test Integration**: ✅ Deterministic "*STOP*" wildcard detection maintained

**Build-to-Runtime Traceability**:
```
Build: Sep  9 2025 06:07:54
Git: 901dbd1-dirty (2025-09-09T10:07:44Z)
READY NUCLEO_F411RE 901dbd1-dirty 2025-09-09T10:07:44Z
```


---
*This testing guide ensures robust verification of the Build Workflow implementation for STM32 Arduino development with J-Link and RTT.*