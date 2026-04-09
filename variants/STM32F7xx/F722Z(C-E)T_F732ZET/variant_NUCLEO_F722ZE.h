/*
 * variant_NUCLEO_F722ZE.h — Nucleo-144 F722ZE board definition (Pin refactor)
 *
 * F722ZET6 with 8 MHz HSE (ST-Link MCO bypass), 216 MHz SYSCLK.
 * USART3 on PD8/PD9 connected to ST-Link VCOM.
 *
 * NOTE: This header is included from the HAL conf chain (before HAL modules
 * are loaded), so it CANNOT include Pin.h. Pin constants like PA5 are used
 * as macros here — they resolve to Pin constexpr values when evaluated in
 * user code, after Arduino.h has included Pin.h.
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

// Default pin used for generic 'Serial' instance (USART3)
#ifndef PIN_SERIAL_RX
  #define PIN_SERIAL_RX           PD9
#endif
#ifndef PIN_SERIAL_TX
  #define PIN_SERIAL_TX           PD8
#endif

// I2C Definitions — I2C1 on Arduino connector D14/D15
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
  #define NUM_ANALOG_INPUTS       24
#endif

// SDMMC signals not available on this package
#define SDMMC_CKIN_NA
#define SDMMC_CDIR_NA
#define SDMMC_D0DIR_NA
#define SDMMC_D123DIR_NA

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
