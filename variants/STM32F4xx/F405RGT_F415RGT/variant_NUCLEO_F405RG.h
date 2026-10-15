/*
 * variant_NUCLEO_F405RG.h — Custom Nucleo-64 F405RG board definition
 *
 * Hardware: Nucleo-64 PCB modified — the original MCU (an STM32F103RB,
 * identified 2026-10-15; earlier notes said F411RE) replaced with an
 * STM32F405RG and an 8 MHz HSE crystal added (so HSE is the real
 * crystal, not the ST-Link MCO bypass the stock board uses).
 * On-board ST-Link V2.1 on ST firmware V2J46M32 (J-Link firmware until
 * 2026-10-14). Its virtual COM port is jumpered to PC6/PC7 (USART6),
 * not to PA2/PA3: the stock PA2/PA3 route carries nothing on this board.
 *
 * NOTE: This header is included from the HAL conf chain (before HAL
 * modules are loaded), so it CANNOT include Pin.h. Pin constants like
 * PA5 are used as macros here — they resolve to Pin constexpr values
 * when evaluated in user code, after Arduino.h has included Pin.h.
 */
#pragma once

// On-board LED: PA5
#ifndef LED_BUILTIN
  #define LED_BUILTIN             PA5
#endif

// On-board user button: PC13
#ifndef USER_BTN
  #define USER_BTN                PC13
#endif

// Timer definitions
#ifndef TIMER_TONE
  #define TIMER_TONE              TIM10
#endif
#ifndef TIMER_SERVO
  #define TIMER_SERVO             TIM11
#endif

// SPI Definitions — SPI1 on Arduino connector
#ifndef PIN_SPI_MOSI
  #define PIN_SPI_MOSI              PA7
#endif
#ifndef PIN_SPI_MISO
  #define PIN_SPI_MISO              PA6
#endif
#ifndef PIN_SPI_SCK
  #define PIN_SPI_SCK               PA5
#endif

// UART Definitions — USART6 jumpered to the ST-Link VCOM (ST-Link TX to
// PC7, ST-Link RX from PC6), not the stock Nucleo USART2 on PA2/PA3
#ifndef SERIAL_UART_INSTANCE
  #define SERIAL_UART_INSTANCE    6
#endif

// Default pin used for generic 'Serial' instance
// Pin-style names (PC6 not PC_6) — debug UART is now registered at runtime
// via uart_set_debug(), so uart.c no longer needs these at compile time
#ifndef PIN_SERIAL_RX
  #define PIN_SERIAL_RX           PC7
#endif
#ifndef PIN_SERIAL_TX
  #define PIN_SERIAL_TX           PC6
#endif

// I2C Definitions — I2C1 on Arduino connector D14/D15
#ifndef PIN_WIRE_SDA
  #define PIN_WIRE_SDA            PB9
#endif
#ifndef PIN_WIRE_SCL
  #define PIN_WIRE_SCL            PB8
#endif

// 8 MHz HSE crystal (custom hardware mod — stock NUCLEO-F411RE uses
// HSE_BYPASS from the ST-Link MCO; this board has a real crystal)
#define HSE_VALUE                 8000000U

// Extra HAL modules — F405-only peripherals not present on F411
#if !defined(HAL_DAC_MODULE_DISABLED)
  #define HAL_DAC_MODULE_ENABLED
#endif
#if !defined(HAL_CAN_MODULE_DISABLED)
  #define HAL_CAN_MODULE_ENABLED
#endif

// Required by core
#ifndef NUM_DIGITAL_PINS
  #define NUM_DIGITAL_PINS        52
#endif
#ifndef NUM_ANALOG_INPUTS
  #define NUM_ANALOG_INPUTS       16
#endif

#ifdef __cplusplus
  #ifndef SERIAL_PORT_MONITOR
    #define SERIAL_PORT_MONITOR   Serial
  #endif
  #ifndef SERIAL_PORT_HARDWARE
    #define SERIAL_PORT_HARDWARE  Serial
  #endif
#endif
