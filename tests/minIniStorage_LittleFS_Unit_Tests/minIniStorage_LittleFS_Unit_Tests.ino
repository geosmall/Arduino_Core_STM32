#include <minIniStorage.h>
#include "../../targets/NUCLEO_F411RE_LITTLEFS.h"
#include <ci_log.h>

minIniStorage config("config.ini");

void setup() {
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect
  }
#endif

  CI_LOG("minIniStorage LittleFS Test Starting\n");
  CI_BUILD_INFO();

  // Initialize BoardStorage first (like in Generic Storage Test)
  if (!BoardStorage::begin(BoardConfig::storage)) {
    CI_LOG("Failed to initialize BoardStorage\n");
    while(1);
  }

  // Now initialize minIniStorage (should recognize already initialized storage)
  if (!config.begin(BoardConfig::storage)) {
    CI_LOG("Failed to initialize minIniStorage\n");
    while(1);
  }

  CI_LOG("LittleFS storage initialized\n");

  // Display storage info
  uint64_t total_bytes = config.totalSize();
  uint64_t used_bytes = config.usedSize();
  uint32_t total_kb = (uint32_t)(total_bytes / 1024);
  uint32_t used_kb = (uint32_t)(used_bytes / 1024);

  char info_buffer[64];
  sprintf(info_buffer, "Total: %lu KB, Used: %lu KB\n", total_kb, used_kb);
  CI_LOG(info_buffer);

  CI_LOG("Testing minIni configuration management...\n");
}

void loop() {
  CI_LOG("=== minIni LittleFS Configuration Test ===\n");

  // Test writing various data types
  CI_LOG("Writing configuration values...\n");

  // Test storage initialization state
  bool isInit = BoardStorage::isInitialized();
  CI_LOG("BoardStorage initialized: ");
  CI_LOG(isInit ? "YES" : "NO");
  CI_LOG("\n");

  if (isInit) {
    // Test basic file creation first
    Storage& storage = BOARD_STORAGE;
    File testFile = storage.open("config.ini", FILE_WRITE);
    if (testFile) {
      CI_LOG("File creation test: SUCCESS\n");
      testFile.close();
    } else {
      CI_LOG("File creation test: FAILED\n");
    }
  } else {
    CI_LOG("Cannot test file creation - storage not initialized\n");
  }

  bool result1 = config.put("network", "ip_address", "192.168.1.50");
  bool result2 = config.put("network", "port", 9090);
  bool result3 = config.put("network", "dhcp_enabled", false);

  CI_LOG("Put results: ");
  CI_LOG(result1 ? "1" : "0");
  CI_LOG(" ");
  CI_LOG(result2 ? "1" : "0");
  CI_LOG(" ");
  CI_LOG(result3 ? "1" : "0");
  CI_LOG("\n");

  config.put("sensor", "temperature_offset", 1.5f);
  config.put("sensor", "calibration_factor", 0.987f);
  config.put("sensor", "sample_rate", 500);

  config.put("system", "device_name", "LittleFS_Controller");
  config.put("system", "firmware_version", "2.1.0");
  config.put("system", "debug_mode", true);

  CI_LOG("Configuration written successfully\n");

  // Test reading back the values
  CI_LOG("Reading configuration values...\n");

  std::string ip = config.gets("network", "ip_address", "none");
  CI_LOG("IP Address: ");
  CI_LOG(ip.c_str());
  CI_LOG("\n");

  int port = config.geti("network", "port", 0);
  char port_str[16];
  sprintf(port_str, "%d", port);
  CI_LOG("Port: ");
  CI_LOG(port_str);
  CI_LOG("\n");

  bool dhcp = config.getbool("network", "dhcp_enabled", true);
  CI_LOG("DHCP: ");
  CI_LOG(dhcp ? "Enabled" : "Disabled");
  CI_LOG("\n");

  float temp_offset = config.getf("sensor", "temperature_offset", 0.0f);
  CI_LOG_FLOAT("Temperature Offset: ", temp_offset, 1);
  CI_LOG("\n");

  float cal_factor = config.getf("sensor", "calibration_factor", 0.0f);
  CI_LOG_FLOAT("Calibration Factor: ", cal_factor, 3);
  CI_LOG("\n");

  int sample_rate = config.geti("sensor", "sample_rate", 0);
  sprintf(port_str, "%d", sample_rate);
  CI_LOG("Sample Rate: ");
  CI_LOG(port_str);
  CI_LOG(" Hz\n");

  std::string device_name = config.gets("system", "device_name", "none");
  CI_LOG("Device Name: ");
  CI_LOG(device_name.c_str());
  CI_LOG("\n");

  std::string fw_version = config.gets("system", "firmware_version", "none");
  CI_LOG("Firmware Version: ");
  CI_LOG(fw_version.c_str());
  CI_LOG("\n");

  bool debug = config.getbool("system", "debug_mode", false);
  CI_LOG("Debug Mode: ");
  CI_LOG(debug ? "Enabled" : "Disabled");
  CI_LOG("\n");

  // Test section and key enumeration
  CI_LOG("=== Configuration Sections ===\n");
  for (int i = 0; ; i++) {
    std::string section = config.getsection(i);
    if (section.empty()) break;
    CI_LOG("Section: ");
    CI_LOG(section.c_str());
    CI_LOG("\n");

    // List keys in this section
    for (int j = 0; ; j++) {
      std::string key = config.getkey(section, j);
      if (key.empty()) break;
      CI_LOG("  Key: ");
      CI_LOG(key.c_str());
      CI_LOG("\n");
    }
  }

  CI_LOG("=== minIni LittleFS Test Complete ===\n");
  CI_LOG("*STOP*\n");

  while(true) {;}
}