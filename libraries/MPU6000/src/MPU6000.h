/*
 * MPU6000 Arduino Library
 *
 * Copyright (C) 2025 Arduino_Core_STM32 Contributors
 *
 * This file is part of the MPU6000 Arduino library, derived from Betaflight.
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

#ifndef MPU6000_H
#define MPU6000_H

#include <Arduino.h>
#include <SPI.h>

/**
 * @brief Arduino wrapper class for MPU-6000 6-axis IMU
 *
 * This library provides an Arduino-compatible interface to the InvenSense
 * MPU-6000 IMU using production-validated Betaflight driver code.
 *
 * License: GPL v3 (Betaflight-derived)
 * WHO_AM_I: 0x68
 */
class MPU6000 {
public:
    MPU6000();
    ~MPU6000();

    /**
     * @brief Initialize the MPU-6000
     * @param spi SPI bus instance (e.g., SPI, SPI_1)
     * @param cs_pin Chip select pin
     * @param freq SPI frequency in Hz (default: 1 MHz, max: 20 MHz)
     * @return true if initialization successful, false otherwise
     */
    bool begin(SPIClass &spi, uint8_t cs_pin, uint32_t freq = 1000000);

    /**
     * @brief Read WHO_AM_I register
     * @return WHO_AM_I value (0x68 for MPU-6000)
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
     * @param dlpf_cfg DLPF setting (0-6):
     *   0 = 256 Hz gyro, 260 Hz accel
     *   1 = 188 Hz gyro, 184 Hz accel
     *   2 = 98 Hz gyro, 94 Hz accel
     *   3 = 42 Hz gyro, 44 Hz accel
     *   4 = 20 Hz gyro, 21 Hz accel
     *   5 = 10 Hz gyro, 10 Hz accel
     *   6 = 5 Hz gyro, 5 Hz accel
     */
    void setDLPF(uint8_t dlpf_cfg);

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
     * When DLPF enabled (dlpf_cfg 0-6):
     *   Internal_Sample_Rate = 1kHz
     *   divider=0 → 1000 Hz
     *   divider=1 → 500 Hz
     *   divider=3 → 250 Hz
     *   divider=7 → 125 Hz
     *   divider=9 → 100 Hz
     *
     * When DLPF disabled (dlpf_cfg 7):
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

#endif // MPU6000_H
