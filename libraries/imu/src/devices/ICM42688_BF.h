/*
 * This file is part of Arduino_Core_STM32 IMU library.
 *
 * This software is derived from Betaflight and is subject to the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * See ICM42688_BF.cpp for full license text and attribution.
 */

#pragma once

#include "../bus/DeviceBus.h"
#include "DeviceBase.h"

/**
 * @brief Intent-based preset configurations (imu_hal.md philosophy)
 *
 * Four presets abstract hardware differences and provide validated
 * filter/ODR combinations for common flight controller use cases.
 */
enum class ImuPreset : uint8_t {
    FILTER_SAFE,      ///< Bring-up, very noisy frames (1kHz, tight filtering)
    FILTER_SMOOTH,    ///< Extra on-chip smoothing (4kHz, moderate filtering)
    FILTER_BALANCED,  ///< Default for 2kHz PID (4kHz, balanced filtering)
    FILTER_ACRO       ///< Minimum phase lag (8kHz, wide filtering)
};

/**
 * @brief ICM42688/ICM42605/IIM42653 IMU driver
 *
 * Factory-pattern C++ class wrapper over Betaflight ICM426xx driver.
 * Constructor performs full initialization - no separate begin() needed.
 *
 * Usage (with presets):
 *   DeviceBusSPI bus(&SPI, CS_PIN);
 *   ICM42688_BF* imu = ICM42688_BF::detect(&bus);
 *   if (imu) {
 *       imu->applyPreset(ImuPreset::FILTER_BALANCED);  // Configure for 2kHz PID
 *       int16_t data[6];  // ax,ay,az,gx,gy,gz
 *       imu->read(data);
 *   }
 */
class ICM42688_BF : public DeviceBase {
protected:
    /**
     * @brief Protected constructor - use detect() factory method
     * @param bus Pointer to DeviceBus (SPI or I2C)
     * @param whoAmI WHO_AM_I register value (chip identification)
     */
    ICM42688_BF(DeviceBus* bus, uint8_t whoAmI);

    /**
     * @brief Select register bank (ICM426xx has 5 banks: 0-4)
     * @param bank Bank number (0-4)
     */
    void setUserBank(uint8_t bank);

    DeviceBus* bus_;  ///< Bus interface pointer

public:
    /**
     * @brief AAF (Anti-Alias Filter) configuration structure
     * Public for advanced users who need fine-grained filter control.
     */
    struct AAFConfig {
        uint8_t  delt;
        uint16_t deltsqr;
        uint8_t  bitshift;
    };

    /**
     * @brief Low-level configuration methods
     * Primarily used internally by applyPreset(), but available for advanced users.
     * For most use cases, prefer applyPreset() for validated configurations.
     */
    void setAccelFSR(uint16_t fsr_g);
    void setGyroFSR(uint16_t fsr_dps);
    void setAccelODR(uint16_t odr_hz);
    void setGyroODR(uint16_t odr_hz);
    void setGyroAAF(const AAFConfig& config);
    void setAccelAAF(const AAFConfig& config);
    void setUIFilters(uint8_t gyro_bw, uint8_t accel_bw, uint8_t gyro_order, uint8_t accel_order);
    void disableAFSR();

    /**
     * @brief Factory method to detect and initialize IMU
     * @param bus Pointer to DeviceBus instance
     * @return Pointer to ICM42688_BF instance, or nullptr if not detected
     *
     * Attempts detection up to 20 times with delays.
     * On success, creates instance and performs full initialization.
     */
    static ICM42688_BF* detect(DeviceBus* bus);

    /**
     * @brief Apply intent-based preset configuration
     * @param preset Preset configuration (SAFE, SMOOTH, BALANCED, ACRO)
     *
     * Configures ODR, FSR, AAF filters, and UI filters per imu_hal.md specification.
     * All presets use ±2000dps/±16g FSR.
     */
    void applyPreset(ImuPreset preset);

    /**
     * @brief Read 6-axis gyro/accel data
     * @param accgyr Pointer to 6-element int16_t array [ax,ay,az,gx,gy,gz]
     *
     * Performs single 12-byte burst read from data registers.
     * Data is in sensor frame, little-endian format.
     */
    void read(int16_t* accgyr) override;

    /**
     * @brief Get human-readable chip name
     * @return Chip name string ("ICM42688P", "ICM42605", etc.)
     */
    const char* typeName() const override;

    // Note: whoAmI_, accScale_, gyrScale_, samplingRateHz_ inherited from DeviceBase
};
