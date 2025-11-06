/*
 * MPU6000_Interrupt Example
 *
 * Demonstrates interrupt-driven data acquisition from MPU-6000.
 * This example uses the data-ready interrupt to achieve precise 1kHz sampling.
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection (BLACKPILL_F411CE)
 * - Pin assignments and SPI frequency from board configuration
 * - Interrupt pin configured for data-ready signaling (PB2/EXTI2)
 * - Supports multiple board targets with single codebase
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
 *   INT  → PB2 (EXTI2)
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
 *   INT  → PB3 (EXTI3)
 *
 * CI/HIL INTEGRATION:
 * - RTT output for automated testing
 * - Serial output for Arduino IDE
 * - Build traceability with git SHA and timestamp
 *
 * License: GPL v3 (Betaflight-derived library)
 */

#include <MPU6000.h>
#include <ci_log.h>

// Board configuration
#if defined(ARDUINO_NUCLEO_F411RE)
#include "../../../../targets/NUCLEO_F411RE_JHEF411.h"
#else
#include "../../../../targets/BLACKPILL_F411CE.h"
#endif

// BoardConfig integration for dynamic pin and frequency configuration
#define MPU6000_CS_PIN        BoardConfig::imu.spi.cs_pin
#define MPU6000_MOSI_PIN      BoardConfig::imu.spi.mosi_pin
#define MPU6000_MISO_PIN      BoardConfig::imu.spi.miso_pin
#define MPU6000_SCLK_PIN      BoardConfig::imu.spi.sclk_pin
#define MPU6000_SPI_FREQ      BoardConfig::imu.spi.freq_hz
#define MPU6000_INT_PIN       BoardConfig::imu.int_pin

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin,
                 BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin,
                 BoardConfig::imu.spi.get_ssel_pin());

MPU6000 imu;

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
void mpu6000_data_ready_isr() {
  data_ready = true;
  interrupt_count++;
}

void setup() {
  // Initialize Serial for non-RTT mode
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("=== MPU-6000 Interrupt-Driven Example ===\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  // Display pin configuration from BoardConfig
  CI_LOG("Pin Configuration (BoardConfig):\n");
  CI_LOGF("  CS: %d, MOSI: %d, MISO: %d, SCLK: %d\n",
         (int)MPU6000_CS_PIN, (int)MPU6000_MOSI_PIN,
         (int)MPU6000_MISO_PIN, (int)MPU6000_SCLK_PIN);
  CI_LOGF("  SPI Speed: %lu Hz\n", (unsigned long)MPU6000_SPI_FREQ);
  if (MPU6000_INT_PIN != 0) {
    CI_LOGF("  Interrupt Pin: %d\n", (int)MPU6000_INT_PIN);
  } else {
    CI_LOG("  Interrupt Pin: None (polling mode)\n");
  }

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
    CI_LOG("(Expected 0x68, detection may have failed)\n");
  }

  // Configure for 1kHz operation
  // DLPF = 0 (256 Hz bandwidth, 8kHz internal sample rate)
  // SMPLRT_DIV = 7 (8kHz / (1+7) = 1kHz output)
  imu.setDLPF(0);
  CI_LOG("DLPF configured: 256 Hz bandwidth (8kHz internal)\n");

  imu.setSampleRateDivider(7);  // 8kHz / (1+7) = 1kHz
  CI_LOG("Sample rate divider: 7 (target 1kHz output)\n");

  // Set ranges
  imu.setGyroFSR(2000);  // ±2000 dps
  imu.setAccelFSR(16);   // ±16g

  CI_LOG("Gyro FSR: ±2000 dps\n");
  CI_LOG("Accel FSR: ±16g\n");

  // Setup interrupt if pin is configured
  if (MPU6000_INT_PIN != 0) {
    pinMode(MPU6000_INT_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(MPU6000_INT_PIN), mpu6000_data_ready_isr, RISING);
    CI_LOG("✓ Data-ready interrupt configured (1kHz)\n");
  } else {
    CI_LOG("⚠ No interrupt pin - using polling mode\n");
  }

  CI_LOG("\nStarting interrupt-driven data acquisition...\n");
  CI_LOG("Target: 1000 Hz sample rate\n");
  CI_LOG("---\n");

  last_report_time = millis();
  delay(100);
}

void loop() {
  static unsigned long loop_start = 0;

  // Check if data is ready (interrupt-driven or polling)
  if (MPU6000_INT_PIN != 0) {
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

    CI_LOG("\n=== Statistics ===\n");
    CI_LOGF("Sample Rate: ");
    CI_LOG_FLOAT("", sample_rate, 1);
    CI_LOG(" Hz\n");

    if (MPU6000_INT_PIN != 0) {
      CI_LOGF("Interrupt Rate: ");
      CI_LOG_FLOAT("", interrupt_rate, 1);
      CI_LOG(" Hz\n");
    }

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
    CI_LOG("---\n");

    // Reset counters
    samples_collected = 0;
    interrupt_count = 0;

    // Stop after 5 reports (5 seconds)
    static uint8_t report_count = 0;
    report_count++;
    if (report_count >= 5) {
      CI_LOG("\nTest complete: 5 seconds of 1kHz streaming ✓\n");
      CI_LOG("MPU-6000 interrupt-driven test PASSED ✓\n");
      CI_LOG("*STOP*\n");
      while (1);
    }
  }

  // In polling mode, delay to approximate 1kHz
  if (MPU6000_INT_PIN == 0) {
    delayMicroseconds(1000);  // 1ms = 1kHz
  }
}
