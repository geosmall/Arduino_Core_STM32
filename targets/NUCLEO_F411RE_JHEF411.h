/*
 * Auto-generated BoardConfig from Betaflight unified target
 * Generated: 2025-10-12 06:47:32
 * Generator: betaflight_target_converter.py
 *
 * Modified for NUCLEO_F411RE HIL testing:
 * - SPI frequencies reduced to 1 MHz (jumper wire compatible)
 */

#pragma once

// Include ConfigTypes.h from targets/config directory
#include "config/ConfigTypes.h"

// Board: JHEF411
// Manufacturer: JHEF
// MCU: STM32F411
// Gyro: MPU6000, ICM42688P
namespace BoardConfig {
  // Storage: W25Q128FV SPI flash on SPI2
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB15, PB14, PB13, PB2, 1000000};

  // IMU: MPU6000, ICM42688P on SPI1
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 1000000};
  static constexpr IMUConfig imu{imu_spi, PB3, 1000000};

  // I2C1: Environmental sensors
  static constexpr I2CConfig sensors{PB8, PB9, 400000};

  // USART1: Serial port
  static constexpr UARTConfig uart1{PB6, PB7, 115200};

  // USART2: Serial port
  static constexpr UARTConfig uart2{PA2, PA3, 115200};

  // RC Receiver: IBus on USART2 (RX=PA3, TX=PA2)
  static constexpr RCReceiverConfig rc_receiver{PA3, PA2, 115200, 1000, 300};

  // ADC: Battery voltage and current monitoring
  static constexpr ADCConfig battery{PA0, PA1, 110, 170};

  // Status LEDs
  static constexpr LEDConfig status_leds{PC13};

  // Motors: DSHOT300 protocol
  namespace Motor {
    static constexpr uint32_t frequency_hz = 1000;

    // TIM1 Bank: Motors 1, 2, 3
    namespace TIM1_Bank {
      static inline TIM_TypeDef* const timer = TIM1;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor1 = {PA8, 1, 0, 0};  // TIM1_CH1
      static constexpr Channel motor2 = {PA9, 2, 0, 0};  // TIM1_CH2
      static constexpr Channel motor3 = {PA10, 3, 0, 0};  // TIM1_CH3
    };

    // TIM3 Bank: Motors 4, 5
    namespace TIM3_Bank {
      static inline TIM_TypeDef* const timer = TIM3;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor4 = {PB0_ALT1, 3, 0, 0};  // TIM3_CH3
      static constexpr Channel motor5 = {PB4, 1, 0, 0};  // TIM3_CH1
    };

  };
}