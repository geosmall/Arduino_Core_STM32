/**
 * Storage_LittleFS_IT.ino - Integration test for Storage abstraction (LittleFS)
 *
 * Category: Integration Test
 * Framework: AUnit
 * Hardware: SPI Flash (W25Q128JV or compatible)
 *
 * Run with: ./system/ci/aflash.sh tests/Storage_LittleFS_IT --use-rtt --build-id
 */

#include <aunit_hil.h>
#include <minIniStorage.h>  // Includes Storage, BoardStorage, and libPrintf dependencies
#include "../../targets/NUCLEO_F411RE_LITTLEFS.h"

// Global state for test sequencing
bool storageReady = false;

// Helper to check storage is ready
bool isStorageReady() {
  return storageReady;
}

// Test 1: Initialization
test(a_storage_init) {
  // Verify board config has valid CS pin
  assertTrue(BoardConfig::storage.cs_pin != 0xFF);

  // Initialize storage
  bool result = BoardStorage::begin(BoardConfig::storage);
  assertTrue(result);

  // Verify storage reports valid info
  Storage* storage = &BOARD_STORAGE;
  assertTrue(storage->name() != nullptr);
  assertTrue(storage->totalSize() > 0);
  assertTrue(storage->isInitialized());

  // Mark ready for other tests
  storageReady = true;
}

// Test 2: File write succeeds
test(b_file_write) {
  assertTrue(isStorageReady());

  Storage* storage = &BOARD_STORAGE;

  // Clean up any existing test file
  storage->remove("/test.txt");

  File f = storage->open("/test.txt", FILE_WRITE);
  assertTrue((bool)f);
  size_t written = f.println("AUnit Storage Test");
  f.close();
  assertTrue(written > 0);
}

// Test 3: File exists after write
test(c_file_exists) {
  assertTrue(isStorageReady());

  Storage* storage = &BOARD_STORAGE;
  assertTrue(storage->exists("/test.txt"));
}

// Test 4: File read returns correct content
test(d_file_read) {
  assertTrue(isStorageReady());

  Storage* storage = &BOARD_STORAGE;
  File f = storage->open("/test.txt", FILE_READ);
  assertTrue((bool)f);
  String content = f.readString();
  f.close();
  assertTrue(content.startsWith("AUnit Storage Test"));
}

// Test 5: File remove succeeds
test(e_file_remove) {
  assertTrue(isStorageReady());

  Storage* storage = &BOARD_STORAGE;
  assertTrue(storage->remove("/test.txt"));
  assertFalse(storage->exists("/test.txt"));
}

void setup() {
  HIL_TEST_SETUP();
  HIL_TEST_TIMEOUT(30);  // 30s timeout for flash operations
}

void loop() {
  HIL_TEST_RUN();
}
