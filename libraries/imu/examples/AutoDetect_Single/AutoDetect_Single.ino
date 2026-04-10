/*
 * AutoDetect_Single - IMU Library Example
 *
 * Demonstrates the high-level IMU_Driver facade with auto-detection.
 * Supports multiple InvenSense IMU chips (ICM-42688-P, MPU-6000, MPU-9250, ICM-206xx).
 *
 * Hardware Setup:
 *   - Uses BoardConfig for automatic board detection
 *   - BlackPill F411CE: SPI2 (PB12/PB13/PB14/PB15) - MPU9250
 *   - NUCLEO_F411RE: SPI1 (PA4/PA5/PA6/PA7) - ICM42688P or MPU6000
 *   - NERO F7: SPI1 (PA7/PA6/PA5/PC4) - ICM20602
 */

#include <Arduino.h>
#include <SPI.h>
#include <IMU_Driver.h>

// Board configuration - Multi-board support
#if defined(ARDUINO_BKMN_NERO)
#include "targets/BKMN-NERO.h"
#elif defined(ARDUINO_BLACKPILL_F411CE)
#include "targets/BLACKPILL_F411CE.h"
#elif defined(ARDUINO_WEACT_G474CE)
#include "targets/WEACT_G474_HIL007.h"
#else
#include "targets/NUCLEO_F411RE_HIL001.h"
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
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println("=== IMU_Driver Auto-Detect Example ===\n");

    // Display pin configuration from BoardConfig
    Serial.println("Pin Configuration (BoardConfig):");
    Serial.printf("  CS: 0x%02X, MOSI: 0x%02X, MISO: 0x%02X, SCLK: 0x%02X\n",
           (int)IMU_CS_PIN.toPinName(), (int)IMU_MOSI_PIN.toPinName(),
           (int)IMU_MISO_PIN.toPinName(), (int)IMU_SCLK_PIN.toPinName());
    Serial.printf("  SPI Speed: %lu Hz\n\n", (unsigned long)IMU_SPI_FREQ);

    // Initialize SPI with BoardConfig pins
    spi_bus.begin();

    // Attach SPI bus to IMU facade
    imu.attachSPI(spi_bus, IMU_CS_PIN, IMU_SPI_FREQ);

    // Auto-detect and initialize IMU
    Serial.println("Attempting auto-detection...");
    if (!imu.begin(ImuType::Auto)) {
        Serial.println("ERROR: IMU detection/initialization failed");
        while (1);
    }

    Serial.printf("Detected: %s (WHO_AM_I=0x%02X)\n",
            imu.typeName(),
            static_cast<uint8_t>(imu.type()));

    Serial.println("\nStreaming IMU data...");
    Serial.println("Format: Accel[m/s²] Gyro[rad/s]\n");
}

void loop()
{
    static uint32_t sample_count = 0;
    static uint32_t last_print_ms = 0;

    // Read IMU sample
    ImuSample sample;
    if (imu.read(sample)) {
        sample_count++;

        // Print at ~10 Hz
        if (millis() - last_print_ms >= 100) {
            Serial.printf("Sample %lu: ", sample_count);
            Serial.printf("ax=%.2f ay=%.2f az=%.2f | ", sample.ax, sample.ay, sample.az);
            Serial.printf("gx=%.3f gy=%.3f gz=%.3f\n", sample.gx, sample.gy, sample.gz);

            last_print_ms = millis();
        }
    }
}
