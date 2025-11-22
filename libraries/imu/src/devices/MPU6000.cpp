/*
 * This file is part of Arduino_Core_STM32 IMU library.
 *
 * This software is derived from Betaflight and is subject to the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Original Betaflight source:
 * https://github.com/betaflight/betaflight/blob/master/src/main/drivers/accgyro/accgyro_spi_mpu6000.c
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

#include "MPU6000.h"
#include "MPU_Common.h"

// ============================================================================
// MPU6000-Specific Product ID Revisions
// ============================================================================
#define MPU6000ES_REV_C4        0x14
#define MPU6000ES_REV_C5        0x15
#define MPU6000ES_REV_D6        0x16
#define MPU6000ES_REV_D7        0x17
#define MPU6000ES_REV_D8        0x18
#define MPU6000_REV_C4          0x54
#define MPU6000_REV_C5          0x55
#define MPU6000_REV_D6          0x56
#define MPU6000_REV_D7          0x57
#define MPU6000_REV_D8          0x58
#define MPU6000_REV_D9          0x59
#define MPU6000_REV_D10         0x5A

// ============================================================================
// MPU6000 Implementation
// ============================================================================

MPU6000::MPU6000(DeviceBus* bus, uint8_t whoAmI)
    : bus_(bus)
{
    whoAmI_ = whoAmI;
    // Device was already reset during detection

    // Clock Source PPL with Z axis gyro reference
    bus_->writeReg(MPU_RA_PWR_MGMT_1, MPU_CLK_SEL_PLLGYROZ);
    delayMicroseconds(15);

    // Disable Primary I2C Interface
    bus_->writeReg(MPU_RA_USER_CTRL, BIT_I2C_IF_DIS);
    delayMicroseconds(15);

    // Enable all sensors
    bus_->writeReg(MPU_RA_PWR_MGMT_2, 0x00);
    delayMicroseconds(15);

    // Sample Rate = 1kHz / (1 + SMPLRT_DIV)
    // Set SMPLRT_DIV = 0 for 1kHz sampling
    bus_->writeReg(MPU_RA_SMPLRT_DIV, 0);
    delayMicroseconds(15);

    // Gyro +/- 2000 DPS Full Scale
    bus_->writeReg(MPU_RA_GYRO_CONFIG, INV_FSR_2000DPS << 3);
    delayMicroseconds(15);

    // Accel +/- 16 G Full Scale
    bus_->writeReg(MPU_RA_ACCEL_CONFIG, INV_FSR_16G << 3);
    delayMicroseconds(15);

    // INT_ANYRD_2CLEAR: Interrupt status cleared on any read
    bus_->writeReg(MPU_RA_INT_PIN_CFG, 1 << 4);
    delayMicroseconds(15);

    // Enable data ready interrupt
    bus_->writeReg(MPU_RA_INT_ENABLE, MPU_RF_DATA_RDY_EN);
    delayMicroseconds(15);

    // Set scale factors
    // MPU6000: ±2000 dps = 16.4 LSB/(dps), ±16g = 2048 LSB/g
    gyrScale_ = 1.0f / 16.4f;     // [dps/LSB]
    accScale_ = 1.0f / 2048.0f;   // [G/LSB]
    samplingRateHz_ = 1000;       // 1 kHz
}

MPU6000* MPU6000::detect(DeviceBus* bus)
{
    if (!bus) {
        return nullptr;
    }

    // Reset the device configuration
    bus->writeReg(MPU_RA_PWR_MGMT_1, BIT_H_RESET);
    delay(100);  // Datasheet specifies 100ms delay after reset

    // Reset the device signal paths
    bus->writeReg(MPU_RA_SIGNAL_PATH_RESET, BIT_GYRO | BIT_ACC | BIT_TEMP);
    delay(100);  // Datasheet specifies 100ms delay after signal path reset

    // Read WHO_AM_I register
    const uint8_t whoAmI = bus->readReg(MPU_RA_WHO_AM_I);
    delayMicroseconds(1);  // Ensure CS high time is met

    if (whoAmI != MPU6000_WHO_AM_I_CONST) {
        return nullptr;  // Not MPU6000
    }

    // Read product ID to verify chip revision
    const uint8_t productID = bus->readReg(MPU_RA_PRODUCT_ID);

    // Verify product revision
    bool validRevision = false;
    switch (productID) {
        case MPU6000ES_REV_C4:
        case MPU6000ES_REV_C5:
        case MPU6000_REV_C4:
        case MPU6000_REV_C5:
        case MPU6000ES_REV_D6:
        case MPU6000ES_REV_D7:
        case MPU6000ES_REV_D8:
        case MPU6000_REV_D6:
        case MPU6000_REV_D7:
        case MPU6000_REV_D8:
        case MPU6000_REV_D9:
        case MPU6000_REV_D10:
            validRevision = true;
            break;
    }

    if (!validRevision) {
        return nullptr;  // Unknown product revision
    }

    // Create and initialize device
    return new MPU6000(bus, whoAmI);
}

void MPU6000::read(int16_t* accgyr)
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

const char* MPU6000::typeName() const
{
    return "MPU6000";
}

// ============================================================================
// Preset Configuration (imu_hal.md specification)
// ============================================================================

// MPU6000 preset configuration structure
struct MPU6000PresetConfig {
    uint8_t  dlpf_cfg;      // CONFIG.DLPF_CFG (0-6)
    uint8_t  smplrt_div;    // SMPLRT_DIV register (0-255)
    uint16_t gyro_odr_hz;   // Effective gyro ODR
};

// Preset LUT from imu_hal.md
// MPU6000: DLPF always active, divider = Gyro ODR / (1 + SMPLRT_DIV)
// Base rate is 8kHz when DLPF disabled (DLPF_CFG=0/7), 1kHz when DLPF enabled (DLPF_CFG=1-6)
static const MPU6000PresetConfig MPU6000_PRESETS[] = {
    {2, 0, 1000},  // SAFE: DLPF=2 (98Hz gyro, 94Hz accel), DIV=0 → 1kHz
    {1, 0, 1000},  // SMOOTH: DLPF=1 (188Hz gyro, 184Hz accel), DIV=0 → 1kHz
    {0, 1, 4000},  // BALANCED: DLPF=0 (256Hz), 8kHz / (1+1) = 4kHz
    {0, 0, 8000}   // ACRO: DLPF=0 (256Hz), 8kHz / (1+0) = 8kHz
};

bool MPU6000::applyPreset(ImuPreset preset)
{
    const MPU6000PresetConfig& cfg = MPU6000_PRESETS[static_cast<uint8_t>(preset)];

    // Set DLPF configuration
    bus_->writeReg(MPU_RA_CONFIG, cfg.dlpf_cfg);
    delayMicroseconds(15);

    // Set sample rate divider
    bus_->writeReg(MPU_RA_SMPLRT_DIV, cfg.smplrt_div);
    delayMicroseconds(15);

    // Update sampling rate for user reference
    samplingRateHz_ = cfg.gyro_odr_hz;

    return true;  // MPU6000 doesn't have read-back verification like ICM42688
}

void MPU6000::enableDataReadyInt1()
{
    bus_->writeReg(MPU_RA_INT_ENABLE, MPU_RF_DATA_RDY_EN);
    delayMicroseconds(15);
}

void MPU6000::disableDataReadyInt1()
{
    bus_->writeReg(MPU_RA_INT_ENABLE, 0x00);
    delayMicroseconds(15);
}
