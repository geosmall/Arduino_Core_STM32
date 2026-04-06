/*
 * variant_NUCLEO_G431KB.h — Nucleo-32 G431KB board definition (Pin refactor)
 *
 * G431KBU6 with HSI 16 MHz (no HSE by default), 170 MHz SYSCLK.
 * LPUART1 on PA2/PA3 connected to ST-Link VCOM.
 *
 * NOTE: This header is included from the HAL conf chain (before HAL modules
 * are loaded), so it CANNOT include Pin.h. Pin constants like PA5 are used
 * as macros here — they resolve to Pin constexpr values when evaluated in
 * user code, after Arduino.h has included Pin.h.
 */
#pragma once

// On-board LED
#ifndef LED_BUILTIN
  #define LED_BUILTIN             PB8
#endif
#define LED_GREEN                 LED_BUILTIN

// No user button on this board
#ifndef USER_BTN
  #define USER_BTN                PNUM_NOT_DEFINED
#endif

// Timer definitions — TIM6/TIM7 (no GPIO output needed)
#ifndef TIMER_TONE
  #define TIMER_TONE              TIM6
#endif
#ifndef TIMER_SERVO
  #define TIMER_SERVO             TIM7
#endif

// SPI Definitions — SPI1
#ifndef PIN_SPI_SS
  #define PIN_SPI_SS              PA4
#endif
#ifndef PIN_SPI_MOSI
  #define PIN_SPI_MOSI            PA7
#endif
#ifndef PIN_SPI_MISO
  #define PIN_SPI_MISO            PA6
#endif
#ifndef PIN_SPI_SCK
  #define PIN_SPI_SCK             PA5
#endif

// UART Definitions — LPUART1 connected to ST-Link VCOM
#ifndef SERIAL_UART_INSTANCE
  #define SERIAL_UART_INSTANCE    101
#endif

// Default pin used for generic 'Serial' instance (LPUART1)
#ifndef PIN_SERIAL_RX
  #define PIN_SERIAL_RX           PA3
#endif
#ifndef PIN_SERIAL_TX
  #define PIN_SERIAL_TX           PA2
#endif

// I2C Definitions — I2C1
#ifndef PIN_WIRE_SDA
  #define PIN_WIRE_SDA            PB7
#endif
#ifndef PIN_WIRE_SCL
  #define PIN_WIRE_SCL            PA15
#endif

// Required by core
#ifndef NUM_DIGITAL_PINS
  #define NUM_DIGITAL_PINS        25
#endif
#ifndef NUM_ANALOG_INPUTS
  #define NUM_ANALOG_INPUTS       9
#endif

// Extra HAL modules
#if !defined(HAL_DAC_MODULE_DISABLED)
  #define HAL_DAC_MODULE_ENABLED
#endif

#ifdef __cplusplus
  #ifndef SERIAL_PORT_MONITOR
    #define SERIAL_PORT_MONITOR   Serial
  #endif
  #ifndef SERIAL_PORT_HARDWARE
    #define SERIAL_PORT_HARDWARE  Serial2
  #endif
#endif
