/*
 * Auto-generated BoardConfig from Betaflight target config
 * Generated: 2026-07-01 04:44:04
 * Generator: betaflight_target_converter.py
 */

#pragma once

// Include ConfigTypes.h from targets/config directory
#include "config/ConfigTypes.h"

// UF2 bootloader config flash region (must match bootuf2 ports/stm32f4/boards.h)
#define BOARD_FLASH_CONFIG_START  0x08008000UL
#define BOARD_FLASH_CONFIG_SIZE   (16 * 1024)
#define BOARD_FLASH_CONFIG_ALIGN  8

// Application blob-storage flash region (xcos2c diagram blobs): the 16 KB
// between the config region above and the app slot (0x08010000, bootuf2
// ports/stm32f4/boards.h BOARD_FLASH_APP_START) — F405 sector 3, one erase
// unit. The bootloader neither reads nor erases it; a bootloader install's
// mass-erase clears it. Application-owned: record format and per-family
// erase/program live with the application, as for the config region.
#define BOARD_FLASH_BLOB_START  0x0800C000UL
#define BOARD_FLASH_BLOB_SIZE   (16 * 1024)
#define BOARD_FLASH_BLOB_ALIGN  8

// Board: BETAFPVF405
// Manufacturer: BEFH
// MCU: STM32F405
// Gyro: MPU6000, ICM42688P
namespace BoardConfig {
  // Storage: W25Q128FV SPI flash on SPI2
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB15, PB14, PB13, PB12, 8000000, SPI2};

  // IMU: MPU6000, ICM42688P on SPI1
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 8000000};
  static constexpr IMUConfig imu{imu_spi, PC4, 1000000, IMUAlignment::CW270_DEG};

  // I2C1: Environmental sensors
  static constexpr I2CConfig sensors{PB9, PB8, 400000, I2C1};

  // USART1: Serial port
  static constexpr UARTConfig uart1{PA9, PA10, 115200, USART1};

  // USART3: Serial port
  static constexpr UARTConfig uart3{PB10, PB11, 115200, USART3};

  // UART4: Serial port
  static constexpr UARTConfig uart4{PA0, PA1, 115200, UART4};

  // USART6: Serial port
  static constexpr UARTConfig uart6{PC6, PC7, 115200, USART6};

  // ADC: Battery voltage and current monitoring
  static constexpr ADCConfig battery{PC2, PC1, 110, 170};

  // Status LEDs
  static constexpr LEDConfig status_leds{PB5};

  // RC Receiver: USART3 (from SERIALRX_UART)
  static constexpr RCReceiverConfig rc_receiver{PB11, PB10, 115200, 1000, 300, USART3};

  // Servo outputs - none configured
  namespace Servo {
    static constexpr uint32_t frequency_hz = 50;

    static constexpr ServoConfig servos[] = {};
    static constexpr int num_servos = 0;
  };
  // Motors: DSHOT600 (CHx + CHxN auto-detected by DShot lib)
  // Quad uses motors 1-4 only; config.h MOTOR5_PIN/MOTOR6_PIN (PC8/PA8) dropped.
  namespace Motor {
    static constexpr uint32_t frequency_hz = 2000;
    static constexpr Protocol protocol = Protocol::DSHOT600;

    // Motor array - hardware timer assignments from Betaflight config
    static constexpr MotorConfig motors[] = {
      {TIM3, PB0, 3, 125, 250, DMA1, 7, 5},  // Motor 1: TIM3_CH3, DMA1 S7
      {TIM3, PB1, 4, 125, 250, DMA1, 2, 5},  // Motor 2: TIM3_CH4, DMA1 S2
      {TIM2, PA3, 4, 125, 250, DMA1, 6, 3},  // Motor 3: TIM2_CH4, DMA1 S6
      {TIM2, PA2, 3, 125, 250, DMA1, 1, 3},  // Motor 4: TIM2_CH3, DMA1 S1
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}