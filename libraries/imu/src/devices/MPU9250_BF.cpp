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

#include "MPU9250_BF.h"
#include "MPU_Common.h"

// ============================================================================
// MPU9250-Specific Definitions
// ============================================================================

// Max SPI frequency: 20 MHz (from Betaflight)
#define MPU9250_MAX_SPI_CLK_HZ 20000000

// DLPF (Digital Low Pass Filter) configuration register
#define MPU9250_CONFIG  0x1A

// ============================================================================
// MPU9250_BF Implementation
// ============================================================================

MPU9250_BF::MPU9250_BF(DeviceBus* bus, uint8_t whoAmI)
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

MPU9250_BF* MPU9250_BF::detect(DeviceBus* bus)
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
            return new MPU9250_BF(bus, whoAmI);
        }

        if (!attemptsRemaining) {
            return nullptr;  // Detection failed
        }
    } while (attemptsRemaining--);

    return nullptr;
}

void MPU9250_BF::read(int16_t* accgyr)
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

const char* MPU9250_BF::typeName() const
{
    // Distinguish between MPU9250 and MPU9255 based on WHO_AM_I
    if (whoAmI_ == MPU9255_WHO_AM_I_CONST) {
        return "MPU9255";
    }
    return "MPU9250";
}
