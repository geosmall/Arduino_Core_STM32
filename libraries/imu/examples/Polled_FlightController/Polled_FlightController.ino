/*
 * IMU Library - Polled Flight Controller Example
 *
 * Demonstrates polling-based IMU data acquisition using the preset API:
 * - Uses ApplyPreset(BALANCED) for validated 2kHz PID loop configuration
 * - Gyro: 4kHz ODR, ±2000 DPS
 * - Accel: 1kHz ODR, ±16G
 * - AAF/UI filters: Configured per imu_hal.md BALANCED preset
 *
 * WHEN TO USE POLLING vs INTERRUPTS:
 * - Use polling when your main loop runs at a fixed rate (e.g., 2kHz flight controller)
 * - Use interrupts when you want to read data only when available (event-driven)
 * - Polling is simpler and requires no interrupt pin
 * - Interrupts are more efficient for variable-rate or slow loops
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection (NUCLEO_F411RE / BLACKPILL_F411CE)
 * - Pin assignments and SPI frequency from board configuration
 * - No interrupt pin required
 */

#include <IMU.h>
#include <SPI.h>
#include <libPrintf.h>

// printf_() output routing for libPrintf
extern "C" void putchar_(char c) {
    Serial.write(c);
}

// Board configuration - Multi-board support
#if defined(ARDUINO_BKMN_NERO)
#include "../../../../targets/BKMN-NERO.h"
#elif defined(ARDUINO_BLACKPILL_F411CE)
#include "../../../../targets/BLACKPILL_F411CE.h"
#else
#include "../../../../targets/NUCLEO_F411RE_HIL001.h"
#endif

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin,
                 BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin,
                 BoardConfig::imu.spi.get_ssel_pin());

// Create IMU instance
IMU imu;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println("\n=== IMU Library - Polled Flight Controller Example ===");

    // Display pin configuration
    Serial.println("Pin Configuration (BoardConfig):");
    printf_("  CS: %d, MOSI: %d, MISO: %d, SCLK: %d\n",
           (int)BoardConfig::imu.spi.cs_pin,
           (int)BoardConfig::imu.spi.mosi_pin,
           (int)BoardConfig::imu.spi.miso_pin,
           (int)BoardConfig::imu.spi.sclk_pin);
    printf_("  SPI Speed: %lu Hz\n", (unsigned long)BoardConfig::imu.spi.freq_hz);
    Serial.println("  Polling Mode: No interrupt pin required\n");

    // Give IMU time to stabilize
    delay(5);

    // Initialize IMU (applies BALANCED preset by default)
    Serial.println("Initializing IMU...");
    if (imu.Init(spi_bus, BoardConfig::imu.spi.cs_pin, BoardConfig::imu.spi.freq_hz) != IMU::Result::OK) {
        Serial.println("ERROR: Failed to initialize IMU!");
        Serial.println("*STOP*");
        while (1) delay(1000);
    }
    Serial.println("✓ IMU initialized successfully");

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
    printf_("Detected chip: %s (0x%02X)\n", chip_name, static_cast<uint8_t>(chip));

    Serial.println();

    // Apply BALANCED preset (already default, but explicit for demonstration)
    // BALANCED: 4kHz gyro, 1kHz accel, ±2000dps/±16g, optimized for 2kHz PID loop
    Serial.println("Applying BALANCED preset...");
    if (imu.ApplyPreset(IMU::Preset::BALANCED) != IMU::Result::OK) {
        Serial.println("ERROR: Preset apply/verify failed!");
        Serial.println("*STOP*");
        while (1) delay(1000);
    }
    Serial.println("Preset verified OK");

    Serial.println("✓ IMU configured for polled operation");
    Serial.println("  Preset: BALANCED (recommended for 2kHz PID loop)");
    Serial.println("  Gyro: ±2000 DPS, 4kHz ODR");
    Serial.println("  Accel: ±16G, 1kHz ODR");
    Serial.println("  AAF: Gyro 258 Hz, Accel 170 Hz");
    Serial.println("  UI Filters: Code 15, 1st-order");
    Serial.println("  Mode: Continuous 2kHz polling loop\n");

    Serial.println("Starting continuous 2kHz polling loop...");
    Serial.println("Will print 20 samples over ~10 seconds\n");
}

void loop() {
    static unsigned long current_time = 0;
    static unsigned long loop_timer = 0;
    static uint32_t sample_count = 0;
    static const uint32_t loop_freq = 2000; // 2kHz like dRehmFlight
    static const uint32_t inv_freq = 1000000 / loop_freq; // 500us period

    current_time = micros();

    // Read IMU at 2kHz
    std::array<int16_t, 6> imu_data;
    if (imu.ReadIMU6(imu_data) == 0) {
        sample_count++;

        // Print every 500ms (every 1000 samples at 2kHz)
        if (sample_count % 1000 == 0) {
            // Convert raw values to physical units
            float ax = imu_data[0] / imu.GetAccelSensitivity();
            float ay = imu_data[1] / imu.GetAccelSensitivity();
            float az = imu_data[2] / imu.GetAccelSensitivity();
            float gx = imu_data[3] / imu.GetGyroSensitivity();
            float gy = imu_data[4] / imu.GetGyroSensitivity();
            float gz = imu_data[5] / imu.GetGyroSensitivity();

            printf_("[%lu] Accel(g): %.3f, %.3f, %.3f | Gyro(dps): %.2f, %.2f, %.2f\n",
                   sample_count, ax, ay, az, gx, gy, gz);
        }

        // Stop after 20 prints (10 seconds)
        if (sample_count >= 20000) {
            printf_("\n✓ Completed %lu samples\n", (unsigned long)sample_count);
            Serial.println("*STOP*");
            while (1) delay(1000);
        }
    }

    // Maintain 2kHz loop rate (500us period)
    while (micros() - current_time < inv_freq);
}
