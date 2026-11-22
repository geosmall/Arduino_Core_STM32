#ifndef SDFS_CONFIG_H
#define SDFS_CONFIG_H

// SDFS Configuration File
// This file defines configurable constants to replace hardcoded magic numbers
// Following SdFat library patterns for professional configuration management

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