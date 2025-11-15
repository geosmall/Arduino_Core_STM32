/*
 * Test_ICM206xx_Direct - Direct ICM206xx_BF Driver Test
 *
 * Tests ICM206xx_BF driver directly (bypassing IMU_BF facade).
 * Validates WHO_AM_I detection, initialization, and data streaming.
 *
 * Supported Devices:
 *   - ICM-20601 (WHO_AM_I = 0xAC)
 *   - ICM-20602 (WHO_AM_I = 0x12)
 *   - ICM-20689 (WHO_AM_I = 0x98)
 *
 * Hardware Platforms:
 *   - NUCLEO_F411RE (with JHEF411 config)
 *   - NERO F7 Flight Controller (BKMN-NERO target, ICM-20602 on SPI1)
 *
 * Build & Test:
 *   NUCLEO: ./system/ci/aflash.sh libraries/imu/examples/Test_ICM206xx_Direct --use-rtt
 *   NERO:   ./system/ci/aflash.sh libraries/imu/examples/Test_ICM206xx_Direct STMicroelectronics:stm32:FlightCtr:pnum=BKMN_NERO --use-rtt
 */

#include <Arduino.h>
#include <SPI.h>
#include <ci_log.h>
#include <IMU_BF.h>  // This will trigger library detection
// Direct access to internal classes
#include "../../src/bus/DeviceBusSPI.h"
#include "../../src/devices/ICM206xx_BF.h"

// Board configuration - Multi-board support
#if defined(STM32F722xx)
// NERO F7 Flight Controller (BKMN-NERO)
#include "../../../../targets/BKMN-NERO.h"
#elif defined(ARDUINO_NUCLEO_F411RE)
// NUCLEO_F411RE development board
#include "../../../../targets/NUCLEO_F411RE_JHEF411.h"
#else
#error "This example requires NERO F7 (FQBN: STMicroelectronics:stm32:FlightCtr:pnum=BKMN_NERO) or NUCLEO_F411RE"
#endif

// BoardConfig integration for dynamic pin configuration
#define ICM206XX_CS_PIN       BoardConfig::imu.spi.cs_pin
#define ICM206XX_MOSI_PIN     BoardConfig::imu.spi.mosi_pin
#define ICM206XX_MISO_PIN     BoardConfig::imu.spi.miso_pin
#define ICM206XX_SCLK_PIN     BoardConfig::imu.spi.sclk_pin
#define ICM206XX_SPI_FREQ     BoardConfig::imu.spi.freq_hz

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin,
                 BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin,
                 BoardConfig::imu.spi.get_ssel_pin());

// Create SPI bus wrapper
DeviceBusSPI* bus = nullptr;
ICM206xx_BF* imu = nullptr;

void setup()
{
    // Initialize Serial for non-RTT mode
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
#endif

    CI_LOG("=== ICM206xx_BF Direct Driver Test ===\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();

    // Display pin configuration from BoardConfig
    CI_LOG("\nPin Configuration (BoardConfig):\n");
    CI_LOGF("  CS: 0x%02X, MOSI: 0x%02X, MISO: 0x%02X, SCLK: 0x%02X\n",
           (int)ICM206XX_CS_PIN, (int)ICM206XX_MOSI_PIN,
           (int)ICM206XX_MISO_PIN, (int)ICM206XX_SCLK_PIN);
    CI_LOGF("  SPI Speed: %lu Hz\n\n", (unsigned long)ICM206XX_SPI_FREQ);

    // Initialize SPI with BoardConfig pins
    spi_bus.begin();

    // Create SPI bus wrapper
    bus = new DeviceBusSPI(&spi_bus, ICM206XX_CS_PIN);
    bus->setFreq(ICM206XX_SPI_FREQ);

    CI_LOG("Attempting ICM206xx detection...\n");

    // Detect ICM206xx (factory pattern with 20 retries)
    imu = ICM206xx_BF::detect(bus);

    if (!imu) {
        CI_LOG("*FAIL* ICM206xx detection failed\n");
        CI_LOG("*STOP*\n");
        while (1);
    }

    CI_LOGF("*PASS* Detected: %s (WHO_AM_I=0x%02X)\n", imu->typeName(), imu->whoAmI_);
    CI_LOG_FLOAT("Gyro scale: ", imu->gyrScale_, 6); CI_LOG(" dps/LSB\n");
    CI_LOG_FLOAT("Accel scale: ", imu->accScale_, 6); CI_LOG(" G/LSB\n");
    CI_LOGF("Sampling rate: %u Hz\n", imu->samplingRateHz_);

    CI_LOG("\nStreaming data (5 seconds)...\n");
    CI_LOG("Format: Scaled values (G and dps)\n\n");
}

void loop()
{
    static uint32_t start_ms = millis();
    static uint32_t sample_count = 0;
    static uint32_t last_print_ms = 0;

    // Read IMU data: ax, ay, az, gx, gy, gz
    int16_t data[6];
    imu->read(data);
    sample_count++;

    // Print at ~10 Hz
    if (millis() - last_print_ms >= 100) {
        CI_LOGF("Sample %lu: ", sample_count);
        CI_LOG_FLOAT("ax=", data[0] * imu->accScale_, 2);
        CI_LOG_FLOAT(" ay=", data[1] * imu->accScale_, 2);
        CI_LOG_FLOAT(" az=", data[2] * imu->accScale_, 2);
        CI_LOG_FLOAT(" | gx=", data[3] * imu->gyrScale_, 1);
        CI_LOG_FLOAT(" gy=", data[4] * imu->gyrScale_, 1);
        CI_LOG_FLOAT(" gz=", data[5] * imu->gyrScale_, 1);
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
