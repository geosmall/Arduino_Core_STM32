#include <aunit_hil.h>
#include <SPI.h>
#include <LittleFS.h>

// Board configuration for hardware abstraction
#if defined(ARDUINO_BLACKPILL_F411CE)
#include "../../targets/BLACKPILL_F411CE.h"
#else
#include "../../targets/NUCLEO_F411RE_LITTLEFS.h"
#endif

/*
 * LittleFS Unit Tests - AUnit Integration
 *
 * HARDWARE DEPENDENCY: SPI Flash Required
 * =======================================================
 * This test requires a working SPI flash chip connected to the target board.
 *
 * Validated Hardware:
 * - SPI Flash: W25Q128JV 16MB (or compatible)
 * - Target Board: STM32F411RE Nucleo
 * - Pin Configuration:
 *   - MOSI: PC12, MISO: PC11, SCLK: PC10, CS: PD2
 *
 * Supported Flash Chips: 20+ chips from Winbond, GigaDevice, Microchip, etc.
 * See libraries/LittleFS/README.md for complete chip compatibility list.
 *
 * HIL Testing: Use with --use-rtt flag for automated testing
 * IDE Testing: Compile without --use-rtt for Serial monitor output
 */

// Hardware configuration - BoardConfig integration
//              MOSI  MISO  SCLK
SPIClass SPIbus(BoardConfig::storage.mosi_pin, BoardConfig::storage.miso_pin, BoardConfig::storage.sclk_pin);
// CS pin from BoardConfig
#define CS_PIN BoardConfig::storage.cs_pin

// Global filesystem instance - initialized once
LittleFS_SPIFlash myfs;
bool filesystemReady = false;

// Test constants
const char* TEST_STRING = "Hello LittleFS Test";

// Helper function to ensure filesystem is ready
bool isFilesystemReady() {
  return filesystemReady;
}

// ============================================================================
// Filesystem Initialization (Run Once)
// ============================================================================

test(a_initialize_filesystem) {
  // Ensure CS pin is HIGH
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  delay(10);

  // Initialize filesystem
  bool fsInitialized = myfs.begin(CS_PIN, SPIbus);
  assertTrue(fsInitialized);

  // Check basic filesystem properties
  uint64_t totalSize = myfs.totalSize();
  assertTrue(totalSize > 0);

  // Expected size for W25Q128JV is 16MB = 16777216 bytes
  assertEqual((unsigned long)totalSize, 16777216UL);

  // Media name should be available
  const char* mediaName = myfs.getMediaName();
  assertNotEqual(mediaName, (const char*)nullptr);

  // Format filesystem for clean test state
  myfs.format();

  // Mark filesystem as ready for other tests
  filesystemReady = true;
}

// ============================================================================
// File Operation Tests
// ============================================================================

test(basic_file_operations) {
  assertTrue(isFilesystemReady());

  // Clean state - remove test file if it exists
  myfs.remove("test.txt");

  // Verify file doesn't exist
  assertFalse(myfs.exists("test.txt"));

  // Create and write file
  File file = myfs.open("test.txt", FILE_WRITE);
  assertTrue((bool)file);

  size_t written = file.print(TEST_STRING);
  assertTrue(written > 0);
  assertEqual((unsigned int)written, (unsigned int)strlen(TEST_STRING));
  file.close();

  // Verify file exists
  assertTrue(myfs.exists("test.txt"));

  // Read back and verify content
  file = myfs.open("test.txt", FILE_READ);
  assertTrue((bool)file);

  String content = file.readString();
  file.close();

  assertTrue(content.equals(TEST_STRING));

  // Test file size
  file = myfs.open("test.txt", FILE_READ);
  assertTrue((bool)file);
  assertEqual((unsigned int)file.size(), (unsigned int)strlen(TEST_STRING));
  file.close();

  // Cleanup
  assertTrue(myfs.remove("test.txt"));
  assertFalse(myfs.exists("test.txt"));
}

test(file_seek_and_position) {
  assertTrue(isFilesystemReady());

  // Clean state
  myfs.remove("seektest.txt");

  // Create file with known content: "Hello LittleFS Test" (19 chars)
  // Positions: H(0)e(1)l(2)l(3)o(4) (5)L(6)i(7)t(8)t(9)l(10)e(11)F(12)S(13) (14)T(15)e(16)s(17)t(18)
  File file = myfs.open("seektest.txt", FILE_WRITE);
  assertTrue((bool)file);
  file.print(TEST_STRING);
  file.close();

  // Test seek operations
  file = myfs.open("seektest.txt", FILE_READ);
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

  // Test seek to specific position (position 12 = 'F')
  assertTrue(file.seek(12));
  assertEqual((unsigned int)file.position(), 12U);

  // Read remaining from position 12 to end: "FS Test" (7 chars)
  String remaining = file.readString();
  assertTrue(remaining.equals("FS Test"));

  file.close();

  // Cleanup
  myfs.remove("seektest.txt");
}

test(file_append_operations) {
  assertTrue(isFilesystemReady());

  // Clean state
  myfs.remove("append.txt");

  // Create initial file
  File file = myfs.open("append.txt", FILE_WRITE);
  assertTrue((bool)file);
  file.print("First");
  file.close();

  // Verify initial content
  file = myfs.open("append.txt", FILE_READ);
  assertTrue((bool)file);
  String content1 = file.readString();
  file.close();
  assertTrue(content1.equals("First"));

  // Append to file using FILE_WRITE_BEGIN + seek
  file = myfs.open("append.txt", FILE_WRITE_BEGIN);
  assertTrue((bool)file);
  file.seek(file.size()); // Go to end
  file.print("Second");
  file.close();

  // Verify combined content
  file = myfs.open("append.txt", FILE_READ);
  assertTrue((bool)file);
  String content2 = file.readString();
  file.close();
  assertTrue(content2.equals("FirstSecond"));

  // Cleanup
  myfs.remove("append.txt");
}

test(directory_operations) {
  assertTrue(isFilesystemReady());

  // Clean state
  myfs.remove("testdir/file.txt");
  myfs.rmdir("testdir");

  // Verify directory doesn't exist
  assertFalse(myfs.exists("testdir"));

  // Create directory
  assertTrue(myfs.mkdir("testdir"));
  assertTrue(myfs.exists("testdir"));

  // Create file in directory
  File file = myfs.open("testdir/file.txt", FILE_WRITE);
  assertTrue((bool)file);
  file.print("Directory content test");
  file.close();

  // Verify file exists in directory
  assertTrue(myfs.exists("testdir/file.txt"));

  // Verify file content
  file = myfs.open("testdir/file.txt", FILE_READ);
  assertTrue((bool)file);
  String content = file.readString();
  file.close();
  assertTrue(content.equals("Directory content test"));

  // Test file size in directory
  file = myfs.open("testdir/file.txt", FILE_READ);
  assertTrue((bool)file);
  assertEqual((unsigned int)file.size(), (unsigned int)strlen("Directory content test"));
  file.close();

  // Cleanup - must remove files before directory
  assertTrue(myfs.remove("testdir/file.txt"));
  assertFalse(myfs.exists("testdir/file.txt"));

  assertTrue(myfs.rmdir("testdir"));
  assertFalse(myfs.exists("testdir"));
}

test(file_rename_operations) {
  assertTrue(isFilesystemReady());

  // Clean state
  myfs.remove("original.txt");
  myfs.remove("renamed.txt");

  // Create file with content
  File file = myfs.open("original.txt", FILE_WRITE);
  assertTrue((bool)file);
  file.print(TEST_STRING);
  file.close();

  // Verify original exists
  assertTrue(myfs.exists("original.txt"));
  assertFalse(myfs.exists("renamed.txt"));

  // Rename file
  assertTrue(myfs.rename("original.txt", "renamed.txt"));

  // Verify rename worked
  assertFalse(myfs.exists("original.txt"));
  assertTrue(myfs.exists("renamed.txt"));

  // Verify content preserved
  file = myfs.open("renamed.txt", FILE_READ);
  assertTrue((bool)file);
  String content = file.readString();
  file.close();
  assertTrue(content.equals(TEST_STRING));

  // Cleanup
  assertTrue(myfs.remove("renamed.txt"));
}

test(large_file_operations) {
  assertTrue(isFilesystemReady());

  // Clean state
  myfs.remove("largefile.dat");

  // Create larger file (1KB in 128-byte chunks)
  File file = myfs.open("largefile.dat", FILE_WRITE);
  assertTrue((bool)file);

  char buffer[128];
  memset(buffer, 'X', sizeof(buffer));

  // Write 8 chunks = 1024 bytes
  for (int i = 0; i < 8; i++) {
    size_t written = file.write((uint8_t*)buffer, sizeof(buffer));
    assertEqual((unsigned int)written, (unsigned int)sizeof(buffer));
  }

  size_t finalSize = file.size();
  file.close();

  // Verify file size
  assertEqual((unsigned int)finalSize, 1024U);

  // Verify we can read it back
  file = myfs.open("largefile.dat", FILE_READ);
  assertTrue((bool)file);
  assertEqual((unsigned int)file.size(), 1024U);

  // Read and verify first chunk
  char readBuffer[128];
  size_t bytesRead = file.read((uint8_t*)readBuffer, sizeof(readBuffer));
  assertEqual((unsigned int)bytesRead, (unsigned int)sizeof(readBuffer));

  // Verify content
  for (size_t i = 0; i < sizeof(readBuffer); i++) {
    assertEqual(readBuffer[i], 'X');
  }

  file.close();

  // Cleanup
  assertTrue(myfs.remove("largefile.dat"));
}

test(filesystem_basic_info) {
  assertTrue(isFilesystemReady());

  // Test basic filesystem information
  uint64_t totalSize = myfs.totalSize();
  uint64_t usedSize = myfs.usedSize();

  // Basic sanity checks
  assertTrue(totalSize > 0);
  assertTrue(usedSize <= totalSize);
  assertEqual((unsigned long)totalSize, 16777216UL); // 16MB for W25Q128JV

  // Create a small test file
  myfs.remove("infotest.txt");
  File file = myfs.open("infotest.txt", FILE_WRITE);
  assertTrue((bool)file);
  file.print("This is a test for filesystem info validation.");
  file.close();

  // Verify the file exists and has expected size
  assertTrue(myfs.exists("infotest.txt"));
  file = myfs.open("infotest.txt", FILE_READ);
  assertTrue((bool)file);
  assertTrue(file.size() > 0);
  file.close();

  // Cleanup
  assertTrue(myfs.remove("infotest.txt"));
  assertFalse(myfs.exists("infotest.txt"));
}

// ============================================================================
// Arduino Integration
// ============================================================================

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

  // Initialize HIL testing environment
  HIL_TEST_SETUP();

  // Set timeout for filesystem operations
  HIL_TEST_TIMEOUT(120);
}

void loop() {
  // Run tests with HIL integration
  HIL_TEST_RUN();
}