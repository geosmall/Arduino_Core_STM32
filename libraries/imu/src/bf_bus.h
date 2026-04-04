/*
 * bf_bus.h - Arduino SPI abstraction for Betaflight drivers
 *
 * This is a clean-room implementation for Arduino compatibility.
 * NOT copied from Betaflight (avoid GPL contamination).
 *
 * License: MIT
 *
 * Purpose: Provide SPI bus abstraction layer that allows Betaflight-derived
 *          MPU drivers to work with Arduino SPI without GPL contamination.
 */

#ifndef BF_BUS_H
#define BF_BUS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
#include <Arduino.h>
#include <SPI.h>
extern "C" {
#else
// Forward declarations for C mode
typedef struct SPIClass SPIClass;
#endif

// External device structure - represents an SPI device
typedef struct extDevice_s {
    SPIClass *spi;          // Arduino SPI bus pointer
    uint8_t cs_pin;         // Chip select pin
    uint32_t freq;          // SPI frequency in Hz
} extDevice_t;

// SPI register write - single byte
void spiWriteReg(const extDevice_t *dev, uint8_t reg, uint8_t data);

// SPI register read with mask (reads single byte)
uint8_t spiReadRegMsk(const extDevice_t *dev, uint8_t reg);

// SPI set clock divisor (no-op in Arduino - handled by SPISettings)
void spiSetClkDivisor(const extDevice_t *dev, uint16_t divisor);

// SPI transfer multiple bytes
// out: data to send (will be modified with received data)
// in: received data buffer (if not NULL, copied from out after transfer)
// length: number of bytes to transfer
bool spiBusTransferMultiple(const extDevice_t *dev, uint8_t *out, uint8_t *in, int length);

// SPI read register burst (convenience wrapper)
bool spiReadRegBuf(const extDevice_t *dev, uint8_t reg, uint8_t *data, uint8_t length);

// SPI write register burst (convenience wrapper)
bool spiWriteRegBuf(const extDevice_t *dev, uint8_t reg, const uint8_t *data, uint8_t length);

#ifdef __cplusplus
}
#endif

#endif // BF_BUS_H
