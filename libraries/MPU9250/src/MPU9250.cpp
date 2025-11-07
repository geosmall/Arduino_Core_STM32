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
#include "ak8963.h"
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
    , mag_initialized(false)
    , mag_scale_x(1.0f)
    , mag_scale_y(1.0f)
    , mag_scale_z(1.0f)
    , mag_bias_x(0.0f)
    , mag_bias_y(0.0f)
    , mag_bias_z(0.0f)
    , mag_scale_factor_x(1.0f)
    , mag_scale_factor_y(1.0f)
    , mag_scale_factor_z(1.0f)
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

    // Initialize magnetometer (AK8963)
    // Note: Magnetometer initialization can fail without affecting gyro/accel
    mag_initialized = initAK8963();

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

// ============================================================================
// AK8963 Magnetometer Support
// ============================================================================

bool MPU9250::writeAK8963Register(uint8_t reg, uint8_t value)
{
    if (!initialized) {
        return false;
    }

    gyroDev_t *gyro = (gyroDev_t *)gyro_dev;

    // Set I2C slave 0 for write operation
    spiWriteReg(&gyro->dev, MPU_RA_I2C_SLV0_ADDR, AK8963_I2C_ADDR);  // Write mode
    spiWriteReg(&gyro->dev, MPU_RA_I2C_SLV0_REG, reg);               // Target register
    spiWriteReg(&gyro->dev, MPU_RA_I2C_SLV0_DO, value);              // Data to write
    spiWriteReg(&gyro->dev, MPU_RA_I2C_SLV0_CTRL, I2C_SLV0_EN | 0x01); // Enable + 1 byte

    delay(10); // Wait for I2C transaction to complete

    return true;
}

bool MPU9250::readAK8963Registers(uint8_t reg, uint8_t count, uint8_t *dest)
{
    if (!initialized || !dest) {
        return false;
    }

    gyroDev_t *gyro = (gyroDev_t *)gyro_dev;

    // Set I2C slave 0 for read operation
    spiWriteReg(&gyro->dev, MPU_RA_I2C_SLV0_ADDR, AK8963_I2C_ADDR | I2C_READ_FLAG);  // Read mode
    spiWriteReg(&gyro->dev, MPU_RA_I2C_SLV0_REG, reg);                               // Target register
    spiWriteReg(&gyro->dev, MPU_RA_I2C_SLV0_CTRL, I2C_SLV0_EN | count);              // Enable + byte count

    delay(1); // Wait for data to populate EXT_SENS_DATA registers

    // Read from EXT_SENS_DATA registers
    for (uint8_t i = 0; i < count; i++) {
        dest[i] = spiReadRegMsk(&gyro->dev, MPU_RA_EXT_SENS_DATA_00 + i);
    }

    return true;
}

uint8_t MPU9250::whoAmIAK8963()
{
    uint8_t who_am_i = 0;
    readAK8963Registers(AK8963_WHO_AM_I, 1, &who_am_i);
    return who_am_i;
}

bool MPU9250::initAK8963()
{
    if (!initialized) {
        return false;
    }

    gyroDev_t *gyro = (gyroDev_t *)gyro_dev;

    // Disable I2C bypass mode (we'll use I2C master instead)
    uint8_t user_ctrl = spiReadRegMsk(&gyro->dev, MPU_RA_USER_CTRL);
    user_ctrl |= I2C_MST_EN;  // Enable I2C master
    spiWriteReg(&gyro->dev, MPU_RA_USER_CTRL, user_ctrl);
    delay(10);

    // Configure I2C master clock to 400 kHz
    spiWriteReg(&gyro->dev, MPU_RA_I2C_MST_CTRL, I2C_MST_CLK_400KHZ);
    delay(10);

    // Power down AK8963
    writeAK8963Register(AK8963_CNTL1, AK8963_CNTL1_MODE_POWER_DOWN);
    delay(100);  // Critical: 100ms delay for mode change

    // Soft reset AK8963
    writeAK8963Register(AK8963_CNTL2, AK8963_CNTL2_SRST);
    delay(100);  // Critical: 100ms delay for reset

    // Verify AK8963 WHO_AM_I
    uint8_t who_am_i = whoAmIAK8963();
    if (who_am_i != AK8963_WHO_AM_I_RESPONSE) {
        // Magnetometer not detected, but don't fail entire init
        return false;
    }

    // Enter fuse ROM access mode to read ASA calibration values
    writeAK8963Register(AK8963_CNTL1, AK8963_MODE_FUSE_ROM_16BIT);
    delay(100);  // Critical: 100ms delay for mode change

    // Read ASA (Sensitivity Adjustment) values
    uint8_t asa[3];
    if (!readAK8963Registers(AK8963_ASAX, 3, asa)) {
        return false;
    }

    // Calculate magnetometer scale factors from ASA values
    // Formula: Scale = ((ASA - 128) / 256 + 1) * 4912 / 32760
    mag_scale_x = ((float)(asa[0] - 128) / 256.0f + 1.0f) * AK8963_SENSITIVITY_SCALE_FACTOR;
    mag_scale_y = ((float)(asa[1] - 128) / 256.0f + 1.0f) * AK8963_SENSITIVITY_SCALE_FACTOR;
    mag_scale_z = ((float)(asa[2] - 128) / 256.0f + 1.0f) * AK8963_SENSITIVITY_SCALE_FACTOR;

    // Power down before changing mode
    writeAK8963Register(AK8963_CNTL1, AK8963_CNTL1_MODE_POWER_DOWN);
    delay(100);  // Critical: 100ms delay for mode change

    // Set continuous measurement mode 2 (100 Hz, 16-bit)
    writeAK8963Register(AK8963_CNTL1, AK8963_MODE_CONT_2_16BIT);
    delay(100);  // Critical: 100ms delay for mode change

    // Configure I2C Slave 0 for continuous auto-reading of magnetometer data
    // This configures the MPU9250 to automatically read 7 bytes from AK8963
    // at the gyro sample rate and populate EXT_SENS_DATA_00 to EXT_SENS_DATA_06
    // (Matches Teensy library approach for high-speed reading)
    spiWriteReg(&gyro->dev, MPU_RA_I2C_SLV0_ADDR, AK8963_I2C_ADDR | I2C_READ_FLAG);
    spiWriteReg(&gyro->dev, MPU_RA_I2C_SLV0_REG, AK8963_HXL);
    spiWriteReg(&gyro->dev, MPU_RA_I2C_SLV0_CTRL, I2C_SLV0_EN | 0x07);  // Enable + 7 bytes
    delay(10);

    return true;
}

bool MPU9250::readMagnetometer(float &mx, float &my, float &mz)
{
    if (!initialized || !mag_initialized) {
        return false;
    }

    gyroDev_t *gyro = (gyroDev_t *)gyro_dev;

    // Read magnetometer data directly from EXT_SENS_DATA registers
    // The MPU9250 I2C master automatically populates these at the gyro sample rate
    // (7 bytes: HXL, HXH, HYL, HYH, HZL, HZH, ST2)
    // This approach matches the Teensy library and eliminates per-read overhead
    uint8_t mag_data[7];
    for (uint8_t i = 0; i < 7; i++) {
        mag_data[i] = spiReadRegMsk(&gyro->dev, MPU_RA_EXT_SENS_DATA_00 + i);
    }

    // Check ST2 status register for overflow
    if (mag_data[6] & AK8963_ST2_HOFL) {
        // Magnetic sensor overflow detected
        return false;
    }

    // Extract raw magnetometer data (LSB-first byte order!)
    int16_t mx_raw = (int16_t)((mag_data[1] << 8) | mag_data[0]);
    int16_t my_raw = (int16_t)((mag_data[3] << 8) | mag_data[2]);
    int16_t mz_raw = (int16_t)((mag_data[5] << 8) | mag_data[4]);

    // Apply ASA scale factors
    mx = (float)mx_raw * mag_scale_x;
    my = (float)my_raw * mag_scale_y;
    mz = (float)mz_raw * mag_scale_z;

    // Apply calibration (hard iron bias + soft iron scale)
    mx = (mx - mag_bias_x) * mag_scale_factor_x;
    my = (my - mag_bias_y) * mag_scale_factor_y;
    mz = (mz - mag_bias_z) * mag_scale_factor_z;

    return true;
}

bool MPU9250::read9DOF(float &gx, float &gy, float &gz,
                       float &ax, float &ay, float &az,
                       float &mx, float &my, float &mz)
{
    // Read gyro and accel
    if (!read6DOF(gx, gy, gz, ax, ay, az)) {
        return false;
    }

    // Read magnetometer
    if (!readMagnetometer(mx, my, mz)) {
        return false;
    }

    return true;
}

bool MPU9250::calibrateMagnetometer()
{
    if (!initialized || !mag_initialized) {
        return false;
    }

    const uint16_t sample_count = 1500;  // 1500 samples over 15 seconds
    const uint16_t sample_delay = 10;    // 10ms between samples (100 Hz)

    float mag_max_x = -1000.0f, mag_max_y = -1000.0f, mag_max_z = -1000.0f;
    float mag_min_x = 1000.0f, mag_min_y = 1000.0f, mag_min_z = 1000.0f;

    // Collect samples
    for (uint16_t i = 0; i < sample_count; i++) {
        float mx, my, mz;

        // Temporarily disable calibration to get raw scaled values
        float temp_bias_x = mag_bias_x;
        float temp_bias_y = mag_bias_y;
        float temp_bias_z = mag_bias_z;
        float temp_scale_x = mag_scale_factor_x;
        float temp_scale_y = mag_scale_factor_y;
        float temp_scale_z = mag_scale_factor_z;

        mag_bias_x = mag_bias_y = mag_bias_z = 0.0f;
        mag_scale_factor_x = mag_scale_factor_y = mag_scale_factor_z = 1.0f;

        if (readMagnetometer(mx, my, mz)) {
            // Track min/max for each axis
            if (mx > mag_max_x) mag_max_x = mx;
            if (mx < mag_min_x) mag_min_x = mx;
            if (my > mag_max_y) mag_max_y = my;
            if (my < mag_min_y) mag_min_y = my;
            if (mz > mag_max_z) mag_max_z = mz;
            if (mz < mag_min_z) mag_min_z = mz;
        }

        // Restore temporary calibration
        mag_bias_x = temp_bias_x;
        mag_bias_y = temp_bias_y;
        mag_bias_z = temp_bias_z;
        mag_scale_factor_x = temp_scale_x;
        mag_scale_factor_y = temp_scale_y;
        mag_scale_factor_z = temp_scale_z;

        delay(sample_delay);
    }

    // Calculate hard iron bias (center of min/max)
    mag_bias_x = (mag_max_x + mag_min_x) / 2.0f;
    mag_bias_y = (mag_max_y + mag_min_y) / 2.0f;
    mag_bias_z = (mag_max_z + mag_min_z) / 2.0f;

    // Calculate soft iron scale factors (normalize to average chord)
    float chord_x = (mag_max_x - mag_min_x) / 2.0f;
    float chord_y = (mag_max_y - mag_min_y) / 2.0f;
    float chord_z = (mag_max_z - mag_min_z) / 2.0f;
    float avg_chord = (chord_x + chord_y + chord_z) / 3.0f;

    mag_scale_factor_x = avg_chord / chord_x;
    mag_scale_factor_y = avg_chord / chord_y;
    mag_scale_factor_z = avg_chord / chord_z;

    return true;
}

void MPU9250::setMagCalibration(float bias_x, float bias_y, float bias_z,
                                float scale_x, float scale_y, float scale_z)
{
    mag_bias_x = bias_x;
    mag_bias_y = bias_y;
    mag_bias_z = bias_z;
    mag_scale_factor_x = scale_x;
    mag_scale_factor_y = scale_y;
    mag_scale_factor_z = scale_z;
}

void MPU9250::getMagCalibration(float &bias_x, float &bias_y, float &bias_z,
                                float &scale_x, float &scale_y, float &scale_z) const
{
    bias_x = mag_bias_x;
    bias_y = mag_bias_y;
    bias_z = mag_bias_z;
    scale_x = mag_scale_factor_x;
    scale_y = mag_scale_factor_y;
    scale_z = mag_scale_factor_z;
}
