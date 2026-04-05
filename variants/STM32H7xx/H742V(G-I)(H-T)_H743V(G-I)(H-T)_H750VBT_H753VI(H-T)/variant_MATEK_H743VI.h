/*
 * variant_MATEK_H743VI.h — Matek H743-WLITE flight controller (Pin refactor)
 *
 * H743VIT6 with 8 MHz HSE crystal, 480 MHz SYSCLK.
 * USART3 on PD8/PD9 for Serial.
 *
 * NOTE: This header is included from the HAL conf chain (before HAL modules
 * are loaded), so it CANNOT include Pin.h. Pin constants like PA5 are used
 * as macros here — they resolve to Pin constexpr values when evaluated in
 * user code, after Arduino.h has included Pin.h.
 */
#pragma once

// On-board LED
#ifndef LED_BUILTIN
  #define LED_BUILTIN             PE3
#endif
#define LED_BLUE                  LED_BUILTIN

// On-board button
#ifndef USER_BTN
  #define USER_BTN                PC13
#endif

// Timer definitions — TIM6/TIM7 (no GPIO output needed)
#ifndef TIMER_TONE
  #define TIMER_TONE              TIM6
#endif
#ifndef TIMER_SERVO
  #define TIMER_SERVO             TIM7
#endif

// SPI Definitions — SPI2
#ifndef PIN_SPI_SS
  #define PIN_SPI_SS              PB12
#endif
#ifndef PIN_SPI_MOSI
  #define PIN_SPI_MOSI            PB15
#endif
#ifndef PIN_SPI_MISO
  #define PIN_SPI_MISO            PB14
#endif
#ifndef PIN_SPI_SCK
  #define PIN_SPI_SCK             PB13
#endif

// UART Definitions — USART3 for Serial
#ifndef SERIAL_UART_INSTANCE
  #define SERIAL_UART_INSTANCE    3
#endif
#ifndef PIN_SERIAL_RX
  #define PIN_SERIAL_RX           PD9
#endif
#ifndef PIN_SERIAL_TX
  #define PIN_SERIAL_TX           PD8
#endif

// I2C Definitions — I2C1
#ifndef PIN_WIRE_SDA
  #define PIN_WIRE_SDA            PB11
#endif
#ifndef PIN_WIRE_SCL
  #define PIN_WIRE_SCL            PB10
#endif

// 8 MHz external crystal
#define HSE_VALUE                 8000000U

// Required by core
#ifndef NUM_DIGITAL_PINS
  #define NUM_DIGITAL_PINS        82
#endif
#ifndef NUM_ANALOG_INPUTS
  #define NUM_ANALOG_INPUTS       16
#endif

/*----------------------------------------------------------------------------
 *        UF2 Bootloader Parameters
 *----------------------------------------------------------------------------*/
/* 128KB DTCM RAM top: 0x20000000 + 0x20000 - 4 = 0x2001FFFC */
#define BOOTUF2_DBL_TAP_MAGIC    0xf01669efUL
#define BOOTUF2_DBL_TAP_ADDR     0x2001FFFCUL

// Extra HAL modules
#if !defined(HAL_DAC_MODULE_DISABLED)
  #define HAL_DAC_MODULE_ENABLED
#endif
#if !defined(HAL_QSPI_MODULE_DISABLED)
  #define HAL_QSPI_MODULE_ENABLED
#endif
#if !defined(HAL_SD_MODULE_DISABLED)
  #define HAL_SD_MODULE_ENABLED
#endif

#ifdef __cplusplus
  #ifndef SERIAL_PORT_MONITOR
    #define SERIAL_PORT_MONITOR   Serial
  #endif
  #ifndef SERIAL_PORT_HARDWARE
    #define SERIAL_PORT_HARDWARE  Serial
  #endif
#endif
