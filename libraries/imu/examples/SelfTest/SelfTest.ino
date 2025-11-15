/*
 * IMU Library Self-Test Example
 *
 * Demonstrates manufacturer self-test integration with TDK InvenSense factory
 * algorithms (100% preserved). Currently supports ICM-42688-P only.
 *
 * SUPPORTED IMUs:
 * - ICM-42688-P (fully supported)
 * - MPU-6000, MPU-9250, ICM-206xx (not yet implemented)
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection (NUCLEO_F411RE / BLACKPILL_F411CE)
 * - Pin assignments and SPI frequency from board configuration
 * - Supports multiple board targets with single codebase
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

    CI_LOG("\n=== IMU Library Self-Test Example ===\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();

    // Display pin configuration
    CI_LOG("Pin Configuration (BoardConfig):\n");
    printf("  CS: %d, MOSI: %d, MISO: %d, SCLK: %d\n",
           (int)BoardConfig::imu.spi.cs_pin,
           (int)BoardConfig::imu.spi.mosi_pin,
           (int)BoardConfig::imu.spi.miso_pin,
           (int)BoardConfig::imu.spi.sclk_pin);
    printf("  SPI Speed: %lu Hz\n\n", (unsigned long)BoardConfig::imu.spi.freq_hz);

    // Give ICM-42688P some time to stabilize
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

    // Run self-test
    CI_LOG("Running manufacturer self-test...\n");
    CI_LOG("This may take a few seconds...\n\n");

    int result = 0;
    std::array<int, 6> bias;
    int rc = imu.RunSelfTest(&result, &bias);

    if (rc != 0) {
        CI_LOG("ERROR: Self-test failed to execute\n");
        printf("Return code: %d\n", rc);
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    // Check results
    bool gyro_pass = (result & 0x01);
    bool accel_pass = (result & 0x02);

    CI_LOG("=== Self-Test Results ===\n");
    printf("Gyro Self-Test:  %s\n", gyro_pass ? "PASS" : "FAIL");
    printf("Accel Self-Test: %s\n\n", accel_pass ? "PASS" : "FAIL");

    if (gyro_pass && accel_pass) {
        CI_LOG("✓ All self-tests PASSED\n\n");
    } else {
        CI_LOG("✗ Self-test FAILED\n\n");
    }

    // Display bias values (converted to physical units)
    // Note: Bias values from get_st_bias are scaled by 2^16
    CI_LOG("=== Bias Values ===\n");
    printf("Gyro Bias (dps):  x=%.6f, y=%.6f, z=%.6f\n",
           (float)bias[0] / (float)(1 << 16),
           (float)bias[1] / (float)(1 << 16),
           (float)bias[2] / (float)(1 << 16));
    printf("Accel Bias (g):   x=%.6f, y=%.6f, z=%.6f\n\n",
           (float)bias[3] / (float)(1 << 16),
           (float)bias[4] / (float)(1 << 16),
           (float)bias[5] / (float)(1 << 16));

    CI_LOG("=== Test Complete ===\n");
    CI_LOG("*STOP*\n");
}

void loop() {
    // Nothing to do
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
