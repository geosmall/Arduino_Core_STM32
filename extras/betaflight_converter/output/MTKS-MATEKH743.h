/*
 * Auto-generated BoardConfig from Betaflight unified target
 * Generated: 2025-10-12 06:48:21
 * Generator: betaflight_target_converter.py
 */

#pragma once

// Include ConfigTypes.h from targets/config directory
#include "../../../../targets/config/ConfigTypes.h"

// Board: MATEKH743
// Manufacturer: MTKS
// MCU: STM32H743
// Gyro: MPU6500, MPU6000, ICM42688P, ICM42605
namespace BoardConfig {
  // IMU: MPU6500, MPU6000, ICM42688P, ICM42605 on SPI1
  static constexpr SPIConfig imu_spi{PD7, PA6, PA5, PC15, 8000000};
  static constexpr IMUConfig imu{imu_spi, PB2, 1000000};

  // I2C1: Airspeed sensor, external compass
  static constexpr I2CConfig airspeed{PB6, PB7, 400000};

  // I2C2: Barometer, compass
  static constexpr I2CConfig baro{PB10, PB11, 400000};

  // LPUART1: Serial port
  static constexpr UARTConfig uart1{PA9, PA10, 115200};

  // USART2: Serial port
  static constexpr UARTConfig uart2{PD5, PD6, 115200};

  // USART3: Serial port
  static constexpr UARTConfig uart3{PD8, PD9, 115200};

  // UART4: Serial port
  static constexpr UARTConfig uart4{PB9, PB8, 115200};

  // USART6: Serial port
  static constexpr UARTConfig uart6{PC6, PC7, 115200};

  // UART7: Serial port
  static constexpr UARTConfig uart7{PE8, PE7, 115200};

  // UART8: Serial port
  static constexpr UARTConfig uart8{PE1, PE0, 115200};

  // ADC: Battery voltage and current monitoring
  static constexpr ADCConfig battery{PC0, PC1, 110, 250};

  // Status LEDs
  static constexpr LEDConfig status_leds{PE3, PE4};

  // Servos: Standard PWM (50 Hz)
  namespace Servo {
    static constexpr uint32_t frequency_hz = 50;

    // TIM15 Bank: Servos 1, 2
    namespace TIM15_Bank {
      static inline TIM_TypeDef* const timer = TIM15;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel servo1 = {PE5, 1, 1000, 2000};  // TIM15_CH1
      static constexpr Channel servo2 = {PE6, 2, 1000, 2000};  // TIM15_CH2
    };

  };
  // Motors: ONESHOT125 protocol
  namespace Motor {
    static constexpr uint32_t frequency_hz = 1000;

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

    // TIM4 Bank: Motors 7, 8
    namespace TIM4_Bank {
      static inline TIM_TypeDef* const timer = TIM4;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor7 = {PD12, 1, 125, 250};  // TIM4_CH1
      static constexpr Channel motor8 = {PD13, 2, 125, 250};  // TIM4_CH2
    };

    // TIM5 Bank: Motors 3, 4, 5, 6
    namespace TIM5_Bank {
      static inline TIM_TypeDef* const timer = TIM5;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor3 = {PA0_ALT1, 1, 125, 250};  // TIM5_CH1
      static constexpr Channel motor4 = {PA1_ALT1, 2, 125, 250};  // TIM5_CH2
      static constexpr Channel motor5 = {PA2_ALT1, 3, 125, 250};  // TIM5_CH3
      static constexpr Channel motor6 = {PA3_ALT1, 4, 125, 250};  // TIM5_CH4
    };

  };
}