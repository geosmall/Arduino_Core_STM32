#pragma once

#include "Storage.h"

// Forward declare to avoid circular dependency
namespace BoardConfig { struct StorageConfig; }

// Board-specific storage configuration and automatic initialization
namespace BoardStorage {
    // Initialize storage using provided board configuration
    bool begin(const BoardConfig::StorageConfig& config);

    // Initialize storage using board configuration (requires board config include in sketch)
    bool begin();

    // Get the configured storage instance
    Storage& getStorage();

    // Check if storage is initialized
    bool isInitialized();

    // Get storage backend type for current board
    StorageBackend getBackendType();

    // Get error message if initialization failed
    const char* getLastError();
}

// Convenience macro for board-configured storage access
#define BOARD_STORAGE BoardStorage::getStorage()