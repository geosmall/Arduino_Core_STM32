/*
 * Auto-generated BoardConfig from Betaflight target config
 * Generated: 2026-05-26 05:20:01
 * Generator: betaflight_target_converter.py
 */

#pragma once

// Include ConfigTypes.h from targets/config directory
#include "config/ConfigTypes.h"

// Board: BETAFPVG473
// Manufacturer: BEFH
// MCU: STM32G47X
// Gyro: ICM42688P
namespace BoardConfig {
  // Storage: W25Q128FV SPI flash on SPI3
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB5, PB4, PB3, PB9, 8000000, SPI3};

  // IMU: ICM42688P on SPI1
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 8000000, SPI1};
  static constexpr IMUConfig imu{imu_spi, PC4, 1000000, IMUAlignment::CW180_DEG};

  // I2C1: Environmental sensors
  static constexpr I2CConfig sensors{PB7, PA15, 400000, I2C1};

  // USART1: Serial port
  static constexpr UARTConfig uart1{PA9, PA10, 115200, USART1};

  // LPUART1: Serial port
  static constexpr UARTConfig uart2{PA2, PA3, 115200, LPUART1};

  // USART3: Serial port
  static constexpr UARTConfig uart3{PB10, PB11, 115200, USART3};

  // UART4: Serial port
  static constexpr UARTConfig uart4{PC10, PC11, 115200, UART4};

  // ADC: Battery voltage and current monitoring
  static constexpr ADCConfig battery{PA0, PA1, 110, 750};

  // Status LEDs
  static constexpr LEDConfig status_leds{PC15, PC14};

  // RC Receiver: USART3 (from SERIALRX_UART)
  static constexpr RCReceiverConfig rc_receiver{PB11, PB10, 115200, 1000, 300, USART3};

  // Servo outputs - none configured
  namespace Servo {
    static constexpr uint32_t frequency_hz = 50;

    static constexpr ServoConfig servos[] = {};
    static constexpr int num_servos = 0;
  };
  // Motors: ONESHOT125 protocol (125-250 µs)
  namespace Motor {
    static constexpr uint32_t frequency_hz = 2000;

    // Motor array - hardware timer assignments from Betaflight config
    static constexpr MotorConfig motors[] = {
      {TIM1, PB0, 2, 125, 250, DMA1, 2, 0},  // Motor 1: TIM1_CH2, DMA1 S2
      {TIM1, PB1, 3, 125, 250, DMA1, 3, 0},  // Motor 2: TIM1_CH3, DMA1 S3
      {TIM16, PB6_ALT2, 1, 125, 250, DMA1, 4, 0},  // Motor 3: TIM16_CH1, DMA1 S4
      {TIM8, PC13_ALT1, 4, 125, 250, DMA1, 5, 0},  // Motor 4: TIM8_CH4, DMA1 S5
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}