#pragma once
#include "config/ConfigTypes.h"

// NUCLEO F411RE HIL-006 Test Rig Configuration
// Hardware: STM32F411RE Nucleo with SBUS receiver
// Purpose: SBUS protocol testing (FrSky, Futaba, FlySky in SBUS mode)
//
// IMPORTANT: SBUS uses inverted serial signal!
//   - STM32F4 can do hardware RX inversion (RXINV bit in USART_CR2)
//   - OR use external inverter circuit
//
namespace BoardConfig {
  // Storage: SPI Flash for LittleFS (same as LITTLEFS target)
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PC12, PC11, PC10, PD2, 1000000};

  // IMU: SPI1 pins for sensor breakout boards
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 1000000};
  static constexpr IMUConfig imu{imu_spi, PC4};

  // GPS: UART communication
  static constexpr UARTConfig gps{PA9, PA10, 115200};

  // RC Receiver: SBUS on USART1 (RX=PA10, TX=PA9)
  // SBUS: 100000 baud, 8E2 (8 data bits, even parity, 2 stop bits), inverted
  static constexpr RCReceiverConfig rc_receiver{PA10, PA9, 100000, 1000, 300};

  // I2C: Available for additional sensors
  static constexpr I2CConfig sensors{PB9, PB8, 400000};

  // Servo/Motor/ESC: Same as LITTLEFS target
  namespace Servo {
    static inline TIM_TypeDef* const timer = TIM3;
    static constexpr uint32_t frequency_hz = 50;

    struct ServoConfig {
      TIM_TypeDef* timer;
      uint32_t pin;
      uint32_t channel;
      uint32_t min_us;
      uint32_t max_us;
    };

    struct Channel {
      uint32_t pin;
      uint32_t ch;
      uint32_t min_us;
      uint32_t max_us;
    };
    static constexpr Channel servo1 = {PB4, 1, 1000, 2000};

    static constexpr ServoConfig servos[] = {
      {TIM3, PB4, 1, 1000, 2000},
      {TIM3, PB0_ALT1, 3, 1000, 2000},
    };
    static constexpr int num_servos = sizeof(servos) / sizeof(servos[0]);
  };

  namespace Motor {
    static constexpr uint32_t frequency_hz = 2000;

    struct MotorConfig {
      TIM_TypeDef* timer;
      uint32_t pin;
      uint32_t channel;
      uint32_t min_us;
      uint32_t max_us;
    };

    static constexpr MotorConfig motors[] = {
      {TIM1, PA8, 1, 125, 250},
      {TIM1, PA9, 2, 125, 250},
      {TIM1, PA10, 3, 125, 250},
    };
    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };

  namespace ESC {
    static inline TIM_TypeDef* const timer = TIM4;
    static constexpr uint32_t frequency_hz = 2000;

    struct Channel {
      uint32_t pin;
      uint32_t ch;
      uint32_t min_us;
      uint32_t max_us;
    };
    static constexpr Channel esc1 = {PB6, 1, 125, 250};
    static constexpr Channel esc2 = {PB7, 2, 125, 250};
  };
}
