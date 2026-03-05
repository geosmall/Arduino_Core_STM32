/*
 * Auto-generated BoardConfig from Betaflight target config
 * Generated: 2026-03-05 05:04:41
 * Generator: betaflight_target_converter.py
 */

#pragma once

// Include ConfigTypes.h from targets/config directory
#include "../../../targets/config/ConfigTypes.h"

// Board: BLACKPILL_F411CE
// Manufacturer: WACT
// MCU: STM32F411
// Gyro: MPU9250
namespace BoardConfig {
  // Storage: W25Q128FV SPI flash on SPI1
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PA7, PA6, PA5, PA4, 8000000};

  // IMU: MPU9250 on SPI2
  static constexpr SPIConfig imu_spi{PB15, PB14, PB13, PB12, 8000000};
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

    static constexpr ServoConfig servos[] = {
      {TIM2, PA2, 3, 1000, 2000},  // Servo 1: TIM2_CH3
      {TIM2, PA15, 1, 1000, 2000},  // Servo 2: TIM2_CH1
      {TIM2, PB3, 2, 1000, 2000},  // Servo 3: TIM2_CH2
    };

    static constexpr int num_servos = sizeof(servos) / sizeof(servos[0]);
  };
  // Motors: ONESHOT125 protocol (125-250 µs)
  namespace Motor {
    static constexpr uint32_t frequency_hz = 2000;

    // Motor array - hardware timer assignments from Betaflight config
    static constexpr MotorConfig motors[] = {
      {TIM3, PB4, 1, 125, 250, DMA1, 4, 5},  // Motor 1: TIM3_CH1, DMA1 S4
      {TIM3, PB5, 2, 125, 250, DMA1, 5, 5},  // Motor 2: TIM3_CH2, DMA1 S5
      {TIM3, PB0_ALT1, 3, 125, 250, DMA1, 7, 5},  // Motor 3: TIM3_CH3, DMA1 S7
      {TIM3, PB1_ALT1, 4, 125, 250, DMA1, 2, 5},  // Motor 4: TIM3_CH4, DMA1 S2
      {TIM4, PB6, 1, 125, 250, DMA1, 0, 2},  // Motor 5: TIM4_CH1, DMA1 S0
      {TIM4, PB7, 2, 125, 250, DMA1, 3, 2},  // Motor 6: TIM4_CH2, DMA1 S3
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}