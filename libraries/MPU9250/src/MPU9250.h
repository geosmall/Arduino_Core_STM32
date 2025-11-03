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

#ifndef MPU9250_H
#define MPU9250_H

#include <Arduino.h>
#include <SPI.h>

/**
 * @brief Arduino wrapper class for MPU-9250/MPU-9255 9-axis IMU
 *
 * This library provides an Arduino-compatible interface to the InvenSense
 * MPU-9250/9255 IMU using production-validated Betaflight driver code.
 *
 * License: GPL v3 (Betaflight-derived)
 * WHO_AM_I: 0x71 (MPU-9250) or 0x73 (MPU-9255)
 */
class MPU9250 {
public:
    MPU9250();
    ~MPU9250();

    /**
     * @brief Initialize the MPU-9250
     * @param spi SPI bus instance (e.g., SPI, SPI_1)
     * @param cs_pin Chip select pin
     * @param freq SPI frequency in Hz (default: 1 MHz, max: 20 MHz)
     * @return true if initialization successful, false otherwise
     */
    bool begin(SPIClass &spi, uint8_t cs_pin, uint32_t freq = 1000000);

    /**
     * @brief Read WHO_AM_I register
     * @return WHO_AM_I value (0x71 for MPU-9250, 0x73 for MPU-9255)
     */
    uint8_t whoAmI();

    /**
     * @brief Read gyroscope data
     * @param gx Gyro X-axis (degrees/second)
     * @param gy Gyro Y-axis (degrees/second)
     * @param gz Gyro Z-axis (degrees/second)
     * @return true if read successful
     */
    bool readGyro(float &gx, float &gy, float &gz);

    /**
     * @brief Read accelerometer data
     * @param ax Accel X-axis (g)
     * @param ay Accel Y-axis (g)
     * @param az Accel Z-axis (g)
     * @return true if read successful
     */
    bool readAccel(float &ax, float &ay, float &az);

    /**
     * @brief Read both gyroscope and accelerometer data
     * @param gx Gyro X-axis (degrees/second)
     * @param gy Gyro Y-axis (degrees/second)
     * @param gz Gyro Z-axis (degrees/second)
     * @param ax Accel X-axis (g)
     * @param ay Accel Y-axis (g)
     * @param az Accel Z-axis (g)
     * @return true if read successful
     */
    bool read6DOF(float &gx, float &gy, float &gz,
                  float &ax, float &ay, float &az);

    /**
     * @brief Set Digital Low-Pass Filter (DLPF) configuration
     * @param gyro_dlpf Gyro DLPF setting (0-7):
     *   0 = 250 Hz, 1 = 184 Hz, 2 = 92 Hz, 3 = 41 Hz,
     *   4 = 20 Hz, 5 = 10 Hz, 6 = 5 Hz, 7 = 3600 Hz
     * @param accel_dlpf Accel DLPF setting (0-7):
     *   0 = 460 Hz, 1 = 184 Hz, 2 = 92 Hz, 3 = 41 Hz,
     *   4 = 20 Hz, 5 = 10 Hz, 6 = 5 Hz, 7 = 460 Hz
     */
    void setDLPF(uint8_t gyro_dlpf, uint8_t accel_dlpf);

    /**
     * @brief Set gyroscope full-scale range
     * @param fsr Full-scale range (250, 500, 1000, or 2000 dps)
     */
    void setGyroFSR(uint16_t fsr);

    /**
     * @brief Set accelerometer full-scale range
     * @param fsr Full-scale range (2, 4, 8, or 16 g)
     */
    void setAccelFSR(uint8_t fsr);

    /**
     * @brief Set sample rate divider
     *
     * Sample Rate = Internal_Sample_Rate / (1 + divider)
     *
     * When DLPF enabled (gyro_dlpf 0-6):
     *   Internal_Sample_Rate = 1kHz
     *   divider=0 → 1000 Hz
     *   divider=1 → 500 Hz
     *   divider=3 → 250 Hz
     *   divider=9 → 100 Hz
     *
     * When DLPF disabled (gyro_dlpf 7):
     *   Internal_Sample_Rate = 8kHz
     *   divider=0 → 8000 Hz
     *   divider=7 → 1000 Hz
     *
     * @param divider Sample rate divider (0-255)
     */
    void setSampleRateDivider(uint8_t divider);

    /**
     * @brief Check if device is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return initialized; }

private:
    void *gyro_dev;  // gyroDev_t pointer
    void *acc_dev;   // accDev_t pointer
    bool initialized;
    float gyro_scale;  // Current gyro scale factor (LSB to dps)
    float accel_scale; // Current accel scale factor (LSB to g)
};

#endif // MPU9250_H
