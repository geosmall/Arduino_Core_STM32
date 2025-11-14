/*
 * AutoDetect_Single - IMU Facade API Example
 *
 * Demonstrates the high-level IMU_BF facade with auto-detection.
 *
 * Hardware Setup (NUCLEO_F411RE):
 *   - ICM42688P connected via SPI
 *   - CS: PA4
 *   - SCK: PA5
 *   - MISO: PA6
 *   - MOSI: PA7
 *
 * Build & Test:
 *   ./system/ci/build.sh libraries/imu/examples/AutoDetect_Single --use-rtt --build-id
 *   ./system/ci/aflash.sh libraries/imu/examples/AutoDetect_Single --use-rtt
 */

#include <Arduino.h>
#include <SPI.h>
#include <ci_log.h>
#include <IMU_BF.h>

// Hardware configuration
#define IMU_CS_PIN    PA4
#define IMU_SPI_FREQ  1000000  // 1 MHz

// Create IMU facade
IMU_BF imu;

void setup()
{
    // Initialize Serial for non-RTT mode
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
#endif

    CI_LOG("=== IMU_BF Facade Test ===\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();

    // Initialize SPI
    SPI.begin();

    // Attach SPI bus to IMU facade
    imu.attachSPI(SPI, IMU_CS_PIN, IMU_SPI_FREQ);

    // Auto-detect and initialize IMU
    CI_LOG("Attempting auto-detection...\n");
    if (!imu.begin(ImuType::Auto)) {
        CI_LOG("*FAIL* IMU detection/initialization failed\n");
        CI_LOG("*STOP*\n");
        while (1);
    }

    CI_LOGF("*PASS* Detected: %s (WHO_AM_I=0x%02X)\n",
            imu.typeName(),
            static_cast<uint8_t>(imu.type()));

    CI_LOG("\nStreaming IMU data (5 seconds)...\n");
    CI_LOG("Format: Accel[m/s²] Gyro[rad/s]\n\n");
}

void loop()
{
    static uint32_t start_ms = millis();
    static uint32_t sample_count = 0;
    static uint32_t last_print_ms = 0;

    // Read IMU sample
    ImuSample sample;
    if (imu.read(sample)) {
        sample_count++;

        // Print at ~10 Hz
        if (millis() - last_print_ms >= 100) {
            CI_LOGF("Sample %lu: ", sample_count);
            CI_LOG_FLOAT("ax=", sample.ax, 2);
            CI_LOG_FLOAT(" ay=", sample.ay, 2);
            CI_LOG_FLOAT(" az=", sample.az, 2);
            CI_LOG_FLOAT(" | gx=", sample.gx, 3);
            CI_LOG_FLOAT(" gy=", sample.gy, 3);
            CI_LOG_FLOAT(" gz=", sample.gz, 3);
            CI_LOG("\n");

            last_print_ms = millis();
        }
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
