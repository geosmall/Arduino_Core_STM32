/*
 * variant_NUCLEO_G474RE.h — Nucleo-64 G474RE board definition (Pin refactor)
 *
 * NOTE: This header is included from the HAL conf chain (before HAL modules
 * are loaded), so it CANNOT include Pin.h. Pin constants like PA5 are used
 * as macros here — they resolve to Pin constexpr values when evaluated in
 * user code, after Arduino.h has included Pin.h.
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
  #define TIMER_TONE              TIM6
#endif
#ifndef TIMER_SERVO
  #define TIMER_SERVO             TIM7
#endif

// SPI Definitions — SPI1 on Arduino connector
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

// Default pin used for generic 'Serial' instance
// PinName values (PA_2 not PA2) — uart.c is a C file that uses these via
// digitalPinToPinName(), which needs a PinName enum, not a Pin constexpr
#ifndef PIN_SERIAL_RX
  #define PIN_SERIAL_RX           PA_3
#endif
#ifndef PIN_SERIAL_TX
  #define PIN_SERIAL_TX           PA_2
#endif

// Required by core
#ifndef NUM_DIGITAL_PINS
  #define NUM_DIGITAL_PINS        51
#endif
#ifndef NUM_ANALOG_INPUTS
  #define NUM_ANALOG_INPUTS       13
#endif

// Extra HAL modules
#if !defined(HAL_DAC_MODULE_DISABLED)
  #define HAL_DAC_MODULE_ENABLED
#endif
#if !defined(HAL_QSPI_MODULE_DISABLED)
  #define HAL_QSPI_MODULE_ENABLED
#endif

#ifdef __cplusplus
  #ifndef SERIAL_PORT_MONITOR
    #define SERIAL_PORT_MONITOR   Serial
  #endif
  #ifndef SERIAL_PORT_HARDWARE
    #define SERIAL_PORT_HARDWARE  Serial
  #endif
#endif
