/*
 * This file is part of Arduino_Core_STM32 IMU library.
 *
 * This software is derived from Betaflight and is subject to the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Original Betaflight source:
 * https://github.com/betaflight/betaflight/blob/master/src/main/drivers/accgyro/accgyro_spi_icm20689.c
 * Copyright: Betaflight contributors
 * License: GPLv3
 *
 * Modifications for Arduino (2024):
 * - C++ class with factory pattern
 * - DeviceBus abstraction for portability
 * - Shared register map (MPU_Common.h)
 * - Support for ICM-20601, ICM-20602, ICM-20689
 * 3. Factory pattern: detect() with retry logic
 * 4. Constructor-based initialization (no separate init function)
 * 5. Uses MPU_Common.h for shared register definitions
 */

#include "ICM206xx.h"
#include "MPU_Common.h"
#include <Arduino.h>

// Max SPI frequency: 8 MHz (Betaflight: ICM20689_MAX_SPI_CLK_HZ)
#define ICM206XX_MAX_SPI_HZ     8000000

// Register-specific constants (not in MPU_Common.h)
#define ICM20689_INT_ANYRD_2CLEAR   0x10  // INT_PIN_CFG: Clear on any read
#define ICM20689_ACCEL_RST          0x02  // SIGNAL_PATH_RESET: Accel reset
#define ICM20689_TEMP_RST           0x01  // SIGNAL_PATH_RESET: Temp reset
#define ICM20689_I2C_IF_DIS         0x10  // USER_CTRL: Disable I2C interface

// Timing constants (from ICM-20690 datasheet and MPU-6000 register map)
#define ICM20689_CLKSEL_SETTLE_US   120   // Clock selection settle time (doubled for margin)
#define ICM20689_RESET_DELAY_MS     100   // Delay after device reset
#define ICM20689_PATH_RESET_DELAY_MS 100  // Delay after signal path reset

// Configuration constants
#define INV_CLK_PLL                 0x01  // Use PLL clock source
#define INV_FSR_2000DPS             0x03  // ±2000 dps full scale range
#define INV_FSR_16G                 0x03  // ±16g full scale range
#define GYRO_DLPF_188HZ             0x01  // Gyro DLPF 188 Hz (conservative)
#define SMPLRT_DIV_1KHZ             0x00  // Sample rate divider: 8kHz/(0+1) = 8kHz, but limited by DLPF

// Scale factors (±2000 dps, ±16g)
#define GYRO_SCALE_2000DPS          (1.0f / 16.4f)    // dps/LSB
#define ACCEL_SCALE_16G             (1.0f / 2048.0f)  // G/LSB

// Protected constructor - performs full initialization
ICM206xx::ICM206xx(DeviceBus* bus, uint8_t whoAmI)
    : bus_(bus)
{
    whoAmI_ = whoAmI;  // Set base class member
    // Set max SPI frequency
    bus_->setFreq(ICM206XX_MAX_SPI_HZ);

    // Configuration (Betaflight: icm20689GyroInit)
    // Device was already reset during detection, proceed with config

    // 1. Select PLL clock source
    bus_->writeReg(MPU_RA_PWR_MGMT_1, INV_CLK_PLL);
    delayMicroseconds(ICM20689_CLKSEL_SETTLE_US);

    // 2. Configure gyro and accel full-scale ranges
    bus_->writeReg(MPU_RA_GYRO_CONFIG, INV_FSR_2000DPS << 3);
    bus_->writeReg(MPU_RA_ACCEL_CONFIG, INV_FSR_16G << 3);

    // 3. Configure DLPF (188 Hz - conservative setting)
    bus_->writeReg(MPU_RA_CONFIG, GYRO_DLPF_188HZ);

    // 4. Set sample rate divider (1 kHz with DLPF enabled)
    bus_->writeReg(MPU_RA_SMPLRT_DIV, SMPLRT_DIV_1KHZ);

    // 5. Data ready interrupt configuration
    bus_->writeReg(MPU_RA_INT_PIN_CFG, ICM20689_INT_ANYRD_2CLEAR);
    bus_->writeReg(MPU_RA_INT_ENABLE, MPU_RF_DATA_RDY_EN);

    // Set scale factors
    gyrScale_ = GYRO_SCALE_2000DPS;
    accScale_ = ACCEL_SCALE_16G;
    samplingRateHz_ = 1000;  // 1 kHz with DLPF enabled
}

// Factory pattern detect() - returns nullptr if not detected
ICM206xx* ICM206xx::detect(DeviceBus* bus)
{
    // Reset the device (Betaflight: icm20689SpiDetect)
    bus->writeReg(MPU_RA_PWR_MGMT_1, MPU9250_BIT_RESET);
    delay(ICM20689_RESET_DELAY_MS);

    // Attempt detection with retries
    uint8_t attemptsRemaining = 20;
    do {
        const uint8_t whoAmI = bus->readReg(MPU_RA_WHO_AM_I);

        // Check for recognized WHO_AM_I values
        switch (whoAmI) {
            case ICM20601_WHO_AM_I_CONST:  // 0xAC
            case ICM20602_WHO_AM_I_CONST:  // 0x12
            case ICM20689_WHO_AM_I_CONST:  // 0x98
                // Device recognized
                // Disable I2C interface (SPI only)
                bus->writeReg(MPU_RA_USER_CTRL, ICM20689_I2C_IF_DIS);

                // Reset signal paths (accel + temp)
                bus->writeReg(MPU_RA_SIGNAL_PATH_RESET, ICM20689_ACCEL_RST | ICM20689_TEMP_RST);
                delay(ICM20689_PATH_RESET_DELAY_MS);

                // Create and return device instance
                return new ICM206xx(bus, whoAmI);

            default:
                // Not recognized, try again
                break;
        }

        delay(150);  // Delay between retries
    } while (attemptsRemaining--);

    return nullptr;  // Detection failed
}

// Read gyro and accel data (DeviceBase interface)
void ICM206xx::read(int16_t* accgyr)
{
    // Read 14 bytes: ax, ay, az, temp, gx, gy, gz (big-endian)
    // Same format as MPU6000/MPU9250
    uint8_t buf[14];
    bus_->readRegs(MPU_RA_ACCEL_XOUT_H, buf, 14);

    // Parse big-endian sensor data
    accgyr[0] = (int16_t)((buf[0] << 8) | buf[1]);   // Accel X
    accgyr[1] = (int16_t)((buf[2] << 8) | buf[3]);   // Accel Y
    accgyr[2] = (int16_t)((buf[4] << 8) | buf[5]);   // Accel Z
    // Skip temperature (buf[6], buf[7])
    accgyr[3] = (int16_t)((buf[8] << 8) | buf[9]);   // Gyro X
    accgyr[4] = (int16_t)((buf[10] << 8) | buf[11]); // Gyro Y
    accgyr[5] = (int16_t)((buf[12] << 8) | buf[13]); // Gyro Z
}

// Type name (DeviceBase interface)
const char* ICM206xx::typeName() const
{
    switch (whoAmI_) {
        case ICM20601_WHO_AM_I_CONST:
            return "ICM20601";
        case ICM20602_WHO_AM_I_CONST:
            return "ICM20602";
        case ICM20689_WHO_AM_I_CONST:
            return "ICM20689";
        default:
            return "ICM206xx";
    }
}

// ============================================================================
// Preset Configuration (imu_hal.md specification)
// ============================================================================

// ICM206xx preset configuration structure (MPU6500-class behavior)
struct ICM206xxPresetConfig {
    uint8_t  dlpf_cfg_gyro;   // CONFIG.DLPF_CFG (0-7)
    uint8_t  dlpf_cfg_accel;  // ACCEL_CONFIG2.A_DLPF_CFG (0-7)
    uint8_t  smplrt_div;      // SMPLRT_DIV (only for DLPF_CFG=1-6)
    uint16_t gyro_odr_hz;     // Effective rate
};

// Preset LUT from imu_hal.md
// ICM206xx (MPU6500-class): DLPF_CFG=0 bypasses divider → 8kHz, DLPF_CFG=1-6 uses divider
static const ICM206xxPresetConfig ICM206xx_PRESETS[] = {
    {2, 2, 0, 1000},  // SAFE: DLPF=2 (~92Hz), DIV=0 → 1kHz
    {1, 1, 0, 1000},  // SMOOTH: DLPF=1 (~176Hz), DIV=0 → 1kHz
    {0, 0, 0, 8000},  // BALANCED: DLPF=0 (bypass), 8kHz (SW decimate if needed)
    {0, 0, 0, 8000}   // ACRO: DLPF=0 (bypass), 8kHz
};

// ACCEL_CONFIG2 register address
#define ICM206XX_RA_ACCEL_CONFIG2    0x1D

bool ICM206xx::applyPreset(ImuPreset preset)
{
    const ICM206xxPresetConfig& cfg = ICM206xx_PRESETS[static_cast<uint8_t>(preset)];

    // Set gyro DLPF configuration
    bus_->writeReg(MPU_RA_CONFIG, cfg.dlpf_cfg_gyro);
    delayMicroseconds(15);

    // Set accel DLPF configuration
    bus_->writeReg(ICM206XX_RA_ACCEL_CONFIG2, cfg.dlpf_cfg_accel);
    delayMicroseconds(15);

    // Set sample rate divider (only effective when DLPF_CFG=1-6)
    bus_->writeReg(MPU_RA_SMPLRT_DIV, cfg.smplrt_div);
    delayMicroseconds(15);

    // Update sampling rate for user reference
    samplingRateHz_ = cfg.gyro_odr_hz;

    return true;
}

void ICM206xx::enableDataReadyInt1()
{
    bus_->writeReg(MPU_RA_INT_ENABLE, MPU_RF_DATA_RDY_EN);
    delayMicroseconds(15);
}

void ICM206xx::disableDataReadyInt1()
{
    bus_->writeReg(MPU_RA_INT_ENABLE, 0x00);
    delayMicroseconds(15);
}
