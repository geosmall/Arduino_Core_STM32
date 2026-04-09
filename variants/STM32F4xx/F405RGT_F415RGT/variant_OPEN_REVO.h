/*
 * variant_OPEN_REVO.h — OpenPilot Revolution F405 board definition (Pin refactor)
 *
 * NOTE: This header is included from the HAL conf chain (before HAL modules
 * are loaded), so it CANNOT include Pin.h. Pin constants like PB5 are used
 * as macros here — they resolve to Pin constexpr values when evaluated in
 * user code, after Arduino.h has included Pin.h.
 */
#pragma once

// On-board LEDs
#ifndef LED_BUILTIN
  #define LED_BUILTIN             PB5
#endif
#define LED2_BUILTIN              PB4

// No user button on Revolution
#ifndef USER_BTN
  #define USER_BTN                PNUM_NOT_DEFINED
#endif

// Timer definitions — TIM6/TIM7 have no GPIO (internal only)
#ifndef TIMER_TONE
  #define TIMER_TONE              TIM6
#endif
#ifndef TIMER_SERVO
  #define TIMER_SERVO             TIM7
#endif

// SPI Definitions — SPI1 (IMU bus)
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

// UART Definitions — USART3 (Flexi port, debug/telemetry)
#ifndef SERIAL_UART_INSTANCE
  #define SERIAL_UART_INSTANCE    3
#endif

// Default pin used for generic 'Serial' instance (Flexi port)
// Pin-style names — debug UART is now registered at runtime
// via uart_set_debug(), so uart.c no longer needs these at compile time
#ifndef PIN_SERIAL_RX
  #define PIN_SERIAL_RX           PB11
#endif
#ifndef PIN_SERIAL_TX
  #define PIN_SERIAL_TX           PB10
#endif

// I2C Definitions — I2C1 (sensors: baro, mag)
#ifndef PIN_WIRE_SDA
  #define PIN_WIRE_SDA            PB9
#endif
#ifndef PIN_WIRE_SCL
  #define PIN_WIRE_SCL            PB8
#endif

// 8 MHz external crystal
#define HSE_VALUE                 8000000U

// Required by core
#ifndef NUM_DIGITAL_PINS
  #define NUM_DIGITAL_PINS        37
#endif
#ifndef NUM_ANALOG_INPUTS
  #define NUM_ANALOG_INPUTS       3
#endif

/*----------------------------------------------------------------------------
 *        UF2 Bootloader Parameters
 *----------------------------------------------------------------------------*/
/* Must match UF2 bootloader: https://github.com/geosmall/bootuf2
 * Magic value: src/board_api.h (DBL_TAP_MAGIC)
 * RAM address: ports/stm32f4/boards.h (BOOTUF2_DBL_TAP_REG) */
#define BOOTUF2_DBL_TAP_MAGIC    0xf01669efUL
#define BOOTUF2_DBL_TAP_ADDR     0x2000FFFCUL  /* Top of 64KB bootloader stack */

/* Extra HAL modules */
#if !defined(HAL_DAC_MODULE_DISABLED)
  #define HAL_DAC_MODULE_ENABLED
#endif

#ifdef __cplusplus
  #ifndef SERIAL_PORT_MONITOR
    #define SERIAL_PORT_MONITOR   Serial
  #endif
  #ifndef SERIAL_PORT_HARDWARE
    #define SERIAL_PORT_HARDWARE  Serial1
  #endif
#endif
