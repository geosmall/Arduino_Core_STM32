/*
 * SDFS Test Example
 * 
 * Single sketch supporting both Arduino IDE (Serial) and J-Run/RTT modes.
 * Controlled via USE_RTT compile flag for deterministic HIL testing.
 * 
 * Arduino IDE mode: Serial output with manual monitoring
 * J-Run/RTT mode:   RTT output with deterministic exit tokens
 * 
 * Hardware connections:
 * - MOSI: PC12 (or PA7 for BlackPill)  
 * - MISO: PC11 (or PA6 for BlackPill)
 * - SCLK: PC10 (or PA5 for BlackPill)  
 * - CS:   PD2  (or PA4 for BlackPill)
 */

#include <SDFS.h>
#include <ci_log.h>

// Pin definitions based on board type
#if defined(ARDUINO_BLACKPILL_F411CE)
#define CS_PIN PA4
#define SPI_MOSI PA7
#define SPI_MISO PA6
#define SPI_SCLK PA5
#else
#define CS_PIN PD2
#define SPI_MOSI PC12
#define SPI_MISO PC11
#define SPI_SCLK PC10
#endif

// Create SDFS instance
SDFS_SPI sdfs;

void setup() {
#ifndef USE_RTT
  // Arduino IDE mode: Initialize Serial and wait
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
#else
  // RTT mode: Initialize RTT
  SEGGER_RTT_Init();
#endif
  
  // Test header
  CI_LOG("SDFS Test Example\n");
  CI_LOG("=========================\n");
  
#ifdef USE_RTT
  // J-Run mode: Enhanced header with build traceability
  CI_LOG("Mode: J-Run/RTT (deterministic)\n");
  CI_LOG("Target: NUCLEO_F411RE\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();
#else
  // Arduino IDE mode: Manual monitoring
  CI_LOG("Mode: Arduino IDE (manual)\n");
#endif
  
  // Configure SPI pins
  SPI.setMOSI(SPI_MOSI);
  SPI.setMISO(SPI_MISO);
  SPI.setSCLK(SPI_SCLK);
  
  // Initialize SDFS
  CI_LOG("Initializing SD card...");
  if (sdfs.begin(CS_PIN)) {
    CI_LOG(" SUCCESS\n");
    
    // Display card information
    CI_LOGF("Media: %s\n", sdfs.getMediaName());
    CI_LOGF("Total Size: %lu MB\n", sdfs.totalSize() / (1024 * 1024));
    CI_LOGF("Used Size: %lu MB\n", sdfs.usedSize() / (1024 * 1024));
    
    // Test file operations
    testFileOperations();
    
  } else {
    CI_LOG(" FAILED\n");
    CI_LOG("Check connections and card insertion\n");
  }
  
  // Test completion
  CI_LOG("\nAll tests completed!\n");
  
#ifdef USE_RTT
  // J-Run mode: Deterministic exit token
  CI_LOG("*STOP*\n");
#else
  // Arduino IDE mode: Continuous signaling
  CI_LOG("Test complete - looping with *STOP* signals\n");
#endif
}

void loop() {
#ifdef USE_RTT
  // J-Run mode: Single execution with halt
  delay(1000);
#else
  // Arduino IDE mode: Periodic signaling for HIL compatibility
  CI_LOG("*STOP*\n");
  delay(5000);
#endif
}

void testFileOperations() {
  CI_LOG("\nTesting file operations:\n");

  // Debug: Check if filesystem is mounted
  bool mount_result = sdfs.begin(CS_PIN);
  CI_LOGF("Filesystem mounted: %s\n", mount_result ? "YES" : "NO");

  // Test direct root directory access after mount failure
  if (!mount_result) {
    CI_LOG("Mount failed - testing basic SD card operations...\n");
    // We know SD card init succeeds, so this is a filesystem mount issue
    return; // Skip file operations if mount failed
  }

  // Try to trigger more disk reads to see what FatFs is trying to access
  CI_LOG("Testing exists() calls to trigger disk I/O...\n");
  CI_LOGF("  exists(\"/\"): %s\n", sdfs.exists("/") ? "YES" : "NO");
  CI_LOGF("  exists(\"/LAGER.CFG\"): %s\n", sdfs.exists("/LAGER.CFG") ? "YES" : "NO");
  CI_LOGF("  exists(\"/LOG000.TXT\"): %s\n", sdfs.exists("/LOG000.TXT") ? "YES" : "NO");

  // Test 1: Write a file
  CI_LOG("Writing test file...");
  File testFile = sdfs.open("/TEST.TXT", FILE_WRITE_BEGIN);
  if (testFile) {
    CI_LOG(" File opened successfully\n");
    size_t written = testFile.println("Hello from SDFS Unified Test!");
    CI_LOGF("  Wrote %u bytes\n", written);
    testFile.printf("Mode: %s\n",
#ifdef USE_RTT
      "J-Run/RTT");
#else
      "Arduino IDE");
#endif
    testFile.printf("millis: %lu\n", millis());
    testFile.close();
    CI_LOG("  File closed - Write test OK\n");
  } else {
    CI_LOG(" FAILED - Could not open file for writing\n");
    // Try to understand why
    CI_LOGF("  exists(\"/\"): %s\n", sdfs.exists("/") ? "YES" : "NO");
    CI_LOGF("  exists(\"/TEST.TXT\"): %s\n", sdfs.exists("/TEST.TXT") ? "YES" : "NO");
    return;
  }
  
  // Test 2: Read the file back
  CI_LOG("Reading test file...");
  testFile = sdfs.open("/TEST.TXT", FILE_READ);
  if (testFile) {
    CI_LOG(" OK\n");
    CI_LOG("File contents:\n");
    while (testFile.available()) {
      char c = testFile.read();
      CI_LOGF("%c", c);
    }
    testFile.close();
  } else {
    CI_LOG(" FAILED\n");
  }
  
  // Test 3: List root directory
  CI_LOG("\nRoot directory listing:\n");
  File root = sdfs.open("/");
  if (root) {
    int fileCount = 0;
    while (true) {
      File entry = root.openNextFile();
      if (!entry) break;
      
      fileCount++;
      CI_LOGF("%s %s", entry.isDirectory() ? "DIR " : "FILE", entry.name());
      if (!entry.isDirectory()) {
        CI_LOGF(" (%lu bytes)", entry.size());
      }
      CI_LOG("\n");
      entry.close();
    }
    CI_LOGF("Total files found: %d\n", fileCount);
    root.close();
  } else {
    CI_LOG("FAILED to open root directory!\n");
  }
}