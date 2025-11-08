/*
 * Auto-generated BoardConfig from Betaflight unified target
 * Generated: 2025-11-07 16:32:23
 * Generator: betaflight_target_converter.py
 */

#pragma once

// Include ConfigTypes.h from targets/config directory
#include "../../../../targets/config/ConfigTypes.h"

// Board: REVO
// Manufacturer: OPEN
// MCU: STM32F405
// Gyro: MPU6500, MPU6000
namespace BoardConfig {
  // Storage: W25Q128FV SPI flash on SPI3
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PC12, PC11, PC10, PB3, 8000000};

  // IMU: MPU6500, MPU6000 on SPI1
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 8000000};
  static constexpr IMUConfig imu{imu_spi, PC4, 1000000};

  // I2C1: Environmental sensors
  static constexpr I2CConfig sensors{PB8, PB9, 400000};

  // USART1: Serial port
  static constexpr UARTConfig uart1{PA9, PA10, 115200};

  // USART3: Serial port
  static constexpr UARTConfig uart3{PB10, PB11, 115200};

  // UART4: Serial port
  static constexpr UARTConfig uart4{PA0, PA1, 115200};

  // USART6: Serial port
  static constexpr UARTConfig uart6{PC6, PC7, 115200};

  // ADC: Battery voltage and current monitoring
  static constexpr ADCConfig battery{PC2, PC1, 110, 170};

  // Status LEDs
  static constexpr LEDConfig status_leds{PB5, PB4};

  // Motors: ONESHOT125 protocol
  namespace Motor {
    static constexpr uint32_t frequency_hz = 1000;

    // TIM2 Bank: Motors 3, 4
    namespace TIM2_Bank {
      static inline TIM_TypeDef* const timer = TIM2;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor3 = {PA3, 4, 125, 250};  // TIM2_CH4
      static constexpr Channel motor4 = {PA2, 3, 125, 250};  // TIM2_CH3
    };

    // TIM3 Bank: Motors 1, 2
    namespace TIM3_Bank {
      static inline TIM_TypeDef* const timer = TIM3;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor1 = {PB0_ALT1, 3, 125, 250};  // TIM3_CH3
      static constexpr Channel motor2 = {PB1_ALT1, 4, 125, 250};  // TIM3_CH4
    };

    // TIM5 Bank: Motors 5, 6
    namespace TIM5_Bank {
      static inline TIM_TypeDef* const timer = TIM5;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor5 = {PA1_ALT1, 2, 125, 250};  // TIM5_CH2
      static constexpr Channel motor6 = {PA0_ALT1, 1, 125, 250};  // TIM5_CH1
    };

  };
}