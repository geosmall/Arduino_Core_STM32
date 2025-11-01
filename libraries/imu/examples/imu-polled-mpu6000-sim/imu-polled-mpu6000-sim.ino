/*
 * IMU Library - MPU-6000 DLPF Simulation Example
 *
 * Demonstrates polling-based IMU data acquisition that simulates MPU-6000 DLPF behavior:
 * - Gyro: ±250 DPS @ 4kHz ODR (high resolution for precise flight control)
 * - Accel: ±2G @ 1kHz ODR (sufficient for orientation/level flight)
 * - AAF: Gyro 258 Hz, Accel 170 Hz (matches MPU-6000 DLPF 260/256 Hz feel)
 * - UI Filters: Code 0 (ODR/2), 1st-order (minimal phase lag, lets AAF dominate)
 *
 * This configuration mimics the MPU-6000 DLPF approach where the anti-alias filter
 * defines the primary corner frequency and the UI filter stays wide with minimal
 * phase lag. This balances resolution, noise rejection, and latency for stable
 * flight applications like dRehmFlight.
 *
 * For Betaflight-style configuration (wider FSR, low-latency UI filters),
 * see imu-polled-bf.ino example.
 *
 * WHEN TO USE POLLING vs INTERRUPTS:
 * - Use polling when your main loop runs at a fixed rate (e.g., 2kHz flight controller)
 * - Use interrupts when you want to read data only when available (event-driven)
 * - Polling is simpler and requires no interrupt pin
 * - Interrupts are more efficient for variable-rate or slow loops
 *
 * POLLING CONSIDERATIONS:
 * - If loop rate matches IMU ODR: Minimal duplicate/missed reads
 * - If loop faster than ODR: May read same data multiple times
 * - If loop slower than ODR: May miss data samples
 * - This example: 2kHz loop with 4kHz gyro, 1kHz accel
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection (NUCLEO_F411RE / BLACKPILL_F411CE)
 * - Pin assignments and SPI frequency from board configuration
 * - No interrupt pin required
 *
 * REFERENCE:
 * - Icm42688p Analysis & Guidance.md - MPU-6000 parity (Section 10)
 *
 * CI/HIL INTEGRATION:
 * - RTT output for automated testing
 * - Serial output for Arduino IDE
 * - Deterministic exit with "*STOP*" wildcard
 * - Build traceability with git SHA and timestamp
 */

#include <IMU.h>
#include <icm42688p.h>
#include <ci_log.h>
#include <SPI.h>
#include <libPrintf.h>

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

    CI_LOG("\n=== IMU Library - Polled ODR/2 1st-Order Filter Example ===\n");
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

    // Initialize IMU
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

    // Configure IMU for polled operation
    CI_LOG("Configuring IMU...\n");

    // Set full-scale range (matching dRehmFlight: ±250 DPS gyro, ±2G accel)
    if (imu.SetGyroFSR(IMU::GyroFS::dps250) != 0 ||
        imu.SetAccelFSR(IMU::AccelFS::gpm2) != 0) {
        CI_LOG("ERROR: Failed to set FSR!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    // Enable sensors for continuous data acquisition
    if (imu.EnableAccelLNMode() != 0 || imu.EnableGyroLNMode() != 0) {
        CI_LOG("ERROR: Failed to enable sensors!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    // Set sample rates (Gyro 4kHz, Accel 1kHz - matching dRehmFlight)
    // Higher gyro ODR provides better resolution for fast movements
    // Lower accel ODR sufficient for orientation/level flight
    if (imu.SetAccelODR(IMU::AccelODR::accel_odr1k) != 0 ||
        imu.SetGyroODR(IMU::GyroODR::gyr_odr4k) != 0) {
        CI_LOG("ERROR: Failed to set ODR!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    // Configure IMU filters (AAF + UI) - MPU-6000 DLPF simulation
    // AAF (Anti-Alias Filter): Protects against aliasing at sensor front-end
    //   Gyro: 258 Hz (matches MPU-6000 DLPF 260 Hz)
    //   Accel: 170 Hz (good vibration rejection for level mode)
    imu.SetGyroFilterHz(ICM42688P_AAF_258HZ);
    imu.SetAccelFilterHz(ICM42688P_AAF_170HZ);

    // UI Filter: MPU-6000 DLPF simulation (ODR/2, 1st-order)
    //   Reference: Icm42688p Analysis & Guidance.md Section 10 (FAQ - MPU-6000 parity)
    //   Lets AAF define the corner frequency with minimal phase lag from UI stage
    //   Using macro: ICM42688P_SET_UI_FILTERS_MPU6000_SIM (BW=0, order=1,1)
    if (imu.SetUiFilters(0, 1, 1) != 0) {
        CI_LOG("ERROR: Failed to set UI filters!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    // Verify filter configuration by reading back registers
    CI_LOG("Verifying filter configuration...\n");
    if (imu.VerifyAafConfig(ICM42688P_AAF_258HZ, ICM42688P_AAF_170HZ) != 0) {
        CI_LOG("ERROR: AAF verification failed!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }
    if (imu.VerifyUiFilters(0, 1, 1) != 0) {
        CI_LOG("ERROR: UI filter verification failed!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }
    CI_LOG("✓ Filter configuration verified by hardware readback\n\n");

    CI_LOG("✓ IMU configured for polled operation\n");
    CI_LOG("  Accel: ±2G, 1kHz ODR\n");
    CI_LOG("  Gyro: ±250 DPS, 4kHz ODR\n");
    CI_LOG("  Filters: AAF (Gyro 258 Hz, Accel 170 Hz), UI (1st-order, ODR/2)\n");
    CI_LOG("  Mode: Continuous 2kHz loop (matching dRehmFlight)\n\n");

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

        // Print every 500 samples (~4Hz at 2kHz loop, matching dRehmFlight debug rate)
        if (sample_count % 500 == 0) {
            printf("Sample %lu: ", sample_count);
            printf("Accel[%6d,%6d,%6d] ",
                   imu_data[0], imu_data[1], imu_data[2]);
            printf("Gyro[%6d,%6d,%6d]\n",
                   imu_data[3], imu_data[4], imu_data[5]);
        }

        // Exit after 20 samples (~10 seconds)
        if (sample_count >= 10000) {
            CI_LOG("\n✓ Data collection complete\n");
            CI_LOG("\n=== Test Complete ===\n");
            CI_LOG("*STOP*\n");
            while(1); // Halt
        }
    }

    // Regulate loop rate to 2kHz (like dRehmFlight's loopRate function)
    loop_timer = micros();
    while (inv_freq > (loop_timer - current_time)) {
        loop_timer = micros();
    }
}

/* --------------------------------------------------------------------------------------
 *  libPrintf putchar_ implementation for RTT/Serial routing
 * -------------------------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

void putchar_(char c) {
#ifdef USE_RTT
    char buf[2] = {c, '\0'};
    SEGGER_RTT_WriteString(0, buf);
#else
    Serial.print(c);
#endif
}

#ifdef __cplusplus
}
#endif
