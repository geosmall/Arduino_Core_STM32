/*
 * ICM42688 Driver Test - Direct Device Access
 *
 * Tests the ICM42688 driver (madflight pattern) with DeviceBus abstraction.
 * Factory pattern: detect() returns nullptr if not found, or initialized instance.
 *
 * Hardware Setup:
 *   - NUCLEO_F411RE ONLY (with JHEF411 config)
 *   - ICM42688P on SPI1 (PA4/PA5/PA6/PA7)
 *
 * Build/Flash:
 *   ./system/ci/aflash.sh libraries/imu/examples/Test_ICM42688_Direct --use-rtt
 */

#include <SPI.h>
#include <ci_log.h>
#include <IMU.h>  // This triggers compilation of all imu library .cpp files

// Direct access to internal components (normally not needed by users)
#include "../../../src/bus/DeviceBusSPI.h"
#include "../../../src/devices/ICM42688.h"

// Board configuration - NUCLEO_F411RE ONLY
#if defined(ARDUINO_NUCLEO_F411RE)
#include "../../../../targets/NUCLEO_F411RE_JHEF411.h"
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
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("=== ICM42688 Driver Test ===\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  // Display pin configuration from BoardConfig
  CI_LOG("\nPin Configuration (BoardConfig):\n");
  CI_LOGF("  CS: 0x%02X, MOSI: 0x%02X, MISO: 0x%02X, SCLK: 0x%02X\n",
         (int)IMU_CS_PIN, (int)IMU_MOSI_PIN,
         (int)IMU_MISO_PIN, (int)IMU_SCLK_PIN);
  CI_LOGF("  SPI Speed: %lu Hz\n\n", (unsigned long)IMU_SPI_FREQ);

  // Initialize SPI with BoardConfig pins
  spi_bus.begin();

  // Create bus interface
  bus = new DeviceBusSPI(&spi_bus, IMU_CS_PIN);
  bus->setFreq(IMU_SPI_FREQ);
  CI_LOG("DeviceBusSPI created\n");

  delay(100);  // IMU power-up delay

  // Detect IMU (factory pattern)
  CI_LOG("Detecting IMU...\n");
  imu = ICM42688::detect(bus);

  if (!imu) {
    CI_LOG("✗ IMU detection failed\n");
    CI_LOG("*TEST_FAIL*\n");
    CI_LOG("*STOP*\n");
    while(1);
  }

  CI_LOGF("✓ Detected: %s (WHO_AM_I=0x%02X)\n", imu->typeName(), imu->whoAmI_);
  CI_LOG_FLOAT("  Accel scale: ", imu->accScale_, 6); CI_LOG(" G/LSB\n");
  CI_LOG_FLOAT("  Gyro scale: ", imu->gyrScale_, 6); CI_LOG(" DPS/LSB\n");
  CI_LOGF("  Sampling rate: %d Hz\n", imu->samplingRateHz_);

  CI_LOG("✓ Initialization complete\n");
  CI_LOG("\nStarting data read test (5 seconds)...\n");
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
    CI_LOGF("Raw: A[%6d,%6d,%6d]  G[%6d,%6d,%6d]\n",
            data[0], data[1], data[2],
            data[3], data[4], data[5]);

    // Print scaled values
    float ax = data[0] * imu->accScale_;
    float ay = data[1] * imu->accScale_;
    float az = data[2] * imu->accScale_;
    float gx = data[3] * imu->gyrScale_;
    float gy = data[4] * imu->gyrScale_;
    float gz = data[5] * imu->gyrScale_;

    CI_LOG("Scaled: A[");
    CI_LOG_FLOAT("", ax, 3); CI_LOG(",");
    CI_LOG_FLOAT("", ay, 3); CI_LOG(",");
    CI_LOG_FLOAT("", az, 3); CI_LOG("]G  G[");
    CI_LOG_FLOAT("", gx, 2); CI_LOG(",");
    CI_LOG_FLOAT("", gy, 2); CI_LOG(",");
    CI_LOG_FLOAT("", gz, 2); CI_LOG("]DPS\n");
  }

  // Test duration: 5 seconds
  if (millis() - testStart > 5000) {
    CI_LOGF("\n✓ Test complete: %d reads in 5 seconds\n", readCount);
    CI_LOGF("  Read rate: ~%d Hz\n", readCount / 5);

    // Validation checks
    bool pass = true;

    // Check read count (should be close to loop rate)
    if (readCount < 100) {  // At least 20 Hz average
      CI_LOG("✗ Warning: Low read count\n");
      pass = false;
    }

    // Note: Can't validate actual sensor values without knowing orientation
    // But we got here, so communication is working

    if (pass) {
      CI_LOG("*TEST_PASS*\n");
    } else {
      CI_LOG("*TEST_FAIL*\n");
    }

    CI_LOG("*STOP*\n");
    while(1);
  }
}
