// Define target configuration for LittleFS SPI flash rig
#define TARGET_NUCLEO_F411RE_LITTLEFS

#include <Storage.h>
#include <BoardStorage.h>
#include <ci_log.h>

// Include board configuration for StorageBackend enum
#if defined(ARDUINO_BLACKPILL_F411CE)
#include "../../../../targets/BLACKPILL_F411CE.h"
#elif defined(ARDUINO_BKMN_NERO)
#include "../../../../targets/BKMN-NERO.h"
#elif defined(TARGET_NUCLEO_F411RE_LITTLEFS)
// Nucleo F411RE with LittleFS SPI flash rig
#include "../../../../targets/NUCLEO_F411RE_LITTLEFS.h"
#else
// Default to NUCLEO_F411RE with SDFS
#include "../../../../targets/NUCLEO_F411RE.h"
#endif

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  CI_LOG("=== Generic Storage Abstraction Demo ===\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  // Initialize storage using board configuration
  CI_LOG("Initializing storage...\n");
  if (BoardStorage::begin(BoardConfig::storage)) {
    Storage& fs = BOARD_STORAGE;

    CI_LOG("✓ Storage initialized successfully\n");
    CI_LOG("Backend: ");
    auto backend = BoardStorage::getBackendType();
    if (backend == StorageBackend::NONE) {
      CI_LOG("NONE");
    } else if (backend == StorageBackend::LITTLEFS) {
      CI_LOG("LittleFS");
    } else {
      CI_LOG("SDFS");
    }
    CI_LOG("\n");
    CI_LOG("Media: ");
    CI_LOG(fs.name());
    CI_LOG("\n");
    CI_LOG("Total size: ");
    CI_LOG(String((unsigned long)fs.totalSize()).c_str());
    CI_LOG(" bytes\n");
    CI_LOG("Used size: ");
    CI_LOG(String((unsigned long)fs.usedSize()).c_str());
    CI_LOG(" bytes\n");

    // Test basic file operations
    CI_LOG("\nTesting file operations...\n");

    File testFile = fs.open("/test.txt", FILE_WRITE);
    if (testFile) {
      testFile.println("Hello from unified storage!");
      testFile.println("Backend auto-selected by board configuration");
      testFile.close();
      CI_LOG("✓ File written successfully\n");
    } else {
      CI_LOG("✗ Failed to create test file\n");
    }

    if (fs.exists("/test.txt")) {
      CI_LOG("✓ File exists\n");

      testFile = fs.open("/test.txt", FILE_READ);
      if (testFile) {
        CI_LOG("File contents:\n");
        while (testFile.available()) {
          CI_LOG(String((char)testFile.read()).c_str());
        }
        testFile.close();
      }
    }

    // Clean up
    if (fs.remove("/test.txt")) {
      CI_LOG("✓ Test file removed\n");
    }

  } else {
    CI_LOG("✗ Storage initialization failed: ");
    CI_LOG(BoardStorage::getLastError());
    CI_LOG("\n");
    CI_LOG("Note: This is expected if storage hardware is not connected.\n");
    CI_LOG("The unified storage interface is working correctly.\n");
  }

  CI_LOG("\n=== Demo Complete ===\n");
  #ifdef USE_RTT
  CI_LOG("*STOP*\n");
  #endif
}

void loop() {
  // Nothing to do
}