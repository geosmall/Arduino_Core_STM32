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
     * @param gyro_dlpf Gyro DLPF setting (0-7):
     *   0 = 250 Hz bandwidth, 8 kHz internal sample rate
     *   1 = 176 Hz bandwidth, 1 kHz internal sample rate
     *   2 = 92 Hz bandwidth, 1 kHz internal sample rate
     *   3 = 41 Hz bandwidth, 1 kHz internal sample rate
     *   4 = 20 Hz bandwidth, 1 kHz internal sample rate
     *   5 = 10 Hz bandwidth, 1 kHz internal sample rate
     *   6 = 5 Hz bandwidth, 1 kHz internal sample rate
     *   7 = 3600 Hz bandwidth (bypass), 8 kHz internal sample rate
     * @param accel_dlpf Accel DLPF setting (0-7):
     *   0 = 218.1 Hz bandwidth, 1 kHz internal sample rate
     *   1 = 218.1 Hz bandwidth, 1 kHz internal sample rate
     *   2 = 99 Hz bandwidth, 1 kHz internal sample rate
     *   3 = 44.8 Hz bandwidth, 1 kHz internal sample rate
     *   4 = 21.2 Hz bandwidth, 1 kHz internal sample rate
     *   5 = 10.2 Hz bandwidth, 1 kHz internal sample rate
     *   6 = 5.05 Hz bandwidth, 1 kHz internal sample rate
     *   7 = 420 Hz bandwidth (bypass), 1 kHz internal sample rate
     *
     * Note: For 1 kHz sample rate, use DLPF 1-6 (not 0 or 7)
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
     * @param divider Sample rate divider (0-255)
     *   Effective sample rate = Internal_Sample_Rate / (1 + divider)
     *   For DLPF enabled (DLPF_CFG 1-6): Internal rate = 1 kHz
     *
     *   Examples:
     *   - divider=0: 1 kHz output (1000 Hz / (1+0))
     *   - divider=4: 200 Hz output (1000 Hz / (1+4))
     *   - divider=9: 100 Hz output (1000 Hz / (1+9))
     *
     *   Note: Data-ready interrupts are automatically enabled by the driver.
     *         Use pinMode() + attachInterrupt() in your sketch to handle them.
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
    ChipVariant chip_variant;
};

#endif // ICM206XX_H
