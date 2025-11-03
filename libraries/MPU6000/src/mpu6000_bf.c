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
 * Arduino adaptation of Betaflight MPU-6000 driver
 * Original authors: Dominic Clifton, John Ihlein
 */

#include "mpu6000_bf.h"
#include "bf_bus.h"
#include "bf_types.h"
#include "mpu_common.h"
#include <Arduino.h>

// 20 MHz max SPI frequency
#define MPU6000_MAX_SPI_CLK_HZ 20000000

// Bits
#define BIT_SLEEP                   0x40
#define BIT_H_RESET                 0x80
#define BITS_CLKSEL                 0x07
#define MPU_CLK_SEL_PLLGYROX        0x01
#define MPU_CLK_SEL_PLLGYROZ        0x03
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

static void mpu6000AccAndGyroInit(gyroDev_t *gyro);

// Internal gyro initialization
static void mpu6000SpiGyroInitInternal(gyroDev_t *gyro)
{
    extDevice_t *dev = &gyro->dev;

    mpu6000AccAndGyroInit(gyro);

    // Accel and Gyro DLPF Setting
    spiWriteReg(dev, MPU6000_CONFIG, gyro->hardware_lpf);
    delayMicroseconds(1);

    spiSetClkDivisor(dev, 0); // Max speed for MPU-6000

    // Initial gyro read to verify communication
    mpu6000GyroRead(gyro);

    // Check for invalid data (all -1 indicates communication failure)
    if (((int8_t)gyro->gyroADCRaw[1]) == -1 && ((int8_t)gyro->gyroADCRaw[0]) == -1) {
        // Communication failure - device not responding properly
        gyro->mpuDetectionResult = MPU_NONE;
    }
}

// Internal accel initialization
static void mpu6000SpiAccInitInternal(accDev_t *acc)
{
    // 512 LSB/g for ±16g range, 4 = scale factor
    acc->acc_1G = 512 * 4;
}

uint8_t mpu6000SpiDetect(const extDevice_t *dev)
{
    // Reset the device configuration
    spiWriteReg(dev, MPU_RA_PWR_MGMT_1, BIT_H_RESET);
    delay(100);  // Datasheet specifies a 100ms delay after reset

    // Reset the device signal paths
    spiWriteReg(dev, MPU_RA_SIGNAL_PATH_RESET, BIT_GYRO | BIT_ACC | BIT_TEMP);
    delay(100);  // Datasheet specifies a 100ms delay after signal path reset

    const uint8_t whoAmI = spiReadRegMsk(dev, MPU_RA_WHO_AM_I);
    delayMicroseconds(1); // Ensure CS high time is met
    uint8_t detectedSensor = MPU_NONE;

    if (whoAmI == MPU6000_WHO_AM_I_CONST) {
        const uint8_t productID = spiReadRegMsk(dev, MPU_RA_PRODUCT_ID);

        /* Look for a product ID we recognise */
        // Verify product revision
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
            detectedSensor = MPU_60x0_SPI;
            break;
        }
    }

    return detectedSensor;
}

static void mpu6000AccAndGyroInit(gyroDev_t *gyro)
{
    extDevice_t *dev = &gyro->dev;

    // Device was already reset during detection so proceed with configuration

    // Clock Source PPL with Z axis gyro reference
    spiWriteReg(dev, MPU_RA_PWR_MGMT_1, MPU_CLK_SEL_PLLGYROZ);
    delayMicroseconds(15);

    // Disable Primary I2C Interface
    spiWriteReg(dev, MPU_RA_USER_CTRL, BIT_I2C_IF_DIS);
    delayMicroseconds(15);

    spiWriteReg(dev, MPU_RA_PWR_MGMT_2, 0x00);
    delayMicroseconds(15);

    // Accel Sample Rate 1kHz
    // Gyroscope Output Rate = 1kHz when the DLPF is enabled
    spiWriteReg(dev, MPU_RA_SMPLRT_DIV, 0);  // No sample rate divider
    delayMicroseconds(15);

    // Gyro +/- 2000 DPS Full Scale (default)
    spiWriteReg(dev, MPU_RA_GYRO_CONFIG, INV_FSR_2000DPS << 3);
    delayMicroseconds(15);

    // Accel +/- 16 G Full Scale (default)
    spiWriteReg(dev, MPU_RA_ACCEL_CONFIG, INV_FSR_16G << 3);
    delayMicroseconds(15);

    spiWriteReg(dev, MPU_RA_INT_PIN_CFG, 0 << 7 | 0 << 6 | 0 << 5 | 1 << 4 | 0 << 3 | 0 << 2 | 0 << 1 | 0 << 0);  // INT_ANYRD_2CLEAR
    delayMicroseconds(15);

    spiWriteReg(dev, MPU_RA_INT_ENABLE, MPU_RF_DATA_RDY_EN);
    delayMicroseconds(15);
}

// Public API functions
void mpu6000InitGyro(gyroDev_t *gyro)
{
    mpu6000SpiGyroInitInternal(gyro);
}

void mpu6000InitAcc(accDev_t *acc)
{
    mpu6000SpiAccInitInternal(acc);
}

bool mpu6000GyroRead(gyroDev_t *gyro)
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

bool mpu6000AccRead(accDev_t *acc)
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
