/*
 *******************************************************************************
 * Copyright (c) 2017-2021, STMicroelectronics
 * All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */
#pragma once

/*----------------------------------------------------------------------------
 *        STM32 pins number - OpenPilot Revolution F405
 *----------------------------------------------------------------------------*/

// Motor outputs (directly accessible for flight controller use)
#define PB0                     0   // Motor 1 - TIM3_CH3
#define PB1                     1   // Motor 2 - TIM3_CH4
#define PA3                     2   // Motor 3 - TIM2_CH4
#define PA2                     3   // Motor 4 - TIM2_CH3
#define PA1                     4   // Motor 5 - TIM5_CH2
#define PA0                     5   // Motor 6 - TIM5_CH1

// SPI1 - IMU (MPU6500/MPU6000)
#define PA4                     6   // SPI1 CS (Gyro)
#define PA5                     7   // SPI1 SCK
#define PA6                     8   // SPI1 MISO
#define PA7                     9   // SPI1 MOSI
#define PC4                     10  // Gyro EXTI

// SPI3 - Flash (M25P16)
#define PB3                     11  // Flash CS
#define PC10                    12  // SPI3 SCK
#define PC11                    13  // SPI3 MISO
#define PC12                    14  // SPI3 MOSI

// I2C1 - Sensors (Baro, Mag)
#define PB8                     15  // I2C1 SCL
#define PB9                     16  // I2C1 SDA

// USART1 - RC Receiver / Main serial
#define PA9                     17  // USART1 TX
#define PA10                    18  // USART1 RX

// USART3
#define PB10                    19  // USART3 TX
#define PB11                    20  // USART3 RX

// USART6
#define PC6                     21  // USART6 TX
#define PC7                     22  // USART6 RX

// LEDs
#define PB5                     23  // LED1 (Blue)
#define PB4                     24  // LED2 (Amber)

// ADC
#define PC2                     PIN_A0  // VBAT
#define PC1                     PIN_A1  // Current
#define PC0                     PIN_A2  // RSSI (Inverter)

// PWM inputs (RC receiver)
#define PB14                    28  // PPM / PWM1 - TIM12_CH1
#define PB15                    29  // PWM2 - TIM12_CH2
#define PC8                     30  // PWM5 / PINIO - TIM8_CH3
#define PC9                     31  // PWM6 - TIM8_CH4

// USB
#define PA11                    32  // USB_DM
#define PA12                    33  // USB_DP
#define PC5                     34  // USB_DETECT

// Debug
#define PA13                    35  // SWDIO
#define PA14                    36  // SWCLK

// Alternate pins number
#define PA0_ALT1                (PA0  | ALT1)
#define PA0_ALT2                (PA0  | ALT2)
#define PA1_ALT1                (PA1  | ALT1)
#define PA1_ALT2                (PA1  | ALT2)
#define PA2_ALT1                (PA2  | ALT1)
#define PA2_ALT2                (PA2  | ALT2)
#define PA3_ALT1                (PA3  | ALT1)
#define PA3_ALT2                (PA3  | ALT2)
#define PA4_ALT1                (PA4  | ALT1)
#define PA5_ALT1                (PA5  | ALT1)
#define PA6_ALT1                (PA6  | ALT1)
#define PA7_ALT1                (PA7  | ALT1)
#define PA7_ALT2                (PA7  | ALT2)
#define PA7_ALT3                (PA7  | ALT3)
#define PA15_ALT1               (PA15 | ALT1)
#define PB0_ALT1                (PB0  | ALT1)
#define PB0_ALT2                (PB0  | ALT2)
#define PB1_ALT1                (PB1  | ALT1)
#define PB1_ALT2                (PB1  | ALT2)
#define PB3_ALT1                (PB3  | ALT1)
#define PB4_ALT1                (PB4  | ALT1)
#define PB5_ALT1                (PB5  | ALT1)
#define PB8_ALT1                (PB8  | ALT1)
#define PB9_ALT1                (PB9  | ALT1)
#define PB14_ALT1               (PB14 | ALT1)
#define PB14_ALT2               (PB14 | ALT2)
#define PB15_ALT1               (PB15 | ALT1)
#define PB15_ALT2               (PB15 | ALT2)
#define PC0_ALT1                (PC0  | ALT1)
#define PC0_ALT2                (PC0  | ALT2)
#define PC1_ALT1                (PC1  | ALT1)
#define PC1_ALT2                (PC1  | ALT2)
#define PC2_ALT1                (PC2  | ALT1)
#define PC2_ALT2                (PC2  | ALT2)
#define PC3_ALT1                (PC3  | ALT1)
#define PC3_ALT2                (PC3  | ALT2)
#define PC4_ALT1                (PC4  | ALT1)
#define PC5_ALT1                (PC5  | ALT1)
#define PC6_ALT1                (PC6  | ALT1)
#define PC7_ALT1                (PC7  | ALT1)
#define PC8_ALT1                (PC8  | ALT1)
#define PC9_ALT1                (PC9  | ALT1)
#define PC10_ALT1               (PC10 | ALT1)
#define PC11_ALT1               (PC11 | ALT1)

// This must be a literal
#define NUM_DIGITAL_PINS        37
// This must be a literal with a value less than or equal to MAX_ANALOG_INPUTS
#define NUM_ANALOG_INPUTS       3

// On-board LED pin number
#ifndef LED_BUILTIN
  #define LED_BUILTIN           PB5
#endif
#define LED2_BUILTIN            PB4

// No user button on Revolution
#ifndef USER_BTN
  #define USER_BTN              PNUM_NOT_DEFINED
#endif

// SPI Definitions - Default to SPI1 (IMU bus)
#ifndef PIN_SPI_SS
  #define PIN_SPI_SS            PA4
#endif
#ifndef PIN_SPI_MOSI
  #define PIN_SPI_MOSI          PA7
#endif
#ifndef PIN_SPI_MISO
  #define PIN_SPI_MISO          PA6
#endif
#ifndef PIN_SPI_SCK
  #define PIN_SPI_SCK           PA5
#endif

// SPI3 - Flash
#define PIN_SPI3_SS             PB3
#define PIN_SPI3_MOSI           PC12
#define PIN_SPI3_MISO           PC11
#define PIN_SPI3_SCK            PC10

// I2C Definitions
#ifndef PIN_WIRE_SDA
  #define PIN_WIRE_SDA          PB9
#endif
#ifndef PIN_WIRE_SCL
  #define PIN_WIRE_SCL          PB8
#endif

// Timer Definitions
// Use TIM6/TIM7 when possible as servo and tone don't need GPIO output pin
#ifndef TIMER_TONE
  #define TIMER_TONE            TIM6
#endif
#ifndef TIMER_SERVO
  #define TIMER_SERVO           TIM7
#endif

// UART Definitions
// Default Serial instance uses USART1 (RC receiver port)
#ifndef SERIAL_UART_INSTANCE
  #define SERIAL_UART_INSTANCE  1
#endif

// Default pin used for 'Serial' instance
#ifndef PIN_SERIAL_RX
  #define PIN_SERIAL_RX         PA10
#endif
#ifndef PIN_SERIAL_TX
  #define PIN_SERIAL_TX         PA9
#endif

/* HAL configuration */
#define HSE_VALUE               8000000U

/* Extra HAL modules */
#if !defined(HAL_DAC_MODULE_DISABLED)
  #define HAL_DAC_MODULE_ENABLED
#endif

/*----------------------------------------------------------------------------
 *        Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

#ifdef __cplusplus
  // These serial port names are intended to allow libraries and architecture-neutral
  // sketches to automatically default to the correct port name for a particular type
  // of use.
  #ifndef SERIAL_PORT_MONITOR
    #define SERIAL_PORT_MONITOR   Serial
  #endif
  #ifndef SERIAL_PORT_HARDWARE
    #define SERIAL_PORT_HARDWARE  Serial1
  #endif
#endif
