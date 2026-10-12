#pragma once
#include "config/ConfigTypes.h"

// Custom NUCLEO-F405RG: OPEN-REVO surrogate and SWO rig
//   Board: NUCLEO-F411RE PCB, STM32F411RE replaced by an STM32F405RG,
//          8 MHz HSE crystal added (variant NUCLEO_F405RG, 168 MHz)
//   Probe: on-board ST-LINK V2.1 reflashed to J-Link, S/N 777055835.
//          Virtual COM port dark since the rework: no CLI, no MSP; read the
//          sketch over SWD (memory reads, RTT).
//   No bootloader: the application is written at 0x08000000 over SWD.
//
// Wired as OPEN-REVO.h, with these differences:
//
// 1. No storage entry. The W25Q flash is wired as on the Revo (SPI3: SCK PC10,
//    MISO PC11, MOSI PC12, CS PB3) but is not declared, so no sketch starts
//    it. PB3 is the SWO trace pin (TRACESWO, wired to the probe on a
//    Nucleo-64) and must stay free for the frame profiler's SWO check. While
//    SWO runs the flash's chip select toggles with the trace; the chip does
//    nothing without clocks on SPI3.
//
// 2. Motors 3 and 4 on TIM4 CH1/CH2 (PB6, PB7) instead of TIM2 CH4/CH3
//    (PA3, PA2). On a Nucleo-64 PA2/PA3 are USART2, the Arduino Serial the
//    sketch's CLI opens. TIM2 CH4's only other pin is PB11, which on this PCB
//    sits where the F411RE has VCAP_1 (to check on the board before use).
//    TIM4 is a general-purpose timer on APB1 as TIM2 is, 16-bit where TIM2 is
//    32-bit; at 2000 Hz the period fits 16 bits. A motor update is the same
//    compare-register write either way.
//
// 3. No BOARD_FLASH_CONFIG_* region (no UF2 bootloader), so the sketch's
//    `save` is disabled, as on HIL-008. No rc_inverter_pin: the sketch drives
//    it only for ARDUINO_OPEN_REVO. No I2C, UART, ADC or GPS entries: the
//    flight sketch reads none of them.
//
// IMU: wired as on the Revo, SPI1. The chip type is auto-detected at init.
// Alignment CW0_DEG is a placeholder for a breadboard mount: to confirm.
// Nothing is connected to the motor, servo or receiver pins; never armed.
namespace BoardConfig {
  // IMU on SPI1, as OPEN-REVO.h
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 8000000};
  static constexpr IMUConfig imu{imu_spi, PC4, 1000000, IMUAlignment::CW0_DEG};

  // Status LEDs at the Revo's pins (nothing fitted: on-board LD2 is on PA5,
  // the IMU clock, and is dark since the rework)
  static constexpr LEDConfig status_leds{PB5, PB4};

  // RC receiver at the Revo's USART1 pins, SBUS 100k; no receiver fitted
  static constexpr RCReceiverConfig rc_receiver{PA10, PA9, 100000, 1000, 300};

  // Servos as OPEN-REVO.h: TIM5 at 50 Hz
  namespace Servo {
    static constexpr uint32_t frequency_hz = 50;

    static constexpr ServoConfig servos[] = {
      {TIM5, PA1, 2, 1000, 2000},  // Servo 1: TIM5_CH2
      {TIM5, PA0, 1, 1000, 2000},  // Servo 2: TIM5_CH1
    };

    static constexpr int num_servos = sizeof(servos) / sizeof(servos[0]);
  };

  // Motors: OneShot125 at 2000 Hz; 1 and 2 as OPEN-REVO.h, 3 and 4 moved (above)
  namespace Motor {
    static constexpr uint32_t frequency_hz = 2000;
    static constexpr Protocol protocol = Protocol::ONESHOT125;

    static constexpr MotorConfig motors[] = {
      {TIM3, PB0, 3, 125, 250},  // Motor 1: TIM3_CH3
      {TIM3, PB1, 4, 125, 250},  // Motor 2: TIM3_CH4
      {TIM4, PB6, 1, 125, 250},  // Motor 3: TIM4_CH1 (Revo: TIM2_CH4 PA3)
      {TIM4, PB7, 2, 125, 250},  // Motor 4: TIM4_CH2 (Revo: TIM2_CH3 PA2)
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}
