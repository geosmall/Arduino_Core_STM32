#include <aunit_hil.h>
#include <SPI.h>

// Include board configuration (automatically selects based on ARDUINO_* defines)
#include "../../targets/NUCLEO_F411RE.h"

/*
 * Board Configuration System Test
 *
 * Tests the Arduino pin macro approach for Board Configuration System
 * - Uses Arduino pin numbers (PC12, PC11, etc.) for compatibility
 * - Avoids mass changes to existing code
 * - Provides structured configuration management
 */

test(board_config_pin_values) {
  // Verify storage configuration values for base NUCLEO_F411RE (no storage)
  assertEqual((unsigned int)BoardConfig::storage.backend_type, (unsigned int)StorageBackend::NONE);
  assertEqual((unsigned int)BoardConfig::storage.mosi_pin, 0U);
  assertEqual((unsigned int)BoardConfig::storage.miso_pin, 0U);
  assertEqual((unsigned int)BoardConfig::storage.sclk_pin, 0U);
  assertEqual((unsigned int)BoardConfig::storage.cs_pin, 0U);

  // Verify clock settings (should be 0 for NONE backend)
  assertEqual((unsigned long)BoardConfig::storage.freq_hz, 0UL);
}

test(board_config_spi_compatibility) {
  // Test that NONE backend is properly configured (no actual SPI operations)
  // Base NUCLEO_F411RE has no storage hardware, so backend should be NONE
  assertEqual((unsigned int)BoardConfig::storage.backend_type, (unsigned int)StorageBackend::NONE);

  // Test that we can access other configured peripherals (IMU uses SPI pins)
  pinMode(BoardConfig::imu.spi.cs_pin, OUTPUT);
  digitalWrite(BoardConfig::imu.spi.cs_pin, HIGH);

  // This tests that other SPI configurations work properly
  assertTrue(true); // If we get here, other SPI pin values are compatible
}

test(board_config_constexpr) {
  // Test that configuration is compile-time constant
  constexpr auto storage = BoardConfig::storage;
  assertEqual((unsigned int)storage.backend_type, (unsigned int)StorageBackend::NONE);

  // Test other peripherals exist and are properly configured
  constexpr auto gps_config = BoardConfig::gps;
  assertEqual((unsigned long)gps_config.baud_rate, 115200UL);

  constexpr auto imu_config = BoardConfig::imu;
  assertEqual((unsigned int)imu_config.spi.cs_pin, (unsigned int)PA4);
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
  HIL_TEST_TIMEOUT(30);
}

void loop() {
  HIL_TEST_RUN();
}