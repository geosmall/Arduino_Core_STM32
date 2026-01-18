/*
 * MPU9250_Basic Example
 *
 * Demonstrates MPU-9250/MPU-9255 detection and basic 6-axis data reading.
 * This is a simple polling-mode example for quick hardware verification.
 *
 * FILTER PRESET: Not aligned with standard presets (see libraries/imu/imu_hal.md)
 * - Uses DLPF_CFG=0 for wide bandwidth (250 Hz gyro, 460 Hz accel)
 * - Polling mode at 100 Hz (not interrupt-driven)
 * - For production use, see MPU9250_Interrupt (implements SMOOTH preset)
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection (BLACKPILL_F411CE)
 * - Pin assignments and SPI frequency from board configuration
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

  Serial.println("=== MPU-9250 Basic Test ===");

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

  // Configure DLPF (wide bandwidth for testing)
  // DLPF_CFG=0: Gyro=250Hz BW, Accel=460Hz BW
  // NOTE: This produces 8 kHz on-sensor rate (SMPLRT_DIV non-functional with DLPF_CFG=0)
  // This example polls at 100 Hz, so intermediate samples are ignored.
  // See libraries/imu/imu_hal.md for standard SAFE/SMOOTH/BALANCED/ACRO presets.
  imu.setDLPF(0, 0);
  Serial.println("DLPF configured: Gyro=250Hz, Accel=460Hz (wide bandwidth)");
  Serial.println("Note: On-sensor rate is 8kHz, but polling at 100Hz for this demo");

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
      Serial.print(sample_count);
      Serial.println(":");

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
      Serial.println("MPU-9250 basic test PASSED ✓");
      Serial.println("*STOP*");
      while (1);
    }
  } else {
    Serial.println("ERROR: Failed to read IMU data");
  }

  delay(10);  // 100 Hz loop rate
}
