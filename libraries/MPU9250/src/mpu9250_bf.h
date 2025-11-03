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

#ifndef MPU9250_BF_H
#define MPU9250_BF_H

#include "bf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// MPU-9250 specific defines
#define MPU9250_CONFIG              0x1A

#define BITS_DLPF_CFG_250HZ         0x00
#define BITS_DLPF_CFG_184HZ         0x01
#define BITS_DLPF_CFG_92HZ          0x02
#define BITS_DLPF_CFG_41HZ          0x03
#define BITS_DLPF_CFG_20HZ          0x04
#define BITS_DLPF_CFG_10HZ          0x05
#define BITS_DLPF_CFG_5HZ           0x06
#define BITS_DLPF_CFG_3600HZ        0x07

#define GYRO_SCALE_FACTOR  0.00053292f  // (4/131) * pi/180   (32.75 LSB = 1 DPS)

#define MPU9250_BIT_RESET           0x80

// RF = Register Flag
#define MPU_RF_DATA_RDY_EN (1 << 0)

// Function prototypes
uint8_t mpu9250SpiDetect(const extDevice_t *dev);
bool mpu9250GyroRead(gyroDev_t *gyro);
bool mpu9250AccRead(accDev_t *acc);
void mpu9250InitGyro(gyroDev_t *gyro);
void mpu9250InitAcc(accDev_t *acc);

// Write-verify helpers
bool mpu9250SpiWriteRegister(const extDevice_t *dev, uint8_t reg, uint8_t data);
bool mpu9250SpiWriteRegisterVerify(const extDevice_t *dev, uint8_t reg, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif // MPU9250_BF_H
