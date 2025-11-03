/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Arduino adaptation of Betaflight MPU-9250 driver
 * Original authors: Dominic Clifton, John Ihlein, Kalyn Doerr (RS2K)
 */

#include "mpu9250_bf.h"
#include "bf_bus.h"
#include "bf_types.h"
#include "mpu_common.h"
#include <Arduino.h>

// 20 MHz max SPI frequency
#define MPU9250_MAX_SPI_CLK_HZ 20000000

// Bits
#define BIT_SLEEP                   0x40
#define BIT_H_RESET                 0x80
#define BITS_CLKSEL                 0x07
#define MPU_CLK_SEL_PLLGYROX        0x01
#define MPU_CLK_SEL_PLLGYROZ        0x03
#define INV_CLK_PLL                 0x01
#define MPU_EXT_SYNC_GYROX          0x02
#define BITS_FS_250DPS              0x00
#define BITS_FS_500DPS              0x08
#define BITS_FS_1000DPS             0x10
#define BITS_FS_2000DPS             0x18
#define BITS_FS_2G                  0x00
#define BITS_FS_4G                  0x08
#define BITS_FS_8G                  0x10
#define BITS_FS_16G                 0x18
#define BITS_FS_MASK                0x18
#define BIT_INT_ANYRD_2CLEAR        0x10
#define BIT_RAW_RDY_EN              0x01
#define BIT_I2C_IF_DIS              0x10
#define BIT_INT_STATUS_DATA         0x01
#define BIT_GYRO                    0x04
#define BIT_ACC                     0x02
#define BIT_TEMP                    0x01
#define BIT_BYPASS_EN               0x02

static void mpu9250AccAndGyroInit(gyroDev_t *gyro);

// MPU-9250 write with timing delays
bool mpu9250SpiWriteRegister(const extDevice_t *dev, uint8_t reg, uint8_t data)
{
    delayMicroseconds(1);
    spiWriteReg(dev, reg, data);
    delayMicroseconds(1);
    return true;
}

// Slow read with timing delays
static bool mpu9250SpiSlowReadRegisterBuffer(const extDevice_t *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
    delayMicroseconds(1);
    spiReadRegBuf(dev, reg | 0x80, data, length);
    delayMicroseconds(1);
    return true;
}

// Write-verify with retry loop (critical for MPU-9250 reliability)
bool mpu9250SpiWriteRegisterVerify(const extDevice_t *dev, uint8_t reg, uint8_t data)
{
    mpu9250SpiWriteRegister(dev, reg, data);
    delayMicroseconds(100);

    uint8_t attemptsRemaining = 20;
    do {
        uint8_t in;
        mpu9250SpiSlowReadRegisterBuffer(dev, reg, &in, 1);
        if (in == data) {
            return true;
        } else {
            mpu9250SpiWriteRegister(dev, reg, data);
            delayMicroseconds(100);
        }
    } while (attemptsRemaining--);
    return false;
}

// Internal gyro initialization
static void mpu9250SpiGyroInitInternal(gyroDev_t *gyro)
{
    extDevice_t *dev = &gyro->dev;

    mpu9250AccAndGyroInit(gyro);

    spiSetClkDivisor(dev, 0); // Max speed now that slow registers are initialized

    // Initial gyro read to verify communication
    mpu9250GyroRead(gyro);

    // Check for invalid data (all -1 indicates communication failure)
    if (((int8_t)gyro->gyroADCRaw[1]) == -1 && ((int8_t)gyro->gyroADCRaw[0]) == -1) {
        // Communication failure
        gyro->mpuDetectionResult = MPU_NONE;
    }
}

// Internal accel initialization
static void mpu9250SpiAccInitInternal(accDev_t *acc)
{
    // 512 LSB/g for ±16g range, 4 = scale factor
    acc->acc_1G = 512 * 4;
}

uint8_t mpu9250SpiDetect(const extDevice_t *dev)
{
    // Reset the device
    mpu9250SpiWriteRegister(dev, MPU_RA_PWR_MGMT_1, MPU9250_BIT_RESET);

    // WHO_AM_I detection with retry loop (MPU-9250 specific)
    uint8_t attemptsRemaining = 20;
    do {
        delay(150);  // Longer delay for MPU-9250
        const uint8_t in = spiReadRegMsk(dev, MPU_RA_WHO_AM_I);
        if (in == MPU9250_WHO_AM_I_CONST || in == MPU9255_WHO_AM_I_CONST) {
            return MPU_9250_SPI;
        }
        if (!attemptsRemaining) {
            return MPU_NONE;
        }
    } while (attemptsRemaining--);

    return MPU_NONE;
}

static void mpu9250AccAndGyroInit(gyroDev_t *gyro)
{
    extDevice_t *dev = &gyro->dev;

    // Reset device
    mpu9250SpiWriteRegister(dev, MPU_RA_PWR_MGMT_1, MPU9250_BIT_RESET);
    delay(50);

    // Power management - PLL with internal oscillator
    mpu9250SpiWriteRegisterVerify(dev, MPU_RA_PWR_MGMT_1, INV_CLK_PLL);

    // Gyro config - ±2000 dps
    mpu9250SpiWriteRegisterVerify(dev, MPU_RA_GYRO_CONFIG, INV_FSR_2000DPS << 3);

    // DLPF config
    mpu9250SpiWriteRegisterVerify(dev, MPU_RA_CONFIG, gyro->hardware_lpf);

    // Sample rate divider
    mpu9250SpiWriteRegisterVerify(dev, MPU_RA_SMPLRT_DIV, 0);  // No divider

    // Accel config - ±16g
    mpu9250SpiWriteRegisterVerify(dev, MPU_RA_ACCEL_CONFIG, INV_FSR_16G << 3);

    // INT pin config - includes BYPASS_EN for magnetometer access
    mpu9250SpiWriteRegisterVerify(dev, MPU_RA_INT_PIN_CFG,
        0 << 7 |  // INT_LEVEL (active high)
        0 << 6 |  // INT_OPEN (push-pull)
        0 << 5 |  // LATCH_INT_EN (50us pulse)
        1 << 4 |  // INT_ANYRD_2CLEAR (clear on any read)
        0 << 3 |  // ACTL_FSYNC
        0 << 2 |  // FSYNC_INT_MODE_EN
        1 << 1 |  // BYPASS_EN (enable I2C bypass for magnetometer)
        0 << 0);  // Reserved

    // Enable data ready interrupt
    mpu9250SpiWriteRegisterVerify(dev, MPU_RA_INT_ENABLE, 0x01);
}

// Public API functions
void mpu9250InitGyro(gyroDev_t *gyro)
{
    mpu9250SpiGyroInitInternal(gyro);
}

void mpu9250InitAcc(accDev_t *acc)
{
    mpu9250SpiAccInitInternal(acc);
}

bool mpu9250GyroRead(gyroDev_t *gyro)
{
    uint8_t buf[7];

    // Read gyro registers starting from GYRO_XOUT_H
    buf[0] = MPU_RA_GYRO_XOUT_H | 0x80; // Set read bit

    if (!spiBusTransferMultiple(&gyro->dev, buf, buf, 7)) {
        return false;
    }

    // Data is in buf[1..6] after the register address byte
    gyro->gyroADCRaw[0] = (int16_t)((buf[1] << 8) | buf[2]);  // X
    gyro->gyroADCRaw[1] = (int16_t)((buf[3] << 8) | buf[4]);  // Y
    gyro->gyroADCRaw[2] = (int16_t)((buf[5] << 8) | buf[6]);  // Z

    return true;
}

bool mpu9250AccRead(accDev_t *acc)
{
    uint8_t buf[7];

    // Read accel registers starting from ACCEL_XOUT_H
    buf[0] = MPU_RA_ACCEL_XOUT_H | 0x80; // Set read bit

    if (!spiBusTransferMultiple(&acc->dev, buf, buf, 7)) {
        return false;
    }

    // Data is in buf[1..6] after the register address byte
    acc->accADC[0] = (int16_t)((buf[1] << 8) | buf[2]);  // X
    acc->accADC[1] = (int16_t)((buf[3] << 8) | buf[4]);  // Y
    acc->accADC[2] = (int16_t)((buf[5] << 8) | buf[6]);  // Z

    return true;
}
