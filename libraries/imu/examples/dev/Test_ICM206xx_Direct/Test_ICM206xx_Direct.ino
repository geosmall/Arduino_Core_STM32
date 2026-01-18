/*
 * Test_ICM206xx_Direct - Direct ICM206xx Driver Test
 *
 * Tests ICM206xx driver directly (bypassing IMU_Driver facade).
 * Validates WHO_AM_I detection, initialization, and data streaming.
 *
 * Supported Devices:
 *   - ICM-20601 (WHO_AM_I = 0xAC)
 *   - ICM-20602 (WHO_AM_I = 0x12)
 *   - ICM-20689 (WHO_AM_I = 0x98)
 *
 * Hardware Platform:
 *   - NERO F7 Flight Controller (BKMN-NERO target, ICM-20602 on SPI1)
 *
 * Build & Test:
 *   ./ci/saflash.sh Arduino_Core_STM32/libraries/imu/examples/dev/Test_ICM206xx_Direct STM32_Robotics:stm32:FlightCtr:pnum=BKMN_NERO
 *
 * License: GPL v3 (Betaflight-derived library)
 */

#include <Arduino.h>
#include <SPI.h>
#include <IMU_Driver.h>  // This will trigger library detection
// Direct access to internal classes
#include "../../../src/bus/DeviceBusSPI.h"
#include "../../../src/devices/ICM206xx.h"

// Board configuration - NERO F7 only (has ICM-20602)
#if defined(ARDUINO_BKMN_NERO)
#include "../../../../targets/BKMN-NERO.h"
#else
#error "This example requires NERO F7 (FQBN: STM32_Robotics:stm32:FlightCtr:pnum=BKMN_NERO)"
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
ICM206xx* imu = nullptr;

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println("=== ICM206xx Direct Driver Test ===");

    // Display pin configuration from BoardConfig
    Serial.println("Pin Configuration (BoardConfig):");
    Serial.print("  CS: ");
    Serial.print((int)ICM206XX_CS_PIN);
    Serial.print(", MOSI: ");
    Serial.print((int)ICM206XX_MOSI_PIN);
    Serial.print(", MISO: ");
    Serial.print((int)ICM206XX_MISO_PIN);
    Serial.print(", SCLK: ");
    Serial.println((int)ICM206XX_SCLK_PIN);
    Serial.print("  SPI Speed: ");
    Serial.print((unsigned long)ICM206XX_SPI_FREQ);
    Serial.println(" Hz");

    // Initialize SPI with BoardConfig pins
    spi_bus.begin();

    // Create SPI bus wrapper
    bus = new DeviceBusSPI(&spi_bus, ICM206XX_CS_PIN);
    bus->setFreq(ICM206XX_SPI_FREQ);

    Serial.println("Attempting ICM206xx detection...");

    // Detect ICM206xx (factory pattern with 20 retries)
    imu = ICM206xx::detect(bus);

    if (!imu) {
        Serial.println("*FAIL* ICM206xx detection failed");
        Serial.println("*STOP*");
        while (1);
    }

    Serial.print("*PASS* Detected: ");
    Serial.print(imu->typeName());
    Serial.print(" (WHO_AM_I=0x");
    Serial.print(imu->whoAmI_, HEX);
    Serial.println(")");
    Serial.print("Gyro scale: ");
    Serial.print(imu->gyrScale_, 6);
    Serial.println(" dps/LSB");
    Serial.print("Accel scale: ");
    Serial.print(imu->accScale_, 6);
    Serial.println(" G/LSB");
    Serial.print("Sampling rate: ");
    Serial.print(imu->samplingRateHz_);
    Serial.println(" Hz");

    Serial.println("\nStreaming data (5 seconds)...");
    Serial.println("Format: Scaled values (G and dps)\n");
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
        Serial.print("Sample ");
        Serial.print(sample_count);
        Serial.print(": ax=");
        Serial.print(data[0] * imu->accScale_, 2);
        Serial.print(" ay=");
        Serial.print(data[1] * imu->accScale_, 2);
        Serial.print(" az=");
        Serial.print(data[2] * imu->accScale_, 2);
        Serial.print(" | gx=");
        Serial.print(data[3] * imu->gyrScale_, 1);
        Serial.print(" gy=");
        Serial.print(data[4] * imu->gyrScale_, 1);
        Serial.print(" gz=");
        Serial.println(data[5] * imu->gyrScale_, 1);

        last_print_ms = millis();
    }

    // Stop after 5 seconds
    if (millis() - start_ms >= 5000) {
        float read_rate_hz = sample_count / 5.0f;

        Serial.println("\n=== Test Complete ===");
        Serial.print("Total samples: ");
        Serial.println(sample_count);
        Serial.print("Read rate: ");
        Serial.print(read_rate_hz, 1);
        Serial.println(" Hz");
        Serial.println("*PASS*");
        Serial.println("*STOP*");
        while (1);
    }
}
