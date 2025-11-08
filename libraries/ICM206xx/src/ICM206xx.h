/*
 * ICM206xx Arduino Library
 *
 * Copyright (C) 2025 Arduino_Core_STM32 Contributors
 *
 * This file is part of the ICM206xx Arduino library, derived from Betaflight.
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

#ifndef ICM206XX_H
#define ICM206XX_H

#include <Arduino.h>
#include <SPI.h>

/**
 * @brief Chip variants in ICM-206xx family
 */
enum class ChipVariant {
    UNKNOWN = 0,
    ICM20601 = 0xAC,
    ICM20602 = 0x12,
    ICM20608 = 0xAF,
    ICM20689 = 0x98
};

/**
 * @brief Arduino wrapper class for ICM-206xx family IMU
 *
 * This library provides an Arduino-compatible interface to the InvenSense
 * ICM-206xx family using production-validated Betaflight driver code.
 *
 * Supported chips:
 * - ICM-20601 (WHO_AM_I: 0xAC)
 * - ICM-20602 (WHO_AM_I: 0x12) ⭐ Primary target
 * - ICM-20608 (WHO_AM_I: 0xAF)
 * - ICM-20689 (WHO_AM_I: 0x98)
 *
 * License: GPL v3 (Betaflight-derived)
 */
class ICM206xx {
public:
    ICM206xx();
    ~ICM206xx();

    /**
     * @brief Initialize the ICM-206xx IMU
     * @param spi SPI bus instance (e.g., SPI, SPI_1)
     * @param cs_pin Chip select pin
     * @param freq SPI frequency in Hz (default: 1 MHz, max: 8 MHz)
     * @return true if initialization successful, false otherwise
     */
    bool begin(SPIClass &spi, uint8_t cs_pin, uint32_t freq = 1000000);

    /**
     * @brief Read WHO_AM_I register
     * @return WHO_AM_I value (0x12, 0xAC, 0xAF, or 0x98)
     */
    uint8_t whoAmI();

    /**
     * @brief Get detected chip variant
     * @return ChipVariant enum
     */
    ChipVariant getChipVariant() const;

    /**
     * @brief Get chip name string
     * @return Chip name (e.g., "ICM-20602")
     */
    const char* getChipName() const;

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
     * @param dlpf_cfg DLPF setting (0-7):
     *   0 = 250 Hz bandwidth
     *   1 = 176 Hz
     *   2 = 92 Hz
     *   3 = 41 Hz
     *   4 = 20 Hz
     *   5 = 10 Hz
     *   6 = 5 Hz
     *   7 = 3600 Hz (bypass)
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
    ChipVariant chip_variant;
};

#endif // ICM206XX_H
