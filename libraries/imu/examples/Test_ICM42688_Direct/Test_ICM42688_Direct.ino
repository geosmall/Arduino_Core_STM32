/*
 * ICM42688_BF Driver Test - Direct Device Access
 *
 * Tests the ICM42688_BF driver (madflight pattern) with DeviceBus abstraction.
 * Factory pattern: detect() returns nullptr if not found, or initialized instance.
 *
 * Hardware: NUCLEO_F411RE with ICM42688P on breadboard SPI
 * Expected: WHO_AM_I = 0x47, gyro/accel data streaming
 *
 * Build/Flash:
 *   ./system/ci/build.sh libraries/imu/examples/Test_ICM42688_Direct --use-rtt --build-id
 *   ./system/ci/aflash.sh libraries/imu/examples/Test_ICM42688_Direct --use-rtt
 */

#include <SPI.h>
#include <ci_log.h>
#include <IMU.h>  // This triggers compilation of all imu library .cpp files

// Direct access to internal components (normally not needed by users)
#include "../../src/bus/DeviceBusSPI.h"
#include "../../src/devices/ICM42688_BF.h"

// Pin configuration for NUCLEO_F411RE
#define IMU_CS_PIN  PA4
#define IMU_SPI_FREQ 1000000  // Start at 1MHz for detection

DeviceBusSPI* bus = nullptr;
ICM42688_BF* imu = nullptr;

void setup() {
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("=== ICM42688_BF Driver Test ===\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  // Initialize SPI bus
  SPI.begin();
  CI_LOGF("SPI initialized (CS=%d, Freq=%d Hz)\n", IMU_CS_PIN, IMU_SPI_FREQ);

  // Create bus interface
  bus = new DeviceBusSPI(&SPI, IMU_CS_PIN);
  bus->setFreq(IMU_SPI_FREQ);
  CI_LOG("DeviceBusSPI created\n");

  delay(100);  // IMU power-up delay

  // Detect IMU (factory pattern)
  CI_LOG("Detecting IMU...\n");
  imu = ICM42688_BF::detect(bus);

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
