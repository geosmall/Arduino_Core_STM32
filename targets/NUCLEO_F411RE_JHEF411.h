/*
 * Auto-generated BoardConfig from Betaflight unified target
 * Generated: 2025-11-27 08:46:10
 * Generator: betaflight_target_converter.py
 * Modified: NUCLEO_F411RE breadboard target - 1 MHz SPI for jumper wire connections
 */

#pragma once

// Include ConfigTypes.h from targets/config directory
#include "config/ConfigTypes.h"

// Board: JHEF411 (NOXE V3) - Breadboard configuration for NUCLEO_F411RE
// Manufacturer: JHEF
// MCU: STM32F411
// Gyro: MPU6000, ICM42688P
namespace BoardConfig {
  // Storage: W25Q128FV SPI flash on SPI2 (1 MHz for breadboard/jumper wires)
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB15, PB14, PB13, PB2, 1000000, SpiDev::Spi2};

  // IMU: MPU6000, ICM42688P on SPI1 (1 MHz for breadboard/jumper wires)
  // Chip alignment: CW0_DEG (bench breadboard rig — chip mounted flat
  // on the NUCLEO PCB. The production JHEF411 board mounts the IMU
  // CW180_DEG; see targets/JHEF-JHEF411.h.)
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 1000000, SpiDev::Spi1};
  static constexpr IMUConfig imu{imu_spi, PB3, 1000000, IMUAlignment::CW0_DEG};

  // I2C1: Environmental sensors
  static constexpr I2CConfig sensors{PB9, PB8, 400000, I2CDev::I2c1};

  // USART1: Serial port
  static constexpr UARTConfig uart1{PB6, PB7, 115200, UartDev::Usart1};

  // USART2: Serial port
  static constexpr UARTConfig uart2{PA2, PA3, 115200, UartDev::Usart2};

  // ADC: Battery voltage and current monitoring
  static constexpr ADCConfig battery{PA0, PA1, 110, 170};

  // Status LEDs
  static constexpr LEDConfig status_leds{PC13};

  // RC Receiver: IBus/SBUS (adjust protocol based on actual wiring)
  static constexpr RCReceiverConfig rc_receiver{PB7, PB6, 115200, 1000, 300, UartDev::Usart1};

  // GPS: USART2 (directly on Nucleo Arduino header D1/D0)
  static constexpr GPSConfig gps{PA2, PA3, 9600};

  // Servo outputs - none configured (flight controller config)
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
      {TIM1, PA8, 1, 125, 250},  // Motor 1: TIM1_CH1
      {TIM1, PA9, 2, 125, 250},  // Motor 2: TIM1_CH2
      {TIM1, PA10, 3, 125, 250},  // Motor 3: TIM1_CH3
      {TIM3, PB0, 3, 125, 250},  // Motor 4: TIM3_CH3
      {TIM3, PB4, 1, 125, 250},  // Motor 5: TIM3_CH1
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}
