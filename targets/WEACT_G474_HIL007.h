#pragma once
#include "config/ConfigTypes.h"

// UF2 bootloader config flash region (must match bootuf2 ports/stm32g4/boards.h)
#define BOARD_FLASH_CONFIG_START  0x08008000UL
#define BOARD_FLASH_CONFIG_SIZE   (16 * 1024)
#define BOARD_FLASH_CONFIG_ALIGN  8

// WeAct STM32G474CEU6 HIL-007 Test Rig Configuration
// Hardware: WeAct G474 Core Board with breadboarded peripherals
// Purpose: BetaFPV G473 test surrogate (same G4xx HAL, same pin assignments)
//
// Peripheral wiring matches BetaFPV G473 layout:
//   SPI1 IMU: PA7/PA6/PA5, CS=PA4, INT=PC4 (ICM42688P breakout)
//   SPI3 Flash: PB5/PB4/PB3, CS=PB9 (W25Q module)
//   I2C1: PB7 SDA / PA15 SCL
//   USART1: PA9 TX / PA10 RX (ELRS / loopback)
//
namespace BoardConfig {
  // Storage: W25Q SPI flash on SPI3 (ALT1 pins)
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB5, PB4, PB3, PB9, 8000000};

  // IMU: ICM42688P on SPI1
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 2000000};
  static constexpr IMUConfig imu{imu_spi, PC4, 1000000};

  // I2C1: Sensors
  static constexpr I2CConfig sensors{PB7, PA15, 400000};

  // USART1: RC receiver / loopback
  static constexpr UARTConfig uart1{PA9, PA10, 115200};

  // RC Receiver: CRSF (ELRS) on USART1 — ER6 signal lead wired to PA10 (RX), 5V/GND from board.
  static constexpr RCReceiverConfig rc_receiver{PA10, PA9, 420000, 1000, 300};

  // ADC: Battery voltage and current monitoring
  static constexpr ADCConfig battery{PA0, PA1, 110, 750};

  // Status LED (WeAct onboard LED on PC13)
  static constexpr LEDConfig status_leds{PC13};

  // Servo outputs - none configured
  namespace Servo {
    static constexpr uint32_t frequency_hz = 50;

    static constexpr ServoConfig servos[] = {};
    static constexpr int num_servos = 0;
  };

  // Motors: match BetaFPV G473 timer assignments
  namespace Motor {
    static constexpr uint32_t frequency_hz = 2000;
    static constexpr Protocol protocol = Protocol::DSHOT600;

    static constexpr MotorConfig motors[] = {
      {TIM1, PB0, 2, 125, 250},  // Motor 1: TIM1_CH2
      {TIM1, PB1, 3, 125, 250},  // Motor 2: TIM1_CH3
      {TIM16, PB6, 1, 125, 250},  // Motor 3: TIM16_CH1
      {TIM8, PC13, 4, 125, 250},  // Motor 4: TIM8_CH4
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}
