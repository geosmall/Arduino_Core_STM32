/*
 * ICM20689_Basic Example
 *
 * Demonstrates ICM-206xx family detection and basic 6-axis data reading.
 * Supports: ICM-20601, ICM-20602, ICM-20608, ICM-20689
 *
 * Hardware Setup - NUCLEO_F411RE_JHEF411:
 *   ICM-20602 → NUCLEO
 *   ------------------
 *   VCC  → 3.3V
 *   GND  → GND
 *   SCK  → PA5 (SPI1_SCK)
 *   MISO → PA6 (SPI1_MISO)
 *   MOSI → PA7 (SPI1_MOSI)
 *   CS   → PA4 (GPIO)
 *
 * Hardware Setup - Generic:
 *   ICM-206xx → STM32
 *   ------------------
 *   VCC  → 3.3V
 *   GND  → GND
 *   SCK  → SPI1_SCK
 *   MISO → SPI1_MISO
 *   MOSI → SPI1_MOSI
 *   CS   → PA4 (or configure below)
 *
 * License: GPL v3 (Betaflight-derived library)
 */

#include <ICM20689.h>
#include <ci_log.h>

// SPI configuration
// NUCLEO_F411RE_JHEF411: SPI1 (PA5=SCK, PA6=MISO, PA7=MOSI), CS=PA4
#if defined(ARDUINO_NUCLEO_F411RE)
  #define ICM20689_CS_PIN    PA4
#else
  #define ICM20689_CS_PIN    PA4
#endif
#define ICM20689_SPI_FREQ  1000000  // 1 MHz for detection, can go up to 8 MHz

ICM20689 imu;

void setup() {
  // Initialize Serial for non-RTT mode
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait for Serial with timeout
#endif

  CI_LOG("=== ICM-206xx Basic Test ===\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  // Initialize ICM-206xx
  CI_LOG("Initializing ICM-206xx...\n");

  if (!imu.begin(SPI, ICM20689_CS_PIN, ICM20689_SPI_FREQ)) {
    CI_LOG("ERROR: ICM-206xx initialization failed!\n");
    CI_LOG("Check connections:\n");
    CI_LOG("  - SPI MOSI, MISO, SCK\n");
    CI_LOG("  - CS pin\n");
    CI_LOG("  - 3.3V power\n");
    CI_LOG("*STOP*\n");
    while (1);
  }

  CI_LOG("ICM-206xx initialized successfully\n");

  // Read WHO_AM_I register
  uint8_t who_am_i = imu.whoAmI();
  CI_LOGF("WHO_AM_I: 0x%02X ", who_am_i);

  // Display detected chip
  const char* chip_name = imu.getChipName();
  CI_LOGF("(%s detected) ✓\n", chip_name);

  // Verify chip variant
  ChipVariant variant = imu.getChipVariant();
  if (variant == ChipVariant::UNKNOWN) {
    CI_LOG("WARNING: Unknown chip variant\n");
  }

  // Configure DLPF (250 Hz bandwidth, low latency)
  imu.setDLPF(0);
  CI_LOG("DLPF configured: 250 Hz\n");

  // Set ranges
  imu.setGyroFSR(2000);  // ±2000 dps
  imu.setAccelFSR(16);   // ±16g

  CI_LOG("Gyro FSR: ±2000 dps\n");
  CI_LOG("Accel FSR: ±16g\n");
  CI_LOG("\nStarting data acquisition...\n");
  CI_LOG("---\n");

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

      CI_LOGF("Sample %lu:\n", sample_count);

      // Print gyro data
      CI_LOG("  Gyro (dps): ");
      CI_LOG_FLOAT("X=", gx, 2);
      CI_LOG(", ");
      CI_LOG_FLOAT("Y=", gy, 2);
      CI_LOG(", ");
      CI_LOG_FLOAT("Z=", gz, 2);
      CI_LOG("\n");

      // Print accel data
      CI_LOG("  Accel (g):  ");
      CI_LOG_FLOAT("X=", ax, 3);
      CI_LOG(", ");
      CI_LOG_FLOAT("Y=", ay, 3);
      CI_LOG(", ");
      CI_LOG_FLOAT("Z=", az, 3);
      CI_LOG("\n");
      CI_LOG("---\n");
    }

    // Stop after 50 samples (5 seconds at 10 Hz)
    if (sample_count >= 50) {
      CI_LOGF("\nTest complete: %lu samples collected\n", sample_count);
      CI_LOGF("%s basic test PASSED ✓\n", imu.getChipName());
      CI_LOG("*STOP*\n");
      while (1);
    }
  } else {
    CI_LOG("ERROR: Failed to read IMU data\n");
  }

  delay(10);  // 100 Hz loop rate
}
