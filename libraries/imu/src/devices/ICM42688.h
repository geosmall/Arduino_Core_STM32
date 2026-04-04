/*
 * This file is part of Arduino_Core_STM32 IMU library.
 *
 * This software is derived from Betaflight and is subject to the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * See ICM42688.cpp for full license text and attribution.
 */

#pragma once

#include "../bus/DeviceBus.h"
#include "DeviceBase.h"  // ImuPreset enum defined here

/**
 * @brief ICM42688/ICM42605/IIM42653 IMU driver
 *
 * Factory-pattern C++ class wrapper over Betaflight ICM426xx driver.
 * Constructor performs full initialization - no separate begin() needed.
 *
 * Usage (with presets):
 *   DeviceBusSPI bus(&SPI, CS_PIN);
 *   ICM42688* imu = ICM42688::detect(&bus);
 *   if (imu) {
 *       imu->applyPreset(ImuPreset::FILTER_BALANCED);  // Configure for 2kHz PID
 *       int16_t data[6];  // ax,ay,az,gx,gy,gz
 *       imu->read(data);
 *   }
 */
class ICM42688 : public DeviceBase {
protected:
    /**
     * @brief Protected constructor - use detect() factory method
     * @param bus Pointer to DeviceBus (SPI or I2C)
     * @param whoAmI WHO_AM_I register value (chip identification)
     */
    ICM42688(DeviceBus* bus, uint8_t whoAmI);

    /**
     * @brief Select register bank (ICM426xx has 5 banks: 0-4)
     * @param bank Bank number (0-4)
     */
    void setUserBank(uint8_t bank);

    DeviceBus* bus_;  ///< Bus interface pointer

    /**
     * @brief Current register bank (0xFF = unknown, forces first write)
     *
     * ISR Safety: Configuration functions (applyPreset, setGyroFSR, etc.)
     * must NOT be called from interrupt context. Only read() is ISR-safe
     * due to its defensive setUserBank(0) call.
     */
    volatile uint8_t currentBank_ = 0xFF;

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

    // ========================================================================
    // DeviceBase Tier 2/3 Extended API Overrides
    // ========================================================================

    /**
     * @brief Set gyroscope FSR (DeviceBase interface)
     */
    bool setGyroFSR(GyroFSR fsr) override;

    /**
     * @brief Set accelerometer FSR (DeviceBase interface)
     */
    bool setAccelFSR(AccelFSR fsr) override;

    /**
     * @brief Read register directly
     */
    uint8_t readReg(uint8_t reg) override;

    /**
     * @brief Write register directly
     */
    bool writeReg(uint8_t reg, uint8_t value) override;

    /**
     * @brief Write register and verify
     */
    bool writeRegVerify(uint8_t reg, uint8_t value) override;

    // ========================================================================
    // ICM42688-specific Low-level Methods (internal use)
    // ========================================================================

    void setAccelFSR_internal(uint16_t fsr_g);
    void setGyroFSR_internal(uint16_t fsr_dps);
    void setAccelODR(uint16_t odr_hz);
    void setGyroODR(uint16_t odr_hz);
    void setGyroAAF(const AAFConfig& config);
    void setAccelAAF(const AAFConfig& config);
    void setUIFilters(uint8_t gyro_bw, uint8_t accel_bw, uint8_t gyro_order, uint8_t accel_order);
    void disableAFSR();

    /**
     * @brief Factory method to detect and initialize IMU
     * @param bus Pointer to DeviceBus instance
     * @return Pointer to ICM42688 instance, or nullptr if not detected
     *
     * Attempts detection up to 20 times with delays.
     * On success, creates instance and performs full initialization.
     */
    static ICM42688* detect(DeviceBus* bus);

    /**
     * @brief Apply intent-based preset configuration
     * @param preset Preset configuration (SAFE, SMOOTH, BALANCED, ACRO)
     * @return true if configuration verified successfully, false if verification failed
     *
     * Configures ODR, FSR, AAF filters, and UI filters per imu_hal.md specification.
     * All presets use ±2000dps/±16g FSR.
     *
     * After writing registers, reads back critical values to verify:
     * - GYRO_CONFIG0/ACCEL_CONFIG0 (FSR + ODR)
     * - INTF_CONFIG1 (AFSR disable)
     * - AAF enable bits (Bank 1 and Bank 2)
     */
    bool applyPreset(ImuPreset preset) override;

    /**
     * @brief Verify current register configuration matches expected preset
     * @param preset Expected preset configuration
     * @return true if all critical registers match expected values
     *
     * Reads back and verifies:
     * - FSR and ODR in GYRO_CONFIG0/ACCEL_CONFIG0
     * - AFSR disable in INTF_CONFIG1
     * - AAF enable bits in Bank 1 (0x0B) and Bank 2 (0x03)
     */
    bool verifyConfiguration(ImuPreset preset) const;

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

    /**
     * @brief Enable data ready interrupt on INT pin
     *
     * Configures INT1 as push-pull, active-high, pulsed output.
     * Interrupt fires when new gyro/accel data is available.
     */
    void enableDataReadyInt() override;

    /**
     * @brief Disable data ready interrupt on INT pin
     */
    void disableDataReadyInt() override;

    // Note: whoAmI_, accScale_, gyrScale_, samplingRateHz_ inherited from DeviceBase
};
