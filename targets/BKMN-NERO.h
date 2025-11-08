/*
 * Auto-generated BoardConfig from Betaflight unified target
 * Generated: 2025-11-07 16:27:54
 * Generator: betaflight_target_converter.py
 */

#pragma once

// Include ConfigTypes.h from targets/config directory
#include "../../../../targets/config/ConfigTypes.h"

// Board: NERO
// Manufacturer: BKMN
// MCU: STM32F7X2
// Gyro: ICM20602
namespace BoardConfig {
  // Storage: SD card on SPI3
  static constexpr StorageConfig storage{StorageBackend::SDFS, PC12, PC11, PC10, PA15, 8000000};

  // IMU: ICM20602 on SPI1
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PC4, 8000000};
  static constexpr IMUConfig imu{imu_spi, PB2, 1000000};

  // I2C1: Environmental sensors
  static constexpr I2CConfig sensors{PB8, PB9, 400000};

  // USART1: Serial port
  static constexpr UARTConfig uart1{PA9, PA10, 115200};

  // USART3: Serial port
  static constexpr UARTConfig uart3{PB10, PB11, 115200};

  // USART6: Serial port
  static constexpr UARTConfig uart6{PC6, PC7, 115200};

  // ADC: Battery voltage and current monitoring
  static constexpr ADCConfig battery{PC3, PC2, 110, 170};

  // Status LEDs
  static constexpr LEDConfig status_leds{PB6, PB5};

  // Motors: ONESHOT125 protocol
  namespace Motor {
    static constexpr uint32_t frequency_hz = 1000;

    // TIM3 Bank: Motors 6
    namespace TIM3_Bank {
      static inline TIM_TypeDef* const timer = TIM3;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor6 = {PB1_ALT1, 4, 125, 250};  // TIM3_CH4
    };

    // TIM5 Bank: Motors 1, 2, 3, 4
    namespace TIM5_Bank {
      static inline TIM_TypeDef* const timer = TIM5;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor1 = {PA0_ALT1, 1, 125, 250};  // TIM5_CH1
      static constexpr Channel motor2 = {PA1_ALT1, 2, 125, 250};  // TIM5_CH2
      static constexpr Channel motor3 = {PA2_ALT1, 3, 125, 250};  // TIM5_CH3
      static constexpr Channel motor4 = {PA3_ALT1, 4, 125, 250};  // TIM5_CH4
    };

    // TIM8 Bank: Motors 7, 8
    namespace TIM8_Bank {
      static inline TIM_TypeDef* const timer = TIM8;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor7 = {PC8_ALT1, 3, 125, 250};  // TIM8_CH3
      static constexpr Channel motor8 = {PC9_ALT1, 4, 125, 250};  // TIM8_CH4
    };

  };
}