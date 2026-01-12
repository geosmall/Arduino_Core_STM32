/*
 * IMU Library - 9-DOF Basic Example
 *
 * Demonstrates 9-axis IMU data reading with automatic chip detection:
 * - MPU-9250/9255: Read gyro + accel + magnetometer (9-DOF)
 * - Other chips: Read gyro + accel only (6-DOF)
 *
 * Magnetometer Support:
 * - Only MPU-9250 (0x71) and MPU-9255 (0x73) have magnetometer
 * - Other chips (ICM-42688-P, MPU-6000, ICM-206xx) are 6-DOF only
 * - Code automatically detects capability and adapts
 *
 * Hardware Configuration:
 * - Uses BoardConfig for automatic board detection
 * - Pin assignments and SPI frequency from board configuration
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

// Sample counter
uint32_t sample_count = 0;
bool has_magnetometer = false;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println("\n=== IMU Library - 9-DOF Basic Example ===\n");

    // Display pin configuration
    Serial.println("Pin Configuration (BoardConfig):");
    Serial.printf("  CS: %d, MOSI: %d, MISO: %d, SCLK: %d\n",
           (int)BoardConfig::imu.spi.cs_pin,
           (int)BoardConfig::imu.spi.mosi_pin,
           (int)BoardConfig::imu.spi.miso_pin,
           (int)BoardConfig::imu.spi.sclk_pin);
    Serial.printf("  SPI Speed: %lu Hz\n\n", (unsigned long)BoardConfig::imu.spi.freq_hz);

    // Initialize IMU
    Serial.println("Initializing IMU...");
    if (imu.Init(spi_bus, BoardConfig::imu.spi.cs_pin, BoardConfig::imu.spi.freq_hz) != IMU::Result::OK) {
        Serial.println("ERROR: IMU initialization failed!");
        Serial.println("Check connections and power.");
        while (1);
    }

    // Detect chip type
    IMU::ChipType chip = imu.GetChipType();
    Serial.print("IMU Chip Detected: ");
    switch (chip) {
        case IMU::ChipType::ICM42688_P:
            Serial.println("ICM-42688-P (0x47) - 6-DOF");
            break;
        case IMU::ChipType::MPU_6000:
            Serial.println("MPU-6000 (0x68) - 6-DOF");
            break;
        case IMU::ChipType::MPU_9250:
            Serial.println("MPU-9250 (0x71) - 9-DOF");
            break;
        case IMU::ChipType::ICM20602:
            Serial.println("ICM-20602 (0x12) - 6-DOF");
            break;
        case IMU::ChipType::ICM20689:
            Serial.println("ICM-20689 (0x98) - 6-DOF");
            break;
        case IMU::ChipType::ICM20601:
            Serial.println("ICM-20601 (0xAC) - 6-DOF");
            break;
        default:
            Serial.println("Unknown");
            break;
    }

    // Check magnetometer capability
    has_magnetometer = imu.HasMagnetometer();
    if (has_magnetometer) {
        Serial.println("\n9-DOF Mode: Magnetometer available");
        Serial.println("Initializing magnetometer (AK8963)...");

        if (imu.InitMagnetometer() == IMU::Result::OK) {
            Serial.println("Magnetometer initialized successfully");
            Serial.println("  Reading gyro + accel + magnetometer");
        } else {
            Serial.println("Magnetometer initialization failed");
            Serial.println("  Falling back to 6-DOF mode");
            has_magnetometer = false;
        }
    } else {
        Serial.println("\n6-DOF Mode: No magnetometer support");
        Serial.println("  Reading gyro + accel only");
    }

    // Apply preset configuration
    Serial.println("\nApplying BALANCED preset...");
    if (imu.ApplyPreset(IMU::Preset::BALANCED) == IMU::Result::OK) {
        Serial.println("Preset applied");
    }

    // Display sensor configuration
    Serial.printf("\nSensor Configuration:\n");
    Serial.printf("  Gyro FSR: +/-2000 dps (sensitivity: %.2f LSB/dps)\n",
              imu.GetGyroSensitivity());
    Serial.printf("  Accel FSR: +/-16g (sensitivity: %.2f LSB/g)\n",
              imu.GetAccelSensitivity());

    Serial.println("\nStreaming IMU data at 10 Hz...\n");
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

            Serial.printf("Sample %lu: ", sample_count);
            Serial.printf("Gyro[%.1f,%.1f,%.1f] ", gx, gy, gz);
            Serial.printf("Accel[%.2f,%.2f,%.2f] ", ax, ay, az);
            Serial.printf("Mag[%.1f,%.1f,%.1f]\n", mag_data[0], mag_data[1], mag_data[2]);
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

            Serial.printf("Sample %lu: ", sample_count);
            Serial.printf("Gyro[%.1f,%.1f,%.1f] ", gx, gy, gz);
            Serial.printf("Accel[%.2f,%.2f,%.2f]\n", ax, ay, az);
        }
    }

    delay(100);  // 10 Hz sampling rate
}
