#include <ICM42688P_Simple.h>
#include <SPI.h>

// Board configuration
#if defined(ARDUINO_BLACKPILL_F411CE)
#include "../../../../targets/BLACKPILL_F411CE.h"
#else
#include "../../../../targets/NUCLEO_F411RE_LITTLEFS.h"
#endif


// Create IMU instance
ICM42688P_Simple imu;

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi(BoardConfig::imu.spi.mosi_pin,
             BoardConfig::imu.spi.miso_pin,
             BoardConfig::imu.spi.sclk_pin,
             BoardConfig::imu.spi.get_ssel_pin());

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("\n=== ICM42688P Minimal Test ===");
  Serial.println("Testing SPI communication and WHO_AM_I register\n");

  // Print pin configuration
  Serial.println("Pin Configuration:");
  Serial.println("  CS (Chip Select): PA4 (Software Control)");
  Serial.println("  MOSI (Master Out): PA7");
  Serial.println("  MISO (Master In): PA6");
  Serial.println("  SCLK (Clock): PA5");
  Serial.printf("  SPI Speed: %lu kHz\n\n", BoardConfig::imu.spi.freq_hz / 1000);

  // Initialize the IMU
  Serial.println("Initializing SPI and ICM42688P...");

  // Initialize ICM42688P with software CS control
  // Parameters: (SPI_instance, CS_pin_for_manual_control, SPI_frequency_Hz)
  if (!imu.begin(spi, BoardConfig::imu.spi.cs_pin, BoardConfig::imu.spi.freq_hz)) {
    Serial.println("ERROR: Failed to initialize ICM42688P!");
    Serial.println("*STOP*");
    while (1) {
      delay(1000);
    }
  }

  Serial.println("SPI initialization complete.\n");

  // Test WHO_AM_I register
  Serial.println("Reading WHO_AM_I register (0x75)...");
  uint8_t who_am_i = imu.readWhoAmI();

  Serial.printf("WHO_AM_I value: 0x%02X (%d decimal)\n", who_am_i, who_am_i);

  // Check if device ID matches expected value
  if (who_am_i == 0x47) {
    Serial.println("✓ SUCCESS: Device ID matches ICM42688P (0x47)");
    Serial.println("✓ SPI communication is working correctly!");
  } else if (who_am_i == 0x00 || who_am_i == 0xFF) {
    Serial.println("✗ ERROR: SPI communication issue");
    Serial.println("  - Check wiring connections");
    Serial.println("  - Verify power supply to IMU");
    Serial.println("  - Check SPI pin assignments");
  } else {
    Serial.println("✗ WARNING: Unexpected device ID");
    Serial.println("  - Device may not be ICM42688P");
    Serial.println("  - Check part number and datasheet");
  }

  Serial.println("\n=== Test Complete ===");
  Serial.println("Entering monitoring loop...\n");
}

void loop() {
  static unsigned long last_check = 0;
  static int check_count = 0;

  // Check WHO_AM_I every 2 seconds
  if (millis() - last_check > 2000) {
    check_count++;

    uint8_t who_am_i = imu.readWhoAmI();
    Serial.printf("Check #%d: WHO_AM_I = 0x%02X", check_count, who_am_i);

    if (imu.isConnected()) {
      Serial.println(" ✓ OK");
    } else {
      Serial.println(" ✗ FAIL");
    }

    // Exit after 5 checks for HIL testing
    if (check_count >= 5) {
      Serial.println("\n=== Monitoring Complete ===");
      Serial.println("*STOP*");
      while (1) delay(1000);
    }

    last_check = millis();
  }

  delay(100);
}
