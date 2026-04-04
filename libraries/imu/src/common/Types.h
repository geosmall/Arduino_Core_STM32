// Arduino_Core_STM32 IMU Library - Common Types
// Based on madflight pattern

#pragma once

#include <stdint.h>

/**
 * @brief IMU sample data structure
 *
 * Contains accelerometer and gyroscope readings in SI units
 */
struct ImuSample {
    float ax, ay, az;      // Accelerometer [m/s²]
    float gx, gy, gz;      // Gyroscope [rad/s]
    uint32_t timestamp_us; // Timestamp [microseconds]
};

/**
 * @brief IMU chip types supported
 */
enum class ImuType {
    Unknown,
    Auto,          // Auto-detection mode
    ICM42688P,     // ICM-42688-P (0x47)
    ICM42605,      // ICM-42605 (0x42)
    IIM42653,      // IIM-42653 (0x56)
    MPU6000,       // MPU-6000 (0x68)
    MPU9250,       // MPU-9250 (0x71)
    ICM20601,      // ICM-20601 (0xAC)
    ICM20602,      // ICM-20602 (0x12)
    ICM20689       // ICM-20689 (0x98)
};
