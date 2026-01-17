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
 * License: GPL v3 (Betaflight-derived library)
 */

#include <IMU.h>
#include <SPI.h>
#include <libPrintf.h>

// libPrintf output routing
extern "C" void putchar_(char c) {
    Serial.write(c);
}

// Board configuration - Multi-board support
#if defined(ARDUINO_BKMN_NERO)
#include "../../../../targets/BKMN-NERO.h"
#elif defined(ARDUINO_BLACKPILL_F411CE)
#include "../../../../targets/BLACKPILL_F411CE.h"
#else
#include "../../../../targets/NUCLEO_F411RE_LITTLEFS.h"
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

    Serial.println("\n=== IMU Library - Magnetometer Calibration ===");

    // Display pin configuration
    Serial.println("\nPin Configuration (BoardConfig):");
    printf_("  CS: %d, MOSI: %d, MISO: %d, SCLK: %d\n",
           (int)BoardConfig::imu.spi.cs_pin,
           (int)BoardConfig::imu.spi.mosi_pin,
           (int)BoardConfig::imu.spi.miso_pin,
           (int)BoardConfig::imu.spi.sclk_pin);
    printf_("  SPI Speed: %lu Hz\n\n", (unsigned long)BoardConfig::imu.spi.freq_hz);

    // Initialize IMU
    Serial.println("Initializing IMU...");
    if (imu.Init(spi_bus, BoardConfig::imu.spi.cs_pin, BoardConfig::imu.spi.freq_hz) != IMU::Result::OK) {
        Serial.println("ERROR: IMU initialization failed!");
        Serial.println("*STOP*");
        while (1);
    }

    // Detect chip type
    IMU::ChipType chip = imu.GetChipType();
    Serial.print("IMU Chip: ");
    switch (chip) {
        case IMU::ChipType::MPU_9250:
            Serial.println("MPU-9250 (0x71) ✓");
            break;
        default:
            Serial.println("NOT MPU-9250/9255!");
            Serial.println("ERROR: Magnetometer calibration requires MPU-9250 or MPU-9255");
            printf_("  Detected chip: 0x%02X\n", (uint8_t)chip);
            Serial.println("*STOP*");
            while (1);
    }

    // Check magnetometer support
    if (!imu.HasMagnetometer()) {
        Serial.println("ERROR: No magnetometer detected!");
        Serial.println("*STOP*");
        while (1);
    }

    // Initialize magnetometer
    Serial.println("\nInitializing magnetometer (AK8963)...");
    if (imu.InitMagnetometer() != IMU::Result::OK) {
        Serial.println("ERROR: Magnetometer initialization failed!");
        Serial.println("*STOP*");
        while (1);
    }
    Serial.println("✓ Magnetometer initialized");

    // Show uncalibrated readings
    Serial.println("\n=== Uncalibrated Readings (10 samples) ===");
    for (int i = 0; i < 10; i++) {
        float mx, my, mz;
        if (imu.ReadMagnetometer(mx, my, mz) == IMU::Result::OK) {
            printf_("  Mag: X=%.2f, Y=%.2f, Z=%.2f uT\n", mx, my, mz);
        }
        delay(100);
    }

    // Calibration instructions
    Serial.println("\n=== Magnetometer Calibration ===");
    Serial.println("INSTRUCTIONS:");
    Serial.println("  1. Hold IMU firmly");
    Serial.println("  2. Move IMU in figure-8 pattern (∞ shape in 3D)");
    Serial.println("  3. Rotate through all orientations smoothly");
    Serial.println("  4. Continue for 15 seconds (1500 samples)");
    Serial.println("\nCalibration will start in 3 seconds...");
    delay(3000);

    // Run calibration
    Serial.println("\nSTART MOVING NOW!");
    Serial.print("Collecting samples");

    if (imu.CalibrateMagnetometer() == IMU::Result::OK) {
        Serial.println(" DONE");
        Serial.println("✓ Calibration complete");
    } else {
        Serial.println("\nERROR: Calibration failed!");
        Serial.println("*STOP*");
        while (1);
    }

    // Display calibration values
    float bias_x, bias_y, bias_z;
    float scale_x, scale_y, scale_z;
    imu.GetMagCalibration(bias_x, bias_y, bias_z, scale_x, scale_y, scale_z);

    Serial.println("\n=== Calibration Values ===");
    Serial.println("Hard Iron Bias (µT):");
    printf_("  bias_x = %.2f\n", bias_x);
    printf_("  bias_y = %.2f\n", bias_y);
    printf_("  bias_z = %.2f\n", bias_z);
    Serial.println("\nSoft Iron Scale Factors:");
    printf_("  scale_x = %.4f\n", scale_x);
    printf_("  scale_y = %.4f\n", scale_y);
    printf_("  scale_z = %.4f\n", scale_z);

    Serial.println("\n=== Store These Values ===");
    Serial.println("Use these values in your flight code:");
    printf_("  imu.SetMagCalibration(%.2f, %.2f, %.2f, %.4f, %.4f, %.4f);\n",
              bias_x, bias_y, bias_z, scale_x, scale_y, scale_z);

    // Show calibrated vs uncalibrated readings
    Serial.println("\n=== Calibrated Readings (10 samples) ===");
    for (int i = 0; i < 10; i++) {
        float mx, my, mz;
        if (imu.ReadMagnetometer(mx, my, mz) == IMU::Result::OK) {
            printf_("  Mag: X=%.2f, Y=%.2f, Z=%.2f uT\n", mx, my, mz);
        }
        delay(100);
    }

    Serial.println("\nMagnetometer calibration complete ✓");
    Serial.println("*STOP*");
    while (1);
}

void loop() {
    // Not used - calibration runs once in setup()
}
