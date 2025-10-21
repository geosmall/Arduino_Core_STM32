#pragma once

// Configuration type definitions for board-specific settings
// Uses Arduino pin macros for compatibility with existing code

// Include STM32 pinmap for NP definition (No Pin)
#include "stm32/pinmap.h"
// Include pins_arduino for PNUM_NOT_DEFINED
#include "pins_arduino.h"

// Storage backend types
enum class StorageBackend {
    NONE,      // No storage hardware attached
    LITTLEFS,  // SPI flash storage
    SDFS       // SD card storage
};


// SPI chip select control modes
enum class CS_Mode {
    SOFTWARE,  // Software-controlled CS pin
    HARDWARE   // Hardware-controlled CS via SPI peripheral
};

namespace BoardConfig {
  struct SPIConfig {
    constexpr SPIConfig(uint32_t mosi, uint32_t miso, uint32_t sclk, uint32_t cs,
                       uint32_t frequency_hz = 1000000,
                       CS_Mode mode = CS_Mode::SOFTWARE)
      : mosi_pin(mosi), miso_pin(miso), sclk_pin(sclk), cs_pin(cs),
        freq_hz(frequency_hz), cs_mode(mode) {}

    const uint32_t mosi_pin, miso_pin, sclk_pin, cs_pin;
    const uint32_t freq_hz;
    const CS_Mode cs_mode;

    // Helper: Get SSEL pin for SPIClass constructor
    // SW mode: returns PNUM_NOT_DEFINED (disables hardware SSEL)
    // HW mode: returns cs_pin (STM32 SPI peripheral controls CS)
    constexpr uint32_t get_ssel_pin() const {
      return (cs_mode == CS_Mode::HARDWARE) ? cs_pin : PNUM_NOT_DEFINED;
    }
  };

  struct UARTConfig {
    constexpr UARTConfig(uint32_t tx, uint32_t rx, uint32_t baud)
      : tx_pin(tx), rx_pin(rx), baud_rate(baud) {}

    const uint32_t tx_pin, rx_pin;
    const uint32_t baud_rate;
  };

  struct I2CConfig {
    constexpr I2CConfig(uint32_t sda, uint32_t scl, uint32_t frequency_hz = 100000)
      : sda_pin(sda), scl_pin(scl), freq_hz(frequency_hz) {}

    const uint32_t sda_pin, scl_pin;
    const uint32_t freq_hz;
  };

  struct StorageConfig {
    constexpr StorageConfig(StorageBackend backend, uint32_t mosi, uint32_t miso,
                           uint32_t sclk, uint32_t cs, uint32_t frequency_hz = 1000000)
      : backend_type(backend), mosi_pin(mosi), miso_pin(miso), sclk_pin(sclk),
        cs_pin(cs), freq_hz(frequency_hz) {}

    const StorageBackend backend_type;
    const uint32_t mosi_pin, miso_pin, sclk_pin, cs_pin;
    const uint32_t freq_hz;
  };

  struct IMUConfig {
    constexpr IMUConfig(const SPIConfig& spi_config, uint32_t interrupt_pin = 0,
                       uint32_t setup_freq_hz = 0)
      : spi(spi_config), int_pin(interrupt_pin), setup_freq_hz(setup_freq_hz) {}

    const SPIConfig spi;
    const uint32_t int_pin;        // 0 = no interrupt
    const uint32_t setup_freq_hz;  // 0 = use spi.freq_hz for setup (slow initialization)

    // Helper: Get effective setup frequency (slow initialization)
    constexpr uint32_t get_setup_freq() const {
      return (setup_freq_hz > 0) ? setup_freq_hz : spi.freq_hz;
    }

    // Helper: Get effective runtime frequency (normal operation)
    constexpr uint32_t get_runtime_freq() const {
      return spi.freq_hz;  // Always use the SPI config frequency for runtime
    }
  };

  struct ADCConfig {
    constexpr ADCConfig(uint32_t voltage_pin, uint32_t current_pin,
                       uint16_t voltage_scale, uint16_t current_scale)
      : voltage_pin(voltage_pin), current_pin(current_pin),
        voltage_scale(voltage_scale), current_scale(current_scale) {}

    const uint32_t voltage_pin;    // ADC pin for battery voltage
    const uint32_t current_pin;    // ADC pin for battery current
    const uint16_t voltage_scale;  // Betaflight vbat_scale (voltage divider ratio * 10)
    const uint16_t current_scale;  // Betaflight ibata_scale (current sensor sensitivity)
  };

  struct LEDConfig {
    constexpr LEDConfig(uint32_t pin1, uint32_t pin2 = 0)
      : led1_pin(pin1), led2_pin(pin2) {}

    const uint32_t led1_pin;  // Primary status LED
    const uint32_t led2_pin;  // Secondary status LED (0 = not present)
  };

  struct RCReceiverConfig {
    constexpr RCReceiverConfig(uint32_t rx, uint32_t tx, uint32_t baud,
                               uint32_t timeout_ms = 1000,
                               uint32_t idle_threshold_us = 300)
      : rx_pin(rx), tx_pin(tx), baud_rate(baud),
        timeout_ms(timeout_ms), idle_threshold_us(idle_threshold_us) {}

    const uint32_t rx_pin;              // UART RX pin (receiver output)
    const uint32_t tx_pin;              // UART TX pin (receiver input, usually unused)
    const uint32_t baud_rate;           // Protocol baudrate (115200=IBus, 100000=SBUS)
    const uint32_t timeout_ms;          // Failsafe timeout in milliseconds
    const uint32_t idle_threshold_us;   // Software idle detection threshold (0=disabled)
  };
}