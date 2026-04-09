#include "BoardStorage.h"
#include "targets/config/ConfigTypes.h"

// No board configuration auto-detection - configuration is passed from sketch

namespace BoardStorage {
    static Storage boardStorage;
    static bool initialized = false;
    static const char* lastError = nullptr;
    static StorageBackend currentBackend = StorageBackend::NONE; // Default
    static SPIClass* customSPI = nullptr;

    bool begin(const BoardConfig::StorageConfig& config) {
        // Handle NONE backend immediately (regardless of initialization state)
        if (config.backend_type == StorageBackend::NONE) {
            // Reset state for NONE backend
            initialized = false;
            currentBackend = StorageBackend::NONE;
            lastError = "No storage hardware configured";
            return false;
        }

        if (initialized) {
            return true; // Already initialized
        }

        // Store the backend type
        currentBackend = config.backend_type;

        // Create custom SPI instance with specific pins (like working LittleFS examples)
        if (customSPI) {
            delete customSPI;
        }
        customSPI = new SPIClass(config.mosi_pin, config.miso_pin, config.sclk_pin);
        customSPI->begin();

        // Initialize CS pin (like working LittleFS examples)
        pinMode(config.cs_pin, OUTPUT);
        digitalWrite(config.cs_pin, HIGH);
        delay(10); // Wait a bit to make sure flash chip is ready

        // Initialize storage with board-specific configuration
        bool success = boardStorage.begin(
            config.backend_type,
            config.cs_pin,
            config.freq_hz,
            *customSPI
        );

        if (success) {
            initialized = true;
            lastError = nullptr;
        } else {
            lastError = boardStorage.getLastError();
            if (!lastError) {
                lastError = "Unknown storage initialization error";
            }
        }

        return success;
    }

    bool begin() {
        // This version requires explicit configuration passing
        // due to library compilation limitations with preprocessor defines
        lastError = "Use BoardStorage::begin(config) with explicit configuration";
        return false;
    }

    Storage& getStorage() {
        return boardStorage;
    }

    bool isInitialized() {
        return initialized;
    }

    StorageBackend getBackendType() {
        return currentBackend;
    }

    const char* getLastError() {
        return lastError;
    }

    // Cleanup function (not exposed in header for simplicity)
    void cleanup() {
        if (customSPI) {
            delete customSPI;
            customSPI = nullptr;
        }
        initialized = false;
        lastError = nullptr;
        currentBackend = StorageBackend::NONE;
    }
}