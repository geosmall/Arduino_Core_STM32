/*
 * Auto-generated BoardConfig from Betaflight unified target
 * Generated: 2025-11-25 15:54:54
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

  // RC Receiver: IBus/SBUS (adjust protocol based on actual wiring)
  static constexpr RCReceiverConfig rc_receiver{PA10, PA9, 115200, 1000, 300};

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
    static constexpr uint32_t frequency_hz = 8000;

    struct MotorConfig {
      TIM_TypeDef* timer;
      uint32_t pin;
      uint32_t channel;
      uint32_t min_us;
      uint32_t max_us;
    };

    // Motor array - hardware timer assignments from Betaflight config
    static constexpr MotorConfig motors[] = {
      {TIM3, PB0_ALT1, 3, 125, 250},  // Motor 1: TIM3_CH3
      {TIM3, PB1_ALT1, 4, 125, 250},  // Motor 2: TIM3_CH4
      {TIM5, PA0_ALT1, 1, 125, 250},  // Motor 3: TIM5_CH1
      {TIM5, PA1_ALT1, 2, 125, 250},  // Motor 4: TIM5_CH2
      {TIM5, PA2_ALT1, 3, 125, 250},  // Motor 5: TIM5_CH3
      {TIM5, PA3_ALT1, 4, 125, 250},  // Motor 6: TIM5_CH4
      {TIM4, PD12, 1, 125, 250},  // Motor 7: TIM4_CH1
      {TIM4, PD13, 2, 125, 250},  // Motor 8: TIM4_CH2
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}