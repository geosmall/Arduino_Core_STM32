#pragma once

#include <Arduino.h>
#include <SPI.h>

#ifdef __cplusplus
extern "C" {
#endif

// SPI SD card initialization function
// Returns true if successful, false otherwise
bool sd_spi_initialize(uint8_t cs_pin, SPIClass *spi_port);

// Get current SPI settings
uint8_t sd_spi_get_cs_pin(void);
SPIClass* sd_spi_get_port(void);

// SPI speed configuration (call before initialization)
// speed_hz: SPI frequency in Hz (e.g., 1000000 for 1MHz)
void sd_spi_set_speed(uint32_t speed_hz);
uint32_t sd_spi_get_speed(void);

// Runtime sector size detection
// Returns actual sector size in bytes (typically 512, but configurable)
uint16_t sd_spi_get_sector_size(void);
bool sd_spi_set_sector_size(uint16_t size);

#ifdef __cplusplus
}
#endif