/*
 * IMU Library - 9-DOF Basic Example
 *
 * Demonstrates 9-axis IMU data reading with automatic chip detection:
 * - MPU-9250/9255: Read gyro + accel + magnetometer (9-DOF)
 * - Other chips: Read gyro + accel only (6-DOF)
 *
 * MAGNETOMETER SUPPORT:
 * - Only MPU-9250 (0x71) and MPU-9255 (0x73) have magnetometer
 * - Other chips (ICM-42688-P, MPU-6000, ICM-206xx) are 6-DOF only
 * - Code automatically detects capability and adapts
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection (NUCLEO_F411RE / BLACKPILL_F411CE)
 * - Pin assignments and SPI frequency from board configuration
 * - No interrupt pin required (polling mode)
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

// Sample counter
uint32_t sample_count = 0;
bool has_magnetometer = false;

void setup() {
    // Initialize communication (Serial or RTT)
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial) delay(10);
#endif

    CI_LOG("\n=== IMU Library - 9-DOF Basic Example ===\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();

    // Display pin configuration
    CI_LOG("Pin Configuration (BoardConfig):\n");
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
        CI_LOG("Check connections and power.\n");
        CI_LOG("*STOP*\n");
        while (1);
    }

    // Detect chip type
    IMU::ChipType chip = imu.GetChipType();
    CI_LOG("IMU Chip Detected: ");
    switch (chip) {
        case IMU::ChipType::ICM42688_P:
            CI_LOG("ICM-42688-P (0x47) - 6-DOF\n");
            break;
        case IMU::ChipType::MPU_6000:
            CI_LOG("MPU-6000 (0x68) - 6-DOF\n");
            break;
        case IMU::ChipType::MPU_9250:
            CI_LOG("MPU-9250 (0x71) - 9-DOF\n");
            break;
        case IMU::ChipType::ICM20602:
            CI_LOG("ICM-20602 (0x12) - 6-DOF\n");
            break;
        case IMU::ChipType::ICM20689:
            CI_LOG("ICM-20689 (0x98) - 6-DOF\n");
            break;
        case IMU::ChipType::ICM20601:
            CI_LOG("ICM-20601 (0xAC) - 6-DOF\n");
            break;
        default:
            CI_LOG("Unknown\n");
            break;
    }

    // Check magnetometer capability
    has_magnetometer = imu.HasMagnetometer();
    if (has_magnetometer) {
        CI_LOG("\n9-DOF Mode: Magnetometer available\n");
        CI_LOG("Initializing magnetometer (AK8963)...\n");

        if (imu.InitMagnetometer() == IMU::Result::OK) {
            CI_LOG("✓ Magnetometer initialized successfully\n");
            CI_LOG("  Reading gyro + accel + magnetometer\n");
        } else {
            CI_LOG("✗ Magnetometer initialization failed\n");
            CI_LOG("  Falling back to 6-DOF mode\n");
            has_magnetometer = false;
        }
    } else {
        CI_LOG("\n6-DOF Mode: No magnetometer support\n");
        CI_LOG("  Reading gyro + accel only\n");
    }

    // Apply preset configuration
    CI_LOG("\nApplying BALANCED preset...\n");
    if (imu.ApplyPreset(IMU::Preset::BALANCED) == IMU::Result::OK) {
        CI_LOG("✓ Preset applied\n");
    }

    // Display sensor configuration
    CI_PRINTF("\nSensor Configuration:\n");
    CI_PRINTF("  Gyro FSR: ±2000 dps (sensitivity: %.2f LSB/dps)\n",
              imu.GetGyroSensitivity());
    CI_PRINTF("  Accel FSR: ±16g (sensitivity: %.2f LSB/g)\n",
              imu.GetAccelSensitivity());

    CI_LOG("\nStarting data acquisition (50 samples at 10 Hz)...\n");
    CI_LOG("---\n");
    delay(100);
}

void loop() {
    // Read IMU data based on capability
    if (has_magnetometer) {
        // 9-DOF mode: Read gyro + accel + mag
        std::array<int16_t, 3> gyro_data;
        std::array<int16_t, 3> accel_data;
        std::array<float, 3> mag_data;

        if (imu.ReadIMU9(gyro_data, accel_data, mag_data) == IMU::Result::OK) {
            sample_count++;

            // Convert raw values to physical units
            float gx = (float)gyro_data[0] / imu.GetGyroSensitivity();
            float gy = (float)gyro_data[1] / imu.GetGyroSensitivity();
            float gz = (float)gyro_data[2] / imu.GetGyroSensitivity();
            float ax = (float)accel_data[0] / imu.GetAccelSensitivity();
            float ay = (float)accel_data[1] / imu.GetAccelSensitivity();
            float az = (float)accel_data[2] / imu.GetAccelSensitivity();

            CI_PRINTF("Sample %lu:\n", sample_count);
            CI_PRINTF("  Gyro (dps):  X=%.2f, Y=%.2f, Z=%.2f\n", gx, gy, gz);
            CI_PRINTF("  Accel (g):   X=%.3f, Y=%.3f, Z=%.3f\n", ax, ay, az);
            CI_PRINTF("  Mag (uT):    X=%.2f, Y=%.2f, Z=%.2f\n",
                      mag_data[0], mag_data[1], mag_data[2]);
            CI_LOG("---\n");
        } else {
            CI_LOG("ERROR: Failed to read 9-DOF data\n");
        }
    } else {
        // 6-DOF mode: Read gyro + accel only
        std::array<int16_t, 6> imu_data;

        if (imu.ReadIMU6(imu_data) == 0) {
            sample_count++;

            // Convert raw values to physical units
            float ax = (float)imu_data[0] / imu.GetAccelSensitivity();
            float ay = (float)imu_data[1] / imu.GetAccelSensitivity();
            float az = (float)imu_data[2] / imu.GetAccelSensitivity();
            float gx = (float)imu_data[3] / imu.GetGyroSensitivity();
            float gy = (float)imu_data[4] / imu.GetGyroSensitivity();
            float gz = (float)imu_data[5] / imu.GetGyroSensitivity();

            CI_PRINTF("Sample %lu:\n", sample_count);
            CI_PRINTF("  Gyro (dps):  X=%.2f, Y=%.2f, Z=%.2f\n", gx, gy, gz);
            CI_PRINTF("  Accel (g):   X=%.3f, Y=%.3f, Z=%.3f\n", ax, ay, az);
            CI_LOG("---\n");
        } else {
            CI_LOG("ERROR: Failed to read 6-DOF data\n");
        }
    }

    // Stop after 50 samples (5 seconds at 10 Hz)
    if (sample_count >= 50) {
        CI_PRINTF("\nTest complete: %lu samples collected\n", sample_count);
        if (has_magnetometer) {
            CI_LOG("9-DOF test PASSED ✓\n");
        } else {
            CI_LOG("6-DOF test PASSED ✓\n");
        }
        CI_LOG("*STOP*\n");
        while (1);
    }

    delay(100);  // 10 Hz sampling rate
}
