// Arduino_Core_STM32 IMU Library - Betaflight Driver Facade
// High-level API wrapping Betaflight-based device drivers
// Based on madflight ImuGizmo adapter pattern

#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "common/Types.h"
#include "bus/DeviceBus.h"
#include "bus/DeviceBusSPI.h"
#include "bus/DeviceBusI2C.h"
#include "devices/DeviceBase.h"
#include "devices/ICM42688_BF.h"
#include "devices/MPU6000_BF.h"

/**
 * @brief High-level IMU facade with auto-detection
 *
 * Provides Arduino-style API for InvenSense IMUs using Betaflight drivers.
 * Supports auto-detection and multiple device families.
 *
 * Usage:
 *   IMU_BF imu;
 *   imu.attachSPI(SPI, PA4);
 *   if (imu.begin()) {
 *     ImuSample sample;
 *     if (imu.read(sample)) {
 *       // Use sample.ax, sample.ay, ...
 *     }
 *   }
 */
class IMU_BF
{
public:
    /**
     * @brief Construct IMU facade
     */
    IMU_BF();

    /**
     * @brief Destructor - cleanup bus and device
     */
    ~IMU_BF();

    /**
     * @brief Attach SPI bus for communication
     * @param spi Reference to Arduino SPIClass instance
     * @param csPin Chip select pin number
     * @param freq_hz SPI frequency in Hz (default: 1 MHz)
     */
    void attachSPI(SPIClass& spi, uint8_t csPin, uint32_t freq_hz = 1000000);

    /**
     * @brief Attach I2C bus for communication
     * @param wire Reference to Arduino TwoWire instance
     * @param addr I2C device address (default: 0x68)
     */
    void attachI2C(TwoWire& wire, uint8_t addr = 0x68);

    /**
     * @brief Initialize IMU with optional device type selection
     * @param type Device type to initialize (Auto = auto-detect)
     * @return true on success, false on failure
     */
    bool begin(ImuType type = ImuType::Auto);

    /**
     * @brief Read IMU data into sample structure
     * @param sample Output structure for accelerometer and gyroscope data
     * @return true on success, false on failure
     */
    bool read(ImuSample& sample);

    /**
     * @brief Get detected IMU type
     * @return ImuType enum value
     */
    ImuType type() const { return detected_type_; }

    /**
     * @brief Get human-readable type name
     * @return C-string with device name
     */
    const char* typeName() const;

    /**
     * @brief Check if IMU is initialized
     * @return true if begin() was successful
     */
    bool isInitialized() const { return (device_ != nullptr); }

private:
    // Bus abstraction (owned)
    DeviceBus* bus_;

    // Device driver (polymorphic - only one instance at a time)
    DeviceBase* device_;

    // Detected device type
    ImuType detected_type_;

    // Auto-detection logic
    bool autoDetect();

    // Prevent copying
    IMU_BF(const IMU_BF&) = delete;
    IMU_BF& operator=(const IMU_BF&) = delete;
};
