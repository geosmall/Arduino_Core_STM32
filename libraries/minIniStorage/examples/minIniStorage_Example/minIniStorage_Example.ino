/*
 * minIniStorage Example
 *
 * Demonstrates INI configuration management using minIniStorage library
 * with automatic storage backend selection (LittleFS or SDFS).
 *
 * This example shows:
 * - Storage initialization with board configuration
 * - Writing configuration values to INI file
 * - Reading configuration values from INI file
 * - Using different data types (string, int, bool, float)
 * - Section and key enumeration
 * - New minIni v1.5 features (hassection, haskey)
 *
 * Hardware Requirements:
 * - STM32 board (tested on Nucleo F411RE)
 * - SPI Flash chip (for LittleFS) OR SD card (for SDFS)
 * - Appropriate target configuration
 */

#include <minIniStorage.h>
#include <ci_log.h>

// Board target configuration - update to match your hardware setup:
// For SDFS (SD Card): NUCLEO_F411RE_SDFS.h
// For LittleFS (SPI Flash): NUCLEO_F411RE_LITTLEFS.h
#include "../../../../../targets/NUCLEO_F411RE_SDFS.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  CI_LOG("=== minIniStorage Example ===\n");
  CI_LOG("minIni v1.5 Configuration Management Demo\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();
  CI_LOG("\n");

  // Create minIniStorage instance for settings file
  minIniStorage config("settings.ini");

  // Initialize storage with board configuration
  CI_LOG("Initializing storage...\n");
  if (config.begin(BoardConfig::storage)) {
    CI_LOG("✓ Storage initialized successfully\n");
    CI_LOG("Total size: ");
    CI_LOG(String((unsigned long)(config.totalSize() / 1024)).c_str());
    CI_LOG(" KB\n");
    CI_LOG("Used size: ");
    CI_LOG(String((unsigned long)(config.usedSize() / 1024)).c_str());
    CI_LOG(" KB\n");
    CI_LOG("\n");
  } else {
    CI_LOG("✗ Storage initialization failed\n");
    CI_LOG("Check hardware connections and target configuration\n");
    return;
  }

  // === Writing Configuration Values ===
  CI_LOG("=== Writing Configuration ===\n");

  // Network settings
  config.put("network", "ip_address", "192.168.1.100");
  config.put("network", "port", 8080);
  config.put("network", "dhcp_enabled", true);

  // Sensor calibration
  config.put("sensor", "temperature_offset", 2.5f);
  config.put("sensor", "sample_rate", 1000);
  config.put("sensor", "enabled", true);

  // System information
  config.put("system", "device_name", "UAV_Controller");
  config.put("system", "firmware_version", "1.0.0");
  config.put("system", "debug_mode", false);

  CI_LOG("Configuration written to settings.ini\n");
  CI_LOG("\n");

  // === Reading Configuration Values ===
  CI_LOG("=== Reading Configuration ===\n");

  // Read network settings
  std::string ip = config.gets("network", "ip_address", "192.168.1.1");
  int port = config.geti("network", "port", 80);
  bool dhcp = config.getbool("network", "dhcp_enabled", false);

  CI_LOG("IP Address: "); CI_LOG(ip.c_str()); CI_LOG("\n");
  CI_LOG("Port: "); CI_LOG(String(port).c_str()); CI_LOG("\n");
  CI_LOG("DHCP: "); CI_LOG(dhcp ? "Enabled" : "Disabled"); CI_LOG("\n");
  CI_LOG("\n");

  // Read sensor settings
  float temp_offset = config.getf("sensor", "temperature_offset", 0.0);
  int sample_rate = config.geti("sensor", "sample_rate", 100);
  bool sensor_enabled = config.getbool("sensor", "enabled", false);

  CI_LOG("Temperature Offset: "); CI_LOG(String(temp_offset).c_str()); CI_LOG("\n");
  CI_LOG("Sample Rate: "); CI_LOG(String(sample_rate).c_str()); CI_LOG(" Hz\n");
  CI_LOG("Sensor: "); CI_LOG(sensor_enabled ? "Enabled" : "Disabled"); CI_LOG("\n");
  CI_LOG("\n");

  // Read system settings
  std::string device_name = config.gets("system", "device_name", "Unknown");
  std::string fw_version = config.gets("system", "firmware_version", "0.0.0");
  bool debug = config.getbool("system", "debug_mode", false);

  CI_LOG("Device Name: "); CI_LOG(device_name.c_str()); CI_LOG("\n");
  CI_LOG("Firmware: "); CI_LOG(fw_version.c_str()); CI_LOG("\n");
  CI_LOG("Debug Mode: "); CI_LOG(debug ? "Enabled" : "Disabled"); CI_LOG("\n");
  CI_LOG("\n");

  // === minIni v1.5 New Features ===
  CI_LOG("=== minIni v1.5 Features ===\n");

  // Check if sections exist
  CI_LOG("Has 'network' section: ");
  CI_LOG(config.hassection("network") ? "Yes" : "No"); CI_LOG("\n");

  CI_LOG("Has 'bluetooth' section: ");
  CI_LOG(config.hassection("bluetooth") ? "Yes" : "No"); CI_LOG("\n");

  // Check if specific keys exist
  CI_LOG("Has 'network.ip_address': ");
  CI_LOG(config.haskey("network", "ip_address") ? "Yes" : "No"); CI_LOG("\n");

  CI_LOG("Has 'network.password': ");
  CI_LOG(config.haskey("network", "password") ? "Yes" : "No"); CI_LOG("\n");
  CI_LOG("\n");

  // === Section and Key Enumeration ===
  CI_LOG("=== Configuration Structure ===\n");

  // Enumerate all sections
  for (int i = 0; ; i++) {
    std::string section = config.getsection(i);
    if (section.empty()) break;

    CI_LOG("Section: "); CI_LOG(section.c_str()); CI_LOG("\n");

    // Enumerate keys in this section
    for (int j = 0; ; j++) {
      std::string key = config.getkey(section, j);
      if (key.empty()) break;

      CI_LOG("  Key: "); CI_LOG(key.c_str()); CI_LOG("\n");
    }
  }
  CI_LOG("\n");

  // === Demonstration Complete ===
  CI_LOG("=== Example Complete ===\n");
  CI_LOG("minIniStorage successfully demonstrated!\n");
  CI_LOG("Check your storage device - settings.ini has been created\n");

  #ifdef USE_RTT
  CI_LOG("*STOP*\n");
  #endif
}

void loop() {
  // Nothing to do in loop
}