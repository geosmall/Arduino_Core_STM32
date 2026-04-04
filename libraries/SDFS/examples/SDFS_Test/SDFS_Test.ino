/*
 * SDFS Test Example
 *
 * Tests SD card filesystem operations via SPI interface.
 *
 * Hardware connections (via BoardConfig):
 * - NUCLEO_F411RE: MOSI: PC12, MISO: PC11, SCLK: PC10, CS: PD2
 * - BLACKPILL_F411CE: MOSI: PA7, MISO: PA6, SCLK: PA5, CS: PA4
 * - NERO F7 (BKMN): MOSI: PC12, MISO: PC11, SCLK: PC10, CS: PA15 (SPI3)
 */

#include <SDFS.h>
#include <libPrintf.h>

// printf_() output routing
extern "C" void putchar_(char c) {
    Serial.write(c);
}

// Board configuration for hardware abstraction
#if defined(ARDUINO_BLACKPILL_F411CE)
#include "targets/BLACKPILL_F411CE.h"
#elif defined(ARDUINO_BKMN_NERO)
#include "targets/BKMN-NERO.h"
#else
#include "targets/NUCLEO_F411RE_SDFS.h"
#endif

// Hardware configuration - BoardConfig integration
#define CS_PIN BoardConfig::storage.cs_pin
#define SPI_MOSI BoardConfig::storage.mosi_pin
#define SPI_MISO BoardConfig::storage.miso_pin
#define SPI_SCLK BoardConfig::storage.sclk_pin

// Create SDFS instance
SDFS_SPI sdfs;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  // Test header
  Serial.println("SDFS Test Example");
  Serial.println("=========================");
  Serial.println("Mode: Arduino IDE (manual)");

  // Configure SPI pins
  SPI.setMOSI(SPI_MOSI);
  SPI.setMISO(SPI_MISO);
  SPI.setSCLK(SPI_SCLK);

  // Initialize SDFS
  Serial.print("Initializing SD card...");
  if (sdfs.begin(CS_PIN)) {
    Serial.println(" SUCCESS");

    // Display card information
    printf_("Media: %s\n", sdfs.getMediaName());
    printf_("Total Size: %lu MB\n", (unsigned long)(sdfs.totalSize() / (1024 * 1024)));
    printf_("Used Size: %lu MB\n", (unsigned long)(sdfs.usedSize() / (1024 * 1024)));

    // Test file operations
    testFileOperations();

  } else {
    Serial.println(" FAILED");
    Serial.println("Check connections and card insertion");
  }

  // Test completion
  Serial.println("\nAll tests completed!");
  Serial.println("Test complete - looping with *STOP* signals");
}

void loop() {
  // Periodic signaling for HIL compatibility
  Serial.println("*STOP*");
  delay(5000);
}

void testFileOperations() {
  Serial.println("\nTesting file operations:");

  // Debug: Check if filesystem is mounted
  bool mount_result = sdfs.begin(CS_PIN);
  printf_("Filesystem mounted: %s\n", mount_result ? "YES" : "NO");

  // Test direct root directory access after mount failure
  if (!mount_result) {
    Serial.println("Mount failed - testing basic SD card operations...");
    return; // Skip file operations if mount failed
  }

  // Try to trigger more disk reads to see what FatFs is trying to access
  Serial.println("Testing exists() calls to trigger disk I/O...");
  printf_("  exists(\"/\"): %s\n", sdfs.exists("/") ? "YES" : "NO");
  printf_("  exists(\"/LAGER.CFG\"): %s\n", sdfs.exists("/LAGER.CFG") ? "YES" : "NO");
  printf_("  exists(\"/LOG000.TXT\"): %s\n", sdfs.exists("/LOG000.TXT") ? "YES" : "NO");

  // Test 1: Write a file
  Serial.print("Writing test file...");
  File testFile = sdfs.open("/TEST.TXT", FILE_WRITE_BEGIN);
  if (testFile) {
    Serial.println(" File opened successfully");
    size_t written = testFile.println("Hello from SDFS Unified Test!");
    printf_("  Wrote %u bytes\n", written);
    testFile.print("Mode: Arduino IDE\n");
    testFile.print("millis: ");
    testFile.println(millis());
    testFile.close();
    Serial.println("  File closed - Write test OK");
  } else {
    Serial.println(" FAILED - Could not open file for writing");
    printf_("  exists(\"/\"): %s\n", sdfs.exists("/") ? "YES" : "NO");
    printf_("  exists(\"/TEST.TXT\"): %s\n", sdfs.exists("/TEST.TXT") ? "YES" : "NO");
    return;
  }

  // Test 2: Read the file back
  Serial.print("Reading test file...");
  testFile = sdfs.open("/TEST.TXT", FILE_READ);
  if (testFile) {
    Serial.println(" OK");
    Serial.println("File contents:");
    while (testFile.available()) {
      char c = testFile.read();
      printf_("%c", c);
    }
    testFile.close();
  } else {
    Serial.println(" FAILED");
  }

  // Test 3: List root directory
  Serial.println("\nRoot directory listing:");
  File root = sdfs.open("/");
  if (root) {
    int fileCount = 0;
    while (true) {
      File entry = root.openNextFile();
      if (!entry) break;

      fileCount++;
      printf_("%s %s", entry.isDirectory() ? "DIR " : "FILE", entry.name());
      if (!entry.isDirectory()) {
        printf_(" (%lu bytes)", (unsigned long)entry.size());
      }
      Serial.println();
      entry.close();
    }
    printf_("Total files found: %d\n", fileCount);
    root.close();
  } else {
    Serial.println("FAILED to open root directory!");
  }
}