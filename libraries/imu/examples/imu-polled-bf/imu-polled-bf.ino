/*
 * IMU Library - Polled Betaflight Configuration Example
 *
 * Demonstrates polling-based IMU data acquisition with EXACT Betaflight defaults:
 * - FSR: Gyro ±2000 DPS, Accel ±16G (wide range for extreme maneuvers)
 * - ODR: Configurable 8k/4k/2k/1k (both gyro and accel match)
 * - AAF: Both 258 Hz (Betaflight standard anti-aliasing)
 * - UI Filters: Code 15 (low-latency), order left at reset default (2nd-order)
 *
 * This configuration exactly matches the Betaflight ICM-42688-P driver
 * (accgyro_spi_icm426xx.c). Betaflight does NOT explicitly write the UI filter
 * order registers - it relies on the chip's power-on reset default (2nd order).
 * This example mirrors that behavior by skipping order writes.
 *
 * Betaflight relies on robust software filters (gyro LPF, D-term, dynamic notch,
 * RPM) for fine noise control, so hardware filters are kept minimal to reduce delay.
 *
 * For MPU-6000 DLPF simulation (narrower FSR, optimized accel filtering),
 * see imu-polled-mpu6000-sim.ino example.
 *
 * BETAFLIGHT ODR SELECTION:
 * Uncomment ONE of the BF_ODR_* defines below to set the output data rate.
 * Betaflight uses 8kHz on capable targets, with fallback to 4k/2k/1k on others.
 *
 * POLLING METHODOLOGY:
 * - Loop rate: 2kHz continuous polling (common flight controller rate)
 * - Higher ODRs (4k/8k) provide oversampling opportunity
 * - Lower ODRs (1k/2k) may match loop rate exactly
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection (NUCLEO_F411RE / BLACKPILL_F411CE)
 * - Pin assignments and SPI frequency from board configuration
 * - No interrupt pin required
 *
 * REFERENCE:
 * - Icm42688p Analysis & Guidance.md - Filter configuration guide (Section 2, 9)
 * - Betaflight driver: src/main/drivers/accgyro/accgyro_spi_icm426xx.c
 *
 * CI/HIL INTEGRATION:
 * - RTT output for automated testing
 * - Serial output for Arduino IDE
 * - Deterministic exit with "*STOP*" wildcard
 * - Build traceability with git SHA and timestamp
 */

// ========================================================================
// BETAFLIGHT ODR CONFIGURATION
// Uncomment ONE option to select IMU output data rate
// ========================================================================
#define BF_ODR_8K   // Default for capable targets (8kHz gyro + accel)
// #define BF_ODR_4K   // 4kHz gyro + accel
// #define BF_ODR_2K   // 2kHz gyro + accel
// #define BF_ODR_1K   // 1kHz gyro + accel

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

// Determine ODR based on configuration
#if defined(BF_ODR_8K)
    #define GYRO_ODR IMU::GyroODR::gyr_odr8k
    #define ACCEL_ODR IMU::AccelODR::accel_odr8k
    #define ODR_STRING "8kHz"
#elif defined(BF_ODR_4K)
    #define GYRO_ODR IMU::GyroODR::gyr_odr4k
    #define ACCEL_ODR IMU::AccelODR::accel_odr4k
    #define ODR_STRING "4kHz"
#elif defined(BF_ODR_2K)
    #define GYRO_ODR IMU::GyroODR::gyr_odr2k
    #define ACCEL_ODR IMU::AccelODR::accel_odr2k
    #define ODR_STRING "2kHz"
#elif defined(BF_ODR_1K)
    #define GYRO_ODR IMU::GyroODR::gyr_odr1k
    #define ACCEL_ODR IMU::AccelODR::accel_odr1k
    #define ODR_STRING "1kHz"
#else
    #error "No BF_ODR_* defined! Uncomment one of: BF_ODR_8K, BF_ODR_4K, BF_ODR_2K, BF_ODR_1K"
#endif

void setup() {
    // Initialize communication (Serial or RTT)
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial) delay(10);
#endif

    CI_LOG("\n=== IMU Library - Betaflight Configuration ===\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();

    // Display configuration
    CI_LOG("Betaflight ICM-42688-P Configuration:\n");
    CI_LOG("  ODR: " ODR_STRING " (gyro + accel)\n");
    CI_LOG("  FSR: ±2000 DPS gyro, ±16G accel\n");
    CI_LOG("  AAF: 258 Hz (both)\n");
    CI_LOG("  UI: Code 15 (low-latency), 2nd-order\n\n");

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

    // Configure IMU - Betaflight defaults
    CI_LOG("Configuring IMU (Betaflight defaults)...\n");

    // 1. Set full-scale range (BF: ±2000 DPS gyro, ±16G accel)
    if (imu.SetGyroFSR(IMU::GyroFS::dps2000) != 0 ||
        imu.SetAccelFSR(IMU::AccelFS::gpm16) != 0) {
        CI_LOG("ERROR: Failed to set FSR!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    // 2. Enable sensors for continuous data acquisition
    if (imu.EnableAccelLNMode() != 0 || imu.EnableGyroLNMode() != 0) {
        CI_LOG("ERROR: Failed to enable sensors!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    // 3. Set output data rates (configured via #define above)
    if (imu.SetAccelODR(ACCEL_ODR) != 0 ||
        imu.SetGyroODR(GYRO_ODR) != 0) {
        CI_LOG("ERROR: Failed to set ODR!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    // 4. Configure AAF (Anti-Alias Filter) - BF: 258 Hz for both
    //    Reference: Betaflight_Filtering.md Section 3
    imu.SetGyroFilterHz(ICM42688P_AAF_258HZ);
    imu.SetAccelFilterHz(ICM42688P_AAF_258HZ);

    // 5. Configure UI filters - BF: Code 15 (low-latency), leave order at reset default
    //    Reference: Icm42688p Analysis & Guidance.md Section 2, 9
    //    Betaflight does NOT write order registers - relies on chip reset default (2nd order)
    //    Using macro: ICM42688P_SET_UI_FILTERS_BETAFLIGHT (BW=15, order=-1,-1)
    if (imu.SetUiFilters(15, -1, -1) != 0) {
        CI_LOG("ERROR: Failed to set UI filters!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    // Verify all filter settings by reading back registers
    CI_LOG("Verifying filter configuration...\n");

    // Verify AAF
    if (imu.VerifyAafConfig(ICM42688P_AAF_258HZ, ICM42688P_AAF_258HZ) != 0) {
        CI_LOG("ERROR: AAF verification failed!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    // Verify UI filters (code 15, skip order verification with -1 since BF doesn't write them)
    if (imu.VerifyUiFilters(15, -1, -1) != 0) {
        CI_LOG("ERROR: UI filter verification failed!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    CI_LOG("✓ Filter configuration verified by hardware readback\n\n");

    CI_LOG("✓ IMU configured for Betaflight operation\n");
    CI_LOG("  Gyro: ±2000 DPS, " ODR_STRING " ODR\n");
    CI_LOG("  Accel: ±16G, " ODR_STRING " ODR\n");
    CI_LOG("  AAF: 258 Hz (both)\n");
    CI_LOG("  UI: Code 15 (low-latency), 2nd-order\n");
    CI_LOG("  Mode: Continuous 2kHz polling loop\n\n");

    CI_LOG("Starting continuous 2kHz polling loop...\n");
    CI_LOG("Will print 20 samples over ~10 seconds\n\n");
}

void loop() {
    static unsigned long current_time = 0;
    static unsigned long loop_timer = 0;
    static uint32_t sample_count = 0;
    static const uint32_t loop_freq = 2000; // 2kHz loop rate
    static const uint32_t inv_freq = 1000000 / loop_freq; // 500us period

    current_time = micros();

    // Read IMU at 2kHz
    std::array<int16_t, 6> imu_data;
    if (imu.ReadIMU6(imu_data) == 0) {
        sample_count++;

        // Print every 500 samples (~4Hz at 2kHz loop)
        if (sample_count % 500 == 0) {
            printf("Sample %lu: ", sample_count);
            printf("Accel[%6d,%6d,%6d] ",
                   imu_data[0], imu_data[1], imu_data[2]);
            printf("Gyro[%6d,%6d,%6d]\n",
                   imu_data[3], imu_data[4], imu_data[5]);
        }

        // Exit after 10000 samples (~5 seconds)
        if (sample_count >= 10000) {
            CI_LOG("\n✓ Data collection complete\n");
            CI_LOG("\n=== Test Complete ===\n");
            CI_LOG("*STOP*\n");
            while(1); // Halt
        }
    }

    // Regulate loop rate to 2kHz
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
