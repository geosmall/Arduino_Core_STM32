#pragma once
#include "config/ConfigTypes.h"

// DevEBox H743 HIL-006 Test Rig Configuration
// Hardware: DevEBox STM32H743VIT6
//   - IMU: ICM42688P on SPI1 (PA7/PA6/PA5, CS=PA4)
//   - RC:  SBUS on USART1 (PA10/PA9), H7 hardware RXINV
//   - LED: PE3
//   - Motors/Servos: None wired (bench test only)
//
namespace BoardConfig {
  // IMU: ICM42688P on SPI1
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 8000000};
  static constexpr IMUConfig imu{imu_spi, 0, 1000000};  // No interrupt pin, 1 MHz setup

  // RC Receiver: SBUS on USART1 (RX=PA10, TX=PA9)
  // 100000 baud for SBUS protocol, H7 hardware RX inversion
  static constexpr RCReceiverConfig rc_receiver{PA10, PA9, 100000, 1000, 300};

  // I2C1: Available for sensors (PB7/PB6)
  static constexpr I2CConfig sensors{PB7, PB6, 400000};

  // Status LED: PE3 (DevEBox schematic)
  static constexpr LEDConfig status_leds{PE3};

  // Motors: Same pins as MATEK H743 (none wired — bench test only)
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
      {TIM3, PB0_ALT1, 3, 125, 250},  // Motor 1: TIM3_CH3
      {TIM3, PB1_ALT1, 4, 125, 250},  // Motor 2: TIM3_CH4
      {TIM5, PA0_ALT1, 1, 125, 250},  // Motor 3: TIM5_CH1
      {TIM5, PA1_ALT1, 2, 125, 250},  // Motor 4: TIM5_CH2
      {TIM5, PA2_ALT1, 3, 125, 250},  // Motor 5: TIM5_CH3
      {TIM5, PA3_ALT1, 4, 125, 250},  // Motor 6: TIM5_CH4
      {TIM4, PD12, 1, 125, 250},      // Motor 7: TIM4_CH1
      {TIM4, PD13, 2, 125, 250},      // Motor 8: TIM4_CH2
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };

  // Servos: Same pins as MATEK H743 (none wired — bench test only)
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
      {TIM15, PE5, 1, 1000, 2000},  // Servo 1: TIM15_CH1
      {TIM15, PE6, 2, 1000, 2000},  // Servo 2: TIM15_CH2
    };

    static constexpr int num_servos = sizeof(servos) / sizeof(servos[0]);
  };
}
