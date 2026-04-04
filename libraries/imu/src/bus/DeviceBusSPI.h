// Arduino_Core_STM32 IMU Library - SPI Bus Implementation
// Based on madflight MPU_InterfaceSPI pattern
// https://github.com/qqqlab/madflight

#pragma once

#include "DeviceBus.h"
#include <SPI.h>

/**
 * @brief SPI bus implementation for IMU communication
 *
 * Handles SPI transactions with proper CS control and timing.
 * Uses SPI_MODE3 (CPOL=1, CPHA=1) which is standard for most IMUs.
 */
class DeviceBusSPI : public DeviceBus {
public:
    /**
     * @brief Construct SPI bus interface
     * @param spi Pointer to SPIClass instance (e.g., &SPI)
     * @param cs Chip select pin number
     */
    DeviceBusSPI(SPIClass *spi, Pin cs) {
        _spi = spi;
        _cs_pin = cs;
        pinMode(_cs_pin, OUTPUT);
        digitalWrite(_cs_pin, HIGH);  // CS idle high
        setFreq(1000000);  // Default 1MHz
    }

    ~DeviceBusSPI() override {
        pinMode(_cs_pin, INPUT);
        _spi = nullptr;
    }

    void setFreq(uint32_t freq) override {
        _freq = freq;
    }

    uint32_t writeRegs(uint8_t reg, uint8_t *data, uint16_t n) override {
        _spi->beginTransaction(SPISettings(_freq, MSBFIRST, SPI_MODE3));
        digitalWrite(_cs_pin, LOW);
        _spi->transfer(reg & 0x7F);  // Write bit: MSB=0
        uint32_t sum = 0;
        for (uint16_t i = 0; i < n; i++) {
            sum += _spi->transfer(data[i]);
        }
        digitalWrite(_cs_pin, HIGH);
        _spi->endTransaction();
        return sum;
    }

    void readRegs(uint8_t reg, uint8_t *data, uint16_t n) override {
        _spi->beginTransaction(SPISettings(_freq, MSBFIRST, SPI_MODE3));
        digitalWrite(_cs_pin, LOW);
        _spi->transfer(reg | 0x80);  // Read bit: MSB=1
        for (uint16_t i = 0; i < n; i++) {
            data[i] = _spi->transfer(0x00);
        }
        digitalWrite(_cs_pin, HIGH);
        _spi->endTransaction();
    }

    bool isSPI() override {
        return true;
    }

private:
    SPIClass *_spi;
    uint32_t _freq;
    Pin _cs_pin;
};
