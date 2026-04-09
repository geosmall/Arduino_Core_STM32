/*
 * variant_BKMN_NERO.h — NERO F7 flight controller (Pin refactor)
 *
 * F722RET6 with 8 MHz HSE, 216 MHz SYSCLK, ICM20602 on SPI1, SD card on SPI3.
 * Motor outputs on PA0-PA3 (TIM5), PB0/PB1 (TIM3), PC8/PC9 (TIM8).
 *
 * NOTE: This header is included from the HAL conf chain (before HAL modules
 * are loaded), so it CANNOT include Pin.h. Pin constants like PA5 are used
 * as macros here — they resolve to Pin constexpr values when evaluated in
 * user code, after Arduino.h has included Pin.h.
 *
 * SystemClock_Config is in variant_BKMN_NERO.cpp (same variant directory).
 */
#pragma once

// On-board LEDs
#ifndef LED_BUILTIN
  #define LED_BUILTIN             PB6
#endif
#ifndef LED2_BUILTIN
  #define LED2_BUILTIN            PB5
#endif

// No user button on this FC
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

// SPI Definitions — SPI1 (ICM20602 IMU)
#ifndef PIN_SPI_SS
  #define PIN_SPI_SS              PC4
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

// UART Definitions — USART1 for Serial
#ifndef SERIAL_UART_INSTANCE
  #define SERIAL_UART_INSTANCE    1
#endif

// Default pin used for generic 'Serial' instance (USART1)
#ifndef PIN_SERIAL_RX
  #define PIN_SERIAL_RX           PA10
#endif
#ifndef PIN_SERIAL_TX
  #define PIN_SERIAL_TX           PA9
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
  #define NUM_DIGITAL_PINS        50
#endif
#ifndef NUM_ANALOG_INPUTS
  #define NUM_ANALOG_INPUTS       15
#endif

/*----------------------------------------------------------------------------
 *        UF2 Bootloader Parameters
 *----------------------------------------------------------------------------*/
/* Must match UF2 bootloader: https://github.com/geosmall/bootuf2
 * Address at end of 64K DTCM RAM: 0x20000000 + 0x10000 - 4 = 0x2000FFFC */
#define BOOTUF2_DBL_TAP_MAGIC    0xf01669efUL
#define BOOTUF2_DBL_TAP_ADDR     0x2000FFFCUL

// SDMMC signals not available on this package
#define SDMMC_CKIN_NA
#define SDMMC_CDIR_NA
#define SDMMC_D0DIR_NA
#define SDMMC_D123DIR_NA

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
