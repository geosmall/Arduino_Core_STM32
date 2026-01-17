/*
 * Test_MPU9250_Direct - Direct MPU9250 Driver Test
 *
 * Tests MPU9250 driver directly (bypassing IMU_Driver facade).
 * Validates WHO_AM_I detection, initialization, and data streaming.
 *
 * Hardware Setup:
 *   - BlackPill F411CE ONLY
 *   - MPU9250 on SPI2 (PB12/PB13/PB14/PB15)
 *
 * Build & Test:
 *   ./ci/saflash.sh Arduino_Core_STM32/libraries/imu/examples/dev/Test_MPU9250_Direct STM32_Robotics:stm32:GenF4:pnum=BLACKPILL_F411CE
 */

#include <Arduino.h>
#include <SPI.h>
#include <libPrintf.h>
#include <IMU_Driver.h>  // This will trigger library detection
// Direct access to internal classes
#include "../../../src/bus/DeviceBusSPI.h"
#include "../../../src/devices/MPU9250.h"

// libPrintf output routing
extern "C" void putchar_(char c) {
    Serial.write(c);
}

// Board configuration - BlackPill F411CE ONLY
#if defined(ARDUINO_BLACKPILL_F411CE)
#include "../../../../targets/BLACKPILL_F411CE.h"
#else
#error "This example requires BLACKPILL_F411CE board. Use FQBN: STM32_Robotics:stm32:GenF4:pnum=BLACKPILL_F411CE"
#endif

// BoardConfig integration for dynamic pin configuration
#define MPU9250_CS_PIN        BoardConfig::imu.spi.cs_pin
#define MPU9250_MOSI_PIN      BoardConfig::imu.spi.mosi_pin
#define MPU9250_MISO_PIN      BoardConfig::imu.spi.miso_pin
#define MPU9250_SCLK_PIN      BoardConfig::imu.spi.sclk_pin
#define MPU9250_SPI_FREQ      BoardConfig::imu.spi.freq_hz

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin,
                 BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin,
                 BoardConfig::imu.spi.get_ssel_pin());

// Create SPI bus wrapper
DeviceBusSPI* bus = nullptr;
MPU9250* mpu = nullptr;

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println("=== MPU9250 Direct Driver Test ===");

    // Display pin configuration from BoardConfig
    Serial.println("\nPin Configuration (BoardConfig):");
    printf_("  CS: 0x%02X, MOSI: 0x%02X, MISO: 0x%02X, SCLK: 0x%02X\n",
           (int)MPU9250_CS_PIN, (int)MPU9250_MOSI_PIN,
           (int)MPU9250_MISO_PIN, (int)MPU9250_SCLK_PIN);
    printf_("  SPI Speed: %lu Hz\n\n", (unsigned long)MPU9250_SPI_FREQ);

    // Initialize SPI with BoardConfig pins
    spi_bus.begin();

    // Create SPI bus wrapper
    bus = new DeviceBusSPI(&spi_bus, MPU9250_CS_PIN);
    bus->setFreq(MPU9250_SPI_FREQ);

    Serial.println("Attempting MPU9250/MPU9255 detection...");

    // Detect MPU9250 (factory pattern with 20 retries built-in)
    mpu = MPU9250::detect(bus);

    if (!mpu) {
        Serial.println("*FAIL* MPU9250/MPU9255 detection failed");
        Serial.println("*STOP*");
        while (1);
    }

    printf_("*PASS* Detected: %s (WHO_AM_I=0x%02X)\n", mpu->typeName(), mpu->whoAmI_);
    printf_("Gyro scale: %.6f dps/LSB\n", mpu->gyrScale_);
    printf_("Accel scale: %.6f G/LSB\n", mpu->accScale_);
    printf_("Sampling rate: %u Hz\n", mpu->samplingRateHz_);

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
    mpu->read(data);
    sample_count++;

    // Print at ~10 Hz
    if (millis() - last_print_ms >= 100) {
        printf_("Sample %lu: ", sample_count);
        printf_("ax=%.2f ay=%.2f az=%.2f",
               data[0] * mpu->accScale_,
               data[1] * mpu->accScale_,
               data[2] * mpu->accScale_);
        printf_(" | gx=%.1f gy=%.1f gz=%.1f\n",
               data[3] * mpu->gyrScale_,
               data[4] * mpu->gyrScale_,
               data[5] * mpu->gyrScale_);

        last_print_ms = millis();
    }

    // Stop after 5 seconds
    if (millis() - start_ms >= 5000) {
        float read_rate_hz = sample_count / 5.0f;

        Serial.println("\n=== Test Complete ===");
        printf_("Total samples: %lu\n", sample_count);
        printf_("Read rate: %.1f Hz\n", read_rate_hz);
        Serial.println("*PASS*");
        Serial.println("*STOP*");
        while (1);
    }
}
