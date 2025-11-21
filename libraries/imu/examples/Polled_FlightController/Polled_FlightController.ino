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
 *
 * CI/HIL INTEGRATION:
 * - RTT output for automated testing
 * - Serial output for Arduino IDE
 * - Deterministic exit with "*STOP*" wildcard
 * - Build traceability with git SHA and timestamp
 */

#include <IMU.h>
#include <ci_log.h>
#include <SPI.h>
#include <libPrintf.h>

// libPrintf requires putchar_() for output routing
extern "C" void putchar_(char c) {
#ifdef USE_RTT
    char buf[2] = {c, '\0'};
    SEGGER_RTT_WriteString(0, buf);
#else
    Serial.write(c);
#endif
}

// Board configuration
#if defined(ARDUINO_BLACKPILL_F411CE)
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

void setup() {
    // Initialize communication (Serial or RTT)
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial) delay(10);
#endif

    CI_LOG("\n=== IMU Library - Polled Flight Controller Example ===\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();

    // Display pin configuration
    CI_LOG("Pin Configuration (BoardConfig):\n");
    printf("  CS: %d, MOSI: %d, MISO: %d, SCLK: %d\n",
           (int)BoardConfig::imu.spi.cs_pin,
           (int)BoardConfig::imu.spi.mosi_pin,
           (int)BoardConfig::imu.spi.miso_pin,
           (int)BoardConfig::imu.spi.sclk_pin);
    printf("  SPI Speed: %lu Hz\n", (unsigned long)BoardConfig::imu.spi.freq_hz);
    CI_LOG("  Polling Mode: No interrupt pin required\n\n");

    // Give IMU time to stabilize
    delay(5);

    // Initialize IMU (applies BALANCED preset by default)
    CI_LOG("Initializing IMU...\n");
    if (imu.Init(spi_bus, BoardConfig::imu.spi.cs_pin, BoardConfig::imu.spi.freq_hz) != IMU::Result::OK) {
        CI_LOG("ERROR: Failed to initialize IMU!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }
    CI_LOG("✓ IMU initialized successfully\n");

    // Detect chip type
    IMU::ChipType chip = imu.GetChipType();
    const char* chip_name = "UNKNOWN";
    switch (chip) {
        case IMU::ChipType::ICM42688_P: chip_name = "ICM-42688-P"; break;
        case IMU::ChipType::MPU_6000:   chip_name = "MPU-6000"; break;
        case IMU::ChipType::MPU_9250:   chip_name = "MPU-9250"; break;
        default: break;
    }
    printf("Detected chip: %s (0x%02X)\n", chip_name, static_cast<uint8_t>(chip));

    // Verify chip is supported by this library version
    if (chip != IMU::ChipType::ICM42688_P) {
        CI_LOG("ERROR: This library currently only supports ICM-42688-P!\n");
        printf("Detected: %s (0x%02X)\n", chip_name, static_cast<uint8_t>(chip));
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }
    CI_LOG("\n");

    // Apply BALANCED preset (already default, but explicit for demonstration)
    // BALANCED: 4kHz gyro, 1kHz accel, ±2000dps/±16g, optimized for 2kHz PID loop
    CI_LOG("Applying BALANCED preset...\n");
    if (imu.ApplyPreset(IMU::Preset::BALANCED) != IMU::Result::OK) {
        CI_LOG("ERROR: Failed to apply preset!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    CI_LOG("✓ IMU configured for polled operation\n");
    CI_LOG("  Preset: BALANCED (recommended for 2kHz PID loop)\n");
    CI_LOG("  Gyro: ±2000 DPS, 4kHz ODR\n");
    CI_LOG("  Accel: ±16G, 1kHz ODR\n");
    CI_LOG("  AAF: Gyro 258 Hz, Accel 170 Hz\n");
    CI_LOG("  UI Filters: Code 15, 1st-order\n");
    CI_LOG("  Mode: Continuous 2kHz polling loop\n\n");

    CI_LOG("Starting continuous 2kHz polling loop...\n");
    CI_LOG("Will print 20 samples over ~10 seconds\n\n");
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

            printf("[%lu] Accel(g): %.3f, %.3f, %.3f | Gyro(dps): %.2f, %.2f, %.2f\n",
                   sample_count, ax, ay, az, gx, gy, gz);
        }

        // Stop after 20 prints (10 seconds)
        if (sample_count >= 20000) {
            printf("\n✓ Completed %lu samples\n", (unsigned long)sample_count);
            CI_LOG("*STOP*\n");
            while (1) delay(1000);
        }
    }

    // Maintain 2kHz loop rate (500us period)
    while (micros() - current_time < inv_freq);
}
