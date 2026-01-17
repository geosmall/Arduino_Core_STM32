/*
 * MPU6000_Basic Example
 *
 * Demonstrates MPU-6000 detection and basic 6-axis data reading.
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection
 * - Pin assignments and SPI frequency from board configuration
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

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin,
                 BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin,
                 BoardConfig::imu.spi.get_ssel_pin());

MPU6000 imu;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("=== MPU-6000 Basic Test ===");

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

  // Configure DLPF (256 Hz bandwidth, low latency)
  imu.setDLPF(0);
  Serial.println("DLPF configured: 256 Hz (gyro/accel)");

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
      Serial.println("MPU-6000 basic test PASSED ✓");
      Serial.println("*STOP*");
      while (1);
    }
  } else {
    Serial.println("ERROR: Failed to read IMU data");
  }

  delay(10);  // 100 Hz loop rate
}
