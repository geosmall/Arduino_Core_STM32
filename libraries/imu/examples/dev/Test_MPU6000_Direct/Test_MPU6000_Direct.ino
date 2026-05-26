/*
 * Test_MPU6000_Direct - Direct MPU6000 Driver Test
 *
 * Tests MPU6000 driver directly (bypassing IMU_Driver facade).
 * Validates WHO_AM_I detection, initialization, and data streaming.
 *
 * Hardware Setup:
 *   - NUCLEO_F411RE with HIL005 config (MPU-6000 on SPI1)
 *   - MPU6000 on SPI1 (PA4/PA5/PA6/PA7), INT=PB3
 *
 * Build & Test:
 *   ./ci/saflash.sh Arduino_Core_STM32/libraries/imu/examples/dev/Test_MPU6000_Direct
 */

#include <Arduino.h>
#include <SPI.h>
#include <IMU_Driver.h>  // This will trigger library detection
// Direct access to internal classes
#include "../../../src/bus/DeviceBusSPI.h"
#include "../../../src/devices/MPU6000.h"

// Board configuration - NUCLEO_F411RE with HIL005
#if defined(ARDUINO_NUCLEO_F411RE)
#include "targets/NUCLEO_F411RE_HIL005.h"
#else
#error "This example requires NUCLEO_F411RE board"
#endif

// BoardConfig integration for dynamic pin configuration
#define MPU6000_CS_PIN        BoardConfig::imu.spi.cs_pin
#define MPU6000_MOSI_PIN      BoardConfig::imu.spi.mosi_pin
#define MPU6000_MISO_PIN      BoardConfig::imu.spi.miso_pin
#define MPU6000_SCLK_PIN      BoardConfig::imu.spi.sclk_pin
#define MPU6000_SPI_FREQ      BoardConfig::imu.spi.freq_hz

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi_bus(BoardConfig::imu.spi.instance,
                 BoardConfig::imu.spi.mosi_pin,
                 BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin,
                 BoardConfig::imu.spi.get_ssel_pin());

// Create SPI bus wrapper
DeviceBusSPI* bus = nullptr;
MPU6000* mpu = nullptr;

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println("=== MPU6000 Direct Driver Test ===");

    // Display pin configuration from BoardConfig
    Serial.println("\nPin Configuration (BoardConfig):");
    Serial.print("  CS: 0x");
    Serial.print((int)MPU6000_CS_PIN.toPinName(), HEX);
    Serial.print(", MOSI: 0x");
    Serial.print((int)MPU6000_MOSI_PIN.toPinName(), HEX);
    Serial.print(", MISO: 0x");
    Serial.print((int)MPU6000_MISO_PIN.toPinName(), HEX);
    Serial.print(", SCLK: 0x");
    Serial.println((int)MPU6000_SCLK_PIN.toPinName(), HEX);
    Serial.print("  SPI Speed: ");
    Serial.print((unsigned long)MPU6000_SPI_FREQ);
    Serial.println(" Hz\n");

    // Initialize SPI with BoardConfig pins
    spi_bus.begin();

    // Create SPI bus wrapper
    bus = new DeviceBusSPI(&spi_bus, MPU6000_CS_PIN);
    bus->setFreq(MPU6000_SPI_FREQ);

    Serial.println("Attempting MPU6000 detection...");

    // Detect MPU6000 (factory pattern with 20 retries)
    for (int attempt = 0; attempt < 20 && !mpu; attempt++) {
        mpu = MPU6000::detect(bus);
        if (!mpu) {
            delay(150);
        }
    }

    if (!mpu) {
        Serial.println("*FAIL* MPU6000 detection failed");
        Serial.println("*STOP*");
        while (1);
    }

    Serial.print("*PASS* Detected: ");
    Serial.print(mpu->typeName());
    Serial.print(" (WHO_AM_I=0x");
    Serial.print(mpu->whoAmI_, HEX);
    Serial.println(")");
    Serial.print("Gyro scale: ");
    Serial.print(mpu->gyrScale_, 6);
    Serial.println(" dps/LSB");
    Serial.print("Accel scale: ");
    Serial.print(mpu->accScale_, 6);
    Serial.println(" G/LSB");
    Serial.print("Sampling rate: ");
    Serial.print(mpu->samplingRateHz_);
    Serial.println(" Hz");

    Serial.println("\nStreaming data (5 seconds)...");
    Serial.println("Format: Raw LSB values\n");
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
        Serial.print("Sample ");
        Serial.print(sample_count);
        Serial.print(": ax=");
        Serial.print(data[0] * mpu->accScale_, 2);
        Serial.print(" ay=");
        Serial.print(data[1] * mpu->accScale_, 2);
        Serial.print(" az=");
        Serial.print(data[2] * mpu->accScale_, 2);
        Serial.print(" | gx=");
        Serial.print(data[3] * mpu->gyrScale_, 1);
        Serial.print(" gy=");
        Serial.print(data[4] * mpu->gyrScale_, 1);
        Serial.print(" gz=");
        Serial.println(data[5] * mpu->gyrScale_, 1);

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
