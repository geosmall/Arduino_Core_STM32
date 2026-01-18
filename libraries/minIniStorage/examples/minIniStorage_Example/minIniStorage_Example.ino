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

// Board target configuration - update to match your hardware setup:
// For SDFS (SD Card): NUCLEO_F411RE_SDFS.h
// For LittleFS (SPI Flash): NUCLEO_F411RE_HIL001.h
#include "../../../../../targets/NUCLEO_F411RE_HIL001.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("=== minIniStorage Example ===");
  Serial.println("minIni v1.5 Configuration Management Demo");
  Serial.println();

  // Create minIniStorage instance for settings file
  minIniStorage config("settings.ini");

  // Initialize storage with board configuration
  Serial.println("Initializing storage...");
  if (config.begin(BoardConfig::storage)) {
    Serial.println("Storage initialized successfully");
    Serial.print("Total size: ");
    Serial.print((unsigned long)(config.totalSize() / 1024));
    Serial.println(" KB");
    Serial.print("Used size: ");
    Serial.print((unsigned long)(config.usedSize() / 1024));
    Serial.println(" KB");
    Serial.println();
  } else {
    Serial.println("Storage initialization failed");
    Serial.println("Check hardware connections and target configuration");
    Serial.println("*STOP*");
    return;
  }

  // === Writing Configuration Values ===
  Serial.println("=== Writing Configuration ===");

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

  Serial.println("Configuration written to settings.ini");
  Serial.println();

  // === Reading Configuration Values ===
  Serial.println("=== Reading Configuration ===");

  // Read network settings
  std::string ip = config.gets("network", "ip_address", "192.168.1.1");
  int port = config.geti("network", "port", 80);
  bool dhcp = config.getbool("network", "dhcp_enabled", false);

  Serial.print("IP Address: "); Serial.println(ip.c_str());
  Serial.print("Port: "); Serial.println(port);
  Serial.print("DHCP: "); Serial.println(dhcp ? "Enabled" : "Disabled");
  Serial.println();

  // Read sensor settings
  float temp_offset = config.getf("sensor", "temperature_offset", 0.0);
  int sample_rate = config.geti("sensor", "sample_rate", 100);
  bool sensor_enabled = config.getbool("sensor", "enabled", false);

  Serial.print("Temperature Offset: "); Serial.println(temp_offset);
  Serial.print("Sample Rate: "); Serial.print(sample_rate); Serial.println(" Hz");
  Serial.print("Sensor: "); Serial.println(sensor_enabled ? "Enabled" : "Disabled");
  Serial.println();

  // Read system settings
  std::string device_name = config.gets("system", "device_name", "Unknown");
  std::string fw_version = config.gets("system", "firmware_version", "0.0.0");
  bool debug = config.getbool("system", "debug_mode", false);

  Serial.print("Device Name: "); Serial.println(device_name.c_str());
  Serial.print("Firmware: "); Serial.println(fw_version.c_str());
  Serial.print("Debug Mode: "); Serial.println(debug ? "Enabled" : "Disabled");
  Serial.println();

  // === minIni v1.5 New Features ===
  Serial.println("=== minIni v1.5 Features ===");

  // Check if sections exist
  Serial.print("Has 'network' section: ");
  Serial.println(config.hassection("network") ? "Yes" : "No");

  Serial.print("Has 'bluetooth' section: ");
  Serial.println(config.hassection("bluetooth") ? "Yes" : "No");

  // Check if specific keys exist
  Serial.print("Has 'network.ip_address': ");
  Serial.println(config.haskey("network", "ip_address") ? "Yes" : "No");

  Serial.print("Has 'network.password': ");
  Serial.println(config.haskey("network", "password") ? "Yes" : "No");
  Serial.println();

  // === Section and Key Enumeration ===
  Serial.println("=== Configuration Structure ===");

  // Enumerate all sections
  for (int i = 0; ; i++) {
    std::string section = config.getsection(i);
    if (section.empty()) break;

    Serial.print("Section: "); Serial.println(section.c_str());

    // Enumerate keys in this section
    for (int j = 0; ; j++) {
      std::string key = config.getkey(section, j);
      if (key.empty()) break;

      Serial.print("  Key: "); Serial.println(key.c_str());
    }
  }
  Serial.println();

  // === Demonstration Complete ===
  Serial.println("=== Example Complete ===");
  Serial.println("minIniStorage successfully demonstrated!");
  Serial.println("Check your storage device - settings.ini has been created");

  Serial.println("*STOP*");
}

void loop() {
  // Nothing to do in loop
}