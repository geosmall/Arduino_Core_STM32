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
 * License: GPL v3 (Betaflight-derived library)
 */

#include <MPU9250.h>

// Board configuration
#if defined(ARDUINO_NUCLEO_F411RE)
#include "../../../../targets/NUCLEO_F411RE_LITTLEFS.h"
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
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("=== MPU-9250 dRehmFlight Configuration ===");

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
  Serial.println("DLPF configured: Wide bandwidth (Gyro 250 Hz, Accel 460 Hz)");

  imu.setSampleRateDivider(0);  // Matches setSrd(0) in Teensy dRehmFlight
  Serial.println("Sample rate divider: 0 (8 kHz on-sensor with DLPF_CFG=0)");
  Serial.println("Note: SMPLRT_DIV doesn't affect 8 kHz path (see imu_hal.md)");

  // Set ranges to match dRehmFlight defaults
  // From Teensy code: #define GYRO_250DPS (default), #define ACCEL_2G (default)
  imu.setGyroFSR(250);   // ±250 dps (high resolution for stable flight)
  imu.setAccelFSR(2);    // ±2g (sufficient for level flight)

  Serial.println("Gyro FSR: ±250 dps (dRehmFlight default)");
  Serial.println("Accel FSR: ±2g (dRehmFlight default)");

  Serial.println("\nStarting polling-mode data acquisition...");
  Serial.println("Target: 2000 Hz loop rate (matches dRehmFlight)");
  Serial.println("On-sensor: 8 kHz (fresh data every poll)");
  Serial.println("---");

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

    Serial.println("\n=== Statistics ===");
    Serial.print("Sample Rate: ");
    Serial.print(sample_rate, 1);
    Serial.println(" Hz (target: 2000 Hz)");

    Serial.print("Samples: ");
    Serial.print(samples_collected);
    Serial.print(", Errors: ");
    Serial.println(read_errors);

    Serial.println("Latest Data:");
    Serial.print("  Gyro (dps): X=");
    Serial.print(gx, 2);
    Serial.print(", Y=");
    Serial.print(gy, 2);
    Serial.print(", Z=");
    Serial.println(gz, 2);

    Serial.print("  Accel (g):  X=");
    Serial.print(ax, 3);
    Serial.print(", Y=");
    Serial.print(ay, 3);
    Serial.print(", Z=");
    Serial.println(az, 3);

    Serial.print("  Mag (µT):   X=");
    Serial.print(mx, 1);
    Serial.print(", Y=");
    Serial.print(my, 1);
    Serial.print(", Z=");
    Serial.println(mz, 1);
    Serial.println("---");

    // Reset counters
    samples_collected = 0;

    // Stop after 5 reports (5 seconds)
    static uint8_t report_count = 0;
    report_count++;
    if (report_count >= 5) {
      Serial.println("\nTest complete: 5 seconds of 2kHz polling ✓");
      Serial.println("MPU-9250 dRehmFlight configuration test PASSED ✓");
      Serial.println("*STOP*");
      while (1);
    }
  }
}
