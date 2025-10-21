// Generic Storage Abstraction Test - SDFS Backend
// Comprehensive validation of Storage abstraction with SDFS SD card

#include <Storage.h>
#include <BoardStorage.h>
#include "../../targets/NUCLEO_F411RE_SDFS.h"
#include <ci_log.h>

void setup() {
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial) delay(10);
#endif

  CI_LOG("=== Generic Storage SDFS Test ===\n");
  CI_BUILD_INFO();

  // Test 1: Check board configuration
  CI_LOG("Test 1: Board Configuration\n");
  CI_LOG("Backend: ");
  auto backend = BoardConfig::storage.backend_type;
  if (backend == StorageBackend::NONE) {
    CI_LOG("NONE\n");
  } else if (backend == StorageBackend::LITTLEFS) {
    CI_LOG("LITTLEFS\n");
  } else {
    CI_LOG("SDFS\n");
  }
  CI_LOG("CS pin: ");
  CI_LOG(String(BoardConfig::storage.cs_pin).c_str());
  CI_LOG("\n");

  // Test 2: Initialize storage
  CI_LOG("Test 2: Storage Initialization\n");
  bool initResult = BoardStorage::begin(BoardConfig::storage);
  CI_LOG("Result: ");
  CI_LOG(initResult ? "SUCCESS" : "FAILED");
  CI_LOG("\n");

  if (!initResult) {
    CI_LOG("Error: ");
    const char* error = BoardStorage::getLastError();
    CI_LOG(error ? error : "Unknown");
    CI_LOG("\n");
  } else {
    // Test 3: Basic storage operations
    CI_LOG("Test 3: Basic Storage Operations\n");
    Storage& fs = BoardStorage::getStorage();

    CI_LOG("Storage name: ");
    CI_LOG(fs.name());
    CI_LOG("\n");

    CI_LOG("Total size: ");
    CI_LOG(String((unsigned long)fs.totalSize()).c_str());
    CI_LOG(" bytes\n");

    // Test 4: File operations
    CI_LOG("Test 4: File Operations\n");
    File testFile = fs.open("/test.txt", FILE_WRITE);
    if (testFile) {
      testFile.println("Generic Storage SDFS Test");
      testFile.close();
      CI_LOG("✓ File written\n");

      if (fs.exists("/test.txt")) {
        CI_LOG("✓ File exists\n");

        testFile = fs.open("/test.txt", FILE_READ);
        if (testFile) {
          String content = testFile.readString();
          testFile.close();
          CI_LOG("Content: ");
          CI_LOG(content.c_str());
        }

        if (fs.remove("/test.txt")) {
          CI_LOG("✓ File removed\n");
        }
      }
    } else {
      CI_LOG("✗ File creation failed\n");
    }
  }

  CI_LOG("\n=== SDFS Test Complete ===\n");
  CI_LOG("*STOP*\n");
}

void loop() {
  // Nothing to do
}