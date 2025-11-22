// Arduino_Core_STM32 IMU Library - Device Base Interface
// Abstract base class for all IMU device drivers

#pragma once

#include <stdint.h>

/**
 * @brief Intent-based preset configurations (imu_hal.md philosophy)
 *
 * Common preset enum shared across all IMU drivers.
 * Each driver implements chip-specific register settings.
 */
enum class ImuPreset : uint8_t {
    FILTER_SAFE,      ///< Bring-up, very noisy frames (1kHz, tight filtering)
    FILTER_SMOOTH,    ///< Extra on-chip smoothing (4kHz, moderate filtering)
    FILTER_BALANCED,  ///< Default for 2kHz PID (4kHz, balanced filtering)
    FILTER_ACRO       ///< Minimum phase lag (8kHz, wide filtering)
};

/**
 * @brief Abstract base class for IMU device drivers
 *
 * Provides common interface for all device types (ICM42688, MPU6000, etc.)
 * Enables polymorphic device handling in IMU wrapper class.
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

    /**
     * @brief Apply intent-based preset configuration
     * @param preset Preset configuration (SAFE, SMOOTH, BALANCED, ACRO)
     * @return true if configuration applied successfully
     *
     * Each driver implements chip-specific register settings.
     * Default implementation returns true (no-op for simple chips).
     */
    virtual bool applyPreset(ImuPreset preset) {
        (void)preset;
        return true;  // Default: no-op, success
    }

    /**
     * @brief Enable data ready interrupt on INT1 pin
     * Default implementation is no-op (not all chips support interrupts).
     */
    virtual void enableDataReadyInt1() {}

    /**
     * @brief Disable data ready interrupt on INT1 pin
     * Default implementation is no-op.
     */
    virtual void disableDataReadyInt1() {}

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
