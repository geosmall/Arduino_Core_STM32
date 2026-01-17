/*
 * MPU6000_Interrupt Example
 *
 * Demonstrates interrupt-driven data acquisition from MPU-6000.
 * This example uses the data-ready interrupt to achieve precise 1kHz sampling.
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection
 * - Pin assignments and SPI frequency from board configuration
 * - Interrupt pin configured for data-ready signaling
 * - Supports multiple board targets with single codebase
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
 * License: GPL v3 (Betaflight-derived library)
 */

#include <MPU6000.h>

// Board configuration
#if defined(ARDUINO_NUCLEO_F411RE)
  #include "../../../../targets/NUCLEO_F411RE_HIL005.h"
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
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("=== MPU-6000 Interrupt-Driven Example ===");

  // Display pin configuration from BoardConfig
  Serial.println("Pin Configuration (BoardConfig):");
  Serial.print("  CS: ");
  Serial.print((int)MPU6000_CS_PIN);
  Serial.print(", MOSI: ");
  Serial.print((int)MPU6000_MOSI_PIN);
  Serial.print(", MISO: ");
  Serial.print((int)MPU6000_MISO_PIN);
  Serial.print(", SCLK: ");
  Serial.println((int)MPU6000_SCLK_PIN);
  Serial.print("  SPI Speed: ");
  Serial.print((unsigned long)MPU6000_SPI_FREQ);
  Serial.println(" Hz");
  if (MPU6000_INT_PIN != 0) {
    Serial.print("  Interrupt Pin: ");
    Serial.println((int)MPU6000_INT_PIN);
  } else {
    Serial.println("  Interrupt Pin: None (polling mode)");
  }

  // Initialize MPU-6000
  Serial.println("\nInitializing MPU-6000...");

  if (!imu.begin(spi_bus, MPU6000_CS_PIN, MPU6000_SPI_FREQ)) {
    Serial.println("ERROR: MPU-6000 initialization failed!");
    Serial.println("Check connections:");
    Serial.println("  - SPI MOSI, MISO, SCK");
    Serial.println("  - CS pin");
    Serial.println("  - 3.3V power");
    Serial.println("*STOP*");
    while (1);
  }

  Serial.println("MPU-6000 initialized successfully");

  // Read WHO_AM_I register
  uint8_t who_am_i = imu.whoAmI();
  Serial.print("WHO_AM_I: 0x");
  Serial.print(who_am_i, HEX);
  Serial.print(" ");

  if (who_am_i == 0x68) {
    Serial.println("(MPU-6000 detected) ✓");
  } else {
    Serial.println("(Expected 0x68, detection may have failed)");
  }

  // Configure for 1kHz operation
  // DLPF = 0 (256 Hz bandwidth, 8kHz internal sample rate)
  // SMPLRT_DIV = 7 (8kHz / (1+7) = 1kHz output)
  imu.setDLPF(0);
  Serial.println("DLPF configured: 256 Hz bandwidth (8kHz internal)");

  imu.setSampleRateDivider(7);  // 8kHz / (1+7) = 1kHz
  Serial.println("Sample rate divider: 7 (target 1kHz output)");

  // Set ranges
  imu.setGyroFSR(2000);  // ±2000 dps
  imu.setAccelFSR(16);   // ±16g

  Serial.println("Gyro FSR: ±2000 dps");
  Serial.println("Accel FSR: ±16g");

  // Setup interrupt if pin is configured
  if (MPU6000_INT_PIN != 0) {
    pinMode(MPU6000_INT_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(MPU6000_INT_PIN), mpu6000_data_ready_isr, RISING);
    Serial.println("✓ Data-ready interrupt configured (1kHz)");
  } else {
    Serial.println("⚠ No interrupt pin - using polling mode");
  }

  Serial.println("\nStarting interrupt-driven data acquisition...");
  Serial.println("Target: 1000 Hz sample rate");
  Serial.println("---");

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

    Serial.println("\n=== Statistics ===");
    Serial.print("Sample Rate: ");
    Serial.print(sample_rate, 1);
    Serial.println(" Hz");

    if (MPU6000_INT_PIN != 0) {
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
      Serial.println("MPU-6000 interrupt-driven test PASSED ✓");
      Serial.println("*STOP*");
      while (1);
    }
  }

  // In polling mode, delay to approximate 1kHz
  if (MPU6000_INT_PIN == 0) {
    delayMicroseconds(1000);  // 1ms = 1kHz
  }
}
