// Arduino_Core_STM32 IMU Library - ICM42688 Driver
// Based on madflight ICM426XX pattern
// https://github.com/qqqlab/madflight

#pragma once

#include "../bus/DeviceBus.h"

/**
 * @brief ICM42688/ICM42605/IIM42653 IMU driver
 *
 * Factory-pattern C++ class wrapper over Betaflight ICM426xx driver.
 * Constructor performs full initialization - no separate begin() needed.
 *
 * Usage:
 *   DeviceBusSPI bus(&SPI, CS_PIN);
 *   ICM42688_BF* imu = ICM42688_BF::detect(&bus);
 *   if (imu) {
 *       int16_t data[6];  // ax,ay,az,gx,gy,gz
 *       imu->read(data);
 *   }
 */
class ICM42688_BF {
protected:
    /**
     * @brief Protected constructor - use detect() factory method
     * @param bus Pointer to DeviceBus (SPI or I2C)
     * @param whoAmI WHO_AM_I register value (chip identification)
     */
    ICM42688_BF(DeviceBus* bus, uint8_t whoAmI);

    /**
     * @brief Select register bank (ICM426xx has 5 banks: 0-4)
     * @param bank Bank number (0-4)
     */
    void setUserBank(uint8_t bank);

    DeviceBus* bus_;  ///< Bus interface pointer

public:
    /**
     * @brief Factory method to detect and initialize IMU
     * @param bus Pointer to DeviceBus instance
     * @return Pointer to ICM42688_BF instance, or nullptr if not detected
     *
     * Attempts detection up to 20 times with delays.
     * On success, creates instance and performs full initialization.
     */
    static ICM42688_BF* detect(DeviceBus* bus);

    /**
     * @brief Read 6-axis gyro/accel data
     * @param accgyr Pointer to 6-element int16_t array [ax,ay,az,gx,gy,gz]
     *
     * Performs single 12-byte burst read from data registers.
     * Data is in sensor frame, little-endian format.
     */
    void read(int16_t* accgyr);

    /**
     * @brief Get human-readable chip name
     * @return Chip name string ("ICM42688P", "ICM42605", etc.)
     */
    const char* typeName() const;

    // Public members for easy access
    uint8_t whoAmI_;           ///< WHO_AM_I register value
    float accScale_;           ///< Accel scale factor [G/LSB]
    float gyrScale_;           ///< Gyro scale factor [dps/LSB]
    uint16_t samplingRateHz_;  ///< Output data rate [Hz]
};
