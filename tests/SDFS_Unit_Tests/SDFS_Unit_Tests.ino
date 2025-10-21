#include <aunit_hil.h>
#include <SDFS.h>

// Board configuration for hardware abstraction
#if defined(ARDUINO_BLACKPILL_F411CE)
#include "../../targets/BLACKPILL_F411CE.h"
#else
#include "../../targets/NUCLEO_F411RE_SDFS.h"
#endif

/*
 * SDFS Unit Tests - AUnit Integration
 *
 * HARDWARE DEPENDENCY: SD Card Required
 * =======================================================
 * This test requires a working SD card connected via SPI to the target board.
 *
 * Validated Hardware:
 * - SD Card: Standard SD/SDHC card (tested with various capacities)
 * - Target Board: STM32F411RE Nucleo
 * - Pin Configuration:
 *   - MOSI: PC12, MISO: PC11, SCLK: PC10, CS: PD2
 *
 * File System: Uses FatFs backend with LittleFS-compatible API
 * Supports: FAT16/FAT32 formatted SD cards
 *
 * HIL Testing: Use with --use-rtt flag for automated testing
 * IDE Testing: Compile without --use-rtt for Serial monitor output
 *
 * Note: Progressive implementation - some advanced tests removed to ensure stability
 */

// Hardware configuration - BoardConfig integration (all SPI pins converted)
#define CS_PIN BoardConfig::storage.cs_pin
#define SPI_MOSI BoardConfig::storage.mosi_pin
#define SPI_MISO BoardConfig::storage.miso_pin
#define SPI_SCLK BoardConfig::storage.sclk_pin

// Global filesystem instance - initialized once
SDFS_SPI sdfs;
bool filesystemReady = false;

// Test constants
const char* TEST_STRING = "Hello SDFS Test";

// Helper function to ensure filesystem is ready
bool isFilesystemReady() {
  return filesystemReady;
}

// ============================================================================
// Start with just the basic tests
// ============================================================================

test(a_initialize_filesystem) {
  // Configure SPI pins
  SPI.setMOSI(SPI_MOSI);
  SPI.setMISO(SPI_MISO);
  SPI.setSCLK(SPI_SCLK);

  // Initialize filesystem
  bool fsInitialized = sdfs.begin(CS_PIN);
  assertTrue(fsInitialized);

  // Check basic filesystem properties
  uint64_t totalSize = sdfs.totalSize();
  assertTrue(totalSize > 0);

  // Media name should be available
  const char* mediaName = sdfs.getMediaName();
  assertNotEqual(mediaName, (const char*)nullptr);

  // Mark filesystem as ready for other tests
  filesystemReady = true;
}

test(basic_file_operations) {
  assertTrue(isFilesystemReady());

  // Clean state - remove test file if it exists
  sdfs.remove("/test.txt");

  // Verify file doesn't exist
  assertFalse(sdfs.exists("/test.txt"));

  // Create and write file
  File file = sdfs.open("/test.txt", FILE_WRITE);
  assertTrue((bool)file);

  size_t written = file.print(TEST_STRING);
  assertTrue(written > 0);
  assertEqual((unsigned int)written, (unsigned int)strlen(TEST_STRING));
  file.close();

  // Verify file exists
  assertTrue(sdfs.exists("/test.txt"));

  // Read back and verify content
  file = sdfs.open("/test.txt", FILE_READ);
  assertTrue((bool)file);

  String content = file.readString();
  file.close();

  assertTrue(content.equals(TEST_STRING));

  // Cleanup
  assertTrue(sdfs.remove("/test.txt"));
  assertFalse(sdfs.exists("/test.txt"));
}

test(filesystem_basic_info) {
  assertTrue(isFilesystemReady());

  // Test basic filesystem information
  uint64_t totalSize = sdfs.totalSize();
  uint64_t usedSize = sdfs.usedSize();

  // Basic sanity checks
  assertTrue(totalSize > 0);
  assertTrue(usedSize <= totalSize);

  // Create a small test file
  sdfs.remove("/infotest.txt");
  File file = sdfs.open("/infotest.txt", FILE_WRITE);
  assertTrue((bool)file);
  file.print("This is a test for SDFS filesystem info validation.");
  file.close();

  // Verify the file exists and has expected size
  assertTrue(sdfs.exists("/infotest.txt"));
  file = sdfs.open("/infotest.txt", FILE_READ);
  assertTrue((bool)file);
  assertTrue(file.size() > 0);
  file.close();

  // Cleanup
  assertTrue(sdfs.remove("/infotest.txt"));
  assertFalse(sdfs.exists("/infotest.txt"));
}

test(file_seek_and_position) {
  assertTrue(isFilesystemReady());

  // Clean state
  sdfs.remove("/seektest.txt");

  // Create file with known content: "Hello SDFS Test" (15 chars)
  // String positions: H(0)e(1)l(2)l(3)o(4) (5)S(6)D(7)F(8)S(9) (10)T(11)e(12)s(13)t(14)
  File file = sdfs.open("/seektest.txt", FILE_WRITE);
  assertTrue((bool)file);
  file.print(TEST_STRING);
  file.close();

  // Test seek operations
  file = sdfs.open("/seektest.txt", FILE_READ);
  assertTrue((bool)file);

  // Test seek to beginning
  assertTrue(file.seek(0));
  assertEqual((unsigned int)file.position(), 0U);

  // Read first 5 characters
  char buffer[6];
  size_t bytesRead = file.read((uint8_t*)buffer, 5);
  buffer[5] = '\0';
  assertEqual((unsigned int)bytesRead, 5U);
  assertTrue(String(buffer).equals("Hello"));

  // Test current position (should be at 5 after reading 5 chars)
  assertEqual((unsigned int)file.position(), 5U);

  // Test seek to specific position (position 11 = 'T' in "Hello SDFS Test")
  assertTrue(file.seek(11));
  assertEqual((unsigned int)file.position(), 11U);

  // Read remaining from position 11 to end: "Test" (4 chars)
  String remaining = file.readString();
  assertTrue(remaining.equals("Test"));

  file.close();

  // Cleanup
  sdfs.remove("/seektest.txt");
}

test(file_append_operations) {
  assertTrue(isFilesystemReady());

  // Clean state
  sdfs.remove("/append.txt");

  // Create initial file
  File file = sdfs.open("/append.txt", FILE_WRITE);
  assertTrue((bool)file);
  file.print("First");
  file.close();

  // Verify initial content
  file = sdfs.open("/append.txt", FILE_READ);
  assertTrue((bool)file);
  String content1 = file.readString();
  file.close();
  assertTrue(content1.equals("First"));

  // Append to file using FILE_WRITE_BEGIN + seek
  file = sdfs.open("/append.txt", FILE_WRITE_BEGIN);
  assertTrue((bool)file);
  file.seek(file.size()); // Go to end
  file.print("Second");
  file.close();

  // Verify combined content
  file = sdfs.open("/append.txt", FILE_READ);
  assertTrue((bool)file);
  String content2 = file.readString();
  file.close();
  assertTrue(content2.equals("FirstSecond"));

  // Cleanup
  sdfs.remove("/append.txt");
}

test(directory_operations) {
  assertTrue(isFilesystemReady());

  // Clean state
  sdfs.remove("/testdir/file.txt");
  sdfs.rmdir("/testdir");

  // Verify directory doesn't exist
  assertFalse(sdfs.exists("/testdir"));

  // Create directory
  assertTrue(sdfs.mkdir("/testdir"));
  assertTrue(sdfs.exists("/testdir"));

  // Create file in directory
  File file = sdfs.open("/testdir/file.txt", FILE_WRITE);
  assertTrue((bool)file);
  file.print("Directory content test");
  file.close();

  // Verify file exists in directory
  assertTrue(sdfs.exists("/testdir/file.txt"));

  // Verify file content
  file = sdfs.open("/testdir/file.txt", FILE_READ);
  assertTrue((bool)file);
  String content = file.readString();
  file.close();
  assertTrue(content.equals("Directory content test"));

  // Cleanup - must remove files before directory
  assertTrue(sdfs.remove("/testdir/file.txt"));
  assertFalse(sdfs.exists("/testdir/file.txt"));

  assertTrue(sdfs.rmdir("/testdir"));
  assertFalse(sdfs.exists("/testdir"));
}

test(file_rename_operations) {
  assertTrue(isFilesystemReady());

  // Clean state
  sdfs.remove("/original.txt");
  sdfs.remove("/renamed.txt");

  // Create file with content
  File file = sdfs.open("/original.txt", FILE_WRITE);
  assertTrue((bool)file);
  file.print(TEST_STRING);
  file.close();

  // Verify original exists
  assertTrue(sdfs.exists("/original.txt"));
  assertFalse(sdfs.exists("/renamed.txt"));

  // Rename file
  assertTrue(sdfs.rename("/original.txt", "/renamed.txt"));

  // Verify rename worked
  assertFalse(sdfs.exists("/original.txt"));
  assertTrue(sdfs.exists("/renamed.txt"));

  // Verify content preserved
  file = sdfs.open("/renamed.txt", FILE_READ);
  assertTrue((bool)file);
  String content = file.readString();
  file.close();
  assertTrue(content.equals(TEST_STRING));

  // Cleanup
  assertTrue(sdfs.remove("/renamed.txt"));
}

// ============================================================================
// Arduino Integration
// ============================================================================

void setup() {
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
#else
  SEGGER_RTT_Init();
#endif

  HIL_TEST_SETUP();
  HIL_TEST_TIMEOUT(60);
}

void loop() {
  HIL_TEST_RUN();
}