/*
 * bf_bus.cpp - Arduino SPI implementation for Betaflight bus abstraction
 *
 * This is a clean-room implementation for Arduino compatibility.
 * NOT copied from Betaflight (avoid GPL contamination).
 *
 * License: MIT
 *
 * Purpose: Implement SPI bus operations for MPU drivers using Arduino SPI API.
 */

#include "bf_bus.h"

// SPI Mode 3 for MPU sensors (CPOL=1, CPHA=1)
// - Clock idle high
// - Data sampled on rising edge, shifted on falling edge
#define MPU_SPI_MODE SPI_MODE3

void spiWriteReg(const extDevice_t *dev, uint8_t reg, uint8_t data)
{
    dev->spi->beginTransaction(SPISettings(dev->freq, MSBFIRST, MPU_SPI_MODE));
    digitalWrite(dev->cs_pin, LOW);

    dev->spi->transfer(reg & 0x7F);  // Clear read bit (bit 7)
    dev->spi->transfer(data);

    digitalWrite(dev->cs_pin, HIGH);
    dev->spi->endTransaction();
}

uint8_t spiReadRegMsk(const extDevice_t *dev, uint8_t reg)
{
    uint8_t data;

    dev->spi->beginTransaction(SPISettings(dev->freq, MSBFIRST, MPU_SPI_MODE));
    digitalWrite(dev->cs_pin, LOW);

    dev->spi->transfer(reg | 0x80);  // Set read bit (bit 7)
    data = dev->spi->transfer(0xFF); // Clock out dummy byte, read response

    digitalWrite(dev->cs_pin, HIGH);
    dev->spi->endTransaction();

    return data;
}

void spiSetClkDivisor(const extDevice_t *dev, uint16_t divisor)
{
    // Arduino handles clock configuration via SPISettings in each transaction
    // This function is a no-op for Arduino compatibility with Betaflight API
    (void)dev;      // Suppress unused parameter warning
    (void)divisor;
}

bool spiBusTransferMultiple(const extDevice_t *dev, uint8_t *out, uint8_t *in, int length)
{
    if (!dev || !out || length <= 0) {
        return false;
    }

    dev->spi->beginTransaction(SPISettings(dev->freq, MSBFIRST, MPU_SPI_MODE));
    digitalWrite(dev->cs_pin, LOW);

    // Transfer all bytes (out is modified in-place with received data)
    for (int i = 0; i < length; i++) {
        out[i] = dev->spi->transfer(out[i]);
    }

    digitalWrite(dev->cs_pin, HIGH);
    dev->spi->endTransaction();

    // Copy to separate input buffer if provided
    if (in != nullptr) {
        memcpy(in, out, length);
    }

    return true;
}

bool spiReadRegBuf(const extDevice_t *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
    if (!dev || !data || length == 0) {
        return false;
    }

    dev->spi->beginTransaction(SPISettings(dev->freq, MSBFIRST, MPU_SPI_MODE));
    digitalWrite(dev->cs_pin, LOW);

    // Send register address with read bit set
    dev->spi->transfer(reg | 0x80);

    // Read data bytes
    for (uint8_t i = 0; i < length; i++) {
        data[i] = dev->spi->transfer(0xFF);
    }

    digitalWrite(dev->cs_pin, HIGH);
    dev->spi->endTransaction();

    return true;
}

bool spiWriteRegBuf(const extDevice_t *dev, uint8_t reg, const uint8_t *data, uint8_t length)
{
    if (!dev || !data || length == 0) {
        return false;
    }

    dev->spi->beginTransaction(SPISettings(dev->freq, MSBFIRST, MPU_SPI_MODE));
    digitalWrite(dev->cs_pin, LOW);

    // Send register address with write bit clear
    dev->spi->transfer(reg & 0x7F);

    // Write data bytes
    for (uint8_t i = 0; i < length; i++) {
        dev->spi->transfer(data[i]);
    }

    digitalWrite(dev->cs_pin, HIGH);
    dev->spi->endTransaction();

    return true;
}
