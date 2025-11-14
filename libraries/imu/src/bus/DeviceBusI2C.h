// Arduino_Core_STM32 IMU Library - I2C Bus Implementation
// Based on madflight MPU_InterfaceI2C pattern
// https://github.com/qqqlab/madflight

#pragma once

#include "DeviceBus.h"
#include <Wire.h>

/**
 * @brief I2C bus implementation for IMU communication
 *
 * Handles I2C transactions with proper repeated start for register reads.
 * Compatible with Arduino Wire library interface.
 */
class DeviceBusI2C : public DeviceBus {
public:
    /**
     * @brief Construct I2C bus interface
     * @param wire Pointer to TwoWire instance (e.g., &Wire)
     * @param addr 7-bit I2C device address
     */
    DeviceBusI2C(TwoWire *wire, uint8_t addr) {
        _wire = wire;
        _addr = addr;
        setFreq(100000);  // Default 100kHz
    }

    ~DeviceBusI2C() override {}

    void setFreq(uint32_t freq) override {
        _wire->setClock(freq);
    }

    uint32_t writeRegs(uint8_t reg, uint8_t *data, uint16_t n) override {
        _wire->beginTransmission(_addr);
        _wire->write(reg);
        for (uint16_t i = 0; i < n; i++) {
            _wire->write(data[i]);
        }
        _wire->endTransmission();
        return 0;
    }

    void readRegs(uint8_t reg, uint8_t *data, uint16_t n) override {
        _wire->beginTransmission(_addr);
        _wire->write(reg);
        _wire->endTransmission(false);  // Repeated start
        uint8_t received = _wire->requestFrom(_addr, n);
        if (received == n) {
            for (uint16_t i = 0; i < n; i++) {
                data[i] = _wire->read();
            }
        }
    }

    bool isSPI() override {
        return false;
    }

private:
    TwoWire *_wire;
    uint8_t _addr;
};
