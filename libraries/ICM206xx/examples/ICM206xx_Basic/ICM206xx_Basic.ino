/*
 * ICM206xx_Basic Example
 *
 * Demonstrates ICM-206xx family detection and basic 6-axis data reading.
 * Supports: ICM-20601, ICM-20602, ICM-20608, ICM-20689
 *
 * Hardware Setup - BKMN-NERO (STM32F7X2):
 *   ICM-20602 → NERO FC
 *   ------------------
 *   VCC  → 3.3V
 *   GND  → GND
 *   SCK  → PA5 (SPI1_SCK)
 *   MISO → PA6 (SPI1_MISO)
 *   MOSI → PA7 (SPI1_MOSI)
 *   CS   → PC4 (GPIO)
 *   INT  → PB2 (optional)
 *
 * License: GPL v3 (Betaflight-derived library)
 */

#include <ICM206xx.h>
#include "../../../../targets/BKMN-NERO.h"

// SPI configuration from BoardConfig
SPIClass spi_bus(BoardConfig::imu_spi.mosi_pin,
                 BoardConfig::imu_spi.miso_pin,
                 BoardConfig::imu_spi.sclk_pin);
#define ICM206xx_CS_PIN    BoardConfig::imu_spi.cs_pin
#define ICM206xx_SPI_FREQ  BoardConfig::imu_spi.freq_hz

ICM206xx imu;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("=== ICM-206xx Basic Test ===");

  // Initialize ICM-206xx
  Serial.println("Initializing ICM-206xx...");
  Serial.println("Board: BKMN-NERO (STM32F7X2)");
  Serial.print("IMU SPI: CS=");
  Serial.print(BoardConfig::imu_spi.cs_pin);
  Serial.print(", MOSI=");
  Serial.print(BoardConfig::imu_spi.mosi_pin);
  Serial.print(", MISO=");
  Serial.print(BoardConfig::imu_spi.miso_pin);
  Serial.print(", SCLK=");
  Serial.println(BoardConfig::imu_spi.sclk_pin);

  if (!imu.begin(spi_bus, ICM206xx_CS_PIN, ICM206xx_SPI_FREQ)) {
    Serial.println("ERROR: ICM-206xx initialization failed!");
    Serial.println("Check connections:");
    Serial.println("  - SPI MOSI, MISO, SCK");
    Serial.println("  - CS pin");
    Serial.println("  - 3.3V power");
    Serial.println("*STOP*");
    while (1);
  }

  Serial.println("ICM-206xx initialized successfully");

  // Read WHO_AM_I register
  uint8_t who_am_i = imu.whoAmI();
  Serial.print("WHO_AM_I: 0x");
  Serial.print(who_am_i, HEX);

  // Display detected chip
  const char* chip_name = imu.getChipName();
  Serial.print(" (");
  Serial.print(chip_name);
  Serial.println(" detected) ✓");

  // Verify chip variant
  ChipVariant variant = imu.getChipVariant();
  if (variant == ChipVariant::UNKNOWN) {
    Serial.println("WARNING: Unknown chip variant");
  }

  // Configure DLPF (250 Hz gyro, 218 Hz accel bandwidth)
  imu.setDLPF(0, 0);
  Serial.println("DLPF configured: Gyro 250 Hz, Accel 218 Hz");

  // Set ranges
  imu.setGyroFSR(2000);  // ±2000 dps
  imu.setAccelFSR(16);   // ±16g

  Serial.println("Gyro FSR: ±2000 dps");
  Serial.println("Accel FSR: ±16g");
  Serial.println("\nStarting data acquisition...");
  Serial.println("---");

  delay(100);
}

void loop() {
  static unsigned long last_print = 0;
  static uint32_t sample_count = 0;

  float gx, gy, gz;  // Gyro data (degrees/second)
  float ax, ay, az;  // Accel data (g)

  // Read 6-axis data
  if (imu.read6DOF(gx, gy, gz, ax, ay, az)) {
    sample_count++;

    // Print at 10 Hz
    if (millis() - last_print >= 100) {
      last_print = millis();

      Serial.print("Sample ");
      Serial.println(sample_count);

      // Print gyro data
      Serial.print("  Gyro (dps): X=");
      Serial.print(gx, 2);
      Serial.print(", Y=");
      Serial.print(gy, 2);
      Serial.print(", Z=");
      Serial.println(gz, 2);

      // Print accel data
      Serial.print("  Accel (g):  X=");
      Serial.print(ax, 3);
      Serial.print(", Y=");
      Serial.print(ay, 3);
      Serial.print(", Z=");
      Serial.println(az, 3);
      Serial.println("---");
    }

    // Stop after 50 samples (5 seconds at 10 Hz)
    if (sample_count >= 50) {
      Serial.print("\nTest complete: ");
      Serial.print(sample_count);
      Serial.println(" samples collected");
      Serial.print(imu.getChipName());
      Serial.println(" basic test PASSED ✓");
      Serial.println("*STOP*");
      while (1);
    }
  } else {
    Serial.println("ERROR: Failed to read IMU data");
  }

  delay(10);  // 100 Hz loop rate
}
