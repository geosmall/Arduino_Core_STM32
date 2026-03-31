/*
 * pins_arduino.h — Pin system for STM32 Arduino Core (refactored)
 *
 * Replaces the old D-number / A-number machinery with direct Pin struct.
 * Includes Pin.h, variant header, and provides HAL integration macros.
 */
#ifndef _PINS_ARDUINO_H_
#define _PINS_ARDUINO_H_

#include "variant.h"

#ifdef __cplusplus
#include "Pin.h"
#endif

/* Avoid PortName issue */
_Static_assert(LastPort <= 0x0F, "PortName must be less than 16");

/*
 * Temporary compatibility shims for files not yet converted to Pin API.
 * Each shim is removed when the consuming file is converted:
 *   digitalPinToPinName — M2 (HardwareSerial), M3 (HardwareTimer, WInterrupts, Tone)
 *   digitalPinIsValid   — M3
 *   digitalPinToPort    — M3
 *   digitalPinToBitMask — M3
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "stm32/pinmap.h"
#include "stm32/PeripheralPins.h"

/* Pass-through: unconverted code already holds PinName values, not D-numbers */
#define digitalPinToPinName(p)      ((PinName)(p))
#define digitalPinIsValid(p)        ((PinName)(p) != NC)
#define digitalPinToPort(p)         (get_GPIO_Port(STM_PORT(digitalPinToPinName(p))))
#define digitalPinToBitMask(p)      (STM_GPIO_PIN(digitalPinToPinName(p)))
#define digitalPinToInterrupt(p)    (digitalPinIsValid(p) ? (p) : (uint32_t)NC)
#define NOT_AN_INTERRUPT            (uint32_t)NC
#define analogInPinToBit(p)         (STM_GPIO_PIN(digitalPinToPinName(p)))

#ifdef __cplusplus
}
#endif

/* Port register access macros — used by remaining HAL integration code */
#define portOutputRegister(P)       (&(P->ODR))
#define portInputRegister(P)        (&(P->IDR))
#define portSetRegister(P)          (&(P->BSRR))
#if defined(STM32F2xx) || defined(STM32F4xx) || defined(STM32F7xx)
#define portClearRegister(P)        (&(P->BSRR))
#else
#define portClearRegister(P)        (&(P->BRR))
#endif
#if defined(STM32F1xx)
#define portModeRegister(P)         (&(P->CRL))
#else
#define portModeRegister(P)         (&(P->MODER))
#endif
#define portConfigRegister(P)       (portModeRegister(P))

/* SPI pin convenience aliases — used by Arduino libraries */
#ifdef __cplusplus
static constexpr Pin MOSI = PIN_SPI_MOSI;
static constexpr Pin MISO = PIN_SPI_MISO;
static constexpr Pin SCK  = PIN_SPI_SCK;
#endif

/* I2C pin convenience aliases — used by Wire library */
#if defined(__cplusplus) && defined(PIN_WIRE_SDA) && defined(PIN_WIRE_SCL)
static constexpr Pin SDA = PIN_WIRE_SDA;
static constexpr Pin SCL = PIN_WIRE_SCL;
#endif

/* Default definitions */
#ifndef ADC_RESOLUTION
  #define ADC_RESOLUTION            10
#endif
#define DACC_RESOLUTION             12
#ifndef PWM_RESOLUTION
  #define PWM_RESOLUTION            8
#endif
#ifndef PWM_FREQUENCY
  #define PWM_FREQUENCY             1000
#endif
#ifndef PWM_MAX_DUTY_CYCLE
  #define PWM_MAX_DUTY_CYCLE        4095
#endif

_Static_assert((ADC_RESOLUTION > 0) && (ADC_RESOLUTION <= 32),
               "ADC_RESOLUTION must be 0 < x <= 32!");
_Static_assert((PWM_RESOLUTION > 0) && (PWM_RESOLUTION <= 32),
               "PWM_RESOLUTION must be 0 < x <= 32!");

#endif /* _PINS_ARDUINO_H_ */
