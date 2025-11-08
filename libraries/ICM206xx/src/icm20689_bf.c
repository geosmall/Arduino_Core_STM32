/*
 * ICM20689 Betaflight Driver
 *
 * Adapted from Betaflight: drivers/accgyro/accgyro_spi_icm20689.c
 * Copyright (C) Betaflight Contributors
 *
 * This file is part of the ICM20689 Arduino library, derived from Betaflight.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#include <Arduino.h>
#include "icm20689_bf.h"
#include "mpu_common.h"
#include "bf_bus.h"

// 8 MHz max SPI frequency (Betaflight tested safe limit)
#define ICM20689_MAX_SPI_CLK_HZ 8000000

// Register 0x37 - INT_PIN_CFG
#define ICM20689_INT_ANYRD_2CLEAR   0x10

// Register 0x68 - SIGNAL_PATH_RESET
#define ICM20689_ACCEL_RST          0x02
#define ICM20689_TEMP_RST           0x01

// Register 0x6a - USER_CTRL
#define ICM20689_I2C_IF_DIS         0x10

// Register 0x6b - PWR_MGMT_1
#define ICM20689_BIT_RESET          0x80

/* Clock selection settle time
 * ICM-20690 datasheet section 10.11:
 * Clock selection takes ~20us to settle
 * Testing shows 60us required, double for margin
 */
#define ICM20689_CLKSEL_SETTLE_US   120

/* Reset and path reset delays
 * MPU-6000 datasheet section 4.28 suggests 100ms
 */
#define ICM20689_RESET_DELAY_MS     100
#define ICM20689_PATH_RESET_DELAY_MS 100

// WHO_AM_I values for ICM-206xx family
#define ICM20601_WHO_AM_I_CONST     0xAC
#define ICM20602_WHO_AM_I_CONST     0x12
#define ICM20608G_WHO_AM_I_CONST    0xAF
#define ICM20689_WHO_AM_I_CONST     0x98

/**
 * @brief Detect ICM-206xx chip via WHO_AM_I register
 *
 * @param dev SPI device structure
 * @return Detected chip type or MPU_NONE
 */
uint8_t icm20689SpiDetect(const extDevice_t *dev)
{
    // Reset the device configuration
    spiWriteReg(dev, MPU_RA_PWR_MGMT_1, ICM20689_BIT_RESET);
    delay(ICM20689_RESET_DELAY_MS);

    // Read WHO_AM_I register
    const uint8_t whoAmI = spiReadRegMsk(dev, MPU_RA_WHO_AM_I);

    uint8_t icmDetected;
    switch (whoAmI) {
    case ICM20601_WHO_AM_I_CONST:
        icmDetected = ICM_20601_SPI;
        break;
    case ICM20602_WHO_AM_I_CONST:
        icmDetected = ICM_20602_SPI;
        break;
    case ICM20608G_WHO_AM_I_CONST:
        icmDetected = ICM_20608_SPI;
        break;
    case ICM20689_WHO_AM_I_CONST:
        icmDetected = ICM_20689_SPI;
        break;
    default:
        icmDetected = MPU_NONE;
        return icmDetected;
    }

    // Device recognized - perform device-specific register accesses

    // Disable Primary I2C Interface
    spiWriteReg(dev, MPU_RA_USER_CTRL, ICM20689_I2C_IF_DIS);

    // Reset the device signal paths
    spiWriteReg(dev, MPU_RA_SIGNAL_PATH_RESET, ICM20689_ACCEL_RST | ICM20689_TEMP_RST);
    delay(ICM20689_PATH_RESET_DELAY_MS);

    return icmDetected;
}

/**
 * @brief Initialize ICM-206xx gyroscope
 *
 * Sequence:
 * 1. Set PLL clock source with settle delay
 * 2. Configure gyro FSR (±2000 dps)
 * 3. Configure accel FSR (±16g)
 * 4. Set DLPF bandwidth
 * 5. Set sample rate divider
 * 6. Configure data ready interrupt
 *
 * @param gyro Gyro device structure
 */
void icm20689InitGyro(gyroDev_t *gyro)
{
    extDevice_t *dev = &gyro->dev;

    // Set SPI clock divisor (up to 8 MHz safe)
    spiSetClkDivisor(dev, ICM20689_MAX_SPI_CLK_HZ);

    // Device was already reset during detection, proceed with configuration

    // 1. Select PLL clock source
    spiWriteReg(dev, MPU_RA_PWR_MGMT_1, INV_CLK_PLL);
    delayMicroseconds(ICM20689_CLKSEL_SETTLE_US);

    // 2. Configure gyro full-scale range (±2000 dps)
    spiWriteReg(dev, MPU_RA_GYRO_CONFIG, INV_FSR_2000DPS << 3);

    // 3. Configure accel full-scale range (±16g)
    spiWriteReg(dev, MPU_RA_ACCEL_CONFIG, INV_FSR_16G << 3);

    // 4. Set DLPF bandwidth
    spiWriteReg(dev, MPU_RA_CONFIG, gyro->hardware_lpf);

    // 5. Set sample rate divider
    // Note: mpuDividerDrops is application-specific (e.g., for 8kHz→1kHz)
    // Default is 0 (no division)
    spiWriteReg(dev, MPU_RA_SMPLRT_DIV, 0);

    // 6. Configure data ready interrupt
    spiWriteReg(dev, MPU_RA_INT_PIN_CFG, ICM20689_INT_ANYRD_2CLEAR);
    spiWriteReg(dev, MPU_RA_INT_ENABLE, MPU_RF_DATA_RDY_EN);
}

/**
 * @brief Initialize ICM-206xx accelerometer
 *
 * @param acc Accelerometer device structure
 */
void icm20689InitAcc(accDev_t *acc)
{
    // Set accelerometer 1G value (for ±8g range: 512 * 4 = 2048 LSB/g)
    // Note: We configure ±16g in gyro init, so this is 512 LSB/g
    acc->acc_1G = 512 * 4;
}

/**
 * @brief Read gyroscope data from ICM-206xx
 *
 * @param gyro Gyro device structure
 * @return true if read successful, false otherwise
 */
bool icm20689GyroRead(gyroDev_t *gyro)
{
    uint8_t data[6];

    // Read 6 bytes starting from GYRO_XOUT_H
    const bool ack = spiReadRegBuf(&gyro->dev, MPU_RA_GYRO_XOUT_H, data, 6);
    if (!ack) {
        return false;
    }

    // Convert to 16-bit signed integers (big-endian)
    gyro->gyroADCRaw[0] = (int16_t)((data[0] << 8) | data[1]);
    gyro->gyroADCRaw[1] = (int16_t)((data[2] << 8) | data[3]);
    gyro->gyroADCRaw[2] = (int16_t)((data[4] << 8) | data[5]);

    return true;
}

/**
 * @brief Read accelerometer data from ICM-206xx
 *
 * @param acc Accelerometer device structure
 * @return true if read successful, false otherwise
 */
bool icm20689AccRead(accDev_t *acc)
{
    uint8_t data[6];

    // Read 6 bytes starting from ACCEL_XOUT_H
    const bool ack = spiReadRegBuf(&acc->dev, MPU_RA_ACCEL_XOUT_H, data, 6);
    if (!ack) {
        return false;
    }

    // Convert to 16-bit signed integers (big-endian)
    acc->ADCRaw[0] = (int16_t)((data[0] << 8) | data[1]);
    acc->ADCRaw[1] = (int16_t)((data[2] << 8) | data[3]);
    acc->ADCRaw[2] = (int16_t)((data[4] << 8) | data[5]);

    // Copy to accADC for compatibility
    acc->accADC[0] = acc->ADCRaw[0];
    acc->accADC[1] = acc->ADCRaw[1];
    acc->accADC[2] = acc->ADCRaw[2];

    return true;
}
