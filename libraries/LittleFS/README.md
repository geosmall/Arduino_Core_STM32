# LittleFS for STM32 Arduino Core

A comprehensive LittleFS implementation for STM32 microcontrollers with CI/HIL testing integration, optimized for embedded systems development.

## Overview

This LittleFS library provides high-performance SPI flash storage for STM32-based embedded systems with:
- **Unified Development Framework**: Seamless Arduino IDE ↔ J-Run/RTT workflow switching
- **Complete CI/HIL Integration**: Deterministic testing with build traceability
- **Production-Ready Examples**: All examples integrated with `ci_log.h` logging abstraction
- **Embedded Systems Focus**: Optimized for configuration storage, data logging, and firmware management

## Supported Hardware

### SPI Flash Chips

**Tested Hardware:**
- **Winbond W25Q128JV-Q** (16MB) - Primary validation platform

**Supported Chips:**

**Winbond Series:**
- W25Q16JV-Q (2MB), W25Q32JV-Q (4MB), W25Q64JV-Q (8MB)
- W25Q128JV-Q (16MB), W25Q256JV-Q (32MB), W25Q512JV-Q (64MB), W25Q01JV-Q (128MB)
- W25Q64JV-M (8MB), W25Q128JV-M (16MB), W25Q256JV-M (32MB), W25Q512JV-M (64MB) - DTR variants
- W25Q256JW-M (32MB)

**GigaDevice Series:**
- GD25Q40C (512KB), GD25Q80C (1MB), GD25Q16E (2MB)
- GD25Q32E (4MB), GD25Q64E (8MB), GD25Q128E (16MB), GD25Q256E (32MB)

**Other Manufacturers:**
- **Microchip**: SST25PF040C (512KB)
- **Adesto/Atmel**: AT25SF041 (512KB)
- **Spansion**: S25FL208K (1MB)

**Capacity Range:** 512KB to 128MB with automatic chip detection

## Key Features

### Storage Architecture
- **Wear Leveling**: Automatic block rotation for flash longevity
- **Power-Safe**: Atomic operations with power-loss protection
- **Compact**: Minimal RAM footprint suitable for embedded systems
- **Compatible API**: Standard Arduino FS.h interface for seamless library switching

### Development Workflow
- **Build Traceability**: Git SHA + UTC timestamp integration in all examples
- **Error Transparency**: Internal LittleFS debugging automatically routed to Serial/RTT
- **Interactive Removal**: All examples support fully automated CI/HIL testing
- **Hardware Detection**: Automatic chip identification and capacity detection

## Installation

### Prerequisites
- **Arduino CLI** v1.3.0 (locked version)
- **STM32 Core** v2.7.1 (STMicroelectronics:stm32)
- **Hardware**: Compatible STM32 board with SPI flash connected

### Library Installation
This library is included as part of the STM32 Arduino Core development environment. No separate installation required.

## Examples

### 1. Hardware Detection (`LittleFS_ChipID`)
Detects and displays complete SPI flash chip information with optional chip erase capability.

**Key Features:**
- Chip ID detection (manufacturer, JEDEC ID)
- Memory architecture analysis (blocks, sectors, pages)
- Optional full chip erase for testing
- Build traceability integration

**Usage:**
```bash
# Arduino IDE development
./scripts/build.sh libraries/LittleFS/examples/LittleFS_ChipID

# CI/HIL testing with RTT
./scripts/aflash.sh libraries/LittleFS/examples/LittleFS_ChipID --use-rtt --build-id
```

### 2. Directory Operations (`ListFiles`)
Directory enumeration with file size and timestamp display.

**Key Features:**
- Recursive directory traversal
- File size and modification time display
- Filesystem usage statistics
- Clean output formatting

### 3. Comprehensive Testing (`LittleFS_Usage`)
Complete filesystem operations testing for validation and development.

**Key Features:**
- File create/read/write/delete operations
- Directory create/rename/delete operations
- File truncation and seeking
- Binary data handling
- Real-time storage usage monitoring

**Operations Demonstrated:**
```cpp
// File operations
File file = myfs.open("/config.json", FILE_WRITE);
file.print("System configuration data");
file.close();

// Directory operations
myfs.mkdir("/logs");
myfs.rename("/logs", "/data_logs");
myfs.rmdir("/data_logs");

// Advanced operations
file.truncate(1024);  // Resize file
file.seek(512);       // Position seeking
```

## Hardware Configuration

### STM32 SPI Configuration
```cpp
// Configure SPI pins for your specific board
//              MOSI  MISO  SCLK
SPIClass SPIbus(PC12, PC11, PC10);  // Example pin assignment
#define CS_PIN PD2                   // Chip select pin
```

### Basic Usage
```cpp
#include <LittleFS.h>
#include "../../../../ci_log.h"  // For unified logging

LittleFS_SPIFlash myfs;

void setup() {
  Serial.begin(115200);
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  // Initialize SPI pins
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  // Mount filesystem
  if (!myfs.begin(CS_PIN, SPIbus)) {
    CI_LOG("LittleFS mount failed!\\n");
    return;
  }

  CI_LOG("LittleFS mounted successfully\\n");
  CI_LOGF("Total: %lu bytes, Used: %lu bytes\\n",
          myfs.totalSize(), myfs.usedSize());
}
```

## API Reference

### Filesystem Operations
```cpp
// Initialization
bool begin(uint8_t cspin, SPIClass &spi = SPI);
void end();

// Storage information
uint64_t totalSize();
uint64_t usedSize();
bool getChipInfo(LFS_W25QXX_info_t &info);

// File operations
File open(const char *path, uint8_t mode = FILE_READ);
bool exists(const char *path);
bool remove(const char *path);
bool rename(const char *pathFrom, const char *pathTo);

// Directory operations
bool mkdir(const char *path);
bool rmdir(const char *path);
File openNextFile();
```

### File Operations
```cpp
// Reading/Writing
int read();
size_t write(uint8_t data);
size_t write(const char *str);
int available();

// Positioning
bool seek(uint32_t pos, int mode = SeekSet);
uint32_t position();
uint32_t size();

// Advanced operations
bool truncate(uint32_t size);
void flush();
void close();
```

## Application Examples

### Configuration Storage
```cpp
// Store system parameters
File config = myfs.open("/system_config.json", FILE_WRITE);
config.printf("{\"sampling_rate\": %d, \"enable_logging\": %s}\\n",
              sample_rate, logging_enabled ? "true" : "false");
config.close();
```

### Data Logging
```cpp
// High-frequency sensor data logging
File datalog = myfs.open("/logs/sensors_001.log", FILE_WRITE_BEGIN);
datalog.printf("%.3f,%.2f,%.2f,%.1f\\n",
               timestamp, temperature, pressure, voltage);
```

### Firmware Management
```cpp
// Over-the-air update storage
File firmware = myfs.open("/firmware/update.bin", FILE_WRITE);
// Stream firmware data from network/radio
firmware.write(firmware_chunk, chunk_size);
```

### Example Applications
- **IoT Devices**: Configuration storage, sensor data buffering, OTA updates
- **Industrial Controls**: Parameter storage, event logging, calibration data
- **Robotics**: Path planning data, sensor fusion parameters, behavior trees
- **UAV Flight Controllers**: Flight parameters, telemetry logs, mission data
- **Data Loggers**: Environmental monitoring, equipment diagnostics, usage tracking

## Testing and Validation

### Automated Testing
All examples support complete automation for CI/CD workflows:

```bash
# Environment validation + build + test execution
./scripts/aflash.sh libraries/LittleFS/examples/LittleFS_Usage \\
  --use-rtt --build-id --env-check

# Expected output includes:
# - Build traceability (Git SHA + UTC timestamp)
# - Hardware detection (Flash ID, capacity)
# - Complete filesystem operation testing
# - Deterministic completion (*STOP* wildcard)
```

### Error Handling
The library automatically routes internal debugging to your logging infrastructure:
- **Arduino IDE Mode**: Debug messages appear in Serial monitor
- **CI/HIL Mode**: Debug messages routed to J-Link RTT for automated capture
- **Production**: Can be disabled with `-DLFS_NO_ERROR` build flag

## Performance Characteristics

### SPI Flash (W25Q128 @ 16MHz)
- **Sequential Read**: ~2MB/s
- **Sequential Write**: ~200KB/s (after page program)
- **Random Access**: <1ms typical
- **Wear Leveling**: 100,000+ erase cycles per block
- **Power Consumption**: <10mA active, <1µA deep power down

### Memory Overhead
- **Base Overhead**: ~8KB (filesystem metadata)
- **File Overhead**: ~16 bytes per file entry
- **Directory Overhead**: ~8KB per directory
- **RAM Usage**: <2KB for buffers and cache

## Troubleshooting

### Common Issues

**"Corrupted dir pair" message:**
- **Normal behavior** when mounting freshly erased chip
- LittleFS detects no valid filesystem and automatically formats
- Not an error - indicates proper initialization sequence

**Mount failures:**
- Verify SPI wiring and CS pin configuration
- Check power supply (3.3V for most chips)
- Ensure adequate decoupling capacitors near flash chip
- Try lower SPI speeds for breadboard setups

**Write failures:**
- Check available space with `usedSize()` / `totalSize()`
- Verify file is properly closed after writing
- Some operations require power-loss protection time

### Debug Output
Enable verbose debugging during development:
```cpp
// In build flags:
-DLFS_YES_DEBUG    // Enable detailed operation tracing
-DLFS_YES_TRACE    // Maximum verbosity for troubleshooting
```

## Integration with SDFS

LittleFS provides identical API compatibility with the SDFS library, enabling seamless switching between SPI flash and SD card storage:

```cpp
// Identical code works with both libraries
#ifdef USE_LITTLEFS
  #include <LittleFS.h>
  LittleFS_SPIFlash storage;
#else
  #include <SDFS.h>
  SDFS_SPI storage;
#endif

void setup() {
  // Same initialization for both
  storage.begin(CS_PIN, SPIbus);
  File config = storage.open("/config.json", FILE_READ);
}
```

## Version History

- **v1.0.0** - Complete STM32 integration with ci_log.h framework
- **Examples** - All examples validated with Winbond W25Q128 hardware
- **CI/HIL** - Full automation support with deterministic testing
- **Documentation** - Comprehensive README with embedded systems focus

## Contributing

This library is part of the STM32 Arduino Core development environment focused on embedded systems applications. For issues, enhancements, or hardware compatibility reports, please use the main repository issue tracker.

## License

LittleFS library components retain their original licenses. Arduino integration and examples are provided under the MIT license for embedded systems development.