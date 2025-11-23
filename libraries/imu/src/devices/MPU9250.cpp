/*
 * This file is part of Arduino_Core_STM32 IMU library.
 *
 * This software is derived from Betaflight and is subject to the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Original Betaflight source:
 * https://github.com/betaflight/betaflight/blob/master/src/main/drivers/accgyro/accgyro_spi_mpu9250.c
 * Copyright: Betaflight contributors
 * License: GPLv3
 *
 * Modifications for Arduino (2024):
 * - C++ class with factory pattern
 * - DeviceBus abstraction for portability
 * - Shared register map (MPU_Common.h)
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MPU9250.h"
#include "MPU_Common.h"

// ============================================================================
// MPU9250-Specific Definitions
// ============================================================================

// Max SPI frequency: 20 MHz (from Betaflight)
#define MPU9250_MAX_SPI_CLK_HZ 20000000

// DLPF (Digital Low Pass Filter) configuration register
#define MPU9250_CONFIG  0x1A

// ============================================================================
// MPU9250 Implementation
// ============================================================================

MPU9250::MPU9250(DeviceBus* bus, uint8_t whoAmI)
    : bus_(bus)
{
    whoAmI_ = whoAmI;

    // Power management - use PLL with Z-axis gyro reference
    bus_->writeReg(MPU_RA_PWR_MGMT_1, INV_CLK_PLL);
    delayMicroseconds(1);

    // Gyro full scale: ±2000 dps
    bus_->writeReg(MPU_RA_GYRO_CONFIG, INV_FSR_2000DPS << 3);
    delayMicroseconds(1);

    // DLPF configuration (from Betaflight default)
    // Note: This would normally come from gyroConfig()->gyro_hardware_lpf
    // Using a conservative 188Hz LPF (DLPF_CFG = 1)
    bus_->writeReg(MPU9250_CONFIG, 0x01);
    delayMicroseconds(1);

    // Sample rate divider: 0 = 1kHz (8kHz / (1 + 0))
    bus_->writeReg(MPU_RA_SMPLRT_DIV, 0);
    delayMicroseconds(1);

    // Accel full scale: ±16g
    bus_->writeReg(MPU_RA_ACCEL_CONFIG, INV_FSR_16G << 3);
    delayMicroseconds(1);

    // INT pin configuration
    // Bit 4: INT_ANYRD_2CLEAR = 1 (interrupt status cleared on any read)
    // Bit 1: BYPASS_EN = 1 (bypass mode for magnetometer access)
    bus_->writeReg(MPU_RA_INT_PIN_CFG, (1 << 4) | (1 << 1));
    delayMicroseconds(1);

    // Enable data ready interrupt
    bus_->writeReg(MPU_RA_INT_ENABLE, 0x01);
    delayMicroseconds(1);

    // Set scale factors
    // MPU9250: ±2000 dps = 16.4 LSB/(dps), ±16g = 2048 LSB/g
    gyrScale_ = 1.0f / 16.4f;     // [dps/LSB]
    accScale_ = 1.0f / 2048.0f;   // [G/LSB]
    samplingRateHz_ = 1000;       // 1 kHz
}

MPU9250* MPU9250::detect(DeviceBus* bus)
{
    if (!bus) {
        return nullptr;
    }

    // Reset the device
    bus->writeReg(MPU_RA_PWR_MGMT_1, MPU9250_BIT_RESET);

    // Wait for reset to complete with retry mechanism (from Betaflight)
    uint8_t attemptsRemaining = 20;
    do {
        delay(150);  // Delay between attempts
        const uint8_t whoAmI = bus->readReg(MPU_RA_WHO_AM_I);

        // Check for MPU9250 (0x71) or MPU9255 (0x73)
        if (whoAmI == MPU9250_WHO_AM_I_CONST || whoAmI == MPU9255_WHO_AM_I_CONST) {
            // Device detected - create and initialize instance
            return new MPU9250(bus, whoAmI);
        }

        if (!attemptsRemaining) {
            return nullptr;  // Detection failed
        }
    } while (attemptsRemaining--);

    return nullptr;
}

void MPU9250::read(int16_t* accgyr)
{
    // Read 14 bytes: AX_H, AX_L, AY_H, AY_L, AZ_H, AZ_L, TEMP_H, TEMP_L, GX_H, GX_L, GY_H, GY_L, GZ_H, GZ_L
    uint8_t buf[14];
    bus_->readRegs(MPU_RA_ACCEL_XOUT_H, buf, 14);

    // Parse big-endian data
    accgyr[0] = (int16_t)((buf[0] << 8) | buf[1]);   // Accel X
    accgyr[1] = (int16_t)((buf[2] << 8) | buf[3]);   // Accel Y
    accgyr[2] = (int16_t)((buf[4] << 8) | buf[5]);   // Accel Z
    // Skip temperature (buf[6-7])
    accgyr[3] = (int16_t)((buf[8] << 8) | buf[9]);   // Gyro X
    accgyr[4] = (int16_t)((buf[10] << 8) | buf[11]); // Gyro Y
    accgyr[5] = (int16_t)((buf[12] << 8) | buf[13]); // Gyro Z
}

const char* MPU9250::typeName() const
{
    // Distinguish between MPU9250 and MPU9255 based on WHO_AM_I
    if (whoAmI_ == MPU9255_WHO_AM_I_CONST) {
        return "MPU9255";
    }
    return "MPU9250";
}

// ============================================================================
// Preset Configuration (imu_hal.md specification)
// ============================================================================

// MPU9250 preset configuration structure
struct MPU9250PresetConfig {
    uint8_t  dlpf_cfg_gyro;   // CONFIG.DLPF_CFG (0-7)
    uint8_t  dlpf_cfg_accel;  // ACCEL_CONFIG2.A_DLPF_CFG (0-7)
    uint8_t  smplrt_div;      // SMPLRT_DIV (only for DLPF_CFG=1-6)
    uint16_t gyro_odr_hz;     // Effective rate
};

// Preset LUT from imu_hal.md
// MPU9250 (MPU6500-class): DLPF_CFG=0 bypasses divider → 8kHz, DLPF_CFG=1-6 uses divider
static const MPU9250PresetConfig MPU9250_PRESETS[] = {
    {2, 2, 0, 1000},  // SAFE: DLPF=2 (92Hz gyro, 94Hz accel), DIV=0 → 1kHz
    {1, 1, 0, 1000},  // SMOOTH: DLPF=1 (184Hz gyro, 184Hz accel), DIV=0 → 1kHz
    {0, 0, 0, 8000},  // BALANCED: DLPF=0 (bypass), 8kHz (SW decimate to 4kHz if needed)
    {0, 0, 0, 8000}   // ACRO: DLPF=0 (bypass), 8kHz
};

// ACCEL_CONFIG2 register address (MPU9250-specific)
#define MPU_RA_ACCEL_CONFIG2    0x1D

bool MPU9250::applyPreset(ImuPreset preset)
{
    const MPU9250PresetConfig& cfg = MPU9250_PRESETS[static_cast<uint8_t>(preset)];

    // Set gyro DLPF configuration
    bus_->writeReg(MPU_RA_CONFIG, cfg.dlpf_cfg_gyro);
    delayMicroseconds(15);

    // Set accel DLPF configuration (MPU9250 has separate accel filter)
    bus_->writeReg(MPU_RA_ACCEL_CONFIG2, cfg.dlpf_cfg_accel);
    delayMicroseconds(15);

    // Set sample rate divider (only effective when DLPF_CFG=1-6)
    bus_->writeReg(MPU_RA_SMPLRT_DIV, cfg.smplrt_div);
    delayMicroseconds(15);

    // Update sampling rate for user reference
    samplingRateHz_ = cfg.gyro_odr_hz;

    // Wait for gyro digital filter to stabilize after DLPF mode change
    // MPU-9250 datasheet: gyro startup time 35ms typical
    delay(50);

    return true;
}

void MPU9250::enableDataReadyInt()
{
    bus_->writeReg(MPU_RA_INT_ENABLE, MPU_RF_DATA_RDY_EN);
    delayMicroseconds(15);
}

void MPU9250::disableDataReadyInt()
{
    bus_->writeReg(MPU_RA_INT_ENABLE, 0x00);
    delayMicroseconds(15);
}

// ============================================================================
// DeviceBase Tier 2/3 Extended API Implementation
// ============================================================================

bool MPU9250::setGyroFSR(GyroFSR fsr)
{
    uint8_t fsr_code;
    float scale;
    switch(fsr) {
        case GyroFSR::DPS_250:  fsr_code = INV_FSR_250DPS;  scale = 1.0f / 131.0f; break;
        case GyroFSR::DPS_500:  fsr_code = INV_FSR_500DPS;  scale = 1.0f / 65.5f;  break;
        case GyroFSR::DPS_1000: fsr_code = INV_FSR_1000DPS; scale = 1.0f / 32.8f;  break;
        case GyroFSR::DPS_2000: fsr_code = INV_FSR_2000DPS; scale = 1.0f / 16.4f;  break;
        default: return false;
    }

    bus_->writeReg(MPU_RA_GYRO_CONFIG, fsr_code << 3);
    delayMicroseconds(15);
    gyrScale_ = scale;
    return true;
}

bool MPU9250::setAccelFSR(AccelFSR fsr)
{
    uint8_t fsr_code;
    float scale;
    switch(fsr) {
        case AccelFSR::G_2:  fsr_code = INV_FSR_2G;  scale = 1.0f / 16384.0f; break;
        case AccelFSR::G_4:  fsr_code = INV_FSR_4G;  scale = 1.0f / 8192.0f;  break;
        case AccelFSR::G_8:  fsr_code = INV_FSR_8G;  scale = 1.0f / 4096.0f;  break;
        case AccelFSR::G_16: fsr_code = INV_FSR_16G; scale = 1.0f / 2048.0f;  break;
        default: return false;
    }

    bus_->writeReg(MPU_RA_ACCEL_CONFIG, fsr_code << 3);
    delayMicroseconds(15);
    accScale_ = scale;
    return true;
}

uint8_t MPU9250::readReg(uint8_t reg)
{
    return bus_->readReg(reg);
}

bool MPU9250::writeReg(uint8_t reg, uint8_t value)
{
    bus_->writeReg(reg, value);
    return true;
}

bool MPU9250::writeRegVerify(uint8_t reg, uint8_t value)
{
    bus_->writeReg(reg, value);
    delayMicroseconds(15);
    uint8_t readback = bus_->readReg(reg);
    return (readback == value);
}
