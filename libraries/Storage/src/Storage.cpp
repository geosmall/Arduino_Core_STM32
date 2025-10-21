#include "Storage.h"
#include <LittleFS.h>
#include <SDFS.h>

// Include board configuration for StorageBackend enum
#if defined(ARDUINO_BLACKPILL_F411CE)
#include "../../../targets/BLACKPILL_F411CE.h"
#elif defined(TARGET_NUCLEO_F411RE_LITTLEFS)
// Nucleo F411RE with LittleFS SPI flash rig
#include "../../../targets/NUCLEO_F411RE_LITTLEFS.h"
#else
// Default to NUCLEO_F411RE with SDFS
#include "../../../targets/NUCLEO_F411RE.h"
#endif

// Global storage instance
Storage storage;

Storage::Storage()
    : currentBackend(StorageBackend::LITTLEFS)
    , initialized(false)
    , lastError(nullptr)
    , littlefs(nullptr)
    , sdfs(nullptr)
{
}

Storage::~Storage() {
    cleanup();
}

bool Storage::begin(StorageBackend backend, uint8_t csPin, uint32_t spiSpeed, SPIClass &spiPort) {
    if (initialized) {
        cleanup();
    }

    currentBackend = backend;
    bool success = false;


    if (backend == StorageBackend::NONE) {
        setError("No storage hardware configured");
        success = false;
    } else if (backend == StorageBackend::LITTLEFS) {
        littlefs = new LittleFS_SPIFlash();
        if (littlefs) {
            success = littlefs->begin(csPin, spiPort);
            if (!success) {
                setError("LittleFS initialization failed");
                delete littlefs;
                littlefs = nullptr;
            }
        } else {
            setError("Failed to allocate LittleFS instance");
        }
    } else if (backend == StorageBackend::SDFS) {
        sdfs = new SDFS_SPI();
        if (sdfs) {
            success = sdfs->begin(csPin, spiPort);
            if (!success) {
                setError("SDFS initialization failed");
                delete sdfs;
                sdfs = nullptr;
            }
        } else {
            setError("Failed to allocate SDFS instance");
        }
    } else {
        setError("Invalid storage backend");
    }

    if (success) {
        initialized = true;
        lastError = nullptr;
    }

    return success;
}

File Storage::open(const char *filepath, uint8_t mode) {
    if (!initialized) {
        setError("Storage not initialized");
        return File();
    }

    if (currentBackend == StorageBackend::LITTLEFS && littlefs) {
        return littlefs->open(filepath, mode);
    } else if (currentBackend == StorageBackend::SDFS && sdfs) {
        return sdfs->open(filepath, mode);
    }

    setError("No active storage backend");
    return File();
}

bool Storage::exists(const char *filepath) {
    if (!initialized) return false;

    if (currentBackend == StorageBackend::LITTLEFS && littlefs) {
        return littlefs->exists(filepath);
    } else if (currentBackend == StorageBackend::SDFS && sdfs) {
        return sdfs->exists(filepath);
    }

    return false;
}

bool Storage::mkdir(const char *filepath) {
    if (!initialized) return false;

    if (currentBackend == StorageBackend::LITTLEFS && littlefs) {
        return littlefs->mkdir(filepath);
    } else if (currentBackend == StorageBackend::SDFS && sdfs) {
        return sdfs->mkdir(filepath);
    }

    return false;
}

bool Storage::rename(const char *oldfilepath, const char *newfilepath) {
    if (!initialized) return false;

    if (currentBackend == StorageBackend::LITTLEFS && littlefs) {
        return littlefs->rename(oldfilepath, newfilepath);
    } else if (currentBackend == StorageBackend::SDFS && sdfs) {
        return sdfs->rename(oldfilepath, newfilepath);
    }

    return false;
}

bool Storage::remove(const char *filepath) {
    if (!initialized) return false;

    if (currentBackend == StorageBackend::LITTLEFS && littlefs) {
        return littlefs->remove(filepath);
    } else if (currentBackend == StorageBackend::SDFS && sdfs) {
        return sdfs->remove(filepath);
    }

    return false;
}

bool Storage::rmdir(const char *filepath) {
    if (!initialized) return false;

    if (currentBackend == StorageBackend::LITTLEFS && littlefs) {
        return littlefs->rmdir(filepath);
    } else if (currentBackend == StorageBackend::SDFS && sdfs) {
        return sdfs->rmdir(filepath);
    }

    return false;
}

uint64_t Storage::usedSize() {
    if (!initialized) return 0;

    if (currentBackend == StorageBackend::LITTLEFS && littlefs) {
        return littlefs->usedSize();
    } else if (currentBackend == StorageBackend::SDFS && sdfs) {
        return sdfs->usedSize();
    }

    return 0;
}

uint64_t Storage::totalSize() {
    if (!initialized) return 0;

    if (currentBackend == StorageBackend::LITTLEFS && littlefs) {
        return littlefs->totalSize();
    } else if (currentBackend == StorageBackend::SDFS && sdfs) {
        return sdfs->totalSize();
    }

    return 0;
}

bool Storage::format() {
    if (!initialized) return false;

    if (currentBackend == StorageBackend::LITTLEFS && littlefs) {
        return littlefs->format();
    } else if (currentBackend == StorageBackend::SDFS && sdfs) {
        return sdfs->format();
    }

    return false;
}

bool Storage::mediaPresent() {
    if (!initialized) return false;

    if (currentBackend == StorageBackend::LITTLEFS && littlefs) {
        return littlefs->mediaPresent();
    } else if (currentBackend == StorageBackend::SDFS && sdfs) {
        return sdfs->mediaPresent();
    }

    return false;
}

const char* Storage::name() {
    if (!initialized) return "Storage";

    if (currentBackend == StorageBackend::LITTLEFS && littlefs) {
        return littlefs->name();
    } else if (currentBackend == StorageBackend::SDFS && sdfs) {
        return sdfs->name();
    }

    return "Storage";
}

void Storage::setError(const char* error) {
    lastError = error;
}

void Storage::cleanup() {
    if (littlefs) {
        delete littlefs;
        littlefs = nullptr;
    }
    if (sdfs) {
        delete sdfs;
        sdfs = nullptr;
    }
    initialized = false;
    lastError = nullptr;
}