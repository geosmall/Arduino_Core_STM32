/*
 * Auto-generated BoardConfig from Betaflight target config
 * Generated: 2026-03-05 05:04:41
 * Generator: betaflight_target_converter.py
 */

#pragma once

// Include ConfigTypes.h from targets/config directory
#include "config/ConfigTypes.h"

// UF2 bootloader config flash region (must match bootuf2 ports/stm32f4/boards.h)
#define BOARD_FLASH_CONFIG_START  0x08008000UL
#define BOARD_FLASH_CONFIG_SIZE   (16 * 1024)
#define BOARD_FLASH_CONFIG_ALIGN  4

// Board: JHEF411
// Manufacturer: JHEF
// MCU: STM32F411
// Gyro: MPU6000, ICM42688P
namespace BoardConfig {
  // Storage: W25Q128FV SPI flash on SPI2
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB15, PB14, PB13, PB2, 8000000};

  // IMU: MPU6000, ICM42688P on SPI1
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 8000000};
  static constexpr IMUConfig imu{imu_spi, PB3, 1000000};

  // I2C1: Environmental sensors
  static constexpr I2CConfig sensors{PB9, PB8, 400000};

  // USART1: Serial port
  static constexpr UARTConfig uart1{PB6, PB7, 115200};

  // USART2: Serial port
  static constexpr UARTConfig uart2{PA2, PA3, 115200};

  // ADC: Battery voltage and current monitoring
  static constexpr ADCConfig battery{PA0, PA1, 110, 170};

  // Status LEDs
  static constexpr LEDConfig status_leds{PC13};

  // RC Receiver: IBus/SBUS (adjust protocol based on actual wiring)
  static constexpr RCReceiverConfig rc_receiver{PB7, PB6, 115200, 1000, 300};

  // Servo outputs - none configured
  namespace Servo {
    static constexpr uint32_t frequency_hz = 50;

    static constexpr ServoConfig servos[] = {};
    static constexpr int num_servos = 0;
  };
  // Motors: ONESHOT125 protocol (125-250 µs)
  namespace Motor {
    static constexpr uint32_t frequency_hz = 2000;
    static constexpr Protocol protocol = Protocol::ONESHOT125;

    // Motor array - hardware timer assignments from Betaflight config
    static constexpr MotorConfig motors[] = {
      {TIM1, PA8, 1, 125, 250, DMA2, 1, 6},  // Motor 1: TIM1_CH1, DMA2 S1
      {TIM1, PA9, 2, 125, 250, DMA2, 2, 6},  // Motor 2: TIM1_CH2, DMA2 S2
      {TIM1, PA10, 3, 125, 250, DMA2, 6, 6},  // Motor 3: TIM1_CH3, DMA2 S6
      {TIM3, PB0, 3, 125, 250, DMA1, 7, 5},  // Motor 4: TIM3_CH3, DMA1 S7
      {TIM3, PB4, 1, 125, 250, DMA1, 4, 5},  // Motor 5: TIM3_CH1, DMA1 S4
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}