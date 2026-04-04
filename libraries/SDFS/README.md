# SDFS - SD Card File System Library

SD card filesystem library for STM32 Arduino Core with Arduino FS.h interface compatibility and configurable architecture.

## Features

- **FS.h Compatible**: API compatible with other Arduino filesystem libraries
- **FatFS Backend**: Built on FatFS R0.15 library for reliable file operations
- **Configuration System**: Configurable timeouts, speeds, and buffer sizes via SDFSConfig.h
- **SPI Interface**: SPI communication with configurable speed management
- **Runtime Detection**: Automatic card capacity and sector size detection
- **Card Type Support**: Automatic detection and handling of SD, SDHC, and SDXC cards
- **STM32 Optimized**: Designed for STM32 microcontrollers with Arduino Core
- **Flight Controller Ready**: Clean codebase suitable for flight controllers

## Hardware Requirements

- STM32 microcontroller with Arduino Core support
- SD card reader with SPI interface
- Connections: MOSI, MISO, SCLK, CS

## Installation

1. Copy the `SDFS` folder to your Arduino libraries directory
2. Restart Arduino IDE
3. Include the library: `#include <SDFS.h>`

## Basic Usage

```cpp
#include <SDFS.h>

SDFS_SPI sdfs;

void setup() {
  Serial.begin(115200);

  // Configure SPI pins (for Nucleo F411RE)
  SPI.setMOSI(PC12);
  SPI.setMISO(PC11);
  SPI.setSCLK(PC10);

  // Initialize SD card
  if (sdfs.begin(PD2)) {  // CS pin
    Serial.println("SD card initialized");

    // Write a file
    File file = sdfs.open("/test.txt", FILE_WRITE_BEGIN);
    if (file) {
      file.println("Hello SDFS!");
      file.close();
    }

    // Read the file
    file = sdfs.open("/test.txt", FILE_READ);
    if (file) {
      while (file.available()) {
        Serial.write(file.read());
      }
      file.close();
    }
  }
}

void loop() {
  // Your application code
}
```

## Configuration System

SDFS uses a comprehensive configuration system via `SDFSConfig.h` that allows customization without modifying source code:

### Basic Configuration

```cpp
// Create libraries/SDFS/src/SDFSConfig.h (optional - defaults work for most cases)
#ifndef SDFS_CONFIG_H
#define SDFS_CONFIG_H

// SPI Speed Configuration
#define SDFS_SPI_INIT_SPEED_HZ    400000   // 400kHz for initialization
#define SDFS_SPI_MAX_SPEED_HZ     8000000  // 8MHz for high-speed operation

// Timeout Configuration
#define SDFS_INIT_TIMEOUT_MS      2000     // Extended init timeout
#define SDFS_CMD_TIMEOUT_MS       1000     // Extended command timeout

// Buffer Configuration
#define SDFS_NAME_MAX             512      // Longer filename/path support
#define SDFS_MAX_OPEN_FILES       4        // More simultaneous open files

#endif
```

### Available Configuration Options

| Configuration | Default | Purpose |
|---------------|---------|---------|
| `SDFS_SPI_INIT_SPEED_HZ` | 400000 | SPI speed during initialization |
| `SDFS_SPI_MID_SPEED_HZ` | 2000000 | SPI speed during detection |
| `SDFS_SPI_MAX_SPEED_HZ` | 4000000 | Maximum SPI speed for operation |
| `SDFS_INIT_TIMEOUT_MS` | 1000 | SD card initialization timeout |
| `SDFS_CMD_TIMEOUT_MS` | 500 | Command response timeout |
| `SDFS_DATA_TIMEOUT_MS` | 300 | Data block timeout |
| `SDFS_BUSY_TIMEOUT_MS` | 100 | Busy wait timeout |
| `SDFS_NAME_MAX` | 255 | Maximum filename/path length (matches LittleFS) |
| `SDFS_MAX_OPEN_FILES` | 2 | Maximum simultaneous open files (matches LittleFS) |
| `SDFS_FILE_MAX` | 2147483647 | Maximum file size for compatibility (matches LittleFS) |
| `SDFS_SECTOR_SIZE` | 512 | Default sector size (runtime detected) |

## Pin Configurations

### Nucleo F411RE
- MOSI: PC12
- MISO: PC11
- SCLK: PC10
- CS: PD2 (configurable)

### BlackPill F411CE
- MOSI: PA7
- MISO: PA6
- SCLK: PA5
- CS: PA4 (configurable)

## API Reference

### SDFS_SPI Class

#### Initialization
- `bool begin(uint8_t csPin)` - Initialize SD card with specified CS pin
- `void end()` - Cleanup and unmount filesystem

#### File Operations (FS.h interface)
- `File open(const char* path, uint8_t mode)` - Open file (FILE_READ, FILE_WRITE, FILE_WRITE_BEGIN)
- `bool exists(const char* path)` - Check if file/directory exists
- `bool remove(const char* path)` - Delete file
- `bool mkdir(const char* path)` - Create directory
- `bool rmdir(const char* path)` - Remove directory
- `bool rename(const char* oldPath, const char* newPath)` - Rename file/directory

#### Information
- `uint64_t totalSize()` - Get total card capacity
- `uint64_t usedSize()` - Get used space
- `bool mediaPresent()` - Check if card is present
- `const char* getMediaName()` - Get media type string

#### Advanced Configuration
- `uint16_t getSectorSize()` - Get runtime detected sector size
- `bool setSectorSize(uint16_t size)` - Set custom sector size (512-4096, power of 2)

## Performance

### Default Performance Profile
- **Initialization**: 400kHz (SD specification compliant)
- **Detection Phase**: 2MHz (safe for card type detection)
- **Normal Operation**: 4MHz (SDHC optimized)
- **Automatic Negotiation**: Based on card capabilities and configuration

### High-Performance Configuration
For systems with short traces and quality SD cards:

```cpp
// In your SDFSConfig.h
#define SDFS_SPI_MAX_SPEED_HZ     8000000  // 8MHz maximum
#define SDFS_CMD_TIMEOUT_MS       200      // Faster timeouts
#define SDFS_DATA_TIMEOUT_MS      150      // Faster data timeout
```

### Conservative Configuration
For development, breadboard, or long wire connections:

```cpp
// In your SDFSConfig.h
#define SDFS_SPI_MAX_SPEED_HZ     1000000  // 1MHz maximum
#define SDFS_INIT_TIMEOUT_MS      5000     // Extended timeouts
#define SDFS_CMD_TIMEOUT_MS       1000     // Extended command timeout
```

## Compatibility

### Tested Hardware
- **STM32F411RE** (Nucleo F411RE) - Primary development platform
- **STM32F411CE** (BlackPill F411CE) - Secondary development platform

### Tested SD Cards
- **SD Cards**: 2GB and below (FAT16)
- **SDHC Cards**: 4GB to 32GB (FAT32) - Primary target
- **SDXC Cards**: Basic support (requires exFAT for >32GB)

### Filesystem Support
- **FAT16**: Full support for SD cards ≤2GB
- **FAT32**: Full support for SDHC cards 4GB-32GB
- **exFAT**: Limited support (depends on FatFS configuration)

## Flight Controller Integration

SDFS is specifically designed for UAV flight controller applications:

### Unified Storage API
```cpp
// Seamlessly switch between storage types
#ifdef USE_SPI_FLASH
  #include <LittleFS.h>
  LittleFS storage;
#else
  #include <SDFS.h>
  SDFS_SPI storage;
#endif

// Same API for both storage types
File logFile = storage.open("/flight.log", FILE_WRITE);
```

### Real-time Data Logging
```cpp
// High-frequency sensor data logging
void logSensorData() {
  File dataLog = sdfs.open("/sensors.csv", FILE_WRITE);
  dataLog.printf("%lu,%.2f,%.2f,%.2f\n",
                 millis(), gyroX, gyroY, gyroZ);
  dataLog.close(); // Ensures data is written to card
}
```

### Configuration Management
```cpp
// Flight controller parameter storage
void saveFlightConfig() {
  File config = sdfs.open("/config.json", FILE_WRITE_BEGIN);
  config.println("{");
  config.printf("  \"pid_p\": %.3f,\n", pid_p);
  config.printf("  \"pid_i\": %.3f,\n", pid_i);
  config.printf("  \"pid_d\": %.3f\n", pid_d);
  config.println("}");
  config.close();
}
```

## Architecture

SDFS follows established embedded library design patterns:

### Configuration-Driven Design
- All timeouts, speeds, and buffer sizes are configurable
- Runtime detection with configurable fallbacks
- Platform-adaptive defaults

### Memory Management
- Dynamic allocation for file handles
- Configurable buffer sizes
- Automatic cleanup and resource management

### Error Handling
- Comprehensive error checking at all layers
- Configurable retry logic
- Graceful degradation for edge cases

## Troubleshooting

### Common Issues

1. **Initialization Fails**
   - Check SPI wiring and connections
   - Verify CS pin configuration
   - Try lower SPI speeds in SDFSConfig.h
   - Ensure adequate power supply (SD cards need clean 3.3V)

2. **Slow Performance**
   - Increase `SDFS_SPI_MAX_SPEED_HZ` in configuration
   - Use high-quality, fast SD cards (Class 10 or better)
   - Minimize SPI trace lengths

3. **File Operations Fail**
   - Check available space with `usedSize()` and `totalSize()`
   - Ensure proper file closure with `file.close()`
   - Verify filesystem isn't corrupted

4. **Compatibility Issues**
   - Try different SD card brands/speeds
   - Reduce SPI speed for problematic cards
   - Check card format (prefer FAT32 for SDHC)

### Debug Configuration

SDFS v1.0.0 uses clean production code without debug options. For debugging, use the unified development framework with `ci_log.h` and RTT integration at the sketch level.

## Version History

| Version | Date | Changes |
|---------|------|---------|
| v1.0.0 | 2025-09-13 | Release with comprehensive configuration system |
| v0.9.0 | 2024-09-09 | Initial SDHC optimization and directory enumeration fixes |
| v0.8.0 | 2024-08-21 | Initial implementation from STM32 FatFs v4.0.0 |

## License

This library is released under the same license as the STM32 Arduino Core project.

## Contributing

SDFS is part of a larger embedded development framework. For issues, improvements, or contributions, please follow the project's development workflow and testing procedures.