/*
 * Test_MPU6000_Direct - Direct MPU6000_BF Driver Test
 *
 * Tests MPU6000_BF driver directly (bypassing IMU_BF facade).
 * Validates WHO_AM_I detection, initialization, and data streaming.
 *
 * Hardware Setup (NUCLEO_F411RE):
 *   - MPU6000 connected via SPI
 *   - CS: PA4
 *   - SCK: PA5
 *   - MISO: PA6
 *   - MOSI: PA7
 *
 * Build & Test:
 *   ./system/ci/build.sh libraries/imu/examples/Test_MPU6000_Direct --use-rtt --build-id
 *   ./system/ci/aflash.sh libraries/imu/examples/Test_MPU6000_Direct --use-rtt
 */

#include <Arduino.h>
#include <SPI.h>
#include <ci_log.h>
#include <IMU_BF.h>  // This will trigger library detection
// Direct access to internal classes
#include "../../src/bus/DeviceBusSPI.h"
#include "../../src/devices/MPU6000_BF.h"

// Hardware configuration
#define IMU_CS_PIN    PA4

// Create SPI bus
DeviceBusSPI* bus = nullptr;
MPU6000_BF* mpu = nullptr;

void setup()
{
    // Initialize Serial for non-RTT mode
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
#endif

    CI_LOG("=== MPU6000_BF Direct Driver Test ===\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();

    // Initialize SPI
    SPI.begin();

    // Create SPI bus (1 MHz for detection)
    bus = new DeviceBusSPI(&SPI, IMU_CS_PIN);
    bus->setFreq(1000000);

    CI_LOG("Attempting MPU6000 detection...\n");

    // Detect MPU6000 (factory pattern with 20 retries)
    for (int attempt = 0; attempt < 20 && !mpu; attempt++) {
        mpu = MPU6000_BF::detect(bus);
        if (!mpu) {
            delay(150);
        }
    }

    if (!mpu) {
        CI_LOG("*FAIL* MPU6000 detection failed\n");
        CI_LOG("*STOP*\n");
        while (1);
    }

    CI_LOGF("*PASS* Detected: %s (WHO_AM_I=0x%02X)\n", mpu->typeName(), mpu->whoAmI_);
    CI_LOGF("Gyro scale: %.6f dps/LSB\n", mpu->gyrScale_);
    CI_LOGF("Accel scale: %.6f G/LSB\n", mpu->accScale_);
    CI_LOGF("Sampling rate: %u Hz\n", mpu->samplingRateHz_);

    CI_LOG("\nStreaming data (5 seconds)...\n");
    CI_LOG("Format: Raw LSB values\n\n");
}

void loop()
{
    static uint32_t start_ms = millis();
    static uint32_t sample_count = 0;
    static uint32_t last_print_ms = 0;

    // Read IMU data: ax, ay, az, gx, gy, gz
    int16_t data[6];
    mpu->read(data);
    sample_count++;

    // Print at ~10 Hz
    if (millis() - last_print_ms >= 100) {
        CI_LOGF("Sample %lu: ", sample_count);
        CI_LOG_FLOAT("ax=", data[0] * mpu->accScale_, 2);
        CI_LOG_FLOAT(" ay=", data[1] * mpu->accScale_, 2);
        CI_LOG_FLOAT(" az=", data[2] * mpu->accScale_, 2);
        CI_LOG_FLOAT(" | gx=", data[3] * mpu->gyrScale_, 1);
        CI_LOG_FLOAT(" gy=", data[4] * mpu->gyrScale_, 1);
        CI_LOG_FLOAT(" gz=", data[5] * mpu->gyrScale_, 1);
        CI_LOG("\n");

        last_print_ms = millis();
    }

    // Stop after 5 seconds
    if (millis() - start_ms >= 5000) {
        float read_rate_hz = sample_count / 5.0f;

        CI_LOG("\n=== Test Complete ===\n");
        CI_LOGF("Total samples: %lu\n", sample_count);
        CI_LOG_FLOAT("Read rate: ", read_rate_hz, 1);
        CI_LOG(" Hz\n");
        CI_LOG("*PASS*\n");
        CI_LOG("*STOP*\n");
        while (1);
    }
}
