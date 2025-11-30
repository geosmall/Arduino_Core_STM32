/*
 * Auto-generated BoardConfig from Betaflight unified target
 * Generated: 2025-11-30 11:26:39
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
  static constexpr I2CConfig sensors{PB9, PB8, 400000};

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

  // RC Receiver: IBus/SBUS (adjust protocol based on actual wiring)
  static constexpr RCReceiverConfig rc_receiver{PA10, PA9, 115200, 1000, 300};

  // Servo outputs - 50 Hz PWM for standard servos
  namespace Servo {
    static constexpr uint32_t frequency_hz = 50;

    struct ServoConfig {
      TIM_TypeDef* timer;
      uint32_t pin;
      uint32_t channel;
      uint32_t min_us;
      uint32_t max_us;
    };

    static constexpr ServoConfig servos[] = {
      {TIM8, PC8_ALT1, 3, 1000, 2000},  // Servo 1: TIM8_CH3
      {TIM8, PC9_ALT1, 4, 1000, 2000},  // Servo 2: TIM8_CH4
    };

    static constexpr int num_servos = sizeof(servos) / sizeof(servos[0]);
  };
  // Motors: ONESHOT125 protocol (125-250 µs)
  namespace Motor {
    static constexpr uint32_t frequency_hz = 2000;

    struct MotorConfig {
      TIM_TypeDef* timer;
      uint32_t pin;
      uint32_t channel;
      uint32_t min_us;
      uint32_t max_us;
    };

    // Motor array - hardware timer assignments from Betaflight config
    static constexpr MotorConfig motors[] = {
      {TIM5, PA0_ALT1, 1, 125, 250},  // Motor 1: TIM5_CH1
      {TIM5, PA1_ALT1, 2, 125, 250},  // Motor 2: TIM5_CH2
      {TIM5, PA2_ALT1, 3, 125, 250},  // Motor 3: TIM5_CH3
      {TIM5, PA3_ALT1, 4, 125, 250},  // Motor 4: TIM5_CH4
      {TIM3, PB0_ALT1, 3, 125, 250},  // Motor 5: TIM3_CH3
      {TIM3, PB1_ALT1, 4, 125, 250},  // Motor 6: TIM3_CH4
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}