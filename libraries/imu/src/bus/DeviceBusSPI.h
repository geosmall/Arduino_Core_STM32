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
 *
 * Each register access is one buffer transfer (address and data together),
 * so a multi-byte read costs one spi_transfer() call, not one per byte.
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

    void setDataFreq(uint32_t freq) override {
        _data_freq = freq;
    }

    /**
     * @brief Cap every clock this bus uses (board wiring limit)
     * @param freq Maximum frequency in Hz; 0 = no cap
     */
    void setMaxFreq(uint32_t freq) {
        _max_freq = freq;
    }

    uint32_t writeRegs(uint8_t reg, uint8_t *data, uint16_t n) override {
        uint8_t buf[MAX_BURST + 1];
        uint32_t sum = 0;
        begin(_freq);
        if (n <= MAX_BURST) {
            buf[0] = reg & 0x7F;  // Write bit: MSB=0
            for (uint16_t i = 0; i < n; i++) {
                buf[1 + i] = data[i];
            }
            _spi->transfer(buf, n + 1);
            for (uint16_t i = 0; i < n; i++) {
                sum += buf[1 + i];
            }
        } else {
            _spi->transfer(reg & 0x7F);
            for (uint16_t i = 0; i < n; i++) {
                sum += _spi->transfer(data[i]);
            }
        }
        end();
        return sum;
    }

    void readRegs(uint8_t reg, uint8_t *data, uint16_t n) override {
        read(_freq, reg, data, n);
    }

    void readData(uint8_t reg, uint8_t *data, uint16_t n) override {
        read(_data_freq ? _data_freq : _freq, reg, data, n);
    }

    bool isSPI() override {
        return true;
    }

private:
    // Largest access sent as one buffer transfer; longer ones go per byte
    static constexpr uint16_t MAX_BURST = 31;

    void begin(uint32_t freq) {
        if (_max_freq && freq > _max_freq) {
            freq = _max_freq;
        }
        _spi->beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE3));
        digitalWrite(_cs_pin, LOW);
    }

    void end() {
        digitalWrite(_cs_pin, HIGH);
        _spi->endTransaction();
    }

    void read(uint32_t freq, uint8_t reg, uint8_t *data, uint16_t n) {
        uint8_t buf[MAX_BURST + 1];
        begin(freq);
        if (n <= MAX_BURST) {
            buf[0] = reg | 0x80;  // Read bit: MSB=1
            for (uint16_t i = 0; i < n; i++) {
                buf[1 + i] = 0x00;
            }
            _spi->transfer(buf, n + 1);
            for (uint16_t i = 0; i < n; i++) {
                data[i] = buf[1 + i];
            }
        } else {
            _spi->transfer(reg | 0x80);
            for (uint16_t i = 0; i < n; i++) {
                data[i] = _spi->transfer(0x00);
            }
        }
        end();
    }

    SPIClass *_spi;
    uint32_t _freq;
    uint32_t _data_freq = 0;
    uint32_t _max_freq = 0;
    Pin _cs_pin;
};
