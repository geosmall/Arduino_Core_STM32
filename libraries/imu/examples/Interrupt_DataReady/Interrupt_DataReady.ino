/*
 * IMU Library - Interrupt-Driven Raw Data Example
 *
 * Demonstrates interrupt-driven raw sensor data acquisition using the IMU library.
 * Uses data-ready interrupt on INT1 pin to trigger readings when new data is available.
 *
 * Hardware Configuration:
 * - Uses BoardConfig for automatic board detection (NUCLEO_F411RE / BLACKPILL_F411CE)
 * - Pin assignments and SPI frequency from board configuration
 * - Interrupt pin configured for data-ready signaling
 */

#include <IMU.h>
#include <SPI.h>

// Board configuration - Multi-board support
#if defined(ARDUINO_BKMN_NERO)
#include "../../../../targets/BKMN-NERO.h"
#elif defined(ARDUINO_BLACKPILL_F411CE)
#include "../../../../targets/BLACKPILL_F411CE.h"
#else
#include "../../../../targets/NUCLEO_F411RE_JHEF411.h"
#endif

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin,
                 BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin,
                 BoardConfig::imu.spi.get_ssel_pin());

// Create IMU instance
IMU imu;

// Interrupt flag
volatile bool data_ready = false;

// Interrupt handler
void imu_data_ready_handler() {
    data_ready = true;
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println("\n=== IMU Library - Interrupt-Driven Data Example ===\n");

    // Display pin configuration
    Serial.println("Pin Configuration (BoardConfig):");
    Serial.printf("  CS: %d, MOSI: %d, MISO: %d, SCLK: %d\n",
           (int)BoardConfig::imu.spi.cs_pin,
           (int)BoardConfig::imu.spi.mosi_pin,
           (int)BoardConfig::imu.spi.miso_pin,
           (int)BoardConfig::imu.spi.sclk_pin);
    Serial.printf("  SPI Speed: %lu Hz\n", (unsigned long)BoardConfig::imu.spi.freq_hz);

    if (BoardConfig::imu.int_pin != 0) {
        Serial.printf("  Interrupt Pin: %d\n\n", (int)BoardConfig::imu.int_pin);
    } else {
        Serial.println("  Interrupt Pin: None configured");
        Serial.println("ERROR: This example requires interrupt pin!");
        while (1) delay(1000);
    }

    // Give IMU time to stabilize
    delay(5);

    // Initialize IMU
    Serial.println("Initializing IMU...");
    if (imu.Init(spi_bus, BoardConfig::imu.spi.cs_pin, BoardConfig::imu.spi.freq_hz) != IMU::Result::OK) {
        Serial.println("ERROR: Failed to initialize IMU!");
        while (1) delay(1000);
    }
    Serial.println("IMU initialized successfully");

    // Detect chip type
    IMU::ChipType chip = imu.GetChipType();
    const char* chip_name = "UNKNOWN";
    switch (chip) {
        case IMU::ChipType::ICM42688_P: chip_name = "ICM-42688-P"; break;
        case IMU::ChipType::MPU_6000:   chip_name = "MPU-6000"; break;
        case IMU::ChipType::MPU_9250:   chip_name = "MPU-9250"; break;
        case IMU::ChipType::ICM20601:   chip_name = "ICM-20601"; break;
        case IMU::ChipType::ICM20602:   chip_name = "ICM-20602"; break;
        case IMU::ChipType::ICM20689:   chip_name = "ICM-20689"; break;
        default: break;
    }
    Serial.printf("Detected chip: %s (0x%02X)\n\n", chip_name, static_cast<uint8_t>(chip));

    // Configure IMU for interrupt-driven operation
    Serial.println("Configuring IMU...");

    // Apply SAFE preset for interrupt mode (1kHz with DLPF enabled)
    if (imu.ApplyPreset(IMU::Preset::SAFE) != IMU::Result::OK) {
        Serial.println("ERROR: Failed to apply SAFE preset!");
        while (1) delay(1000);
    }

    // Configure interrupt pin
    pinMode(BoardConfig::imu.int_pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(BoardConfig::imu.int_pin),
                    imu_data_ready_handler, RISING);

    // Enable data ready interrupt on INT
    if (imu.EnableDataReadyInt() != 0) {
        Serial.println("ERROR: Failed to enable data ready interrupt!");
        while (1) delay(1000);
    }

    Serial.println("IMU configured for interrupt-driven operation");
    Serial.println("  Preset: SAFE (1kHz with DLPF filtering)");
    Serial.println("  Gyro: +/-2000 DPS, 1kHz ODR");
    Serial.println("  Accel: +/-16G, 1kHz ODR");
    Serial.println("  INT1: Data Ready enabled\n");

    Serial.println("Streaming interrupt-driven IMU data...\n");
}

void loop() {
    static int sample_count = 0;

    if (data_ready) {
        data_ready = false;

        std::array<int16_t, 6> imu_data;
        if (imu.ReadIMU6(imu_data) == 0) {
            sample_count++;

            // Print every 20th sample (~50 Hz output at 1kHz sampling)
            if (sample_count % 20 == 0) {
                Serial.printf("Sample %d: ", sample_count);
                Serial.printf("Accel[%6d,%6d,%6d] ",
                       imu_data[0], imu_data[1], imu_data[2]);
                Serial.printf("Gyro[%6d,%6d,%6d]\n",
                       imu_data[3], imu_data[4], imu_data[5]);
            }
        }
    }
}
