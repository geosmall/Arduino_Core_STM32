// Arduino_Core_STM32 IMU Library - MPU6000 Betaflight Driver
// Modified from Betaflight for Arduino integration
// Original: https://github.com/betaflight/betaflight/.../accgyro_spi_mpu6000.c
// Pattern: Madflight device wrapper
//
// Modifications applied:
// 1. "spiWriteReg(dev, " → "bus_->writeReg("
// 2. "spiReadRegMsk(dev, " → "bus_->readReg("
// 3. "extDevice_t" → "DeviceBus"
// 4. C++ class with factory pattern
// 5. Constructor-based initialization
// 6. Removed: Betaflight framework dependencies

#pragma once

#include "../bus/DeviceBus.h"
#include "DeviceBase.h"
#include <Arduino.h>

/**
 * @brief MPU6000 device driver (Betaflight-based)
 *
 * Supports MPU-6000 and MPU-6050 IMUs using Betaflight's proven driver.
 * Uses factory pattern for detection and constructor-based initialization.
 */
class MPU6000 : public DeviceBase {
protected:
    /**
     * @brief Protected constructor - use detect() factory method
     * @param bus Pointer to DeviceBus instance
     * @param whoAmI WHO_AM_I register value (0x68)
     */
    MPU6000(DeviceBus* bus, uint8_t whoAmI);

public:
    /**
     * @brief Factory method - detect and initialize MPU6000
     * @param bus Pointer to DeviceBus instance
     * @return Pointer to MPU6000 instance or nullptr if not detected
     */
    static MPU6000* detect(DeviceBus* bus);

    /**
     * @brief Read gyro and accel data (6 int16_t values)
     * @param accgyr Output array: ax, ay, az, gx, gy, gz (raw LSB)
     */
    void read(int16_t* accgyr) override;

    /**
     * @brief Get human-readable device name
     * @return C-string with device name
     */
    const char* typeName() const override;

    /**
     * @brief Apply intent-based preset configuration
     * @param preset Preset configuration (SAFE, SMOOTH, BALANCED, ACRO)
     * @return true (always succeeds for MPU6000)
     *
     * Configures DLPF and sample rate divider per imu_hal.md specification.
     * All presets use ±2000dps/±16g FSR.
     */
    bool applyPreset(ImuPreset preset) override;

    /**
     * @brief Enable data ready interrupt on INT pin
     */
    void enableDataReadyInt1() override;

    /**
     * @brief Disable data ready interrupt on INT pin
     */
    void disableDataReadyInt1() override;

    // Note: whoAmI_, accScale_, gyrScale_, samplingRateHz_ inherited from DeviceBase

private:
    DeviceBus* bus_;         // Bus abstraction (owned by caller)
};
