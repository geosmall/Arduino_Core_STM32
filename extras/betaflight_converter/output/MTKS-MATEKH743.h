/*
 * Auto-generated BoardConfig from Betaflight target config
 * Generated: 2026-03-05 05:04:41
 * Generator: betaflight_target_converter.py
 */

#pragma once

// Include ConfigTypes.h from targets/config directory
#include "../../../targets/config/ConfigTypes.h"

// Board: MATEKH743
// Manufacturer: MTKS
// MCU: STM32H743
// Gyro: MPU6000, MPU6500, ICM42605, ICM42688P
namespace BoardConfig {
  // IMU: MPU6000, MPU6500, ICM42605, ICM42688P on SPI1
  static constexpr SPIConfig imu_spi{PD7, PA6, PA5, PC15, 8000000};
  static constexpr IMUConfig imu{imu_spi, PB2, 1000000};

  // I2C1: Airspeed sensor, external compass
  static constexpr I2CConfig airspeed{PB7, PB6, 400000};

  // I2C2: Barometer, compass
  static constexpr I2CConfig baro{PB11, PB10, 400000};

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

  // Servo outputs - 50 Hz PWM for standard servos
  namespace Servo {
    static constexpr uint32_t frequency_hz = 50;

    static constexpr ServoConfig servos[] = {
      {TIM15, PE5, 1, 1000, 2000},  // Servo 1: TIM15_CH1
      {TIM15, PE6, 2, 1000, 2000},  // Servo 2: TIM15_CH2
    };

    static constexpr int num_servos = sizeof(servos) / sizeof(servos[0]);
  };
  // Motors: ONESHOT125 protocol (125-250 µs)
  namespace Motor {
    static constexpr uint32_t frequency_hz = 2000;

    // Motor array - hardware timer assignments from Betaflight config
    static constexpr MotorConfig motors[] = {
      {TIM3, PB0_ALT1, 3, 125, 250, DMA1, 0, 0},  // Motor 1: TIM3_CH3, DMA1 S0
      {TIM3, PB1_ALT1, 4, 125, 250, DMA1, 1, 0},  // Motor 2: TIM3_CH4, DMA1 S1
      {TIM5, PA0_ALT1, 1, 125, 250, DMA1, 2, 0},  // Motor 3: TIM5_CH1, DMA1 S2
      {TIM5, PA1_ALT1, 2, 125, 250, DMA1, 3, 0},  // Motor 4: TIM5_CH2, DMA1 S3
      {TIM5, PA2_ALT1, 3, 125, 250, DMA1, 4, 0},  // Motor 5: TIM5_CH3, DMA1 S4
      {TIM5, PA3_ALT1, 4, 125, 250, DMA1, 5, 0},  // Motor 6: TIM5_CH4, DMA1 S5
      {TIM4, PD12, 1, 125, 250, DMA1, 6, 0},  // Motor 7: TIM4_CH1, DMA1 S6
      {TIM4, PD13, 2, 125, 250, DMA1, 7, 0},  // Motor 8: TIM4_CH2, DMA1 S7
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}