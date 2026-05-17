/*
 * variant_WEACT_G474CE.h — WeAct G474CEU core board (Pin refactor)
 *
 * G474CEU6 with 8 MHz HSE, 170 MHz SYSCLK.
 *
 * NOTE: This header is included from the HAL conf chain (before HAL modules
 * are loaded), so it CANNOT include Pin.h. Pin constants like PA5 are used
 * as macros here — they resolve to Pin constexpr values when evaluated in
 * user code, after Arduino.h has included Pin.h.
 */
#pragma once

// On-board LED
#ifndef LED_BUILTIN
  #define LED_BUILTIN             PC6
#endif

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

// SPI Definitions — SPI1 (ICM42688P IMU)
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

// UART Definitions — USART2 for Serial (PA2/PA3), leaving USART1 (PA9/PA10) free
// for CRSF/ELRS or other peripheral use.
#ifndef SERIAL_UART_INSTANCE
  #define SERIAL_UART_INSTANCE    2
#endif

// Default pin used for generic 'Serial' instance (USART2)
#ifndef PIN_SERIAL_RX
  #define PIN_SERIAL_RX           PA3
#endif
#ifndef PIN_SERIAL_TX
  #define PIN_SERIAL_TX           PA2
#endif

// I2C Definitions — I2C1 (sensors)
#ifndef PIN_WIRE_SDA
  #define PIN_WIRE_SDA            PB7
#endif
#ifndef PIN_WIRE_SCL
  #define PIN_WIRE_SCL            PA15
#endif

// 8 MHz external crystal
#define HSE_VALUE                 8000000U

// Required by core
#ifndef NUM_DIGITAL_PINS
  #define NUM_DIGITAL_PINS        42
#endif
#ifndef NUM_ANALOG_INPUTS
  #define NUM_ANALOG_INPUTS       21
#endif

/*----------------------------------------------------------------------------
 *        UF2 Bootloader Parameters
 *----------------------------------------------------------------------------*/
/* Must match UF2 bootloader: https://github.com/geosmall/bootuf2
 * Magic value: src/board_api.h (DBL_TAP_MAGIC)
 * RAM address: ports/stm32g4/boards.h (BOOTUF2_DBL_TAP_REG) */
#define BOOTUF2_DBL_TAP_MAGIC    0xf01669efUL
#define BOOTUF2_DBL_TAP_ADDR     0x2001FFFCUL  /* Top of 128KB SRAM */

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
