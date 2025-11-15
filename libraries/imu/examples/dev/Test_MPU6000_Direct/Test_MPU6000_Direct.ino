/*
 * Test_MPU6000_Direct - Direct MPU6000_BF Driver Test
 *
 * Tests MPU6000_BF driver directly (bypassing IMU_BF facade).
 * Validates WHO_AM_I detection, initialization, and data streaming.
 *
 * Hardware Setup:
 *   - NUCLEO_F411RE ONLY (with JHEF411 config)
 *   - MPU6000 on SPI1 (PA4/PA5/PA6/PA7)
 *
 * Build & Test:
 *   ./system/ci/aflash.sh libraries/imu/examples/Test_MPU6000_Direct --use-rtt
 */

#include <Arduino.h>
#include <SPI.h>
#include <ci_log.h>
#include <IMU_BF.h>  // This will trigger library detection
// Direct access to internal classes
#include "../../src/bus/DeviceBusSPI.h"
#include "../../src/devices/MPU6000_BF.h"

// Board configuration - NUCLEO_F411RE ONLY
#if defined(ARDUINO_NUCLEO_F411RE)
#include "../../../../targets/NUCLEO_F411RE_JHEF411.h"
#else
#error "This example requires NUCLEO_F411RE board. Use default FQBN: STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE"
#endif

// BoardConfig integration for dynamic pin configuration
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

// Create SPI bus wrapper
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

    // Display pin configuration from BoardConfig
    CI_LOG("\nPin Configuration (BoardConfig):\n");
    CI_LOGF("  CS: 0x%02X, MOSI: 0x%02X, MISO: 0x%02X, SCLK: 0x%02X\n",
           (int)MPU6000_CS_PIN, (int)MPU6000_MOSI_PIN,
           (int)MPU6000_MISO_PIN, (int)MPU6000_SCLK_PIN);
    CI_LOGF("  SPI Speed: %lu Hz\n\n", (unsigned long)MPU6000_SPI_FREQ);

    // Initialize SPI with BoardConfig pins
    spi_bus.begin();

    // Create SPI bus wrapper
    bus = new DeviceBusSPI(&spi_bus, MPU6000_CS_PIN);
    bus->setFreq(MPU6000_SPI_FREQ);

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
