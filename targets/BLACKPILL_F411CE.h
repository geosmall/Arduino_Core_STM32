#pragma once
#include "config/ConfigTypes.h"

// BLACKPILL F411CE development board configuration
// Based on INav unified target (Cleanflight/INAV)
// Reference: blackpill/target.h and blackpill/target.c

namespace BoardConfig {
  // Storage: SPI2 flash (M25P16 16Mbit) - hardwired on some variants
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB15, PB14, PB13, PB12, 8000000};

  // IMU: SPI1 (MPU6000/MPU6500/MPU9250)
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 1000000};
  static constexpr IMUConfig imu{imu_spi, PB2};  // EXTI2 interrupt pin (individual line)

  // GPS: UART2
  static constexpr UARTConfig gps{PA2, PA3, 115200};

  // RC Receiver: IBus on UART1 (RX=PB3, TX=PA15)
  static constexpr RCReceiverConfig rc_receiver{PB3, PA15, 115200, 1000, 300};

  // I2C1: Magnetometer, barometer, rangefinder, pitot
  static constexpr I2CConfig sensors{PB7, PB6, 400000};

  // ADC: Battery voltage, current, RSSI, airspeed
  namespace ADC {
    static constexpr uint8_t vbat_pin = PB0;      // ADC1_IN8
    static constexpr uint8_t current_pin = PB1;   // ADC1_IN9
    static constexpr uint8_t rssi_pin = PA0;      // ADC1_IN0
    static constexpr uint8_t airspeed_pin = PA1;  // ADC1_IN1
  }

  // Status LEDs
  namespace LED {
    static constexpr uint8_t led0 = PC13;
    static constexpr uint8_t led1 = PC14;
  }

  // Beeper
  static constexpr uint8_t beeper_pin = PB9;
  static constexpr bool beeper_inverted = true;

  // WS2812 LED strip
  static constexpr uint8_t ws2812_pin = PB10;

  // Motor outputs (6 channels) - organized by timer
  namespace Motor {
    // TIM3 motors (S1-S2)
    static constexpr TimerInfo timer3 = TIM3;
    static constexpr uint32_t frequency_hz = 1000;  // 1 kHz for OneShot125

    struct MotorChannel {
      uint8_t ch;
      uint8_t pin;
      uint16_t min_us;
      uint16_t max_us;
    };

    // TIM3 outputs
    static constexpr MotorChannel motor1{1, PB4, 125, 250};  // TIM3_CH1
    static constexpr MotorChannel motor2{2, PB5, 125, 250};  // TIM3_CH2

    // TIM1 outputs (S3-S5)
    static constexpr TimerInfo timer1 = TIM1;
    static constexpr MotorChannel motor3{1, PA8, 125, 250};  // TIM1_CH1
    static constexpr MotorChannel motor4{2, PA9, 125, 250};  // TIM1_CH2
    static constexpr MotorChannel motor5{3, PA10, 125, 250}; // TIM1_CH3

    // TIM4 outputs (S6)
    static constexpr TimerInfo timer4 = TIM4;
    static constexpr MotorChannel motor6{3, PB8, 125, 250};  // TIM4_CH3
  }

  // UART configurations
  namespace UART {
    // UART1: General serial (RX/Telemetry)
    static constexpr UARTConfig uart1{PA15, PB3, 115200};

    // UART2: GPS/SmartPort (shared with GPS config above)
    static constexpr UARTConfig uart2{PA2, PA3, 115200};
  }
}