#pragma once
#include "config/ConfigTypes.h"

// NUCLEO F411RE + SPI Flash HIL Test Rig Configuration
// Hardware: STM32F411RE Nucleo with SPI flash breadboard
// Validated: W25Q128JV 16MB SPI flash chip
// Uses Arduino pin macros for compatibility with existing code
namespace BoardConfig {
  // Storage: SPI Flash for LittleFS (breadboard setup)
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PC12, PC11, PC10, PD2, 1000000};
  //                                 MOSI  MISO  SCLK  CS   Freq

  // IMU: Available SPI pins for sensors (if needed)
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 1000000};
  static constexpr IMUConfig imu{imu_spi, PC4};

  // GPS: UART communication (if needed)
  static constexpr UARTConfig gps{PA9, PA10, 115200};

  // RC Receiver: IBus on USART1 (RX=PA10, TX=PA9)
  static constexpr RCReceiverConfig rc_receiver{PA10, PA9, 115200, 1000, 300};

  // I2C: Available for additional sensors (if needed)
  static constexpr I2CConfig sensors{PB9, PB8, 400000};

  // Servo: TIM3 @ 50 Hz
  namespace Servo {
    static inline TIM_TypeDef* const timer = TIM3;
    static constexpr uint32_t frequency_hz = 50;

    struct Channel {
      uint32_t pin;
      uint32_t ch;
      uint32_t min_us;
      uint32_t max_us;
    };

    static constexpr Channel servo1 = {PB4, 1, 1000, 2000};  // TIM3_CH1 (D5)
  };

  // ESC: TIM4 @ 2 kHz (OneShot125 protocol, 125-250 µs)
  namespace ESC {
    static inline TIM_TypeDef* const timer = TIM4;
    static constexpr uint32_t frequency_hz = 2000;

    struct Channel {
      uint32_t pin;
      uint32_t ch;
      uint32_t min_us;
      uint32_t max_us;
    };

    static constexpr Channel esc1 = {PB6, 1, 125, 250};  // TIM4_CH1 (D10)
    static constexpr Channel esc2 = {PB7, 2, 125, 250};  // TIM4_CH2 (CN7-21)
  };
}