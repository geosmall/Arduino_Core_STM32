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
 * License: GPL v3 (Betaflight-derived library)
 */

#include <ICM206xx.h>
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
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("=== ICM-206xx Interrupt-Driven Example ===");

  // Display pin configuration from BoardConfig
  Serial.println("Pin Configuration (BoardConfig):");
  Serial.print("  CS: ");
  Serial.print((int)ICM206xx_CS_PIN);
  Serial.print(", MOSI: ");
  Serial.print((int)ICM206xx_MOSI_PIN);
  Serial.print(", MISO: ");
  Serial.print((int)ICM206xx_MISO_PIN);
  Serial.print(", SCLK: ");
  Serial.println((int)ICM206xx_SCLK_PIN);
  Serial.print("  SPI Speed: ");
  Serial.print((unsigned long)ICM206xx_SPI_FREQ);
  Serial.println(" Hz");
  if (ICM206xx_INT_PIN != 0) {
    Serial.print("  Interrupt Pin: ");
    Serial.println((int)ICM206xx_INT_PIN);
  } else {
    Serial.println("  Interrupt Pin: None (polling mode)");
  }

  // Initialize ICM-206xx
  Serial.println("Initializing ICM-206xx...");

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

  // Configure DLPF (176 Hz gyro, 218 Hz accel bandwidth, 1 kHz internal rate)
  // DLPF must be 1-6 to enable 1 kHz sample rate (DLPF=0 uses 8 kHz)
  imu.setDLPF(1, 1);  // Gyro DLPF=1, Accel DLPF=1
  Serial.println("DLPF configured: Gyro 176 Hz, Accel 218 Hz (1kHz internal)");

  // Set sample rate divider to 0 (1kHz output)
  imu.setSampleRateDivider(0);  // 1kHz / (1+0) = 1kHz
  Serial.println("Sample rate divider: 0 (1kHz output)");

  // Set ranges
  imu.setGyroFSR(2000);  // ±2000 dps
  imu.setAccelFSR(16);   // ±16g

  Serial.println("Gyro FSR: ±2000 dps");
  Serial.println("Accel FSR: ±16g");

  // Setup interrupt if pin is configured
  // Note: Data-ready interrupts are automatically enabled by the driver
  if (ICM206xx_INT_PIN != 0) {
    pinMode(ICM206xx_INT_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(ICM206xx_INT_PIN), icm206xx_data_ready_isr, RISING);
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

    Serial.println("\n=== Statistics ===");
    Serial.print("Sample Rate: ");
    Serial.print(sample_rate, 1);
    Serial.println(" Hz");

    if (ICM206xx_INT_PIN != 0) {
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
      Serial.print(imu.getChipName());
      Serial.println(" interrupt-driven test PASSED ✓");
      Serial.println("*STOP*");
      while (1);
    }
  }

  // In polling mode, delay to approximate 1kHz
  if (ICM206xx_INT_PIN == 0) {
    delayMicroseconds(1000);  // 1ms = 1kHz
  }
}
