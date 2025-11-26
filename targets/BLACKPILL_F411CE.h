/*
 * BLACKPILL F411CE BoardConfig
 * Hardware: WeAct Studio MiniSTM32F4x1 (STM32F411CEU6)
 * Reference: https://github.com/WeActStudio/WeActStudio.MiniSTM32F4x1
 *
 * Total Pins Assigned: 29 / 32 available
 * Available Pins: PA11, PA12, PC15 (only PA11 has timer capability for servo expansion)

   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   Pin Assignments by Function:
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

   MOTORS (6 pins) - OneShot125 @ 8kHz
      ├─ Motor 1: PB4  (TIM3_CH1)
      ├─ Motor 2: PB5  (TIM3_CH2)
      ├─ Motor 3: PB0  (TIM3_CH3 via PB0_ALT1) ✓ No conflict
      ├─ Motor 4: PB1  (TIM3_CH4 via PB1_ALT1) ✓ No conflict
      ├─ Motor 5: PB6  (TIM4_CH1)
      └─ Motor 6: PB7  (TIM4_CH2)

   SERVOS (3 pins) - Standard PWM @ 50 Hz
      ├─ Servo 1: PA2  (TIM2_CH3)
      ├─ Servo 2: PA15 (TIM2_CH1)
      └─ Servo 3: PB3  (TIM2_CH2)

   STORAGE - SPI Flash (4 pins)
      ├─ MOSI: PA7
      ├─ MISO: PA6
      ├─ SCLK: PA5
      └─ CS:   PA4

   IMU - MPU9250 SPI (5 pins)
      ├─ MOSI: PB15
      ├─ MISO: PB14
      ├─ SCLK: PB13
      ├─ CS:   PB12
      └─ INT:  PB2

   SENSORS - I2C (2 pins)
      ├─ SDA: PB9
      └─ SCL: PB8

   GPS - UART1 (2 pins)
      ├─ TX: PA9
      └─ RX: PA10

   RC RECEIVER - UART (2 pins)
      ├─ RX: PA3
      └─ TX: PB10

   BATTERY - ADC (2 pins)
      ├─ Voltage: PA1
      └─ Current: PA0

   INDICATORS (3 pins)
      ├─ Status LED: PC13
      ├─ Beeper:     PC14
      └─ WS2812:     PA8

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

  // Motor outputs - motor array format (MotorManager compatible)
  namespace Motor {
    static constexpr uint32_t frequency_hz = 8000;  // 8 kHz for OneShot125

    struct MotorConfig {
      TIM_TypeDef* timer;
      uint32_t pin;
      uint32_t channel;
      uint32_t min_us;
      uint32_t max_us;
    };

    // Motor array - hardware timer assignments for BLACKPILL F411CE
    // Original validated configuration: TIM3 (M1-M4), TIM4 (M5-M6)
    static constexpr MotorConfig motors[] = {
      {TIM3, PB4, 1, 125, 250},       // Motor 1: TIM3_CH1
      {TIM3, PB5, 2, 125, 250},       // Motor 2: TIM3_CH2
      {TIM3, PB0_ALT1, 3, 125, 250},  // Motor 3: TIM3_CH3 (ALT1)
      {TIM3, PB1_ALT1, 4, 125, 250},  // Motor 4: TIM3_CH4 (ALT1)
      {TIM4, PB6, 1, 125, 250},       // Motor 5: TIM4_CH1
      {TIM4, PB7, 2, 125, 250},       // Motor 6: TIM4_CH2
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };

  // Servo outputs - 50 Hz PWM for standard servos
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
      {TIM2, PA2,  3, 1000, 2000},  // Servo 1: TIM2_CH3
      {TIM2, PA15, 1, 1000, 2000},  // Servo 2: TIM2_CH1
      {TIM2, PB3,  2, 1000, 2000},  // Servo 3: TIM2_CH2
    };

    static constexpr int num_servos = sizeof(servos) / sizeof(servos[0]);
  };

}