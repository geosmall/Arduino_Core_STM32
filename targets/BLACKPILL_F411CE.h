/*
 * Auto-generated BoardConfig from Betaflight unified target
 * Generated: 2025-11-26 17:41:46
 * Generator: betaflight_target_converter.py
 */

#pragma once

// Include ConfigTypes.h from targets/config directory
#include "../../../../targets/config/ConfigTypes.h"

// Board: BLACKPILL_F411CE
// Manufacturer: WACT
// MCU: STM32F411
// Gyro: MPU9250
namespace BoardConfig {
  // Storage: W25Q128FV SPI flash on SPI1
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PA7, PA6, PA5, PA4, 8000000};

  // IMU: MPU9250 on SPI2
  // Manual override: 1 MHz for breadboard/jumper wire setup (generator default: 8 MHz)
  static constexpr SPIConfig imu_spi{PB15, PB14, PB13, PB12, 1000000};
  static constexpr IMUConfig imu{imu_spi, PB2, 1000000};

  // I2C1: Environmental sensors
  static constexpr I2CConfig sensors{PB9, PB8, 400000};

  // USART1: Serial port
  static constexpr UARTConfig uart1{PA9, PA10, 115200};

  // USART2: Serial port
  static constexpr UARTConfig uart2{PA2, PA3, 115200};

  // ADC: Battery voltage and current monitoring
  static constexpr ADCConfig battery{PA1, PA0, 110, 170};

  // Status LEDs
  static constexpr LEDConfig status_leds{PC13};

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
      {TIM2, PA2, 3, 1000, 2000},  // Servo 1: TIM2_CH3
      {TIM2, PA15, 1, 1000, 2000},  // Servo 2: TIM2_CH1
      {TIM2, PB3, 2, 1000, 2000},  // Servo 3: TIM2_CH2
    };

    static constexpr int num_servos = sizeof(servos) / sizeof(servos[0]);
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
      {TIM3, PB4, 1, 125, 250},  // Motor 1: TIM3_CH1
      {TIM3, PB5, 2, 125, 250},  // Motor 2: TIM3_CH2
      {TIM3, PB0_ALT1, 3, 125, 250},  // Motor 3: TIM3_CH3
      {TIM3, PB1_ALT1, 4, 125, 250},  // Motor 4: TIM3_CH4
      {TIM4, PB6, 1, 125, 250},  // Motor 5: TIM4_CH1
      {TIM4, PB7, 2, 125, 250},  // Motor 6: TIM4_CH2
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}