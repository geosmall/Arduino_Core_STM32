/*
 * IMU Library - Magnetometer Calibration Example
 *
 * Interactive magnetometer calibration workflow for MPU-9250/9255.
 * Calculates hard iron (bias) and soft iron (scale) corrections.
 *
 * CALIBRATION PROCEDURE:
 * 1. Detect MPU-9250/9255 chip
 * 2. Initialize magnetometer
 * 3. Prompt user to start calibration
 * 4. User performs figure-8 motion for 15 seconds
 * 5. Display calibration values (bias + scale factors)
 * 6. Show corrected vs uncorrected readings
 *
 * FIGURE-8 MOTION:
 * - Rotate IMU smoothly in all orientations
 * - Move in figure-8 pattern (like drawing ∞ in 3D space)
 * - Ensure all axes experience full range of motion
 * - 15 seconds = 1500 samples at 100 Hz
 *
 * CALIBRATION VALUES:
 * - Hard iron bias (µT): Offset from sphere center
 * - Soft iron scale: Normalize ellipsoid to sphere
 * - Store these values for use in flight code
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection
 * - MPU-9250/9255 required (will error on other chips)
 *
 * CI/HIL INTEGRATION:
 * - RTT output for automated testing
 * - Serial output for Arduino IDE
 */

#include <IMU.h>
#include <ci_log.h>
#include <SPI.h>
#include <libPrintf.h>

// CI_PRINTF requires putchar_() for libPrintf output routing
extern "C" void putchar_(char c) {
#ifdef USE_RTT
    SEGGER_RTT_PutChar(0, c);
#else
    Serial.write(c);
#endif
}

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

void setup() {
    // Initialize communication (Serial or RTT)
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial) delay(10);
#endif

    CI_LOG("\n=== IMU Library - Magnetometer Calibration ===\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();

    // Display pin configuration
    CI_LOG("\nPin Configuration (BoardConfig):\n");
    CI_PRINTF("  CS: %d, MOSI: %d, MISO: %d, SCLK: %d\n",
           (int)BoardConfig::imu.spi.cs_pin,
           (int)BoardConfig::imu.spi.mosi_pin,
           (int)BoardConfig::imu.spi.miso_pin,
           (int)BoardConfig::imu.spi.sclk_pin);
    CI_PRINTF("  SPI Speed: %lu Hz\n\n", (unsigned long)BoardConfig::imu.spi.freq_hz);

    // Initialize IMU
    CI_LOG("Initializing IMU...\n");
    if (imu.Init(spi_bus, BoardConfig::imu.spi.cs_pin, BoardConfig::imu.spi.freq_hz) != IMU::Result::OK) {
        CI_LOG("ERROR: IMU initialization failed!\n");
        CI_LOG("*STOP*\n");
        while (1);
    }

    // Detect chip type
    IMU::ChipType chip = imu.GetChipType();
    CI_LOG("IMU Chip: ");
    switch (chip) {
        case IMU::ChipType::MPU_9250:
            CI_LOG("MPU-9250 (0x71) ✓\n");
            break;
        default:
            CI_LOG("NOT MPU-9250/9255!\n");
            CI_LOG("ERROR: Magnetometer calibration requires MPU-9250 or MPU-9255\n");
            CI_PRINTF("  Detected chip: 0x%02X\n", (uint8_t)chip);
            CI_LOG("*STOP*\n");
            while (1);
    }

    // Check magnetometer support
    if (!imu.HasMagnetometer()) {
        CI_LOG("ERROR: No magnetometer detected!\n");
        CI_LOG("*STOP*\n");
        while (1);
    }

    // Initialize magnetometer
    CI_LOG("\nInitializing magnetometer (AK8963)...\n");
    if (imu.InitMagnetometer() != IMU::Result::OK) {
        CI_LOG("ERROR: Magnetometer initialization failed!\n");
        CI_LOG("*STOP*\n");
        while (1);
    }
    CI_LOG("✓ Magnetometer initialized\n");

    // Show uncalibrated readings
    CI_LOG("\n=== Uncalibrated Readings (10 samples) ===\n");
    for (int i = 0; i < 10; i++) {
        float mx, my, mz;
        if (imu.ReadMagnetometer(mx, my, mz) == IMU::Result::OK) {
            CI_PRINTF("  Mag: X=%.2f, Y=%.2f, Z=%.2f uT\n", mx, my, mz);
        }
        delay(100);
    }

    // Calibration instructions
    CI_LOG("\n=== Magnetometer Calibration ===\n");
    CI_LOG("INSTRUCTIONS:\n");
    CI_LOG("  1. Hold IMU firmly\n");
    CI_LOG("  2. Move IMU in figure-8 pattern (∞ shape in 3D)\n");
    CI_LOG("  3. Rotate through all orientations smoothly\n");
    CI_LOG("  4. Continue for 15 seconds (1500 samples)\n");
    CI_LOG("\nCalibration will start in 3 seconds...\n");
    delay(3000);

    // Run calibration
    CI_LOG("\nSTART MOVING NOW!\n");
    CI_LOG("Collecting samples");

    if (imu.CalibrateMagnetometer() == IMU::Result::OK) {
        CI_LOG(" DONE\n");
        CI_LOG("✓ Calibration complete\n");
    } else {
        CI_LOG("\nERROR: Calibration failed!\n");
        CI_LOG("*STOP*\n");
        while (1);
    }

    // Display calibration values
    float bias_x, bias_y, bias_z;
    float scale_x, scale_y, scale_z;
    imu.GetMagCalibration(bias_x, bias_y, bias_z, scale_x, scale_y, scale_z);

    CI_LOG("\n=== Calibration Values ===\n");
    CI_LOG("Hard Iron Bias (µT):\n");
    CI_PRINTF("  bias_x = %.2f\n", bias_x);
    CI_PRINTF("  bias_y = %.2f\n", bias_y);
    CI_PRINTF("  bias_z = %.2f\n", bias_z);
    CI_LOG("\nSoft Iron Scale Factors:\n");
    CI_PRINTF("  scale_x = %.4f\n", scale_x);
    CI_PRINTF("  scale_y = %.4f\n", scale_y);
    CI_PRINTF("  scale_z = %.4f\n", scale_z);

    CI_LOG("\n=== Store These Values ===\n");
    CI_LOG("Use these values in your flight code:\n");
    CI_PRINTF("  imu.SetMagCalibration(%.2f, %.2f, %.2f, %.4f, %.4f, %.4f);\n",
              bias_x, bias_y, bias_z, scale_x, scale_y, scale_z);

    // Show calibrated vs uncalibrated readings
    CI_LOG("\n=== Calibrated Readings (10 samples) ===\n");
    for (int i = 0; i < 10; i++) {
        float mx, my, mz;
        if (imu.ReadMagnetometer(mx, my, mz) == IMU::Result::OK) {
            CI_PRINTF("  Mag: X=%.2f, Y=%.2f, Z=%.2f uT\n", mx, my, mz);
        }
        delay(100);
    }

    CI_LOG("\nMagnetometer calibration complete ✓\n");
    CI_LOG("*STOP*\n");
    while (1);
}

void loop() {
    // Not used - calibration runs once in setup()
}
