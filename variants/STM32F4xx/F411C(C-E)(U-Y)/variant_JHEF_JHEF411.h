/*
 * variant_JHEF_JHEF411.h — NOXE V3 (JHEF) flight controller (Pin refactor)
 *
 * F411CE with 8 MHz HSE, MPU6000/ICM42688P on SPI1, W25Q128FV on SPI2.
 * Motor outputs on PA8/PA9/PA10 (TIM1) and PB0/PB4 (TIM3).
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

// Timer definitions
#ifndef TIMER_TONE
  #define TIMER_TONE              TIM10
#endif
#ifndef TIMER_SERVO
  #define TIMER_SERVO             TIM11
#endif

// SPI Definitions — SPI1 (MPU6000/ICM42688P)
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

// UART Definitions — USART2 for Serial debug
#ifndef SERIAL_UART_INSTANCE
  #define SERIAL_UART_INSTANCE    2
#endif

// Default pin used for generic 'Serial' instance (USART2)
// Pin-style names (PA2 not PA_2) — debug UART is now registered at runtime
// via uart_set_debug(), so uart.c no longer needs these at compile time
#ifndef PIN_SERIAL_RX
  #define PIN_SERIAL_RX           PA3
#endif
#ifndef PIN_SERIAL_TX
  #define PIN_SERIAL_TX           PA2
#endif

// I2C Definitions — I2C1 (sensors)
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
  #define NUM_DIGITAL_PINS        36
#endif
#ifndef NUM_ANALOG_INPUTS
  #define NUM_ANALOG_INPUTS       10
#endif

/*----------------------------------------------------------------------------
 *        UF2 Bootloader Parameters
 *----------------------------------------------------------------------------*/
/* Must match UF2 bootloader: https://github.com/geosmall/bootuf2
 * Magic value: src/board_api.h (DBL_TAP_MAGIC)
 * RAM address: ports/stm32f4/boards.h (BOOTUF2_DBL_TAP_REG) */
#define BOOTUF2_DBL_TAP_MAGIC    0xf01669efUL
#define BOOTUF2_DBL_TAP_ADDR     0x2000FFFCUL  /* Top of 64KB RAM */

#ifdef __cplusplus
  #ifndef SERIAL_PORT_MONITOR
    #define SERIAL_PORT_MONITOR   Serial
  #endif
  #ifndef SERIAL_PORT_HARDWARE
    #define SERIAL_PORT_HARDWARE  Serial
  #endif
#endif
