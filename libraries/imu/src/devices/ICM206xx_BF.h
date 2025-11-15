/*
 * ICM206xx_BF.h - ICM-206xx IMU Driver (Betaflight-based, Madflight pattern)
 *
 * Supports: ICM-20601, ICM-20602, ICM-20608G, ICM-20689
 *
 * Adapted from Betaflight: src/main/drivers/accgyro/accgyro_spi_icm20689.c
 * Original: https://github.com/betaflight/betaflight
 * License: GPLv3
 *
 * Pattern: Madflight device driver architecture
 * - Protected constructor (initialization in constructor)
 * - Static factory detect() method
 * - DeviceBase inheritance for polymorphic dispatch
 *
 * Hardware:
 * - Max SPI: 8 MHz
 * - Gyro range: ±2000 dps (16.4 LSB/dps)
 * - Accel range: ±16g (2048 LSB/g)
 * - Sampling: 1 kHz (8 kHz / (SMPLRT_DIV + 1))
 */

#pragma once

#include "../bus/DeviceBus.h"
#include "DeviceBase.h"
#include <stdint.h>

class ICM206xx_BF : public DeviceBase {
protected:
    // Protected constructor - initialization happens here
    ICM206xx_BF(DeviceBus* bus, uint8_t whoAmI);

public:
    // Factory pattern - returns nullptr if not detected
    static ICM206xx_BF* detect(DeviceBus* bus);

    // DeviceBase interface implementation
    void read(int16_t* accgyr) override;
    const char* typeName() const override;

    // Public member variables for configuration readout
    uint8_t whoAmI_;        // Detected WHO_AM_I value
    float accScale_;        // Accelerometer scale factor (G/LSB)
    float gyrScale_;        // Gyroscope scale factor (dps/LSB)
    uint16_t samplingRateHz_; // Sampling rate (Hz)

private:
    DeviceBus* bus_;
};
