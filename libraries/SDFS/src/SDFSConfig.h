#ifndef SDFS_CONFIG_H
#define SDFS_CONFIG_H

// SDFS Configuration File
// This file defines configurable constants to replace hardcoded magic numbers
// Following SdFat library patterns for professional configuration management

//==============================================================================
// SPI Communication Settings
//==============================================================================

// Initial SPI speed for SD card initialization (400kHz standard)
#ifndef SDFS_SPI_INIT_SPEED_HZ
#define SDFS_SPI_INIT_SPEED_HZ 400000
#endif

// Maximum SPI speed for data transfer (4MHz safe default for most cards)
#ifndef SDFS_SPI_MAX_SPEED_HZ
#define SDFS_SPI_MAX_SPEED_HZ 4000000
#endif

// Medium speed for SDHC detection phase (2MHz)
#ifndef SDFS_SPI_MID_SPEED_HZ
#define SDFS_SPI_MID_SPEED_HZ 2000000
#endif

//==============================================================================
// Timeout Settings
//==============================================================================

// SD card initialization timeout in milliseconds
#ifndef SDFS_INIT_TIMEOUT_MS
#define SDFS_INIT_TIMEOUT_MS 1000
#endif

// Command response timeout in milliseconds
#ifndef SDFS_CMD_TIMEOUT_MS
#define SDFS_CMD_TIMEOUT_MS 500
#endif

// Data block read/write timeout in milliseconds
#ifndef SDFS_DATA_TIMEOUT_MS
#define SDFS_DATA_TIMEOUT_MS 300
#endif

// Busy wait timeout in milliseconds
#ifndef SDFS_BUSY_TIMEOUT_MS
#define SDFS_BUSY_TIMEOUT_MS 100
#endif

//==============================================================================
// Buffer and Path Settings
//==============================================================================

// SD card sector size (typically 512 bytes, but can be runtime detected)
#ifndef SDFS_SECTOR_SIZE
#define SDFS_SECTOR_SIZE 512
#endif

//==============================================================================
// Compatibility Settings (LittleFS-aligned naming)
//==============================================================================

// Maximum filename/path length (matches LittleFS naming and default)
#ifndef SDFS_NAME_MAX
#define SDFS_NAME_MAX 255
#endif

// Maximum open files simultaneously (matches LittleFS pattern)
#ifndef SDFS_MAX_OPEN_FILES
#define SDFS_MAX_OPEN_FILES 2
#endif

// Maximum file size for cross-compatibility (matches LittleFS limit)
#ifndef SDFS_FILE_MAX
#define SDFS_FILE_MAX 2147483647
#endif

#endif // SDFS_CONFIG_H