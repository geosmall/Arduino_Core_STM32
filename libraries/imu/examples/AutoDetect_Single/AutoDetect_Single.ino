/*
 * AutoDetect_Single - IMU Facade API Example
 *
 * Demonstrates the high-level IMU_Driver facade with auto-detection.
 *
 * Hardware Setup:
 *   - Uses BoardConfig for automatic board detection
 *   - BlackPill F411CE: SPI2 (PB12/PB13/PB14/PB15) - MPU9250
 *   - NUCLEO_F411RE: SPI1 (PA4/PA5/PA6/PA7) - ICM42688P or MPU6000
 *   - NERO F7: SPI1 (PA7/PA6/PA5/PC4) - ICM20602
 *
 * Build & Test:
 *   # BlackPill F411CE:
 *   ./system/ci/aflash.sh libraries/imu/examples/AutoDetect_Single STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE --use-rtt
 *
 *   # NUCLEO_F411RE:
 *   ./system/ci/aflash.sh libraries/imu/examples/AutoDetect_Single --use-rtt
 *
 *   # NERO F7:
 *   ./system/ci/aflash.sh libraries/imu/examples/AutoDetect_Single STMicroelectronics:stm32:FlightCtr:pnum=BKMN_NERO --use-rtt
 */

#include <Arduino.h>
#include <SPI.h>
#include <ci_log.h>
#include <IMU_Driver.h>

// Board configuration - Multi-board support
#if defined(ARDUINO_BKMN_NERO)
#include "../../../../targets/BKMN-NERO.h"
#elif defined(ARDUINO_BLACKPILL_F411CE)
#include "../../../../targets/BLACKPILL_F411CE.h"
#else
#include "../../../../targets/NUCLEO_F411RE_JHEF411.h"
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

// Create IMU facade
IMU_Driver imu;

void setup()
{
    // Initialize Serial for non-RTT mode
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
#endif

    CI_LOG("=== IMU_Driver Facade Test ===\n");
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

    // Attach SPI bus to IMU facade
    imu.attachSPI(spi_bus, IMU_CS_PIN, IMU_SPI_FREQ);

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
