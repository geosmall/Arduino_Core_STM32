/*
 * variant_BLACKPILL_F411CE.h — WeAct BlackPill F411CE board definition (Pin refactor)
 *
 * NOTE: This header is included from the HAL conf chain (before HAL modules
 * are loaded), so it CANNOT include Pin.h. Pin constants like PA5 are used
 * as macros here — they resolve to Pin constexpr values when evaluated in
 * user code, after Arduino.h has included Pin.h.
 */
#pragma once

// On-board LED: PC13 (active low)
#ifndef LED_BUILTIN
  #define LED_BUILTIN             PC13
#endif

// On-board user button: PA0
#ifndef USER_BTN
  #define USER_BTN                PA0
#endif

// Timer definitions
#ifndef TIMER_TONE
  #define TIMER_TONE              TIM10
#endif
#ifndef TIMER_SERVO
  #define TIMER_SERVO             TIM11
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

// UART Definitions — USART2 (PA9/PA10 reserved for motor outputs on FCs)
#ifndef SERIAL_UART_INSTANCE
  #define SERIAL_UART_INSTANCE    2
#endif

// Default pin used for generic 'Serial' instance
// Pin-style names (PA2 not PA_2) — debug UART is now registered at runtime
// via uart_set_debug(), so uart.c no longer needs these at compile time
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
  #define PIN_WIRE_SCL            PB6
#endif

// 25 MHz external crystal (WeAct Studio BlackPill v2.0/v3.0)
#ifndef HSE_VALUE
  #define HSE_VALUE               25000000U
#endif

// Required by core
#ifndef NUM_DIGITAL_PINS
  #define NUM_DIGITAL_PINS        36
#endif
#ifndef NUM_ANALOG_INPUTS
  #define NUM_ANALOG_INPUTS       10
#endif

#ifdef __cplusplus
  #ifndef SERIAL_PORT_MONITOR
    #define SERIAL_PORT_MONITOR   Serial
  #endif
  #ifndef SERIAL_PORT_HARDWARE
    #define SERIAL_PORT_HARDWARE  Serial
  #endif
#endif
