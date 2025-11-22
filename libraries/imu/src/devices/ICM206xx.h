/*
 * ICM206xx.h - ICM-206xx IMU Driver (Betaflight-based, Madflight pattern)
 *
 * Supports: ICM-20601, ICM-20602, ICM-20608G, ICM-20689
 *
 * Adapted from Betaflight: src/main/drivers/accgyro/accgyro_spi_icm20689.c
 * Original: https://github.com/betaflight/betaflight
 * License: GPLv3
 *
 * Pattern: Madflight device driver architecture
 * - Protected constructor (initialization in constructor)
 * - Static factory detect() method
 * - DeviceBase inheritance for polymorphic dispatch
 *
 * Hardware:
 * - Max SPI: 8 MHz
 * - Gyro range: ±2000 dps (16.4 LSB/dps)
 * - Accel range: ±16g (2048 LSB/g)
 * - Sampling: 1 kHz (8 kHz / (SMPLRT_DIV + 1))
 */

#pragma once

#include "../bus/DeviceBus.h"
#include "DeviceBase.h"
#include <stdint.h>

class ICM206xx : public DeviceBase {
protected:
    // Protected constructor - initialization happens here
    ICM206xx(DeviceBus* bus, uint8_t whoAmI);

public:
    // Factory pattern - returns nullptr if not detected
    static ICM206xx* detect(DeviceBus* bus);

    // DeviceBase interface implementation
    void read(int16_t* accgyr) override;
    const char* typeName() const override;

    /**
     * @brief Apply intent-based preset configuration
     * @param preset Preset configuration (SAFE, SMOOTH, BALANCED, ACRO)
     * @return true (always succeeds for ICM206xx)
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
    DeviceBus* bus_;
};
