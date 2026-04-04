// Arduino_Core_STM32 IMU Library - Device Bus Abstraction
// Based on madflight MPU_Interface pattern
// https://github.com/qqqlab/madflight

#pragma once

#include <Arduino.h>

/**
 * @brief Abstract interface for IMU device communication (SPI or I2C)
 *
 * Provides a unified interface for reading/writing IMU registers over
 * different bus types. Derived classes implement SPI or I2C specifics.
 */
class DeviceBus {
public:
    virtual ~DeviceBus() {}

    /**
     * @brief Set bus communication frequency
     * @param freq Frequency in Hz (e.g., 1000000 for 1MHz SPI)
     */
    virtual void setFreq(uint32_t freq) = 0;

    /**
     * @brief Write multiple bytes to consecutive registers
     * @param reg Starting register address
     * @param data Pointer to data buffer
     * @param n Number of bytes to write
     * @return Sum of bytes written (implementation specific)
     */
    virtual uint32_t writeRegs(uint8_t reg, uint8_t *data, uint16_t n) = 0;

    /**
     * @brief Read multiple bytes from consecutive registers
     * @param reg Starting register address
     * @param data Pointer to buffer for received data
     * @param n Number of bytes to read
     */
    virtual void readRegs(uint8_t reg, uint8_t *data, uint16_t n) = 0;

    /**
     * @brief Check if this is an SPI bus
     * @return true if SPI, false if I2C
     */
    virtual bool isSPI() = 0;

    /**
     * @brief Write a single byte to a register (convenience wrapper)
     * @param reg Register address
     * @param data Byte to write
     * @return Result from writeRegs()
     */
    uint32_t writeReg(uint8_t reg, uint8_t data) {
        return writeRegs(reg, &data, 1);
    }

    /**
     * @brief Read a single byte from a register (convenience wrapper)
     * @param reg Register address
     * @return Byte read from register
     */
    uint8_t readReg(uint8_t reg) {
        uint8_t data = 0;
        readRegs(reg, &data, 1);
        return data;
    }
};
