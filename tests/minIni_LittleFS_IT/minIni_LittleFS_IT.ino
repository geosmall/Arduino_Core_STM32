/**
 * minIni_LittleFS_IT.ino - Integration test for minIni config (LittleFS)
 *
 * Category: Integration Test
 * Framework: AUnit
 * Hardware: SPI Flash (W25Q128JV or compatible)
 *
 * Run with: ./system/ci/aflash.sh tests/minIni_LittleFS_IT --use-rtt --build-id
 */

#include <aunit_hil.h>
#include <minIniStorage.h>
#include "../../targets/NUCLEO_F411RE_LITTLEFS.h"

// Global instances
minIniStorage config("config.ini");
bool storageReady = false;

bool isStorageReady() {
  return storageReady;
}

// Test 1: Storage initialization
test(a_storage_init) {
  // Initialize BoardStorage
  bool result = BoardStorage::begin(BoardConfig::storage);
  assertTrue(result);

  // Initialize minIniStorage
  result = config.begin(BoardConfig::storage);
  assertTrue(result);

  // Verify storage info
  assertTrue(config.totalSize() > 0);

  storageReady = true;
}

// Test 2: Write configuration values
test(b_config_write) {
  assertTrue(isStorageReady());

  // Write string
  assertTrue(config.put("network", "ip_address", "192.168.1.50"));

  // Write integer
  assertTrue(config.put("network", "port", 9090));

  // Write boolean
  assertTrue(config.put("network", "dhcp_enabled", false));

  // Write integer (sensor section)
  assertTrue(config.put("sensor", "sample_rate", 500));

  // Write more strings
  assertTrue(config.put("system", "device_name", "LittleFS_Controller"));
  assertTrue(config.put("system", "firmware_version", "2.1.0"));
  assertTrue(config.put("system", "debug_mode", true));
}

// Test 3: Read and verify configuration values
test(c_config_read) {
  assertTrue(isStorageReady());

  // Read and verify string
  std::string ip = config.gets("network", "ip_address", "none");
  assertTrue(ip == "192.168.1.50");

  // Read and verify integer
  int port = config.geti("network", "port", 0);
  assertEqual(port, 9090);

  // Read and verify boolean
  bool dhcp = config.getbool("network", "dhcp_enabled", true);
  assertFalse(dhcp);

  // Read and verify integer
  int sample_rate = config.geti("sensor", "sample_rate", 0);
  assertEqual(sample_rate, 500);

  // Read and verify strings
  std::string device_name = config.gets("system", "device_name", "none");
  assertTrue(device_name == "LittleFS_Controller");

  std::string fw_version = config.gets("system", "firmware_version", "none");
  assertTrue(fw_version == "2.1.0");

  // Read and verify boolean
  bool debug = config.getbool("system", "debug_mode", false);
  assertTrue(debug);
}

// Test 4: Section enumeration
test(d_section_enum) {
  assertTrue(isStorageReady());

  // Count sections (should have at least 3: network, sensor, system)
  int sectionCount = 0;
  for (int i = 0; i < 10; i++) {
    std::string section = config.getsection(i);
    if (section.empty()) break;
    sectionCount++;
  }
  assertTrue(sectionCount >= 3);

  // Verify specific section exists
  bool foundNetwork = false;
  bool foundSensor = false;
  bool foundSystem = false;

  for (int i = 0; i < 10; i++) {
    std::string section = config.getsection(i);
    if (section.empty()) break;
    if (section == "network") foundNetwork = true;
    if (section == "sensor") foundSensor = true;
    if (section == "system") foundSystem = true;
  }

  assertTrue(foundNetwork);
  assertTrue(foundSensor);
  assertTrue(foundSystem);
}

// Test 5: Key enumeration within section
test(e_key_enum) {
  assertTrue(isStorageReady());

  // Count keys in network section (should have 3: ip_address, port, dhcp_enabled)
  int keyCount = 0;
  for (int i = 0; i < 10; i++) {
    std::string key = config.getkey("network", i);
    if (key.empty()) break;
    keyCount++;
  }
  assertTrue(keyCount >= 3);
}

void setup() {
  HIL_TEST_SETUP();
  HIL_TEST_TIMEOUT(60);  // 60s timeout for flash operations
}

void loop() {
  HIL_TEST_RUN();
}
