#pragma once

#include <Arduino.h>
#include <FS.h>
#include <SPI.h>

// Forward declare from ConfigTypes.h
enum class StorageBackend;

// Forward declarations
class LittleFS_SPIFlash;
class SDFS_SPI;

// Generic storage interface that abstracts SDFS and LittleFS
class Storage : public FS {
public:
    Storage();
    virtual ~Storage();

    // Initialize storage with backend selection and board configuration
    bool begin(StorageBackend backend, Pin csPin, uint32_t spiSpeed = 2000000, SPIClass &spiPort = SPI);

    // FS interface implementation (delegates to active backend)
    virtual File open(const char *filepath, uint8_t mode = FILE_READ) override;
    virtual bool exists(const char *filepath) override;
    virtual bool mkdir(const char *filepath) override;
    virtual bool rename(const char *oldfilepath, const char *newfilepath) override;
    virtual bool remove(const char *filepath) override;
    virtual bool rmdir(const char *filepath) override;
    virtual uint64_t usedSize() override;
    virtual uint64_t totalSize() override;
    virtual bool format() override;
    virtual bool mediaPresent() override;
    virtual const char* name() override;

    // Storage backend information
    StorageBackend getBackend() const { return currentBackend; }
    bool isInitialized() const { return initialized; }

    // Error handling
    const char* getLastError() const { return lastError; }

private:
    StorageBackend currentBackend;
    bool initialized;
    const char* lastError;

    // Backend instances (only one will be active)
    LittleFS_SPIFlash* littlefs;
    SDFS_SPI* sdfs;

    // Helper methods
    void setError(const char* error);
    void cleanup();
};

// Global storage instance
extern Storage storage;