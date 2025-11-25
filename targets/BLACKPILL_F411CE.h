/*
 * BLACKPILL F411CE BoardConfig
 * Hardware: WeAct Studio MiniSTM32F4x1 (STM32F411CEU6)
 * Reference: https://github.com/WeActStudio/WeActStudio.MiniSTM32F4x1
 *
 * Updated to match NUCLEO_F411RE_JHEF411.h format (2025-01-04):
 * - Added IMUConfig frequency parameter
 * - Converted to typed configs (ADCConfig, LEDConfig)
 * - Standardized motor struct to {pin, ch, min, max} order
 * - SPI frequencies set to 1 MHz for jumper wire compatibility
 */

#pragma once
#include "config/ConfigTypes.h"

// BLACKPILL F411CE development board configuration

namespace BoardConfig {
  // see: https://github.com/WeActStudio/WeActStudio.MiniSTM32F4x1/blob/master/HDK/MiniF4x1Cx_V31.pdf
  // StorageConfig(StorageBackend, mosi, miso, sclk, cs, frequency_hz)
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PA7, PA6, PA5, PA4, 8000000};

  // IMU: MPU6000/MPU6500/MPU9250 on SPI2
  // SPIConfig(mosi, miso, sclk, cs, frequency_hz)
  static constexpr SPIConfig imu_spi{PB15, PB14, PB13, PB12, 1000000};
  // IMUConfig(spi_config, interrupt_pin, frequency_hz)
  static constexpr IMUConfig imu{imu_spi, PB2, 1000000};  // EXTI2 interrupt pin (individual line)

  // GPS: UART1
  // UARTConfig(tx, rx, baud)
  static constexpr UARTConfig gps{PA9, PA10, 115200};

  // RC Receiver: IBus on UART2 (RX=PB3, TX=PA15)
  // RCReceiverConfig(rx, tx, baud, timeout_ms, idle_threshold_us)
  static constexpr RCReceiverConfig rc_receiver{PA3, PB10, 115200, 1000, 300};

  // I2C1: Magnetometer, barometer, rangefinder, pitot
  // I2CConfig(sda, scl, frequency_hz)
  static constexpr I2CConfig sensors{PB9, PB8, 400000};

  // ADC: Battery voltage, current
  // ADCConfig(vbat_pin, current_pin, vbat_scale, current_scale)
  static constexpr ADCConfig battery{PA1, PA0, 110, 170};

  // Status LEDs
  static constexpr LEDConfig status_leds{PC13};

  // Beeper
  static constexpr uint8_t beeper_pin = PC14;
  static constexpr bool beeper_inverted = false;

  // WS2812 LED strip
  static constexpr uint8_t ws2812_pin = PA8;

  // Motor outputs - organized by timer banks (dRehmFlight compatible)
  namespace Motor {
    static constexpr uint32_t frequency_hz = 8000;  // 8 kHz for OneShot125

    struct MotorChannel {
      uint32_t pin;
      uint32_t ch;
      uint32_t min_us;
      uint32_t max_us;
    };

    // TIM1 Bank (M1-M3) - using TIM2 on BLACKPILL (PA0, PA1, PA2)
    namespace TIM1_Bank {
      static inline TIM_TypeDef* const timer = TIM2;
      static constexpr MotorChannel motor1{PA0, 1, 125, 250};  // TIM2_CH1
      static constexpr MotorChannel motor2{PA1, 2, 125, 250};  // TIM2_CH2
      static constexpr MotorChannel motor3{PA2, 3, 125, 250};  // TIM2_CH3
      static constexpr MotorChannel motor4{PA3, 4, 125, 250};  // TIM2_CH4 (spare)
      static constexpr MotorChannel motor5{PA15, 1, 125, 250}; // TIM2_CH1 (spare, ALT)
    }

    // TIM3 Bank (M4-M5) - using TIM3 on BLACKPILL (PB0_ALT1, PB4)
    namespace TIM3_Bank {
      static inline TIM_TypeDef* const timer = TIM3;
      static constexpr MotorChannel motor4{PB0_ALT1, 3, 125, 250}; // TIM3_CH3 (ALT1)
      static constexpr MotorChannel motor5{PB4, 1, 125, 250};      // TIM3_CH1
      static constexpr MotorChannel motor6{PB5, 2, 125, 250};      // TIM3_CH2 (spare)
      static constexpr MotorChannel motor7{PB1_ALT1, 4, 125, 250}; // TIM3_CH4 (spare, ALT1)
    }
  }

}