# Storage - Unified Storage Abstraction for STM32

Unified storage interface that abstracts LittleFS (SPI flash) and SDFS (SD card) backends with automatic board configuration integration.

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                      Application Layer                          │
├─────────────────────────────────────────────────────────────────┤
│  minIniStorage          User Sketches         Other Libraries   │
│  (INI config)           (file I/O)            (data logging)    │
└────────┬────────────────────┬─────────────────────┬─────────────┘
         │                    │                     │
         ▼                    ▼                     ▼
┌─────────────────────────────────────────────────────────────────┐
│                     BoardStorage (namespace)                     │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ • begin(BoardConfig::StorageConfig)                         ││
│  │ • getStorage() → Storage&                                   ││
│  │ • BOARD_STORAGE macro                                       ││
│  └─────────────────────────────────────────────────────────────┘│
│         Reads BoardConfig to auto-select backend                 │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                      Storage (class)                             │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │ Unified FS interface:                                       ││
│  │ • open(), exists(), remove(), mkdir(), rmdir(), rename()    ││
│  │ • totalSize(), usedSize(), format(), mediaPresent()         ││
│  │                                                             ││
│  │ Extends Arduino FS base class                               ││
│  └─────────────────────────────────────────────────────────────┘│
└───────────────┬─────────────────────────────┬───────────────────┘
                │                             │
                ▼                             ▼
┌───────────────────────────────┐ ┌───────────────────────────────┐
│         LittleFS              │ │           SDFS                │
│  ┌─────────────────────────┐  │ │  ┌─────────────────────────┐  │
│  │ • SPI flash storage     │  │ │  │ • SD card via SPI       │  │
│  │ • Wear leveling         │  │ │  │ • FatFs backend         │  │
│  │ • Power-safe writes     │  │ │  │ • FAT16/FAT32 support   │  │
│  │ • W25Qxx, AT25SF, etc.  │  │ │  │ • Runtime card detect   │  │
│  └─────────────────────────┘  │ │  └─────────────────────────┘  │
└───────────────────────────────┘ └───────────────────────────────┘
                │                             │
                ▼                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                        SPI Hardware                              │
│                   (STM32 HAL SPI Driver)                         │
└─────────────────────────────────────────────────────────────────┘
```

## Quick Start

### Using BoardConfig (Recommended)

```cpp
#include <Storage.h>
#include <BoardStorage.h>
#include "targets/NUCLEO_F411RE_HIL001.h"  // or your board config

void setup() {
    // Initialize with board configuration
    if (BoardStorage::begin(BoardConfig::storage)) {
        Storage& fs = BOARD_STORAGE;

        // Write a file
        File f = fs.open("/data.txt", FILE_WRITE);
        f.println("Hello, Storage!");
        f.close();

        // Read it back
        f = fs.open("/data.txt", FILE_READ);
        while (f.available()) {
            Serial.write(f.read());
        }
        f.close();
    }
}
```

### Direct Initialization

```cpp
#include <Storage.h>

Storage myStorage;

void setup() {
    // Initialize with explicit parameters
    if (myStorage.begin(StorageBackend::LITTLEFS, PA4, 2000000, SPI)) {
        File f = myStorage.open("/test.txt", FILE_WRITE);
        // ...
    }
}
```

## API Reference

### BoardStorage Namespace

| Function | Description |
|----------|-------------|
| `begin(config)` | Initialize with BoardConfig::StorageConfig |
| `begin()` | Initialize using default board configuration |
| `getStorage()` | Get reference to Storage instance |
| `isInitialized()` | Check if storage is ready |
| `getBackendType()` | Get current backend (LITTLEFS, SDFS, NONE) |
| `getLastError()` | Get error message if init failed |

**Convenience Macro:**
```cpp
#define BOARD_STORAGE BoardStorage::getStorage()
```

### Storage Class

Storage inherits from the Arduino `FS` base class (defined in `cores/arduino/FS.h`), implementing its pure virtual methods and adding storage-specific functionality.

**Inherited from FS** (standard Arduino filesystem interface):

| Method | Description |
|--------|-------------|
| `open(path, mode)` | Open file (FILE_READ, FILE_WRITE) |
| `exists(path)` | Check if file/directory exists |
| `remove(path)` | Delete a file |
| `rename(oldPath, newPath)` | Rename/move a file |
| `mkdir(path)` | Create directory |
| `rmdir(path)` | Remove directory |
| `totalSize()` | Total storage capacity (bytes) |
| `usedSize()` | Used storage space (bytes) |
| `format()` | Format the storage medium |
| `mediaPresent()` | Check if media is inserted (SD cards) |
| `name()` | Get storage backend name string |

**Added by Storage** (not part of FS base class):

| Method | Description |
|--------|-------------|
| `begin(backend, csPin, spiSpeed, spiPort)` | Initialize storage with backend selection |
| `getBackend()` | Get current StorageBackend enum value |
| `isInitialized()` | Check if storage was successfully initialized |
| `getLastError()` | Get last error message string |

## BoardConfig Integration

Storage integrates with the BoardConfig system for automatic hardware configuration:

```cpp
// In targets/NUCLEO_F411RE_HIL001.h
namespace BoardConfig {
    constexpr StorageConfig storage = {
        .backend = StorageBackend::LITTLEFS,
        .cs_pin = PA4,
        .spi_speed = 2000000,
        .mosi_pin = PA7,
        .miso_pin = PA6,
        .sck_pin = PA5
    };
}
```

The backend is selected at compile time based on your target header:
- `NUCLEO_F411RE_HIL001.h` → LittleFS on SPI flash
- `NUCLEO_F411RE_SDFS.h` → SDFS on SD card
- `BKMN_NERO.h` → Board-specific storage configuration

## Dependencies

This library depends on:
- **LittleFS** - SPI flash filesystem
- **SDFS** - SD card filesystem with FatFs

Both are included in the STM32 Robotics core.

## Integration with minIniStorage

The Storage library provides the backend for minIniStorage INI configuration:

```cpp
#include <minIniStorage.h>
#include "targets/NUCLEO_F411RE_HIL001.h"

minIniStorage config("settings.ini");

void setup() {
    config.begin(BoardConfig::storage);

    // Read/write configuration
    int value = config.geti("section", "key", 0);
    config.put("section", "key", 42);
}
```

See `StorageGlue.h` in minIniStorage for the glue layer implementation.

## Example

See `examples/Storage_Demo/` for a complete demonstration of:
- Automatic backend selection
- File write/read operations
- Storage information queries
- Error handling
