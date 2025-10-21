/*  minIniStorage - INI Configuration Management with Generic Storage Abstraction
 *
 *  Provides unified configuration management supporting both:
 *  - LittleFS (SPI flash storage for configuration, firmware)
 *  - SDFS (SD card storage for data logging, bulk storage)
 *
 *  Storage backend automatically selected based on BoardConfig.
 *  Maintains minIni's clean C++ std::string-based API.
 *
 *  Usage:
 *    #include <minIniStorage.h>
 *    #include "targets/NUCLEO_F411RE_LITTLEFS.h"  // or SDFS variant
 *
 *    minIniStorage config("settings.ini");
 *    std::string ip = config.gets("network", "ip_address", "192.168.1.1");
 *    config.put("user", "last_login", "2025-09-22");
 */

#pragma once

#include <Arduino.h>
#include <Storage.h>
#include <BoardStorage.h>
#include "minIni.h"

class minIniStorage {
private:
    std::string filename;
    bool storage_initialized;

public:
    // Constructor takes INI filename
    minIniStorage(const std::string& iniFilename)
        : filename(iniFilename), storage_initialized(false) {
    }

    // Initialize storage with board configuration
    // This must be called before any INI operations
    bool begin(const BoardConfig::StorageConfig& config) {
        // Check if already initialized
        if (BoardStorage::isInitialized()) {
            storage_initialized = true;
            return true;
        }

        if (BoardStorage::begin(config)) {
            storage_initialized = true;
            return true;
        }
        return false;
    }

    // Check if storage is ready
    bool ready() const {
        return storage_initialized;
    }

    // Get storage info
    uint64_t totalSize() const {
        if (!storage_initialized) return 0;
        return BOARD_STORAGE.totalSize();
    }

    uint64_t usedSize() const {
        if (!storage_initialized) return 0;
        return BOARD_STORAGE.usedSize();
    }

    // minIni wrapper methods - delegate to internal minIni instance
    bool getbool(const std::string& Section, const std::string& Key, bool DefValue = false) const {
        if (!storage_initialized) return DefValue;
        minIni ini(filename);
        return ini.getbool(Section, Key, DefValue);
    }

    long getl(const std::string& Section, const std::string& Key, long DefValue = 0) const {
        if (!storage_initialized) return DefValue;
        minIni ini(filename);
        return ini.getl(Section, Key, DefValue);
    }

    int geti(const std::string& Section, const std::string& Key, int DefValue = 0) const {
        if (!storage_initialized) return DefValue;
        minIni ini(filename);
        return ini.geti(Section, Key, DefValue);
    }

    std::string gets(const std::string& Section, const std::string& Key, const std::string& DefValue = "") const {
        if (!storage_initialized) return DefValue;
        minIni ini(filename);
        return ini.gets(Section, Key, DefValue);
    }

    float getf(const std::string& Section, const std::string& Key, float DefValue = 0.0) const {
        if (!storage_initialized) return DefValue;
        minIni ini(filename);
        return ini.getf(Section, Key, DefValue);
    }

    std::string getsection(int idx) const {
        if (!storage_initialized) return "";
        minIni ini(filename);
        return ini.getsection(idx);
    }

    std::string getkey(const std::string& Section, int idx) const {
        if (!storage_initialized) return "";
        minIni ini(filename);
        return ini.getkey(Section, idx);
    }

    // minIni v1.5 new functionality
    bool hassection(const std::string& Section) const {
        if (!storage_initialized) return false;
        minIni ini(filename);
        return ini.hassection(Section);
    }

    bool haskey(const std::string& Section, const std::string& Key) const {
        if (!storage_initialized) return false;
        minIni ini(filename);
        return ini.haskey(Section, Key);
    }

    // Write methods
    bool put(const std::string& Section, const std::string& Key, long Value) {
        if (!storage_initialized) return false;
        minIni ini(filename);
        return ini.put(Section, Key, Value);
    }

    bool put(const std::string& Section, const std::string& Key, int Value) {
        if (!storage_initialized) return false;
        minIni ini(filename);
        return ini.put(Section, Key, Value);
    }

    bool put(const std::string& Section, const std::string& Key, bool Value) {
        if (!storage_initialized) return false;
        minIni ini(filename);
        return ini.put(Section, Key, Value);
    }

    bool put(const std::string& Section, const std::string& Key, const std::string& Value) {
        if (!storage_initialized) return false;
        minIni ini(filename);
        return ini.put(Section, Key, Value);
    }

    bool put(const std::string& Section, const std::string& Key, const char* Value) {
        if (!storage_initialized) return false;
        minIni ini(filename);
        return ini.put(Section, Key, Value);
    }

    bool put(const std::string& Section, const std::string& Key, float Value) {
        if (!storage_initialized) return false;
        minIni ini(filename);
        return ini.put(Section, Key, Value);
    }

    // Delete methods
    bool del(const std::string& Section, const std::string& Key) {
        if (!storage_initialized) return false;
        minIni ini(filename);
        return ini.del(Section, Key);
    }

    bool del(const std::string& Section) {
        if (!storage_initialized) return false;
        minIni ini(filename);
        return ini.del(Section);
    }
};