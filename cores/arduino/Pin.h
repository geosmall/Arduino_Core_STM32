/*
 * Pin.h — User-facing pin type for STM32 Arduino Core
 *
 * A 2-byte constexpr struct replacing the old uint32_t Arduino pin numbers
 * and #define PAx macros. Explicit conversion to PinName at HAL boundaries.
 *
 * See CORE_REFACTOR.md for design rationale.
 */
#ifndef _PIN_H_
#define _PIN_H_

#include "PinNames.h"  // PinName enum, PortName enum, PinNamesTypes.h

#ifdef __cplusplus

struct Pin {
    PortName port;
    uint8_t  pin;

    constexpr Pin(PortName pt, uint8_t pn) : port(pt), pin(pn) {}
    constexpr Pin() : port(PortEND), pin(255) {}

    constexpr bool IsValid() const { return port < PortEND && pin < 16; }
    constexpr bool operator==(const Pin& rhs) const {
        return port == rhs.port && pin == rhs.pin;
    }
    constexpr bool operator!=(const Pin& rhs) const { return !(*this == rhs); }

    // Explicit conversion to PinName for HAL boundary crossings
    constexpr PinName toPinName() const {
        return IsValid() ? (PinName)((port << 4) | pin) : NC;
    }
};

// Invalid sentinel
constexpr Pin NC_PIN = {};

// Port A — always present
constexpr Pin PA0  = {PortA, 0};
constexpr Pin PA1  = {PortA, 1};
constexpr Pin PA2  = {PortA, 2};
constexpr Pin PA3  = {PortA, 3};
constexpr Pin PA4  = {PortA, 4};
constexpr Pin PA5  = {PortA, 5};
constexpr Pin PA6  = {PortA, 6};
constexpr Pin PA7  = {PortA, 7};
constexpr Pin PA8  = {PortA, 8};
constexpr Pin PA9  = {PortA, 9};
constexpr Pin PA10 = {PortA, 10};
constexpr Pin PA11 = {PortA, 11};
constexpr Pin PA12 = {PortA, 12};
constexpr Pin PA13 = {PortA, 13};
constexpr Pin PA14 = {PortA, 14};
constexpr Pin PA15 = {PortA, 15};

// Port B — always present
constexpr Pin PB0  = {PortB, 0};
constexpr Pin PB1  = {PortB, 1};
constexpr Pin PB2  = {PortB, 2};
constexpr Pin PB3  = {PortB, 3};
constexpr Pin PB4  = {PortB, 4};
constexpr Pin PB5  = {PortB, 5};
constexpr Pin PB6  = {PortB, 6};
constexpr Pin PB7  = {PortB, 7};
constexpr Pin PB8  = {PortB, 8};
constexpr Pin PB9  = {PortB, 9};
constexpr Pin PB10 = {PortB, 10};
constexpr Pin PB11 = {PortB, 11};
constexpr Pin PB12 = {PortB, 12};
constexpr Pin PB13 = {PortB, 13};
constexpr Pin PB14 = {PortB, 14};
constexpr Pin PB15 = {PortB, 15};

#if defined GPIOC_BASE
constexpr Pin PC0  = {PortC, 0};
constexpr Pin PC1  = {PortC, 1};
constexpr Pin PC2  = {PortC, 2};
constexpr Pin PC3  = {PortC, 3};
constexpr Pin PC4  = {PortC, 4};
constexpr Pin PC5  = {PortC, 5};
constexpr Pin PC6  = {PortC, 6};
constexpr Pin PC7  = {PortC, 7};
constexpr Pin PC8  = {PortC, 8};
constexpr Pin PC9  = {PortC, 9};
constexpr Pin PC10 = {PortC, 10};
constexpr Pin PC11 = {PortC, 11};
constexpr Pin PC12 = {PortC, 12};
constexpr Pin PC13 = {PortC, 13};
constexpr Pin PC14 = {PortC, 14};
constexpr Pin PC15 = {PortC, 15};
#endif

#if defined GPIOD_BASE
constexpr Pin PD0  = {PortD, 0};
constexpr Pin PD1  = {PortD, 1};
constexpr Pin PD2  = {PortD, 2};
constexpr Pin PD3  = {PortD, 3};
constexpr Pin PD4  = {PortD, 4};
constexpr Pin PD5  = {PortD, 5};
constexpr Pin PD6  = {PortD, 6};
constexpr Pin PD7  = {PortD, 7};
constexpr Pin PD8  = {PortD, 8};
constexpr Pin PD9  = {PortD, 9};
constexpr Pin PD10 = {PortD, 10};
constexpr Pin PD11 = {PortD, 11};
constexpr Pin PD12 = {PortD, 12};
constexpr Pin PD13 = {PortD, 13};
constexpr Pin PD14 = {PortD, 14};
constexpr Pin PD15 = {PortD, 15};
#endif

#if defined GPIOE_BASE
constexpr Pin PE0  = {PortE, 0};
constexpr Pin PE1  = {PortE, 1};
constexpr Pin PE2  = {PortE, 2};
constexpr Pin PE3  = {PortE, 3};
constexpr Pin PE4  = {PortE, 4};
constexpr Pin PE5  = {PortE, 5};
constexpr Pin PE6  = {PortE, 6};
constexpr Pin PE7  = {PortE, 7};
constexpr Pin PE8  = {PortE, 8};
constexpr Pin PE9  = {PortE, 9};
constexpr Pin PE10 = {PortE, 10};
constexpr Pin PE11 = {PortE, 11};
constexpr Pin PE12 = {PortE, 12};
constexpr Pin PE13 = {PortE, 13};
constexpr Pin PE14 = {PortE, 14};
constexpr Pin PE15 = {PortE, 15};
#endif

#if defined GPIOF_BASE
constexpr Pin PF0  = {PortF, 0};
constexpr Pin PF1  = {PortF, 1};
constexpr Pin PF2  = {PortF, 2};
constexpr Pin PF3  = {PortF, 3};
constexpr Pin PF4  = {PortF, 4};
constexpr Pin PF5  = {PortF, 5};
constexpr Pin PF6  = {PortF, 6};
constexpr Pin PF7  = {PortF, 7};
constexpr Pin PF8  = {PortF, 8};
constexpr Pin PF9  = {PortF, 9};
constexpr Pin PF10 = {PortF, 10};
constexpr Pin PF11 = {PortF, 11};
constexpr Pin PF12 = {PortF, 12};
constexpr Pin PF13 = {PortF, 13};
constexpr Pin PF14 = {PortF, 14};
constexpr Pin PF15 = {PortF, 15};
#endif

#if defined GPIOG_BASE
constexpr Pin PG0  = {PortG, 0};
constexpr Pin PG1  = {PortG, 1};
constexpr Pin PG2  = {PortG, 2};
constexpr Pin PG3  = {PortG, 3};
constexpr Pin PG4  = {PortG, 4};
constexpr Pin PG5  = {PortG, 5};
constexpr Pin PG6  = {PortG, 6};
constexpr Pin PG7  = {PortG, 7};
constexpr Pin PG8  = {PortG, 8};
constexpr Pin PG9  = {PortG, 9};
constexpr Pin PG10 = {PortG, 10};
constexpr Pin PG11 = {PortG, 11};
constexpr Pin PG12 = {PortG, 12};
constexpr Pin PG13 = {PortG, 13};
constexpr Pin PG14 = {PortG, 14};
constexpr Pin PG15 = {PortG, 15};
#endif

#if defined GPIOH_BASE
constexpr Pin PH0  = {PortH, 0};
constexpr Pin PH1  = {PortH, 1};
constexpr Pin PH2  = {PortH, 2};
constexpr Pin PH3  = {PortH, 3};
constexpr Pin PH4  = {PortH, 4};
constexpr Pin PH5  = {PortH, 5};
constexpr Pin PH6  = {PortH, 6};
constexpr Pin PH7  = {PortH, 7};
constexpr Pin PH8  = {PortH, 8};
constexpr Pin PH9  = {PortH, 9};
constexpr Pin PH10 = {PortH, 10};
constexpr Pin PH11 = {PortH, 11};
constexpr Pin PH12 = {PortH, 12};
constexpr Pin PH13 = {PortH, 13};
constexpr Pin PH14 = {PortH, 14};
constexpr Pin PH15 = {PortH, 15};
#endif

#if defined GPIOI_BASE
constexpr Pin PI0  = {PortI, 0};
constexpr Pin PI1  = {PortI, 1};
constexpr Pin PI2  = {PortI, 2};
constexpr Pin PI3  = {PortI, 3};
constexpr Pin PI4  = {PortI, 4};
constexpr Pin PI5  = {PortI, 5};
constexpr Pin PI6  = {PortI, 6};
constexpr Pin PI7  = {PortI, 7};
constexpr Pin PI8  = {PortI, 8};
constexpr Pin PI9  = {PortI, 9};
constexpr Pin PI10 = {PortI, 10};
constexpr Pin PI11 = {PortI, 11};
constexpr Pin PI12 = {PortI, 12};
constexpr Pin PI13 = {PortI, 13};
constexpr Pin PI14 = {PortI, 14};
constexpr Pin PI15 = {PortI, 15};
#endif

#if defined GPIOJ_BASE
constexpr Pin PJ0  = {PortJ, 0};
constexpr Pin PJ1  = {PortJ, 1};
constexpr Pin PJ2  = {PortJ, 2};
constexpr Pin PJ3  = {PortJ, 3};
constexpr Pin PJ4  = {PortJ, 4};
constexpr Pin PJ5  = {PortJ, 5};
constexpr Pin PJ6  = {PortJ, 6};
constexpr Pin PJ7  = {PortJ, 7};
constexpr Pin PJ8  = {PortJ, 8};
constexpr Pin PJ9  = {PortJ, 9};
constexpr Pin PJ10 = {PortJ, 10};
constexpr Pin PJ11 = {PortJ, 11};
constexpr Pin PJ12 = {PortJ, 12};
constexpr Pin PJ13 = {PortJ, 13};
constexpr Pin PJ14 = {PortJ, 14};
constexpr Pin PJ15 = {PortJ, 15};
#endif

#if defined GPIOK_BASE
constexpr Pin PK0  = {PortK, 0};
constexpr Pin PK1  = {PortK, 1};
constexpr Pin PK2  = {PortK, 2};
constexpr Pin PK3  = {PortK, 3};
constexpr Pin PK4  = {PortK, 4};
constexpr Pin PK5  = {PortK, 5};
constexpr Pin PK6  = {PortK, 6};
constexpr Pin PK7  = {PortK, 7};
constexpr Pin PK8  = {PortK, 8};
constexpr Pin PK9  = {PortK, 9};
constexpr Pin PK10 = {PortK, 10};
constexpr Pin PK11 = {PortK, 11};
constexpr Pin PK12 = {PortK, 12};
constexpr Pin PK13 = {PortK, 13};
constexpr Pin PK14 = {PortK, 14};
constexpr Pin PK15 = {PortK, 15};
#endif

#endif /* __cplusplus */
#endif /* _PIN_H_ */
