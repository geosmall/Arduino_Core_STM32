#include <ICM42688P_Simple.h>
#include <SPI.h>
#include <ci_log.h>

// Board configuration
#if defined(ARDUINO_BLACKPILL_F411CE)
#include "../../../../targets/BLACKPILL_F411CE.h"
#else
#include "../../../../targets/NUCLEO_F411RE_JHEF411.h"
#endif


// Create IMU instance
ICM42688P_Simple imu;

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi(BoardConfig::imu.spi.mosi_pin,
             BoardConfig::imu.spi.miso_pin,
             BoardConfig::imu.spi.sclk_pin,
             BoardConfig::imu.spi.get_ssel_pin());

void setup() {
  // Initialize communication (Serial or RTT)
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial) delay(10);
#endif

  CI_LOG("\n=== ICM42688P Minimal Test ===\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();
  CI_LOG("Testing SPI communication and WHO_AM_I register\n\n");

  // Print pin configuration
  CI_LOG("Pin Configuration:\n");
  CI_LOG("  CS (Chip Select): PA4 (Software Control)\n");
  CI_LOG("  MOSI (Master Out): PA7\n");
  CI_LOG("  MISO (Master In): PA6\n");
  CI_LOG("  SCLK (Clock): PA5\n");
  CI_LOGF("  SPI Speed: %lu kHz\n\n", BoardConfig::imu.spi.freq_hz / 1000);

  // Initialize the IMU
  CI_LOG("Initializing SPI and ICM42688P...\n");

  // Initialize ICM42688P with software CS control
  // Parameters: (SPI_instance, CS_pin_for_manual_control, SPI_frequency_Hz)
  if (!imu.begin(spi, BoardConfig::imu.spi.cs_pin, BoardConfig::imu.spi.freq_hz)) {
    CI_LOG("ERROR: Failed to initialize ICM42688P!\n");
    CI_LOG("*STOP*\n");
    while (1) {
      delay(1000);
    }
  }

  CI_LOG("SPI initialization complete.\n\n");

  // Test WHO_AM_I register
  CI_LOG("Reading WHO_AM_I register (0x75)...\n");
  uint8_t who_am_i = imu.readWhoAmI();

  CI_LOGF("WHO_AM_I value: 0x%02X (%d decimal)\n", who_am_i, who_am_i);

  // Check if device ID matches expected value
  if (who_am_i == 0x47) {
    CI_LOG("✓ SUCCESS: Device ID matches ICM42688P (0x47)\n");
    CI_LOG("✓ SPI communication is working correctly!\n");
  } else if (who_am_i == 0x00 || who_am_i == 0xFF) {
    CI_LOG("✗ ERROR: SPI communication issue\n");
    CI_LOG("  - Check wiring connections\n");
    CI_LOG("  - Verify power supply to IMU\n");
    CI_LOG("  - Check SPI pin assignments\n");
  } else {
    CI_LOG("✗ WARNING: Unexpected device ID\n");
    CI_LOG("  - Device may not be ICM42688P\n");
    CI_LOG("  - Check part number and datasheet\n");
  }

  CI_LOG("\n=== Test Complete ===\n");
  CI_LOG("Entering monitoring loop...\n\n");
}

void loop() {
  static unsigned long last_check = 0;
  static int check_count = 0;

  // Check WHO_AM_I every 2 seconds
  if (millis() - last_check > 2000) {
    check_count++;

    uint8_t who_am_i = imu.readWhoAmI();
    CI_LOGF("Check #%d: WHO_AM_I = 0x%02X", check_count, who_am_i);

    if (imu.isConnected()) {
      CI_LOG(" ✓ OK\n");
    } else {
      CI_LOG(" ✗ FAIL\n");
    }

    // Exit after 5 checks for HIL testing
    if (check_count >= 5) {
      CI_LOG("\n=== Monitoring Complete ===\n");
      CI_LOG("*STOP*\n");
      while (1) delay(1000);
    }

    last_check = millis();
  }

  delay(100);
}