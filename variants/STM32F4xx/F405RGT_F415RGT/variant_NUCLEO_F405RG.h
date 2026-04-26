/*
 * variant_NUCLEO_F405RG.h — Custom Nucleo-64 F405RG board definition
 *
 * Hardware: NUCLEO-F411RE PCB modified — STM32F411RE replaced with
 * STM32F405RG and an 8 MHz HSE crystal added (so HSE is the real
 * crystal, not the ST-Link MCO bypass that the stock F411RE uses).
 * ST-Link v2.1 reflashed with J-Link firmware.
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

// UART Definitions — USART2 connected to ST-Link VCOM
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
