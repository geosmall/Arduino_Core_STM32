/*
 * BoardConfig for OpenPilot Revolution F405
 * Based on Betaflight unified target, hardware-validated 2026-02-01
 *
 * RC Input (3-pin SBUS header):
 *   - Signal: PA10 (USART1 RX)
 *   - Inverter: PC0 (GPIO output, HIGH=SBUS inverted, LOW=iBus non-inverted)
 *   - Validated: ~27k transitions/sec with SBUS @ 100kbaud
 *
 * HAND-MAINTAINED — DO NOT OVERWRITE WITH CONVERTER OUTPUT WITHOUT A CONFIRMATION
 *
 * This file started as betaflight_converter output from bf_configs/REVO/ and has
 * since been edited by hand. Re-running the converter emits to
 * extras/betaflight_converter/output/OPEN-REVO.h and does not touch this file;
 * copying that output over this one silently drops everything below, none of
 * which the converter can derive from the Betaflight config:
 *
 *   - Output split: 4 motors + 2 servos. The Betaflight config declares
 *     MOTOR1..MOTOR6_PIN and no servos; the split is an airframe decision.
 *     TIM5 carries the servos at 50 Hz, TIM2/TIM3 the motors at 2000 Hz —
 *     PWM frequency is per-timer, so the two rates cannot share a timer.
 *   - SBUS at 100k baud plus rc_inverter_pin (PC0). The converter emits
 *     115200 with no inverter, having no SERIALRX_UART in the source config.
 *   - BOARD_FLASH_CONFIG_* region, which must match bootuf2 stm32f4 boards.h.
 *   - IMU alignment provenance.
 *
 * Also note the converter emits an explicit trailing peripheral instance on the
 * SPI/I2C/UART configs (e.g. `..., SPI3`) that this file omits, leaving those
 * to auto-resolve.
 *
 * To pick up genuine upstream target changes, regenerate into output/, diff
 * against this file, and port individual changes across by hand.
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
  // Chip alignment from Betaflight GYRO_1_ALIGN CW270_DEG
  // (bf_configs/REVO/config.h).
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 8000000};
  static constexpr IMUConfig imu{imu_spi, PC4, 1000000, IMUAlignment::CW270_DEG};

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

  // Servo outputs - 50 Hz PWM on the M5/M6 output pads
  //
  // The board's 6 output pads are split 4 motors + 2 servos. PWM frequency is a
  // per-timer property, so motors and servos cannot share a timer: TIM5 drives
  // only servos at 50 Hz, while TIM3 and TIM2 drive only motors at 2000 Hz.
  namespace Servo {
    static constexpr uint32_t frequency_hz = 50;

    static constexpr ServoConfig servos[] = {
      {TIM5, PA1, 2, 1000, 2000},  // Servo 1: TIM5_CH2 (M5 pad)
      {TIM5, PA0, 1, 1000, 2000},  // Servo 2: TIM5_CH1 (M6 pad)
    };

    static constexpr int num_servos = sizeof(servos) / sizeof(servos[0]);
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
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}