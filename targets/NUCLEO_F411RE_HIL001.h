#pragma once
#include "config/ConfigTypes.h"

// NUCLEO F411RE HIL-001 Test Rig Configuration
// Hardware: STM32F411RE Nucleo with breadboard peripherals
// Validated: ICM-42688P IMU, W25Q128JV 16MB SPI flash
//
// Timer Allocation:
//   TIM1 @ 2 kHz  - Motor outputs (PA8, PA9, PA10)
//   TIM3 @ 50 Hz  - Servo outputs (PB4, PB0)
//   TIM4 @ 2 kHz  - ESC outputs (PB6, PB7)
//   TIM2          - Available for input capture (PA0, PB10)
//
namespace BoardConfig {
  // Storage: SPI Flash for LittleFS (breadboard setup)
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PC12, PC11, PC10, PD2, 1000000, SpiDev::Spi3};
  //                                                              MOSI  MISO  SCLK  CS   Freq

  // IMU: SPI1 pins for sensor breakout boards
  // Chip alignment: CW0_DEG (bench breadboard, flat mount).
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 1000000, SpiDev::Spi1};
  static constexpr IMUConfig imu{imu_spi, PC4, 1000000, IMUAlignment::CW0_DEG};

  // GPS: UART communication
  static constexpr UARTConfig gps{PA9, PA10, 115200, UartDev::Usart1};

  // RC Receiver: IBus on USART1 (RX=PA10, TX=PA9)
  static constexpr RCReceiverConfig rc_receiver{PA10, PA9, 115200, 1000, 300, UartDev::Usart1};

  // I2C: Available for additional sensors
  static constexpr I2CConfig sensors{PB9, PB8, 400000, I2CDev::I2c1};

  // ============================================================================
  // Servo: TIM3 @ 50 Hz (standard RC servo frequency)
  // ============================================================================
  namespace Servo {
    static inline TIM_TypeDef* const timer = TIM3;
    static constexpr uint32_t frequency_hz = 50;

    // Individual channel access (DualTimerPWM_Verification style)
    struct Channel {
      Pin pin;
      uint32_t ch;
      uint32_t min_us;
      uint32_t max_us;
    };
    static constexpr Channel servo1 = {PB4, 1, 1000, 2000};  // TIM3_CH1

    // Array access (DualTimerPWM_Multi style)
    // PB0 on TIM3_CH3: AF resolved by pinmap_function_for_peripheral (no ALT needed)
    static constexpr ServoConfig servos[] = {
      {TIM3, PB4, 1, 1000, 2000},   // Servo 1: TIM3_CH1
      {TIM3, PB0, 3, 1000, 2000},   // Servo 2: TIM3_CH3
    };
    static constexpr int num_servos = sizeof(servos) / sizeof(servos[0]);
  };

  // ============================================================================
  // Motor: TIM1 @ 2 kHz (OneShot125 protocol, 125-250 us)
  // ============================================================================
  namespace Motor {
    static constexpr uint32_t frequency_hz = 2000;
    static constexpr Protocol protocol = Protocol::ONESHOT125;

    // DMA overrides: per-channel dedicated streams (matches Betaflight dma_opt=1)
    static constexpr MotorConfig motors[] = {
      {TIM1, PA8, 1, 125, 250, DMA2, 1, 6},   // Motor 1: TIM1_CH1, DMA2_STREAM1
      {TIM1, PA9, 2, 125, 250, DMA2, 2, 6},   // Motor 2: TIM1_CH2, DMA2_STREAM2
      {TIM1, PA10, 3, 125, 250, DMA2, 6, 6},  // Motor 3: TIM1_CH3, DMA2_STREAM6
    };
    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };

  // ============================================================================
  // ESC: TIM4 @ 2 kHz (OneShot125 protocol, 125-250 us)
  // ============================================================================
  namespace ESC {
    static inline TIM_TypeDef* const timer = TIM4;
    static constexpr uint32_t frequency_hz = 2000;

    // Individual channel access (DualTimerPWM_Verification style)
    struct Channel {
      Pin pin;
      uint32_t ch;
      uint32_t min_us;
      uint32_t max_us;
    };
    static constexpr Channel esc1 = {PB6, 1, 125, 250};  // TIM4_CH1
    static constexpr Channel esc2 = {PB7, 2, 125, 250};  // TIM4_CH2
  };
}
