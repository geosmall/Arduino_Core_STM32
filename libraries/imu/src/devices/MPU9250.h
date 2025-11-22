// Arduino_Core_STM32 IMU Library - MPU9250 Betaflight Driver
// Modified from Betaflight for Arduino integration
// Original: https://github.com/betaflight/betaflight/.../accgyro_spi_mpu9250.c
// Pattern: Madflight device wrapper
//
// Modifications applied:
// 1. "spiWriteReg(dev, " → "bus_->writeReg("
// 2. "spiReadRegMsk(dev, " → "bus_->readReg("
// 3. "spiReadRegBuf(dev, " → "bus_->readRegs("
// 4. "extDevice_t" → "DeviceBus"
// 5. C++ class with factory pattern
// 6. Constructor-based initialization
// 7. Removed: Betaflight framework dependencies

#pragma once

#include "../bus/DeviceBus.h"
#include "DeviceBase.h"
#include <Arduino.h>

/**
 * @brief MPU9250 device driver (Betaflight-based)
 *
 * Supports MPU-9250 and MPU-9255 9-axis IMUs using Betaflight's proven driver.
 * Uses factory pattern for detection and constructor-based initialization.
 *
 * Note: This driver handles the 6-axis gyro/accel only. Magnetometer access
 * would require additional I2C auxiliary bus implementation.
 */
class MPU9250 : public DeviceBase {
protected:
    /**
     * @brief Protected constructor - use detect() factory method
     * @param bus Pointer to DeviceBus instance
     * @param whoAmI WHO_AM_I register value (0x71 or 0x73)
     */
    MPU9250(DeviceBus* bus, uint8_t whoAmI);

public:
    /**
     * @brief Factory method - detect and initialize MPU9250
     * @param bus Pointer to DeviceBus instance
     * @return Pointer to MPU9250 instance or nullptr if not detected
     */
    static MPU9250* detect(DeviceBus* bus);

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
     * @return true (always succeeds for MPU9250)
     *
     * Configures DLPF and sample rate divider per imu_hal.md specification.
     * All presets use ±2000dps/±16g FSR.
     */
    bool applyPreset(ImuPreset preset) override;

    /**
     * @brief Enable data ready interrupt on INT pin
     */
    void enableDataReadyInt() override;

    /**
     * @brief Disable data ready interrupt on INT pin
     */
    void disableDataReadyInt() override;

    // ========================================================================
    // DeviceBase Tier 2/3 Extended API Overrides
    // ========================================================================

    bool setGyroFSR(GyroFSR fsr) override;
    bool setAccelFSR(AccelFSR fsr) override;
    uint8_t readReg(uint8_t reg) override;
    bool writeReg(uint8_t reg, uint8_t value) override;
    bool writeRegVerify(uint8_t reg, uint8_t value) override;

    // Note: whoAmI_, accScale_, gyrScale_, samplingRateHz_ inherited from DeviceBase

private:
    DeviceBus* bus_;         // Bus abstraction (owned by caller)
};
