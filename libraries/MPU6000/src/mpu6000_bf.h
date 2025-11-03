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

#ifndef MPU6000_BF_H
#define MPU6000_BF_H

#include "bf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// MPU-6000 specific defines
#define MPU6000_CONFIG              0x1A

#define BITS_DLPF_CFG_256HZ         0x00
#define BITS_DLPF_CFG_188HZ         0x01
#define BITS_DLPF_CFG_98HZ          0x02
#define BITS_DLPF_CFG_42HZ          0x03
#define BITS_DLPF_CFG_20HZ          0x04
#define BITS_DLPF_CFG_10HZ          0x05
#define BITS_DLPF_CFG_5HZ           0x06

#define GYRO_SCALE_FACTOR  0.00053292f  // (4/131) * pi/180   (32.75 LSB = 1 DPS)

// Product ID values for MPU-6000
#define MPU6000ES_REV_C4 0x14
#define MPU6000ES_REV_C5 0x15
#define MPU6000ES_REV_D6 0x16
#define MPU6000ES_REV_D7 0x17
#define MPU6000ES_REV_D8 0x18
#define MPU6000_REV_C4 0x54
#define MPU6000_REV_C5 0x55
#define MPU6000_REV_D6 0x56
#define MPU6000_REV_D7 0x57
#define MPU6000_REV_D8 0x58
#define MPU6000_REV_D9 0x59
#define MPU6000_REV_D10 0x5A

// Function prototypes
uint8_t mpu6000SpiDetect(const extDevice_t *dev);
bool mpu6000GyroRead(gyroDev_t *gyro);
bool mpu6000AccRead(accDev_t *acc);
void mpu6000InitGyro(gyroDev_t *gyro);
void mpu6000InitAcc(accDev_t *acc);

#ifdef __cplusplus
}
#endif

#endif // MPU6000_BF_H
