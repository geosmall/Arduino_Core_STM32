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
#include <Storage.h>
#include <BoardStorage.h>
#include "../../targets/NUCLEO_F411RE_LITTLEFS.h"

// Global state for test sequencing
bool storageReady = false;

// Helper to check storage is ready
bool isStorageReady() {
  return storageReady;
}

// Test 1: Initialization (runs first due to 'a_' prefix)
test(a_storage_init) {
  // Verify board config
  assertTrue(BoardConfig::storage.backend_type == StorageBackend::LITTLEFS);
  assertTrue(BoardConfig::storage.cs_pin != 0xFF);

  // Initialize storage
  bool result = BoardStorage::begin(BoardConfig::storage);
  assertTrue(result);

  // Verify storage reports valid info
  Storage& fs = BoardStorage::getStorage();
  assertTrue(fs.name() != nullptr);
  assertTrue(fs.totalSize() > 0);

  // Mark ready for other tests
  storageReady = true;
}

// Test 2: File write succeeds
test(b_file_write) {
  assertTrue(isStorageReady());

  Storage& fs = BoardStorage::getStorage();

  // Clean up any existing test file
  fs.remove("/test_aunit.txt");

  File f = fs.open("/test_aunit.txt", FILE_WRITE);
  assertTrue((bool)f);
  size_t written = f.println("AUnit Storage Test");
  f.close();
  assertTrue(written > 0);
}

// Test 3: File exists after write
test(c_file_exists) {
  assertTrue(isStorageReady());

  Storage& fs = BoardStorage::getStorage();
  assertTrue(fs.exists("/test_aunit.txt"));
}

// Test 4: File read returns correct content
test(d_file_read) {
  assertTrue(isStorageReady());

  Storage& fs = BoardStorage::getStorage();
  File f = fs.open("/test_aunit.txt", FILE_READ);
  assertTrue((bool)f);
  String content = f.readString();
  f.close();
  assertTrue(content.startsWith("AUnit Storage Test"));
}

// Test 5: File remove succeeds
test(e_file_remove) {
  assertTrue(isStorageReady());

  Storage& fs = BoardStorage::getStorage();
  assertTrue(fs.remove("/test_aunit.txt"));
  assertFalse(fs.exists("/test_aunit.txt"));
}

void setup() {
  HIL_TEST_SETUP();
}

void loop() {
  HIL_TEST_RUN();
}
