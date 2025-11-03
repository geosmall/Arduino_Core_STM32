/*
 * MPU9250 Arduino Library
 *
 * Copyright (C) 2025 Arduino_Core_STM32 Contributors
 *
 * This file is part of the MPU9250 Arduino library, derived from Betaflight.
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

#include "MPU9250.h"
#include "mpu9250_bf.h"
#include "mpu_common.h"
#include "bf_bus.h"
#include "bf_types.h"
#include <stdlib.h>

// Default gyro scale for ±2000 dps
#define GYRO_SCALE_2000DPS (2000.0f / 32768.0f)

// Default accel scale for ±16g
#define ACCEL_SCALE_16G (16.0f / 32768.0f)

MPU9250::MPU9250()
    : gyro_dev(nullptr)
    , acc_dev(nullptr)
    , initialized(false)
    , gyro_scale(GYRO_SCALE_2000DPS)
    , accel_scale(ACCEL_SCALE_16G)
{
}

MPU9250::~MPU9250()
{
    if (gyro_dev) {
        free(gyro_dev);
        gyro_dev = nullptr;
    }
    if (acc_dev) {
        free(acc_dev);
        acc_dev = nullptr;
    }
}

bool MPU9250::begin(SPIClass &spi, uint8_t cs_pin, uint32_t freq)
{
    // Allocate device structures
    gyro_dev = malloc(sizeof(gyroDev_t));
    acc_dev = malloc(sizeof(accDev_t));

    if (!gyro_dev || !acc_dev) {
        return false;
    }

    // Initialize structures to zero
    memset(gyro_dev, 0, sizeof(gyroDev_t));
    memset(acc_dev, 0, sizeof(accDev_t));

    gyroDev_t *gyro = (gyroDev_t *)gyro_dev;
    accDev_t *acc = (accDev_t *)acc_dev;

    // Setup SPI device
    gyro->dev.spi = &spi;
    gyro->dev.cs_pin = cs_pin;
    gyro->dev.freq = freq;

    // Share the same SPI device for accel
    acc->dev = gyro->dev;

    // Configure chip select pin
    pinMode(cs_pin, OUTPUT);
    digitalWrite(cs_pin, HIGH);

    // Initialize SPI
    spi.begin();

    // Attempt detection
    uint8_t detected = mpu9250SpiDetect(&gyro->dev);

    if (detected != MPU_9250_SPI) {
        initialized = false;
        return false;
    }

    // Set hardware LPF to 250 Hz by default
    gyro->hardware_lpf = BITS_DLPF_CFG_250HZ;

    // Initialize gyro and accelerometer
    mpu9250InitGyro(gyro);
    mpu9250InitAcc(acc);

    initialized = true;
    return true;
}

uint8_t MPU9250::whoAmI()
{
    if (!initialized) {
        return 0;
    }

    gyroDev_t *gyro = (gyroDev_t *)gyro_dev;
    return spiReadRegMsk(&gyro->dev, 0x75); // MPU_RA_WHO_AM_I
}

bool MPU9250::readGyro(float &gx, float &gy, float &gz)
{
    if (!initialized) {
        return false;
    }

    gyroDev_t *gyro = (gyroDev_t *)gyro_dev;

    if (!mpu9250GyroRead(gyro)) {
        return false;
    }

    // Convert raw data to degrees/second
    gx = (float)gyro->gyroADCRaw[0] * gyro_scale;
    gy = (float)gyro->gyroADCRaw[1] * gyro_scale;
    gz = (float)gyro->gyroADCRaw[2] * gyro_scale;

    return true;
}

bool MPU9250::readAccel(float &ax, float &ay, float &az)
{
    if (!initialized) {
        return false;
    }

    accDev_t *acc = (accDev_t *)acc_dev;

    if (!mpu9250AccRead(acc)) {
        return false;
    }

    // Convert raw data to g
    ax = (float)acc->accADC[0] * accel_scale;
    ay = (float)acc->accADC[1] * accel_scale;
    az = (float)acc->accADC[2] * accel_scale;

    return true;
}

bool MPU9250::read6DOF(float &gx, float &gy, float &gz,
                       float &ax, float &ay, float &az)
{
    bool gyro_ok = readGyro(gx, gy, gz);
    bool accel_ok = readAccel(ax, ay, az);

    return gyro_ok && accel_ok;
}

void MPU9250::setDLPF(uint8_t gyro_dlpf, uint8_t accel_dlpf)
{
    if (!initialized || gyro_dlpf > 7 || accel_dlpf > 7) {
        return;
    }

    gyroDev_t *gyro = (gyroDev_t *)gyro_dev;
    gyro->hardware_lpf = gyro_dlpf;

    // Write to CONFIG register (gyro DLPF)
    mpu9250SpiWriteRegisterVerify(&gyro->dev, 0x1A, gyro_dlpf); // MPU_RA_CONFIG

    // Write to ACCEL_CONFIG2 register (accel DLPF)
    mpu9250SpiWriteRegisterVerify(&gyro->dev, 0x1D, accel_dlpf); // MPU_RA_ACCEL_CONFIG2
}

void MPU9250::setGyroFSR(uint16_t fsr)
{
    if (!initialized) {
        return;
    }

    gyroDev_t *gyro = (gyroDev_t *)gyro_dev;
    uint8_t fsr_bits;

    switch (fsr) {
        case 250:
            fsr_bits = INV_FSR_250DPS << 3;
            gyro_scale = 250.0f / 32768.0f;
            break;
        case 500:
            fsr_bits = INV_FSR_500DPS << 3;
            gyro_scale = 500.0f / 32768.0f;
            break;
        case 1000:
            fsr_bits = INV_FSR_1000DPS << 3;
            gyro_scale = 1000.0f / 32768.0f;
            break;
        case 2000:
        default:
            fsr_bits = INV_FSR_2000DPS << 3;
            gyro_scale = 2000.0f / 32768.0f;
            break;
    }

    // Write to GYRO_CONFIG register with write-verify
    mpu9250SpiWriteRegisterVerify(&gyro->dev, 0x1B, fsr_bits); // MPU_RA_GYRO_CONFIG
}

void MPU9250::setAccelFSR(uint8_t fsr)
{
    if (!initialized) {
        return;
    }

    gyroDev_t *gyro = (gyroDev_t *)gyro_dev;
    accDev_t *acc = (accDev_t *)acc_dev;
    uint8_t fsr_bits;

    switch (fsr) {
        case 2:
            fsr_bits = INV_FSR_2G << 3;
            accel_scale = 2.0f / 32768.0f;
            acc->acc_1G = 512 * 8;  // 4096 LSB/g for ±2g
            break;
        case 4:
            fsr_bits = INV_FSR_4G << 3;
            accel_scale = 4.0f / 32768.0f;
            acc->acc_1G = 512 * 4;  // 2048 LSB/g for ±4g
            break;
        case 8:
            fsr_bits = INV_FSR_8G << 3;
            accel_scale = 8.0f / 32768.0f;
            acc->acc_1G = 512 * 2;  // 1024 LSB/g for ±8g
            break;
        case 16:
        default:
            fsr_bits = INV_FSR_16G << 3;
            accel_scale = 16.0f / 32768.0f;
            acc->acc_1G = 512;      // 512 LSB/g for ±16g
            break;
    }

    // Write to ACCEL_CONFIG register with write-verify
    mpu9250SpiWriteRegisterVerify(&gyro->dev, 0x1C, fsr_bits); // MPU_RA_ACCEL_CONFIG
}

void MPU9250::setSampleRateDivider(uint8_t divider)
{
    if (!initialized) {
        return;
    }

    gyroDev_t *gyro = (gyroDev_t *)gyro_dev;
    gyro->mpuDividerDrops = divider;

    // Write to SMPLRT_DIV register with write-verify
    mpu9250SpiWriteRegisterVerify(&gyro->dev, 0x19, divider); // MPU_RA_SMPLRT_DIV
}
