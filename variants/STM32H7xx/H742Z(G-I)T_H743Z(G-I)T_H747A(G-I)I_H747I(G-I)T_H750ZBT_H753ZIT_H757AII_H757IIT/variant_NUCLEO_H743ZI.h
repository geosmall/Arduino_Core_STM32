/*
 * variant_NUCLEO_H743ZI.h — Nucleo-144 H743ZI board definition (Pin refactor)
 *
 * H743ZIT6 with 8 MHz HSE (ST-Link MCO bypass), 480 MHz SYSCLK.
 * USART3 on PD8/PD9 connected to ST-Link VCOM.
 *
 * NOTE: This header is included from the HAL conf chain (before HAL modules
 * are loaded), so it CANNOT include Pin.h.
 */
#pragma once

// On-board LEDs
#define LED_GREEN                 PB0
#ifndef LED_BUILTIN
  #define LED_BUILTIN             LED_GREEN
#endif
#define LED_BLUE                  PB7
#define LED_RED                   PB14

// On-board user button
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

// SPI Definitions — SPI1 on Arduino connector
#ifndef PIN_SPI_SS
  #define PIN_SPI_SS              PD14
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

// UART Definitions — USART3 connected to ST-Link VCOM
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
  #define PIN_WIRE_SDA            PB9
#endif
#ifndef PIN_WIRE_SCL
  #define PIN_WIRE_SCL            PB8
#endif

// 8 MHz HSE from ST-Link MCO (bypass mode, not crystal)
#define HSE_VALUE                 8000000U

// Required by core
#ifndef NUM_DIGITAL_PINS
  #define NUM_DIGITAL_PINS        117
#endif
#ifndef NUM_ANALOG_INPUTS
  #define NUM_ANALOG_INPUTS       28
#endif

/*----------------------------------------------------------------------------
 *        UF2 Bootloader Parameters
 *----------------------------------------------------------------------------*/
#define BOOTUF2_DBL_TAP_MAGIC    0xf01669efUL
#define BOOTUF2_DBL_TAP_ADDR     0x2001FFFCUL

// Extra HAL modules
#if !defined(HAL_DAC_MODULE_DISABLED)
  #define HAL_DAC_MODULE_ENABLED
#endif
#if !defined(HAL_ETH_MODULE_DISABLED)
  #define HAL_ETH_MODULE_ENABLED
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
