// Arduino_Core_STM32 IMU Library - Device Base Interface
// Abstract base class for all IMU device drivers

#pragma once

#include <stdint.h>

/**
 * @brief Abstract base class for IMU device drivers
 *
 * Provides common interface for all device types (ICM42688, MPU6000, etc.)
 * Enables polymorphic device handling in IMU_Driver facade.
 */
class DeviceBase
{
public:
    virtual ~DeviceBase() {}

    /**
     * @brief Read accelerometer and gyroscope data
     * @param accgyr Output array [ax, ay, az, gx, gy, gz] in raw LSB
     */
    virtual void read(int16_t* accgyr) = 0;

    /**
     * @brief Get human-readable device type name
     * @return C-string with device name (e.g., "ICM42688P", "MPU6000")
     */
    virtual const char* typeName() const = 0;

    // Public scale factors for data conversion
    uint8_t whoAmI_;         // WHO_AM_I register value
    float accScale_;         // Accelerometer scale [G/LSB]
    float gyrScale_;         // Gyroscope scale [dps/LSB]
    uint16_t samplingRateHz_; // Configured sampling rate [Hz]

protected:
    DeviceBase()
        : whoAmI_(0)
        , accScale_(0.0f)
        , gyrScale_(0.0f)
        , samplingRateHz_(0)
    {}
};
