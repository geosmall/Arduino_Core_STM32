/*
 * ICM42688 Driver Test - Direct Device Access
 *
 * Tests the ICM42688 driver (madflight pattern) with DeviceBus abstraction.
 * Factory pattern: detect() returns nullptr if not found, or initialized instance.
 *
 * Hardware Setup:
 *   - NUCLEO_F411RE ONLY (with JHEF411 config)
 *   - ICM42688P on SPI1 (PA4/PA5/PA6/PA7)
 */

#include <SPI.h>
#include <libPrintf.h>
#include <IMU.h>  // This triggers compilation of all imu library .cpp files

// Direct access to internal components (normally not needed by users)
#include "../../../src/bus/DeviceBusSPI.h"
#include "../../../src/devices/ICM42688.h"

// printf_() output routing
extern "C" void putchar_(char c) {
    Serial.write(c);
}

// Board configuration - NUCLEO_F411RE ONLY
#if defined(ARDUINO_NUCLEO_F411RE)
#include "../../../../targets/NUCLEO_F411RE_HIL001.h"
#else
#error "This example requires NUCLEO_F411RE board. Use default FQBN: STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE"
#endif

// BoardConfig integration for dynamic pin configuration
#define IMU_CS_PIN        BoardConfig::imu.spi.cs_pin
#define IMU_MOSI_PIN      BoardConfig::imu.spi.mosi_pin
#define IMU_MISO_PIN      BoardConfig::imu.spi.miso_pin
#define IMU_SCLK_PIN      BoardConfig::imu.spi.sclk_pin
#define IMU_SPI_FREQ      BoardConfig::imu.spi.freq_hz

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin,
                 BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin,
                 BoardConfig::imu.spi.get_ssel_pin());

DeviceBusSPI* bus = nullptr;
ICM42688* imu = nullptr;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("=== ICM42688 Driver Test ===");

  // Display pin configuration from BoardConfig
  Serial.println("\nPin Configuration (BoardConfig):");
  printf_("  CS: 0x%02X, MOSI: 0x%02X, MISO: 0x%02X, SCLK: 0x%02X\n",
         (int)IMU_CS_PIN, (int)IMU_MOSI_PIN,
         (int)IMU_MISO_PIN, (int)IMU_SCLK_PIN);
  printf_("  SPI Speed: %lu Hz\n\n", (unsigned long)IMU_SPI_FREQ);

  // Initialize SPI with BoardConfig pins
  spi_bus.begin();

  // Create bus interface
  bus = new DeviceBusSPI(&spi_bus, IMU_CS_PIN);
  bus->setFreq(IMU_SPI_FREQ);
  Serial.println("DeviceBusSPI created");

  delay(100);  // IMU power-up delay

  // Detect IMU (factory pattern)
  Serial.println("Detecting IMU...");
  imu = ICM42688::detect(bus);

  if (!imu) {
    Serial.println("✗ IMU detection failed");
    Serial.println("*TEST_FAIL*");
    Serial.println("*STOP*");
    while(1);
  }

  printf_("✓ Detected: %s (WHO_AM_I=0x%02X)\n", imu->typeName(), imu->whoAmI_);
  printf_("  Accel scale: %.6f G/LSB\n", imu->accScale_);
  printf_("  Gyro scale: %.6f DPS/LSB\n", imu->gyrScale_);
  printf_("  Sampling rate: %d Hz\n", imu->samplingRateHz_);

  Serial.println("✓ Initialization complete");
  Serial.println("\nStarting data read test (5 seconds)...");
}

void loop() {
  static uint32_t lastPrint = 0;
  static uint32_t testStart = 0;
  static uint16_t readCount = 0;

  if (testStart == 0) {
    testStart = millis();
  }

  // Read IMU data
  int16_t data[6];  // ax, ay, az, gx, gy, gz
  imu->read(data);
  readCount++;

  // Print at 10 Hz
  if (millis() - lastPrint >= 100) {
    lastPrint = millis();

    // Print raw values
    printf_("Raw: A[%6d,%6d,%6d]  G[%6d,%6d,%6d]\n",
            data[0], data[1], data[2],
            data[3], data[4], data[5]);

    // Print scaled values
    float ax = data[0] * imu->accScale_;
    float ay = data[1] * imu->accScale_;
    float az = data[2] * imu->accScale_;
    float gx = data[3] * imu->gyrScale_;
    float gy = data[4] * imu->gyrScale_;
    float gz = data[5] * imu->gyrScale_;

    printf_("Scaled: A[%.3f,%.3f,%.3f]G  G[%.2f,%.2f,%.2f]DPS\n",
            ax, ay, az, gx, gy, gz);
  }

  // Test duration: 5 seconds
  if (millis() - testStart > 5000) {
    printf_("\n✓ Test complete: %d reads in 5 seconds\n", readCount);
    printf_("  Read rate: ~%d Hz\n", readCount / 5);

    // Validation checks
    bool pass = true;

    // Check read count (should be close to loop rate)
    if (readCount < 100) {  // At least 20 Hz average
      Serial.println("✗ Warning: Low read count");
      pass = false;
    }

    // Note: Can't validate actual sensor values without knowing orientation
    // But we got here, so communication is working

    if (pass) {
      Serial.println("*TEST_PASS*");
    } else {
      Serial.println("*TEST_FAIL*");
    }

    Serial.println("*STOP*");
    while(1);
  }
}
