/*
 * BoardConfig for OpenPilot Revolution F405
 * Based on Betaflight unified target, hardware-validated 2026-02-01
 *
 * RC Input (3-pin SBUS header):
 *   - Signal: PA10 (USART1 RX)
 *   - Inverter: PC0 (GPIO output, HIGH=SBUS inverted, LOW=iBus non-inverted)
 *   - Validated: ~27k transitions/sec with SBUS @ 100kbaud
 */

#pragma once

// Include ConfigTypes.h from targets/config directory
#include "config/ConfigTypes.h"

// UF2 bootloader config flash region (must match bootuf2 ports/stm32f4/boards.h)
#define BOARD_FLASH_CONFIG_START  0x08008000UL
#define BOARD_FLASH_CONFIG_SIZE   (16 * 1024)
#define BOARD_FLASH_CONFIG_ALIGN  4

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
  static constexpr I2CConfig sensors{PB9, PB8, 400000};

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

  // RC Receiver: Hardware-validated configuration
  // 3-pin SBUS header connected to USART1 with hardware inverter on PC0
  static constexpr RCReceiverConfig rc_receiver{PA10, PA9, 100000, 1000, 300};  // SBUS: 100k baud
  static constexpr Pin rc_inverter_pin = PC0;  // HIGH=SBUS (inverted), LOW=iBus (non-inverted)

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
      {TIM3, PB0, 3, 125, 250},  // Motor 1: TIM3_CH3
      {TIM3, PB1, 4, 125, 250},  // Motor 2: TIM3_CH4
      {TIM2, PA3, 4, 125, 250},  // Motor 3: TIM2_CH4
      {TIM2, PA2, 3, 125, 250},  // Motor 4: TIM2_CH3
      {TIM5, PA1, 2, 125, 250},  // Motor 5: TIM5_CH2
      {TIM5, PA0, 1, 125, 250},  // Motor 6: TIM5_CH1
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}