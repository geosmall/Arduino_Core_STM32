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
 * @brief Gyroscope Full-Scale Range (common across all supported IMUs)
 *
 * All supported IMU chips (ICM-42688-P, MPU-6000, MPU-9250, ICM-206xx)
 * share the same gyroscope FSR options. Register encoding varies by chip.
 */
enum class GyroFSR : uint8_t {
    DPS_250  = 0,   ///< ±250 degrees/sec (high resolution, aerobatic)
    DPS_500  = 1,   ///< ±500 degrees/sec
    DPS_1000 = 2,   ///< ±1000 degrees/sec
    DPS_2000 = 3    ///< ±2000 degrees/sec (default, widest range)
};

/**
 * @brief Accelerometer Full-Scale Range (common across all supported IMUs)
 *
 * All supported IMU chips share the same accelerometer FSR options.
 * Register encoding varies by chip.
 */
enum class AccelFSR : uint8_t {
    G_2  = 0,   ///< ±2g (highest resolution)
    G_4  = 1,   ///< ±4g
    G_8  = 2,   ///< ±8g
    G_16 = 3    ///< ±16g (default, widest range)
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
     * @brief Enable data ready interrupt on INT pin
     * Default implementation is no-op (not all chips support interrupts).
     */
    virtual void enableDataReadyInt() {}

    /**
     * @brief Disable data ready interrupt on INT pin
     * Default implementation is no-op.
     */
    virtual void disableDataReadyInt() {}

    // ========================================================================
    // Tier 2 Extended API: FSR Configuration (works on all chips)
    // ========================================================================

    /**
     * @brief Set gyroscope full-scale range
     * @param fsr Full-scale range (DPS_250, DPS_500, DPS_1000, DPS_2000)
     * @return true on success, false on failure
     *
     * Updates both the hardware register and gyrScale_ tracking value.
     */
    virtual bool setGyroFSR(GyroFSR fsr) { (void)fsr; return false; }

    /**
     * @brief Set accelerometer full-scale range
     * @param fsr Full-scale range (G_2, G_4, G_8, G_16)
     * @return true on success, false on failure
     *
     * Updates both the hardware register and accScale_ tracking value.
     */
    virtual bool setAccelFSR(AccelFSR fsr) { (void)fsr; return false; }

    // ========================================================================
    // Tier 3 Extended API: Direct Register Access (power users)
    // ========================================================================

    /**
     * @brief Read register directly
     * @param reg Register address
     * @return Register value
     */
    virtual uint8_t readReg(uint8_t reg) { (void)reg; return 0; }

    /**
     * @brief Write register directly
     * @param reg Register address
     * @param value Value to write
     * @return true on success
     */
    virtual bool writeReg(uint8_t reg, uint8_t value) {
        (void)reg; (void)value; return false;
    }

    /**
     * @brief Write register and verify
     * @param reg Register address
     * @param value Value to write
     * @return true if write verified successfully
     */
    virtual bool writeRegVerify(uint8_t reg, uint8_t value) {
        (void)reg; (void)value; return false;
    }

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
