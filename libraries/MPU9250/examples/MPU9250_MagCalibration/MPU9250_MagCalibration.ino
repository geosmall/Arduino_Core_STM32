/*
 * MPU9250_MagCalibration Example
 *
 * Demonstrates magnetometer calibration for the MPU-9250 AK8963.
 * This example helps you determine hard iron bias and soft iron scale factors
 * to compensate for magnetic distortions.
 *
 * CALIBRATION PROCEDURE:
 * 1. Upload this sketch to your board
 * 2. When prompted, rotate the IMU in a figure-8 pattern for 15 seconds
 * 3. Move slowly and smoothly in all directions (pitch, roll, yaw)
 * 4. Try to cover a full sphere of orientations
 * 5. The sketch will display calibration values at the end
 * 6. Copy these values into your application code
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection
 * - Pin assignments and SPI frequency from board configuration
 * - Supports NUCLEO_F411RE and BLACKPILL_F411CE
 *
 * Hardware Setup - BLACKPILL_F411CE:
 *   MPU-9250 → Blackpill
 *   -----------------
 *   VCC  → 3.3V
 *   GND  → GND
 *   SCK  → PB13 (SPI2_SCK)
 *   MISO → PB14 (SPI2_MISO)
 *   MOSI → PB15 (SPI2_MOSI)
 *   CS   → PB12 (GPIO)
 *
 * CI/HIL INTEGRATION:
 * - RTT output for automated testing
 * - Serial output for Arduino IDE
 * - Build traceability with git SHA and timestamp
 *
 * License: GPL v3 (Betaflight-derived library)
 */

#include <MPU9250.h>
#include <ci_log.h>

// Board configuration
#if defined(ARDUINO_NUCLEO_F411RE)
#include "../../../../targets/NUCLEO_F411RE_JHEF411.h"
#else
#include "../../../../targets/BLACKPILL_F411CE.h"
#endif

// BoardConfig integration for dynamic pin and frequency configuration
#define MPU9250_CS_PIN        BoardConfig::imu.spi.cs_pin
#define MPU9250_MOSI_PIN      BoardConfig::imu.spi.mosi_pin
#define MPU9250_MISO_PIN      BoardConfig::imu.spi.miso_pin
#define MPU9250_SCLK_PIN      BoardConfig::imu.spi.sclk_pin
#define MPU9250_SPI_FREQ      BoardConfig::imu.spi.freq_hz

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin,
                 BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin,
                 BoardConfig::imu.spi.get_ssel_pin());

MPU9250 imu;

void setup() {
  // Initialize Serial for non-RTT mode
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("=== MPU-9250 Magnetometer Calibration ===\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  // Display pin configuration from BoardConfig
  CI_LOG("Pin Configuration (BoardConfig):\n");
  CI_LOGF("  CS: %d, MOSI: %d, MISO: %d, SCLK: %d\n",
         (int)MPU9250_CS_PIN, (int)MPU9250_MOSI_PIN,
         (int)MPU9250_MISO_PIN, (int)MPU9250_SCLK_PIN);
  CI_LOGF("  SPI Speed: %lu Hz\n", (unsigned long)MPU9250_SPI_FREQ);

  // Initialize MPU-9250
  CI_LOG("\nInitializing MPU-9250...\n");

  if (!imu.begin(spi_bus, MPU9250_CS_PIN, MPU9250_SPI_FREQ)) {
    CI_LOG("ERROR: MPU-9250 initialization failed!\n");
    CI_LOG("Check connections:\n");
    CI_LOG("  - SPI MOSI, MISO, SCK\n");
    CI_LOG("  - CS pin\n");
    CI_LOG("  - 3.3V power\n");
    CI_LOG("*STOP*\n");
    while (1);
  }

  CI_LOG("MPU-9250 initialized successfully\n");

  // Read WHO_AM_I register
  uint8_t who_am_i = imu.whoAmI();
  CI_LOGF("WHO_AM_I: 0x%02X ", who_am_i);

  if (who_am_i == 0x71) {
    CI_LOG("(MPU-9250 detected) ✓\n");
  } else if (who_am_i == 0x73) {
    CI_LOG("(MPU-9255 detected) ✓\n");
  } else {
    CI_LOG("(Expected 0x71 or 0x73, detection may have failed)\n");
  }

  CI_LOG("\n=== Magnetometer Calibration Procedure ===\n");
  CI_LOG("This calibration corrects for:\n");
  CI_LOG("  - Hard iron bias (constant magnetic offsets)\n");
  CI_LOG("  - Soft iron distortion (non-uniform magnetic fields)\n");
  CI_LOG("\n");
  CI_LOG("INSTRUCTIONS:\n");
  CI_LOG("  1. When calibration starts, you have 15 seconds\n");
  CI_LOG("  2. Rotate the IMU in a figure-8 pattern\n");
  CI_LOG("  3. Move slowly and smoothly in ALL directions\n");
  CI_LOG("  4. Try to cover a complete sphere of orientations\n");
  CI_LOG("  5. Calibration will complete automatically\n");
  CI_LOG("\n");
  CI_LOG("Press any key to start calibration...\n");

  // Wait for user input (skip in RTT mode)
#ifndef USE_RTT
  while (!Serial.available()) {
    delay(100);
  }
  while (Serial.available()) {
    Serial.read();  // Clear buffer
  }
#else
  delay(3000);  // 3 second delay in RTT mode
#endif

  CI_LOG("\nStarting calibration in 3 seconds...\n");
  delay(1000);
  CI_LOG("3...\n");
  delay(1000);
  CI_LOG("2...\n");
  delay(1000);
  CI_LOG("1...\n");
  delay(1000);
  CI_LOG("\n*** START FIGURE-8 MOTION NOW! ***\n\n");

  // Perform calibration
  unsigned long cal_start = millis();
  bool cal_success = imu.calibrateMagnetometer();
  unsigned long cal_duration = millis() - cal_start;

  if (!cal_success) {
    CI_LOG("\nERROR: Calibration failed!\n");
    CI_LOG("Check that magnetometer is working properly.\n");
    CI_LOG("*STOP*\n");
    while (1);
  }

  CI_LOG("\n=== Calibration Complete! ===\n");
  CI_LOGF("Duration: %lu seconds\n", cal_duration / 1000);
  CI_LOG("\n");

  // Get calibration values
  float bias_x, bias_y, bias_z;
  float scale_x, scale_y, scale_z;
  imu.getMagCalibration(bias_x, bias_y, bias_z, scale_x, scale_y, scale_z);

  // Display calibration values
  CI_LOG("=== Calibration Values ===\n");
  CI_LOG("Copy these values into your application:\n\n");
  CI_LOG("// Magnetometer calibration\n");
  CI_LOG("float mag_bias_x = ");
  CI_LOG_FLOAT("", bias_x, 2);
  CI_LOG(";\n");
  CI_LOG("float mag_bias_y = ");
  CI_LOG_FLOAT("", bias_y, 2);
  CI_LOG(";\n");
  CI_LOG("float mag_bias_z = ");
  CI_LOG_FLOAT("", bias_z, 2);
  CI_LOG(";\n");
  CI_LOG("float mag_scale_x = ");
  CI_LOG_FLOAT("", scale_x, 3);
  CI_LOG(";\n");
  CI_LOG("float mag_scale_y = ");
  CI_LOG_FLOAT("", scale_y, 3);
  CI_LOG(";\n");
  CI_LOG("float mag_scale_z = ");
  CI_LOG_FLOAT("", scale_z, 3);
  CI_LOG(";\n\n");
  CI_LOG("// In setup():\n");
  CI_LOG("imu.setMagCalibration(mag_bias_x, mag_bias_y, mag_bias_z,\n");
  CI_LOG("                      mag_scale_x, mag_scale_y, mag_scale_z);\n\n");

  // Verify calibration by reading a few samples
  CI_LOG("=== Verification (5 samples) ===\n");
  for (int i = 0; i < 5; i++) {
    float mx, my, mz;
    if (imu.readMagnetometer(mx, my, mz)) {
      CI_LOGF("Sample %d: ", i + 1);
      CI_LOG_FLOAT("X=", mx, 1);
      CI_LOG(", ");
      CI_LOG_FLOAT("Y=", my, 1);
      CI_LOG(", ");
      CI_LOG_FLOAT("Z=", mz, 1);
      CI_LOG(" µT\n");
    }
    delay(100);
  }

  CI_LOG("\nCalibration complete! ✓\n");
  CI_LOG("*STOP*\n");
}

void loop() {
  // Calibration is done in setup()
  delay(1000);
}
