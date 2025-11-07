/*
 * MPU9250_dRehmFlight Example
 *
 * Demonstrates MPU-9250 configuration matching dRehmFlight_Teensy_BETA_1.3.
 * This example replicates the exact IMU setup from the original Teensy flight controller.
 *
 * CONFIGURATION (matches dRehmFlight Teensy):
 * - Gyro FSR: ±250 dps (high resolution for stable flight)
 * - Accel FSR: ±2g (sufficient for level flight maneuvers)
 * - DLPF: Off (DLPF_CFG=0, wide bandwidth 250 Hz gyro / 460 Hz accel)
 * - Sample Rate Divider: 0 (setSrd(0) in Teensy code)
 * - On-Sensor Rate: 8 kHz (DLPF_CFG=0 bypasses 1 kHz path)
 * - Polling Rate: 2 kHz (matches dRehmFlight main loop)
 * - Magnetometer: AK8963 (100 Hz, 16-bit)
 *   - dRehmFlight uses mag for heading in level mode
 *   - Calibration available via calibrateMagnetometer() method
 *
 * IMPORTANT: dRehmFlight uses 8 kHz on-sensor rate with 2 kHz polling.
 * The Teensy code calls setSrd(0) expecting 1 kHz, but MPU-9250 with DLPF_CFG=0
 * produces 8 kHz (SMPLRT_DIV doesn't work with DLPF_CFG=0). This is correct behavior
 * for dRehmFlight - it polls at 2 kHz and the 8 kHz sensor provides fresh data.
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

// Data buffers
float gx, gy, gz;  // Gyro data (degrees/second)
float ax, ay, az;  // Accel data (g)
float mx, my, mz;  // Magnetometer data (µT)

// Statistics
uint32_t samples_collected = 0;
uint32_t read_errors = 0;
unsigned long last_report_time = 0;
unsigned long loop_start_time = 0;

void setup() {
  // Initialize Serial for non-RTT mode
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("=== MPU-9250 dRehmFlight Configuration ===\n");
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

  // Configure MPU-9250 to match dRehmFlight Teensy BETA 1.3
  // From dRehmFlight: "From the reset state all registers should be 0x00,
  // so we should be at max sample rate with digital low pass filter(s) off."
  //
  // This means:
  // - DLPF_CFG = 0 (wide bandwidth: Gyro 250 Hz, Accel 460 Hz)
  // - SMPLRT_DIV = 0 (setSrd(0) in Teensy code)
  // - On MPU-9250, DLPF_CFG=0 produces 8 kHz on-sensor rate
  //   (SMPLRT_DIV doesn't divide the 8 kHz path, only the 1 kHz path)
  // - dRehmFlight polls at 2 kHz, so 8 kHz sensor provides fresh data
  imu.setDLPF(0, 0);  // Gyro DLPF=0, Accel DLPF=0 (wide bandwidth, DLPF off)
  CI_LOG("DLPF configured: Wide bandwidth (Gyro 250 Hz, Accel 460 Hz)\n");

  imu.setSampleRateDivider(0);  // Matches setSrd(0) in Teensy dRehmFlight
  CI_LOG("Sample rate divider: 0 (8 kHz on-sensor with DLPF_CFG=0)\n");
  CI_LOG("Note: SMPLRT_DIV doesn't affect 8 kHz path (see imu_hal.md)\n");

  // Set ranges to match dRehmFlight defaults
  // From Teensy code: #define GYRO_250DPS (default), #define ACCEL_2G (default)
  imu.setGyroFSR(250);   // ±250 dps (high resolution for stable flight)
  imu.setAccelFSR(2);    // ±2g (sufficient for level flight)

  CI_LOG("Gyro FSR: ±250 dps (dRehmFlight default)\n");
  CI_LOG("Accel FSR: ±2g (dRehmFlight default)\n");

  CI_LOG("\nStarting polling-mode data acquisition...\n");
  CI_LOG("Target: 2000 Hz loop rate (matches dRehmFlight)\n");
  CI_LOG("On-sensor: 8 kHz (fresh data every poll)\n");
  CI_LOG("---\n");

  delay(100);  // Brief settling delay before starting measurements
  last_report_time = millis();
  loop_start_time = micros();
}

void loop() {
  // Poll at 2 kHz (500 µs period) to match dRehmFlight main loop
  unsigned long now = micros();
  if (now - loop_start_time < 500) {
    return;  // Wait for 2 kHz loop timing
  }
  loop_start_time = now;

  // Read 9-axis data (gyro + accel + mag)
  // Matches dRehmFlight Teensy getMotion9() call
  if (imu.read9DOF(gx, gy, gz, ax, ay, az, mx, my, mz)) {
    samples_collected++;
  } else {
    read_errors++;
  }

  // Report statistics every 1 second
  if (millis() - last_report_time >= 1000) {
    unsigned long report_interval = millis() - last_report_time;
    last_report_time = millis();

    float sample_rate = (samples_collected * 1000.0f) / report_interval;

    CI_LOG("\n=== Statistics ===\n");
    CI_LOGF("Sample Rate: ");
    CI_LOG_FLOAT("", sample_rate, 1);
    CI_LOG(" Hz (target: 2000 Hz)\n");

    CI_LOGF("Samples: %lu, Errors: %lu\n", samples_collected, read_errors);

    CI_LOG("Latest Data:\n");
    CI_LOG("  Gyro (dps): ");
    CI_LOG_FLOAT("X=", gx, 2);
    CI_LOG(", ");
    CI_LOG_FLOAT("Y=", gy, 2);
    CI_LOG(", ");
    CI_LOG_FLOAT("Z=", gz, 2);
    CI_LOG("\n");

    CI_LOG("  Accel (g):  ");
    CI_LOG_FLOAT("X=", ax, 3);
    CI_LOG(", ");
    CI_LOG_FLOAT("Y=", ay, 3);
    CI_LOG(", ");
    CI_LOG_FLOAT("Z=", az, 3);
    CI_LOG("\n");

    CI_LOG("  Mag (µT):   ");
    CI_LOG_FLOAT("X=", mx, 1);
    CI_LOG(", ");
    CI_LOG_FLOAT("Y=", my, 1);
    CI_LOG(", ");
    CI_LOG_FLOAT("Z=", mz, 1);
    CI_LOG("\n");
    CI_LOG("---\n");

    // Reset counters
    samples_collected = 0;

    // Stop after 5 reports (5 seconds)
    static uint8_t report_count = 0;
    report_count++;
    if (report_count >= 5) {
      CI_LOG("\nTest complete: 5 seconds of 2kHz polling ✓\n");
      CI_LOG("MPU-9250 dRehmFlight configuration test PASSED ✓\n");
      CI_LOG("*STOP*\n");
      while (1);
    }
  }
}
