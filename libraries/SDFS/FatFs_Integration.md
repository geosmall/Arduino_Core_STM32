# FatFs Integration Documentation

This document explains how the SDFS library integrates with the FatFs filesystem and provides guidance for future maintenance and configuration management.

## Overview

The SDFS library provides an Arduino-compatible interface for SD card access using the FatFs filesystem library with a custom SPI disk I/O layer. The implementation follows established embedded library design patterns with a comprehensive configuration system, following the LittleFS API pattern to allow seamless switching between SPI flash (LittleFS) and SD card (SDFS) storage.

## Architecture Overview

### Layer Structure

```
┌─────────────────────────────────────────┐
│            Arduino Sketch               │
├─────────────────────────────────────────┤
│              SDFS Library               │
│  ┌──────────────┐  ┌─────────────────┐  │
│  │     SDFS     │  │    SDFSFile     │  │
│  │     (FS)     │  │   (FileImpl)    │  │
│  └──────────────┘  └─────────────────┘  │
├─────────────────────────────────────────┤
│          Configuration System           │
│  ┌───────────────────────────────────┐  │
│  │         SDFSConfig.h              │  │
│  └───────────────────────────────────┘  │
├─────────────────────────────────────────┤
│               FatFs Core                │
│  ┌───────────────────────────────────┐  │
│  │           ff.c / ff.h             │  │
│  └───────────────────────────────────┘  │
├─────────────────────────────────────────┤
│           Disk I/O Abstraction          │
│  ┌───────────────────────────────────┐  │
│  │         sd_spi_diskio.cpp         │  │
│  └───────────────────────────────────┘  │
├─────────────────────────────────────────┤
│            Hardware Layer               │
│  ┌───────────────────────────────────┐  │
│  │       STM32 SPI + SD Card         │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

## Configuration System

### SDFSConfig.h Architecture

SDFS v1.0+ uses a configuration-driven design following SdFat library patterns. All magic numbers have been eliminated and replaced with configurable constants.

#### Configuration Categories

**SPI Communication Settings**
```cpp
#ifndef SDFS_SPI_INIT_SPEED_HZ
#define SDFS_SPI_INIT_SPEED_HZ 400000      // SD specification: 400kHz max during init
#endif

#ifndef SDFS_SPI_MID_SPEED_HZ
#define SDFS_SPI_MID_SPEED_HZ 2000000      // Safe speed for card detection
#endif

#ifndef SDFS_SPI_MAX_SPEED_HZ
#define SDFS_SPI_MAX_SPEED_HZ 4000000      // Operational speed for SDHC cards
#endif
```

**Timeout Configuration**
```cpp
#ifndef SDFS_INIT_TIMEOUT_MS
#define SDFS_INIT_TIMEOUT_MS 1000          // Card initialization timeout
#endif

#ifndef SDFS_CMD_TIMEOUT_MS
#define SDFS_CMD_TIMEOUT_MS 500            // Command response timeout
#endif

#ifndef SDFS_DATA_TIMEOUT_MS
#define SDFS_DATA_TIMEOUT_MS 300           // Data block transfer timeout
#endif

#ifndef SDFS_BUSY_TIMEOUT_MS
#define SDFS_BUSY_TIMEOUT_MS 100           // Busy wait timeout
#endif
```

**Buffer and Memory Settings**
```cpp
#ifndef SDFS_NAME_MAX
#define SDFS_NAME_MAX 255           // Maximum filename/path length (LittleFS compatible)
#endif

#ifndef SDFS_SECTOR_SIZE
#define SDFS_SECTOR_SIZE 512               // Default sector size (runtime detected)
#endif

#ifndef SDFS_MAX_RETRIES
#define SDFS_MAX_RETRIES 3                 // Operation retry count
#endif
```

**Debug and Platform Settings**
```cpp
#ifndef SDFS_DEBUG_LEVEL
#define SDFS_DEBUG_LEVEL 0                 // 0=off, 1=basic, 2=verbose
#endif

#ifndef SDFS_USE_RTT
#define SDFS_USE_RTT 0                     // Enable RTT debugging
#endif

#ifndef SDFS_DEFAULT_CS_PIN
#define SDFS_DEFAULT_CS_PIN 10             // Default CS pin
#endif
```

### Configuration Usage Patterns

#### Development Configuration
```cpp
// SDFSConfig.h for development/debugging
#define SDFS_SPI_MAX_SPEED_HZ     1000000  // Conservative speed
#define SDFS_INIT_TIMEOUT_MS      5000     // Extended timeouts
#define SDFS_CMD_TIMEOUT_MS       2000     // Extended timeouts
#define SDFS_DEBUG_LEVEL          2        // Verbose debugging
#define SDFS_USE_RTT              1        # RTT debug output
```

#### High-Performance Configuration
```cpp
// SDFSConfig.h for flight controllers
#define SDFS_SPI_MAX_SPEED_HZ     8000000  // High performance
#define SDFS_CMD_TIMEOUT_MS       200      // Fast timeouts
#define SDFS_DATA_TIMEOUT_MS      150      // Fast timeouts
#define SDFS_DEBUG_LEVEL          0        // No debug overhead
```

#### Platform-Specific Configuration
```cpp
// SDFSConfig.h for STM32F411 with specific hardware requirements
#define SDFS_SPI_MAX_SPEED_HZ     4000000  // F411 safe maximum
#define SDFS_NAME_MAX      128      // Memory-constrained filename length
#define SDFS_DEFAULT_CS_PIN       PD2     // Nucleo F411RE CS pin
```

## FatFs Source Files

### Current Version
- **FatFs Version**: R0.15 (Revision 80286)
- **Source Date**: Copied from STMicroelectronics FatFs library v4.0.0
- **Original Author**: ChaN (http://elm-chan.org/fsw/ff/)

### Files Included

The following FatFs files are embedded in `src/fatfs/`:

| File | Source | Purpose |
|------|--------|---------|
| `ff.h` | FatFs core | Main FatFs header file |
| `ff.c` | FatFs core | Main FatFs implementation |
| `ffconf.h` | Modified from `ffconf_template.h` | FatFs configuration |
| `diskio.h` | FatFs core | Disk I/O abstraction layer header |
| `integer.h` | FatFs core | FatFs integer type definitions |

### Configuration Changes

The `ffconf.h` file has been modified from the original template:

```c
#define FF_USE_MKFS    1    // Enable f_mkfs() for formatting support
```

All other settings remain at their default values for basic functionality.

## Runtime Detection System

### Sector Size Detection

SDFS v1.0+ implements runtime sector size detection:

```cpp
// Runtime sector size variables
static uint16_t actual_sector_size = SDFS_SECTOR_SIZE;  // Runtime detected size

// Detection functions
uint16_t sd_spi_get_sector_size(void);                  // Get current sector size
bool sd_spi_set_sector_size(uint16_t size);            // Set custom sector size (512-4096)
```

**Supported Sector Sizes**: 512, 1024, 2048, 4096 bytes (power of 2)
**Default Fallback**: 512 bytes (standard SD specification)

### Card Capacity Detection

Dynamic CSD (Card-Specific Data) reading replaces hardcoded capacity:

```cpp
// CSD parsing for SDHC/SDXC cards (Version 2.0)
if (is_sdhc_card) {
    uint32_t c_size = ((uint32_t)(csd[7] & 0x3F) << 16) |
                      ((uint32_t)csd[8] << 8) |
                      csd[9];
    card_sector_count = (c_size + 1) * 1024;  // SDHC formula
}
```

**Benefits**:
- Supports cards from 2GB to 2TB
- Eliminates 32MB hardcoded limit
- Provides accurate capacity reporting

### Speed Negotiation

Three-phase speed negotiation system:

1. **Initialization Phase**: `SDFS_SPI_INIT_SPEED_HZ` (400kHz)
2. **Detection Phase**: `SDFS_SPI_MID_SPEED_HZ` (2MHz)
3. **Operation Phase**: `SDFS_SPI_MAX_SPEED_HZ` (4MHz default, configurable up to 8MHz)

## FatFs Disk I/O Abstraction Layer

FatFs uses a standardized disk I/O interface defined in `diskio.h`. Our implementation in `sd_spi_diskio.cpp` provides:

### Required Functions

```c
DSTATUS disk_initialize(BYTE pdrv);    // Initialize disk drive
DSTATUS disk_status(BYTE pdrv);        // Get disk status
DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count);    // Read sectors
DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);  // Write sectors
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff);   // I/O control operations
DWORD get_fattime(void);               // Get current time for timestamps
```

### Drive Mapping

- **Drive 0**: SPI SD Card (configured via `SDFS_SPI::begin()`)
- **Drives 1-9**: Not used (return `STA_NOINIT`)

### SPI SD Card Implementation

The `sd_spi_diskio.cpp` file implements:

1. **Configurable Low-level SPI Communication**: Uses SDFSConfig.h constants
2. **Comprehensive SD Card Initialization**: Configurable CMD0, CMD8, ACMD41 sequence
3. **Runtime Block Operations**: Dynamic sector size support
4. **Comprehensive Error Handling**: Configurable timeouts and retry logic

Key features:
- **Configurable SPI**: CS pin and SPI port selection
- **Comprehensive Speed Management**: Three-phase configurable speed progression
- **Standard Compliance**: SD Card specification v2.0 commands
- **Runtime Adaptation**: Dynamic sector size and capacity detection

## API Mapping

### SDFS to FatFs Function Mapping

| SDFS Method | FatFs Function | Configuration Impact |
|-------------|----------------|---------------------|
| `open()` | `f_open()` | Path length limited by `SDFS_NAME_MAX` |
| `read()` | `f_read()` | Transfer size uses runtime `actual_sector_size` |
| `write()` | `f_write()` | Transfer size uses runtime `actual_sector_size` |
| `seek()` | `f_lseek()` | Timeout controlled by `SDFS_CMD_TIMEOUT_MS` |
| `mkdir()` | `f_mkdir()` | Path validation uses `SDFS_NAME_MAX` |
| `remove()` | `f_unlink()` | Operation timeout configurable |
| `rename()` | `f_rename()` | Path validation for both old and new names |
| `exists()` | `f_stat()` | Special root directory handling |
| `format()` | `f_mkfs()` | Uses detected `actual_sector_size` |
| `usedSize()` | `f_getfree()` | Calculation uses runtime sector count |

### File Handle Management

- **FIL structures**: Dynamically allocated for file operations
- **DIR structures**: Dynamically allocated for directory operations
- **Reference Counting**: Handled by Arduino FS.h FileImpl base class
- **Memory Limits**: Configurable via `SDFS_NAME_MAX` (LittleFS compatible naming)

## Configuration Best Practices

### Platform Optimization

**STM32F411RE (Nucleo)**
```cpp
// Optimized for 100MHz Cortex-M4
#define SDFS_SPI_MAX_SPEED_HZ     4000000  // Safe for F411 at 100MHz
#define SDFS_NAME_MAX      255      // Standard length (LittleFS compatible)
#define SDFS_CMD_TIMEOUT_MS       500      // Balanced performance/reliability
```

**STM32F411CE (BlackPill)**
```cpp
// Optimized for breadboard development
#define SDFS_SPI_MAX_SPEED_HZ     2000000  // Conservative for breadboard
#define SDFS_INIT_TIMEOUT_MS      2000     // Extended for development
#define SDFS_DEBUG_LEVEL          1        // Basic debugging enabled
```

### Application-Specific Tuning

**Flight Controller Configuration**
```cpp
// Real-time performance optimization
#define SDFS_SPI_MAX_SPEED_HZ     6000000  // High-speed logging
#define SDFS_CMD_TIMEOUT_MS       200      // Fast timeout for real-time
#define SDFS_DATA_TIMEOUT_MS      150      // Quick data operations
#define SDFS_MAX_RETRIES          1        // Minimize retry delays
```

**Data Logger Configuration**
```cpp
// Reliability over speed
#define SDFS_SPI_MAX_SPEED_HZ     2000000  // Conservative speed
#define SDFS_INIT_TIMEOUT_MS      5000     # Extended initialization
#define SDFS_CMD_TIMEOUT_MS       1000     // Extended timeouts
#define SDFS_MAX_RETRIES          5        // Multiple retry attempts
```

## Updating FatFs

### When to Update

Consider updating FatFs when:
- Security vulnerabilities are discovered
- New features are needed (exFAT, long filename support, etc.)
- Bug fixes are available
- Performance improvements are released

### Update Procedure

1. **Download Latest FatFs**
   ```bash
   wget http://elm-chan.org/fsw/ff/arc/ff15.zip
   unzip ff15.zip
   ```

2. **Backup Current Implementation**
   ```bash
   cp -r src/fatfs src/fatfs_backup_$(date +%Y%m%d)
   cp src/SDFSConfig.h src/SDFSConfig_backup_$(date +%Y%m%d).h
   ```

3. **Replace Core Files**
   ```bash
   cp ff15/source/ff.h src/fatfs/
   cp ff15/source/ff.c src/fatfs/
   cp ff15/source/diskio.h src/fatfs/
   cp ff15/source/integer.h src/fatfs/
   ```

4. **Update Configuration**
   - Compare `ff15/source/ffconf.h` with `src/fatfs/ffconf.h`
   - Merge any new configuration options
   - Ensure `FF_USE_MKFS 1` remains enabled
   - Check for API changes in configuration structure

5. **Verify Configuration System Compatibility**
   - Ensure all `SDFS_*` constants are still referenced correctly
   - Check that runtime detection functions remain compatible
   - Verify configuration-driven timeouts still work

6. **Check API Compatibility**
   - Review `ff15/doc/updates.txt` for breaking changes
   - Update SDFS wrapper functions if needed
   - Pay attention to:
     - Function signature changes
     - New error codes
     - Configuration option changes

7. **Test Compilation**
   ```bash
   arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE libraries/SDFS/examples/SDFS_Test
   ```

8. **Validate Functionality**
   - Test basic file operations (create, read, write, delete)
   - Test directory operations
   - Test formatting with runtime sector sizes
   - Test configuration system functionality
   - Verify no memory leaks
   - Check performance impact
   - Validate HIL test suite passes

9. **Update Documentation**
   - Update version information in this file
   - Document any API changes
   - Update configuration examples if needed
   - Update README.md with new features

### Common Update Issues

1. **Configuration Changes**
   - New options may be added to `ffconf.h`
   - Some options may be renamed or deprecated
   - Solution: Carefully merge configurations, maintain SDFS customizations

2. **API Changes**
   - Function parameters may change
   - New error codes may be introduced
   - Solution: Update wrapper functions in SDFS.cpp, update configuration handling

3. **Memory Layout Changes**
   - FIL/DIR structure sizes may change
   - Solution: Usually transparent due to dynamic allocation and configuration system

4. **Disk I/O Interface Changes**
   - diskio.h interface is stable but may add new commands
   - Solution: Implement new commands or return appropriate errors, ensure configuration constants work

### Version History

| Date | FatFs Version | SDFS Version | Notes |
|------|---------------|--------------|-------|
| 2025-09-13 | R0.15 (80286) | v1.0.0 | Comprehensive configuration system, runtime detection |
| 2024-09-09 | R0.15 (80286) | v0.9.0 | SDHC optimization, directory enumeration fixes |
| 2024-08-21 | R0.15 (80286) | v0.8.0 | Initial implementation from STM32 FatFs v4.0.0 |

## Troubleshooting

### Configuration-Related Issues

1. **Compilation Errors**
   - Check `SDFSConfig.h` syntax and defines
   - Verify all required files are present
   - Check for conflicting FatFs installations
   - Ensure configuration values are within valid ranges

2. **Runtime Configuration Problems**
   - Verify SPI speed settings are appropriate for hardware
   - Check timeout values are reasonable for card type
   - Ensure sector size detection is working correctly
   - Validate configuration constants are being used

3. **Performance Issues**
   - Adjust `SDFS_SPI_MAX_SPEED_HZ` for your hardware setup
   - Tune timeout values in configuration
   - Consider cluster size optimization in `f_mkfs()`
   - Profile disk I/O operations with configuration logging

### Debug Configuration

Enable debugging via configuration:
```cpp
// In SDFSConfig.h
#define SDFS_DEBUG_LEVEL 2        // Verbose debugging
#define SDFS_USE_RTT 1           // Use RTT for debug output

// Optional: Enable specific debug categories
#define SDFS_DEBUG_SPI 1         // SPI communication debug
#define SDFS_DEBUG_FATFS 1       // FatFs operation debug
```

### HIL Testing Integration

The SDFS configuration system integrates with the HIL testing framework:

```bash
# Test with specific configuration
./scripts/aflash.sh libraries/SDFS/examples/SDFS_Test --use-rtt --build-id

# Configuration variants testing
SDFS_CONFIG=high_perf ./scripts/aflash.sh libraries/SDFS/examples/SDFS_Test
SDFS_CONFIG=conservative ./scripts/aflash.sh libraries/SDFS/examples/SDFS_Test
```

## References

- **FatFs Official Site**: http://elm-chan.org/fsw/ff/
- **FatFs Documentation**: http://elm-chan.org/fsw/ff/doc/00index_e.html
- **SD Card Specification**: https://www.sdcard.org/downloads/pls/
- **Arduino FS Library**: https://github.com/arduino/ArduinoCore-API/blob/master/api/FS.h
- **LittleFS Reference**: https://github.com/ARMmbed/littlefs
- **SdFat Configuration Patterns**: https://github.com/greiman/SdFat (configuration reference)