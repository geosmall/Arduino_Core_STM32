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
 * License: GPL v3 (Betaflight-derived library)
 */

#include <MPU9250.h>

// Board configuration
#if defined(ARDUINO_NUCLEO_F411RE)
#include "../../../../targets/NUCLEO_F411RE_HIL001.h"
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
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("=== MPU-9250 Magnetometer Calibration ===");

  // Display pin configuration from BoardConfig
  Serial.println("Pin Configuration (BoardConfig):");
  Serial.print("  CS: ");
  Serial.print((int)MPU9250_CS_PIN);
  Serial.print(", MOSI: ");
  Serial.print((int)MPU9250_MOSI_PIN);
  Serial.print(", MISO: ");
  Serial.print((int)MPU9250_MISO_PIN);
  Serial.print(", SCLK: ");
  Serial.println((int)MPU9250_SCLK_PIN);
  Serial.print("  SPI Speed: ");
  Serial.print((unsigned long)MPU9250_SPI_FREQ);
  Serial.println(" Hz");

  // Initialize MPU-9250
  Serial.println("Initializing MPU-9250...");

  if (!imu.begin(spi_bus, MPU9250_CS_PIN, MPU9250_SPI_FREQ)) {
    Serial.println("ERROR: MPU-9250 initialization failed!");
    Serial.println("Check connections:");
    Serial.println("  - SPI MOSI, MISO, SCK");
    Serial.println("  - CS pin");
    Serial.println("  - 3.3V power");
    Serial.println("*STOP*");
    while (1);
  }

  Serial.println("MPU-9250 initialized successfully");

  // Read WHO_AM_I register
  uint8_t who_am_i = imu.whoAmI();
  Serial.print("WHO_AM_I: 0x");
  Serial.print(who_am_i, HEX);
  Serial.print(" ");

  if (who_am_i == 0x71) {
    Serial.println("(MPU-9250 detected) ✓");
  } else if (who_am_i == 0x73) {
    Serial.println("(MPU-9255 detected) ✓");
  } else {
    Serial.println("(Expected 0x71 or 0x73, detection may have failed)");
  }

  Serial.println("\n=== Magnetometer Calibration Procedure ===");
  Serial.println("This calibration corrects for:");
  Serial.println("  - Hard iron bias (constant magnetic offsets)");
  Serial.println("  - Soft iron distortion (non-uniform magnetic fields)");
  Serial.println();
  Serial.println("INSTRUCTIONS:");
  Serial.println("  1. When calibration starts, you have 15 seconds");
  Serial.println("  2. Rotate the IMU in a figure-8 pattern");
  Serial.println("  3. Move slowly and smoothly in ALL directions");
  Serial.println("  4. Try to cover a complete sphere of orientations");
  Serial.println("  5. Calibration will complete automatically");
  Serial.println();
  Serial.println("Starting calibration in 3 seconds...");
  delay(1000);
  Serial.println("3...");
  delay(1000);
  Serial.println("2...");
  delay(1000);
  Serial.println("1...");
  delay(1000);
  Serial.println("\n*** START FIGURE-8 MOTION NOW! ***\n");

  // Perform calibration
  unsigned long cal_start = millis();
  bool cal_success = imu.calibrateMagnetometer();
  unsigned long cal_duration = millis() - cal_start;

  if (!cal_success) {
    Serial.println("\nERROR: Calibration failed!");
    Serial.println("Check that magnetometer is working properly.");
    Serial.println("*STOP*");
    while (1);
  }

  Serial.println("\n=== Calibration Complete! ===");
  Serial.print("Duration: ");
  Serial.print(cal_duration / 1000);
  Serial.println(" seconds");
  Serial.println();

  // Get calibration values
  float bias_x, bias_y, bias_z;
  float scale_x, scale_y, scale_z;
  imu.getMagCalibration(bias_x, bias_y, bias_z, scale_x, scale_y, scale_z);

  // Display calibration values
  Serial.println("=== Calibration Values ===");
  Serial.println("Copy these values into your application:\n");
  Serial.println("// Magnetometer calibration");
  Serial.print("float mag_bias_x = ");
  Serial.print(bias_x, 2);
  Serial.println(";");
  Serial.print("float mag_bias_y = ");
  Serial.print(bias_y, 2);
  Serial.println(";");
  Serial.print("float mag_bias_z = ");
  Serial.print(bias_z, 2);
  Serial.println(";");
  Serial.print("float mag_scale_x = ");
  Serial.print(scale_x, 3);
  Serial.println(";");
  Serial.print("float mag_scale_y = ");
  Serial.print(scale_y, 3);
  Serial.println(";");
  Serial.print("float mag_scale_z = ");
  Serial.print(scale_z, 3);
  Serial.println(";\n");
  Serial.println("// In setup():");
  Serial.println("imu.setMagCalibration(mag_bias_x, mag_bias_y, mag_bias_z,");
  Serial.println("                      mag_scale_x, mag_scale_y, mag_scale_z);\n");

  // Verify calibration by reading a few samples
  Serial.println("=== Verification (5 samples) ===");
  for (int i = 0; i < 5; i++) {
    float mx, my, mz;
    if (imu.readMagnetometer(mx, my, mz)) {
      Serial.print("Sample ");
      Serial.print(i + 1);
      Serial.print(": X=");
      Serial.print(mx, 1);
      Serial.print(", Y=");
      Serial.print(my, 1);
      Serial.print(", Z=");
      Serial.print(mz, 1);
      Serial.println(" µT");
    }
    delay(100);
  }

  Serial.println("\nCalibration complete! ✓");
  Serial.println("*STOP*");
}

void loop() {
  // Calibration is done in setup()
  delay(1000);
}
