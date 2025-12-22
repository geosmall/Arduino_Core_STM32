/*
 * MPU6000_Basic Example
 *
 * Demonstrates MPU-6000 detection and basic 6-axis data reading.
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection (BLACKPILL_F411CE)
 * - Pin assignments and SPI frequency from board configuration
 *
 * Hardware Setup - BLACKPILL_F411CE:
 *   MPU-6000 → Blackpill
 *   -----------------
 *   VCC  → 3.3V
 *   GND  → GND
 *   SCK  → PB13 (SPI2_SCK)
 *   MISO → PB14 (SPI2_MISO)
 *   MOSI → PB15 (SPI2_MOSI)
 *   CS   → PB12 (GPIO)
 *   INT  → PB2 (Optional - interrupt pin)
 *
 * Hardware Setup - NUCLEO_F411RE:
 *   MPU-6000 → NUCLEO
 *   -----------------
 *   VCC  → 3.3V
 *   GND  → GND
 *   SCK  → PA5 (SPI1_SCK)
 *   MISO → PA6 (SPI1_MISO)
 *   MOSI → PA7 (SPI1_MOSI)
 *   CS   → PA4 (GPIO)
 *
 * Hardware Setup - OPEN_REVO (OpenPilot Revolution):
 *   On-board MPU-6000/MPU-6500 on SPI1
 *   -----------------
 *   SCK  → PA5 (SPI1_SCK)
 *   MISO → PA6 (SPI1_MISO)
 *   MOSI → PA7 (SPI1_MOSI)
 *   CS   → PA4 (GPIO)
 *   EXTI → PC4 (Interrupt)
 *
 * License: GPL v3 (Betaflight-derived library)
 */

#include <MPU6000.h>
#include <ci_log.h>

// Board configuration
#if defined(ARDUINO_NUCLEO_F411RE)
  #include "../../../../targets/NUCLEO_F411RE_JHEF411.h"
#elif defined(ARDUINO_OPEN_REVO)
  #include "../../../../targets/OPEN-REVO.h"
#else
  #include "../../../../targets/BLACKPILL_F411CE.h"
#endif

// BoardConfig integration for dynamic pin and frequency configuration
#define MPU6000_CS_PIN        BoardConfig::imu.spi.cs_pin
#define MPU6000_MOSI_PIN      BoardConfig::imu.spi.mosi_pin
#define MPU6000_MISO_PIN      BoardConfig::imu.spi.miso_pin
#define MPU6000_SCLK_PIN      BoardConfig::imu.spi.sclk_pin
#define MPU6000_SPI_FREQ      BoardConfig::imu.spi.freq_hz

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin,
                 BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin,
                 BoardConfig::imu.spi.get_ssel_pin());

MPU6000 imu;

void setup() {
  // Initialize Serial for non-RTT mode
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait for Serial with timeout
#endif

  CI_LOG("=== MPU-6000 Basic Test ===\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  // Display pin configuration from BoardConfig
  CI_LOG("Pin Configuration (BoardConfig):\n");
  CI_LOGF("  CS: %d, MOSI: %d, MISO: %d, SCLK: %d\n",
         (int)MPU6000_CS_PIN, (int)MPU6000_MOSI_PIN,
         (int)MPU6000_MISO_PIN, (int)MPU6000_SCLK_PIN);
  CI_LOGF("  SPI Speed: %lu Hz\n", (unsigned long)MPU6000_SPI_FREQ);

  // Initialize MPU-6000
  CI_LOG("\nInitializing MPU-6000...\n");

  if (!imu.begin(spi_bus, MPU6000_CS_PIN, MPU6000_SPI_FREQ)) {
    CI_LOG("ERROR: MPU-6000 initialization failed!\n");
    CI_LOG("Check connections:\n");
    CI_LOG("  - SPI MOSI, MISO, SCK\n");
    CI_LOG("  - CS pin\n");
    CI_LOG("  - 3.3V power\n");
    CI_LOG("*STOP*\n");
    while (1);
  }

  CI_LOG("MPU-6000 initialized successfully\n");

  // Read WHO_AM_I register
  uint8_t who_am_i = imu.whoAmI();
  CI_LOGF("WHO_AM_I: 0x%02X ", who_am_i);

  if (who_am_i == 0x68) {
    CI_LOG("(MPU-6000 detected) ✓\n");
  } else {
    CI_LOGF("(Expected 0x68, detection may have failed)\n");
  }

  // Configure DLPF (256 Hz bandwidth, low latency)
  imu.setDLPF(0);
  CI_LOG("DLPF configured: 256 Hz (gyro/accel)\n");

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
      CI_LOG("MPU-6000 basic test PASSED ✓\n");
      CI_LOG("*STOP*\n");
      while (1);
    }
  } else {
    CI_LOG("ERROR: Failed to read IMU data\n");
  }

  delay(10);  // 100 Hz loop rate
}
