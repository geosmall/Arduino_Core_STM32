/*
 * variant_NUCLEO_F411RE.h — Nucleo-64 F411RE board definition (Pin refactor)
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
  #define TIMER_TONE              TIM10
#endif
#ifndef TIMER_SERVO
  #define TIMER_SERVO             TIM11
#endif

// UART Definitions — USART2 connected to ST-Link VCOM
#ifndef SERIAL_UART_INSTANCE
  #define SERIAL_UART_INSTANCE    2
#endif

// Default pin used for generic 'Serial' instance
// PinName values — HardwareSerial not yet converted to Pin API
#ifndef PIN_SERIAL_RX
  #define PIN_SERIAL_RX           PA_3
#endif
#ifndef PIN_SERIAL_TX
  #define PIN_SERIAL_TX           PA_2
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
