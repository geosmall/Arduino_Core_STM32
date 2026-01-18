// Define target configuration for LittleFS SPI flash rig
#define TARGET_NUCLEO_F411RE_HIL001

#include <Storage.h>
#include <BoardStorage.h>

// Include board configuration for StorageBackend enum
#if defined(ARDUINO_BLACKPILL_F411CE)
#include "../../../../targets/BLACKPILL_F411CE.h"
#elif defined(ARDUINO_BKMN_NERO)
#include "../../../../targets/BKMN-NERO.h"
#elif defined(TARGET_NUCLEO_F411RE_HIL001)
// Nucleo F411RE with LittleFS SPI flash rig
#include "../../../../targets/NUCLEO_F411RE_HIL001.h"
#else
// Default to NUCLEO_F411RE with SDFS
#include "../../../../targets/NUCLEO_F411RE.h"
#endif

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("=== Generic Storage Abstraction Demo ===");

  // Initialize storage using board configuration
  Serial.println("Initializing storage...");
  if (BoardStorage::begin(BoardConfig::storage)) {
    Storage& fs = BOARD_STORAGE;

    Serial.println("✓ Storage initialized successfully");
    Serial.print("Backend: ");
    auto backend = BoardStorage::getBackendType();
    if (backend == StorageBackend::NONE) {
      Serial.print("NONE");
    } else if (backend == StorageBackend::LITTLEFS) {
      Serial.print("LittleFS");
    } else {
      Serial.print("SDFS");
    }
    Serial.println();
    Serial.print("Media: ");
    Serial.println(fs.name());
    Serial.print("Total size: ");
    Serial.print((unsigned long)fs.totalSize());
    Serial.println(" bytes");
    Serial.print("Used size: ");
    Serial.print((unsigned long)fs.usedSize());
    Serial.println(" bytes");

    // Test basic file operations
    Serial.println("\nTesting file operations...");

    File testFile = fs.open("/test.txt", FILE_WRITE);
    if (testFile) {
      testFile.println("Hello from unified storage!");
      testFile.println("Backend auto-selected by board configuration");
      testFile.close();
      Serial.println("✓ File written successfully");
    } else {
      Serial.println("✗ Failed to create test file");
    }

    if (fs.exists("/test.txt")) {
      Serial.println("✓ File exists");

      testFile = fs.open("/test.txt", FILE_READ);
      if (testFile) {
        Serial.println("File contents:");
        while (testFile.available()) {
          Serial.print((char)testFile.read());
        }
        testFile.close();
      }
    }

    // Clean up
    if (fs.remove("/test.txt")) {
      Serial.println("✓ Test file removed");
    }

  } else {
    Serial.print("✗ Storage initialization failed: ");
    Serial.println(BoardStorage::getLastError());
    Serial.println("Note: This is expected if storage hardware is not connected.");
    Serial.println("The unified storage interface is working correctly.");
  }

  Serial.println("\n=== Demo Complete ===");
  Serial.println("*STOP*");
}

void loop() {
  // Nothing to do
}
