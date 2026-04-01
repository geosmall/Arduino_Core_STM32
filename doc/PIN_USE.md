# Pin System Reference

How GPIO pins are named, stored, and converted in the refactored STM32 Arduino Core.

---

## Two Types, One Boundary

The pin system has two types with a single conversion point between them:

| Type | Where | Purpose |
|------|-------|---------|
| `Pin` | User code, Arduino API, BoardConfig, variant headers | Type-safe C++ struct |
| `PinName` | HAL layer, PinMap tables, PeripheralPins.c | ST vendor code currency |

```
User code        HAL boundary         ST HAL / LL drivers
─────────        ────────────         ───────────────────
Pin PA0  ──→  pin.toPinName()  ──→  PinName PA_0 = 0x00
```

A `Pin` is a 2-byte constexpr struct. A `PinName` is a uint32_t-sized enum. They encode the same information differently:

```cpp
// Pin (cores/arduino/Pin.h)
struct Pin {
    PortName port;   // PortA=0, PortB=1, ...
    uint8_t  pin;    // 0..15
};
constexpr Pin PA0 = {PortA, 0};

// PinName (cores/arduino/stm32/PinNames.h)
PA_0 = (PortA << 4) | 0   // = 0x00
PB7  = (PortB << 4) | 7   // = 0x17
```

Conversion is one-way and explicit:

```cpp
constexpr PinName toPinName() const {
    return IsValid() ? (PinName)((port << 4) | pin) : NC;
}
```

### Why both exist

PinName is the currency of ST's HAL code — every `PinMap` table, every `pinmap_peripheral()` call, every `PeripheralPins.c` entry. That's thousands of lines of vendor code copied unchanged. Rewriting it all for Pin would break the clean boundary between "our code" and "ST's code."

Pin gives compile-time type safety. The old API used `uint32_t` everywhere — a wrong value compiled and failed silently at runtime. With Pin, passing an ADC channel number or timer index where a pin is expected is a compile error.

---

## Where Pin Constants Come From

`Pin.h` defines constexpr constants for every pin on every port:

```cpp
// Always present
constexpr Pin PA0  = {PortA, 0};
constexpr Pin PA1  = {PortA, 1};
// ...
constexpr Pin PB15 = {PortB, 15};

// Conditional on chip package
#if defined GPIOC_BASE
constexpr Pin PC0  = {PortC, 0};
// ...
#endif
```

The invalid sentinel is `NC_PIN` (port=PortEND, pin=255). `pin.IsValid()` returns false for it.

---

## Arduino API

User-facing functions take `Pin` directly:

```cpp
// Digital I/O
pinMode(PA5, OUTPUT);
digitalWrite(PA5, HIGH);
int val = digitalRead(PC13);

// Analog I/O
uint32_t adc = analogRead(PA0);
analogWrite(PA5, 128);
```

These are declared in `Arduino.h` under `#ifdef __cplusplus` and implemented in `wiring_digital.cpp` / `wiring_analog.cpp`. Each function validates the pin and converts once at the HAL boundary:

```cpp
void digitalWrite(Pin pin, uint32_t ulVal) {
    digitalWriteFast(pin.toPinName(), ulVal);  // single conversion
}
```

### Resolution functions (no pin parameter)

`analogReadResolution()`, `analogWriteResolution()`, `analogWriteFrequency()`, and `analogReference()` don't take pins — they're declared in `wiring_analog.h` with C linkage.

---

## Variant Headers

Each board variant defines macros for default peripherals. These use Pin-style names (`PA5`, not `PA_5`), which resolve to Pin constexpr values when evaluated in C++ context:

```cpp
// variant_NUCLEO_F411RE.h
#define LED_BUILTIN     PA5
#define PIN_SPI_MOSI    PA7
#define PIN_SPI_MISO    PA6
#define PIN_SPI_SCK     PA5
#define PIN_SERIAL_RX   PA3
#define PIN_SERIAL_TX   PA2
#define PIN_WIRE_SDA    PB9
#define PIN_WIRE_SCL    PB8
```

The variant `.cpp` file contains only `SystemClock_Config()`. The old `digitalPin[]` and `analogInputPin[]` arrays are eliminated — no lookup tables, no D-number or A-number indirection.

---

## BoardConfig

Board-specific configurations use Pin constants directly in constexpr structs:

```cpp
// targets/NUCLEO_F411RE_HIL001.h
namespace BoardConfig {
    static constexpr StorageConfig storage{
        StorageBackend::LITTLEFS,
        PC12, PC11, PC10, PD2,  // MOSI, MISO, SCLK, CS
        1000000                  // 1 MHz
    };

    static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 1000000};
    static constexpr IMUConfig imu{imu_spi, PC4};  // INT pin
}
```

Config types are defined in `targets/config/ConfigTypes.h`. Pin fields are currently `uint32_t` (M7 converts them to `Pin`).

---

## HAL Boundary Layer

Internal functions in `cores/arduino/stm32/` take `PinName`. These are not user-facing — they're called from the wiring layer after `toPinName()` conversion:

```
analogRead(Pin)                          ← user calls this
  └→ adc_read_value(PinName, resolution) ← HAL layer (analog.cpp)
       └→ HAL_ADC_*()                    ← ST vendor code

analogWrite(Pin, value)
  ├→ dac_write_value(PinName, value, init)   if DAC pin
  ├→ pwm_start(PinName, freq, value, res)    if timer pin
  └→ pinMode(pin, OUTPUT) + digitalWrite()   digital fallback
```

### PinMap Tables

`PeripheralPins.c` (one per variant) maps PinName values to peripheral instances and alternate functions:

```c
// PinMap_TIM entries for F411RE
{PA_0, TIM2, STM_PIN_DATA_EXT(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF1_TIM2, 1, 0)},
{PA_0, TIM5, STM_PIN_DATA_EXT(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF2_TIM5, 1, 0)},
```

Query functions in `pinmap.h`:

| Function | Purpose |
|----------|---------|
| `pinmap_peripheral(pn, map)` | Get peripheral instance for a pin |
| `pinmap_function(pn, map)` | Get AF config (first match) |
| `pinmap_function_for_peripheral(pn, periph, map)` | Get AF for a specific peripheral (solves ALT pin trap) |
| `pinmap_pinout_for_peripheral(pn, periph, map)` | Configure GPIO AF for a specific peripheral |
| `pin_in_pinmap(pn, map)` | Check if pin has any entry in table |

See `doc/PIN_MAPPING.md` for the ALT pin trap and when to use peripheral-aware vs. bare lookup.

### PinName Decomposition

Low-level macros for extracting hardware details from a PinName:

```cpp
STM_PORT(pn)      // port number: 0=A, 1=B, 2=C, ...
STM_PIN(pn)       // pin number: 0..15
STM_GPIO_PIN(pn)  // GPIO bitmask: (1 << pin_number)
STM_LL_GPIO_PIN(pn) // LL GPIO bitmask from lookup table

get_GPIO_Port(port_num)  // port number → GPIO_TypeDef* (GPIOA, GPIOB, ...)
```

### Fast GPIO

`digital_io.h` provides inline functions that bypass the Arduino API for direct register access. These take PinName:

```cpp
digitalWriteFast(PinName pn, uint32_t val);
digitalReadFast(PinName pn);
digitalToggleFast(PinName pn);
```

The standard `digitalWrite(Pin)` calls `digitalWriteFast(pin.toPinName(), val)` internally, so the overhead difference is just the `toPinName()` conversion (constexpr when possible, trivial shift+or when not).

---

## What Was Removed

The old upstream core had six ways to name a pin and three indirection layers. The refactor eliminated:

| Removed | What it was |
|---------|-------------|
| `#define PA0 47` | Arduino pin number macros (variant-specific integers) |
| `digitalPin[]` arrays | PinName lookup by Arduino pin number |
| `analogInputPin[]` arrays | A-number to D-number mapping |
| `pins_arduino_digital.h` | D0..D63 macros (450 lines) |
| `pins_arduino_analog.h` | A0..A63 macros, PNUM_ANALOG_BASE encoding (1120 lines) |
| `analogInputToPinName()` | A-number → D-number → PinName triple indirection |
| `pinNametoDigitalPin()` | Reverse PinName → D-number lookup |

What remains as compatibility shims (in `pins_arduino.h`, consumed by unconverted HAL code):

```cpp
#define digitalPinToPinName(p)   ((PinName)(p))   // pass-through
#define digitalPinToPort(p)      (get_GPIO_Port(STM_PORT(digitalPinToPinName(p))))
#define digitalPinToBitMask(p)   (STM_GPIO_PIN(digitalPinToPinName(p)))
```

These shims are removed as their consumers are converted to Pin API across milestones.
