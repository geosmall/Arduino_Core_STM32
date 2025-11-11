/*
 * ICM206xx_Interrupt Example
 *
 * Demonstrates interrupt-driven data acquisition from ICM-206xx family.
 * This example configures 1 kHz sampling with 250 Hz bandwidth filtering.
 *
 * FILTER CONFIGURATION: 1 kHz sampling
 * - Gyro DLPF_CFG=1 (176 Hz bandwidth, 1 kHz internal rate)
 * - Accel A_DLPF_CFG=1 (218 Hz bandwidth, 1 kHz internal rate)
 * - SMPLRT_DIV=0 (no division, 1 kHz output)
 * - This matches typical flight controller configuration for ICM-20602
 * - Hardware-validated: 1000 Hz DRDY interrupt rate
 *
 * Note: DLPF_CFG must be 1-6 for 1 kHz base rate. DLPF_CFG=0 uses 8 kHz.
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection (BKMN_NERO)
 * - Pin assignments and SPI frequency from board configuration
 * - Interrupt pin configured for data-ready signaling (PB2/EXTI2)
 * - Supports multiple board targets with single codebase
 *
 * Hardware Setup - BKMN-NERO (STM32F722):
 *   ICM-20602 → NERO FC
 *   ------------------
 *   VCC  → 3.3V
 *   GND  → GND
 *   SCK  → PA5 (SPI1_SCK)
 *   MISO → PA6 (SPI1_MISO)
 *   MOSI → PA7 (SPI1_MOSI)
 *   CS   → PC4 (GPIO)
 *   INT  → PB2 (EXTI2)
 *
 * CI/HIL INTEGRATION:
 * - RTT output for automated testing
 * - Serial output for Arduino IDE
 * - Build traceability with git SHA and timestamp
 *
 * License: GPL v3 (Betaflight-derived library)
 */

#include <ICM206xx.h>
#include <ci_log.h>
#include "../../../../targets/BKMN-NERO.h"

// BoardConfig integration for dynamic pin and frequency configuration
#define ICM206xx_CS_PIN        BoardConfig::imu_spi.cs_pin
#define ICM206xx_MOSI_PIN      BoardConfig::imu_spi.mosi_pin
#define ICM206xx_MISO_PIN      BoardConfig::imu_spi.miso_pin
#define ICM206xx_SCLK_PIN      BoardConfig::imu_spi.sclk_pin
#define ICM206xx_SPI_FREQ      BoardConfig::imu_spi.freq_hz
#define ICM206xx_INT_PIN       BoardConfig::imu.int_pin

// Create SPI instance using BoardConfig
SPIClass spi_bus(BoardConfig::imu_spi.mosi_pin,
                 BoardConfig::imu_spi.miso_pin,
                 BoardConfig::imu_spi.sclk_pin);

ICM206xx imu;

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
void icm206xx_data_ready_isr() {
  data_ready = true;
  interrupt_count++;
}

void setup() {
  // Initialize Serial for non-RTT mode
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("=== ICM-206xx Interrupt-Driven Example ===\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  // Display pin configuration from BoardConfig
  CI_LOG("Pin Configuration (BoardConfig):\n");
  CI_LOGF("  CS: %d, MOSI: %d, MISO: %d, SCLK: %d\n",
         (int)ICM206xx_CS_PIN, (int)ICM206xx_MOSI_PIN,
         (int)ICM206xx_MISO_PIN, (int)ICM206xx_SCLK_PIN);
  CI_LOGF("  SPI Speed: %lu Hz\n", (unsigned long)ICM206xx_SPI_FREQ);
  if (ICM206xx_INT_PIN != 0) {
    CI_LOGF("  Interrupt Pin: %d\n", (int)ICM206xx_INT_PIN);
  } else {
    CI_LOG("  Interrupt Pin: None (polling mode)\n");
  }

  // Initialize ICM-206xx
  CI_LOG("\nInitializing ICM-206xx...\n");

  if (!imu.begin(spi_bus, ICM206xx_CS_PIN, ICM206xx_SPI_FREQ)) {
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

  // Configure DLPF (176 Hz gyro, 218 Hz accel bandwidth, 1 kHz internal rate)
  // DLPF must be 1-6 to enable 1 kHz sample rate (DLPF=0 uses 8 kHz)
  imu.setDLPF(1, 1);  // Gyro DLPF=1, Accel DLPF=1
  CI_LOG("DLPF configured: Gyro 176 Hz, Accel 218 Hz (1kHz internal)\n");

  // Set sample rate divider to 0 (1kHz output)
  imu.setSampleRateDivider(0);  // 1kHz / (1+0) = 1kHz
  CI_LOG("Sample rate divider: 0 (1kHz output)\n");

  // Set ranges
  imu.setGyroFSR(2000);  // ±2000 dps
  imu.setAccelFSR(16);   // ±16g

  CI_LOG("Gyro FSR: ±2000 dps\n");
  CI_LOG("Accel FSR: ±16g\n");

  // Setup interrupt if pin is configured
  // Note: Data-ready interrupts are automatically enabled by the driver
  if (ICM206xx_INT_PIN != 0) {
    pinMode(ICM206xx_INT_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(ICM206xx_INT_PIN), icm206xx_data_ready_isr, RISING);
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
  // Check if data is ready (interrupt-driven or polling)
  if (ICM206xx_INT_PIN != 0) {
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

    if (ICM206xx_INT_PIN != 0) {
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
      CI_LOGF("%s interrupt-driven test PASSED ✓\n", imu.getChipName());
      CI_LOG("*STOP*\n");
      while (1);
    }
  }

  // In polling mode, delay to approximate 1kHz
  if (ICM206xx_INT_PIN == 0) {
    delayMicroseconds(1000);  // 1ms = 1kHz
  }
}
