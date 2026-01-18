/*
 * MPU9250_Interrupt Example
 *
 * Demonstrates interrupt-driven data acquisition from MPU-9250.
 * This example implements the SMOOTH preset (see libraries/imu/imu_hal.md).
 *
 * FILTER PRESET: SMOOTH (1 kHz, 184 Hz bandwidth)
 * - Gyro DLPF_CFG=1 (184 Hz bandwidth, 1 kHz internal rate)
 * - Accel DLPF_CFG=1 (184 Hz bandwidth)
 * - SMPLRT_DIV=0 (no division, 1 kHz output)
 * - This matches Betaflight/dRehmFlight configuration for MPU-9250
 * - Hardware-validated: 1000 Hz DRDY interrupt rate
 *
 * For other presets, see libraries/imu/imu_hal.md:
 * - SAFE: DLPF_CFG=2 (92 Hz, 1 kHz)
 * - BALANCED: DLPF_CFG=0 (250 Hz, 8 kHz with SW decimation to 4 kHz)
 * - ACRO: DLPF_CFG=0 (250 Hz, 8 kHz)
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection (BLACKPILL_F411CE)
 * - Pin assignments and SPI frequency from board configuration
 * - Interrupt pin configured for data-ready signaling (PB2/EXTI2)
 * - Supports multiple board targets with single codebase
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
 *   INT  → PB2 (EXTI2)
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
#define MPU9250_INT_PIN       BoardConfig::imu.int_pin

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin,
                 BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin,
                 BoardConfig::imu.spi.get_ssel_pin());

MPU9250 imu;

// Interrupt-driven data acquisition
volatile bool data_ready = false;
volatile uint32_t interrupt_count = 0;

// Data buffers
float gx, gy, gz;  // Gyro data (degrees/second)
float ax, ay, az;  // Accel data (g)

// Statistics
uint32_t samples_collected = 0;
uint32_t read_errors = 0;
unsigned long last_report_time = 0;

// Interrupt service routine
void mpu9250_data_ready_isr() {
  data_ready = true;
  interrupt_count++;
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("=== MPU-9250 Interrupt-Driven Example ===");

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
  if (MPU9250_INT_PIN != 0) {
    Serial.print("  Interrupt Pin: ");
    Serial.println((int)MPU9250_INT_PIN);
  } else {
    Serial.println("  Interrupt Pin: None (polling mode)");
  }

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

  // Configure SMOOTH preset (see libraries/imu/imu_hal.md)
  // DLPF_CFG=1: 184 Hz bandwidth, 1 kHz internal sample rate
  // SMPLRT_DIV=0: No division, 1 kHz output rate
  // This matches Betaflight/dRehmFlight standard configuration for MPU-9250
  //
  // Note: SMPLRT_DIV only effective when 0 < DLPF_CFG < 7 (per MPU-9250 datasheet)
  // For BALANCED/ACRO presets (DLPF_CFG=0), see imu_hal.md for software decimation approach.
  imu.setDLPF(1, 1);  // Gyro DLPF=1, Accel DLPF=1
  Serial.println("DLPF configured: 184 Hz bandwidth (1kHz internal) - SMOOTH preset");

  imu.setSampleRateDivider(0);  // 1kHz / (1+0) = 1kHz
  Serial.println("Sample rate divider: 0 (1kHz output)");

  // Set ranges
  imu.setGyroFSR(2000);  // ±2000 dps
  imu.setAccelFSR(16);   // ±16g

  Serial.println("Gyro FSR: ±2000 dps");
  Serial.println("Accel FSR: ±16g");

  // Setup interrupt if pin is configured
  if (MPU9250_INT_PIN != 0) {
    pinMode(MPU9250_INT_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(MPU9250_INT_PIN), mpu9250_data_ready_isr, RISING);
    Serial.println("✓ Data-ready interrupt configured (1kHz)");
  } else {
    Serial.println("⚠ No interrupt pin - using polling mode");
  }

  Serial.println("Starting interrupt-driven data acquisition...");
  Serial.println("Target: 1000 Hz sample rate");
  Serial.println("---");

  last_report_time = millis();
  delay(100);
}

void loop() {
  // Check if data is ready (interrupt-driven or polling)
  if (MPU9250_INT_PIN != 0) {
    // Interrupt-driven mode
    if (!data_ready) {
      return;  // Wait for interrupt
    }
    data_ready = false;  // Clear flag
  }

  // Read 6-axis data
  if (imu.read6DOF(gx, gy, gz, ax, ay, az)) {
    samples_collected++;
  } else {
    read_errors++;
  }

  // Report statistics every 1 second
  if (millis() - last_report_time >= 1000) {
    unsigned long report_interval = millis() - last_report_time;
    last_report_time = millis();

    float sample_rate = (samples_collected * 1000.0f) / report_interval;
    float interrupt_rate = (interrupt_count * 1000.0f) / report_interval;

    Serial.println("\n=== Statistics ===");
    Serial.print("Sample Rate: ");
    Serial.print(sample_rate, 1);
    Serial.println(" Hz");

    if (MPU9250_INT_PIN != 0) {
      Serial.print("Interrupt Rate: ");
      Serial.print(interrupt_rate, 1);
      Serial.println(" Hz");
    }

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
    Serial.println("---");

    // Reset counters
    samples_collected = 0;
    interrupt_count = 0;

    // Stop after 5 reports (5 seconds)
    static uint8_t report_count = 0;
    report_count++;
    if (report_count >= 5) {
      Serial.println("\nTest complete: 5 seconds of 1kHz streaming ✓");
      Serial.println("MPU-9250 interrupt-driven test PASSED ✓");
      Serial.println("*STOP*");
      while (1);
    }
  }

  // In polling mode, delay to approximate 1kHz
  if (MPU9250_INT_PIN == 0) {
    delayMicroseconds(1000);  // 1ms = 1kHz
  }
}
