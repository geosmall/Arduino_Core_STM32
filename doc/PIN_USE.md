# Pin System Reference

How GPIO pins are named, stored, and converted in the refactored STM32 Arduino Core.

### Background: the old pin system

The upstream stm32duino core used Arduino-style pin numbering where each physical pin was assigned an integer index via `#define` macros in variant headers (e.g., `#define PA0 47`). While beginner-friendly, this encourages treating pins as interchangeable integers — code like `for (int pin = 0; pin < 10; pin++) digitalWrite(pin, HIGH)` compiles and runs but sweeps across unrelated GPIO pins, a pattern that makes no sense on a real board and can damage hardware. Mapping between these integers and actual hardware required lookup arrays (`digitalPin[]`, `analogInputPin[]`) and indirection macros (`digitalPinToPinName()`). This created six different ways to refer to the same physical pin (`PA_0`, `PA0`, `D46`, `A0`, `PA0_ALT1`, `0x00`) and a class of silent AF-resolution bugs where the wrong peripheral could be selected for multi-function pins.

This fork targets developers building flight controllers and robotics systems. The integer abstraction hides information on hardware use and can cause silent non-obvious compile time bugs. The refactored core replaces the integer system with two types: a `Pin` struct for user code and the existing `PinName` enum for the HAL layer. See [What Was Removed](#what-was-removed) for the full list of eliminated constructs.

---

## Two Types, One Boundary

The pin system has two types with a single conversion point between them:

| Type | Where | Purpose |
|------|-------|---------|
| `Pin` | User code, Arduino API, BoardConfig, variant headers | Type-safe C++ struct |
| `PinName` | HAL layer, PinMap tables, PeripheralPins.c | ST vendor code currency |

```
User code       HAL boundary         ST HAL / LL drivers
─────────       ────────────         ───────────────────
Pin PA0    ──→  pin.toPinName() ──→  PinName PA_0 = 0x00
```

A `Pin` is a small constexpr struct (port enum + 1-byte pin index). A `PinName` is a uint32_t-sized enum. They encode the same information differently:

```cpp
// Pin (cores/arduino/Pin.h)
struct Pin {
    PortName port;   // PortA=0, PortB=1, ...
    uint8_t  pin;    // 0..15
};
constexpr Pin PA0 = {PortA, 0};

// PinName (cores/arduino/stm32/PinNames.h)
PA_0 = (PortA << 4) | 0   // = 0x00
PB_7 = (PortB << 4) | 7   // = 0x17
```

Conversion is one-way and explicit:

```cpp
constexpr PinName toPinName() const {
    return IsValid() ? (PinName)((port << 4) | pin) : NC;
}
```

### Why both exist

PinName is a `uint32_t` enum with bit-packed fields:

```
PinName bit layout (uint32_t):

  ┌─────────────┬────────┬─────────┬─────────┐
  │  31 ... 11  │ 10 9 8 │ 7 6 5 4 │ 3 2 1 0 │
  │   unused    │  ALT   │  port   │   pin   │
  └─────────────┴────────┴─────────┴─────────┘

  PB_0      = 0x010  →  ALT=0  port=1(B)  pin=0
  PB_0_ALT1 = 0x110  →  ALT=1  port=1(B)  pin=0   ← same physical pin,
  PB_0_ALT2 = 0x210  →  ALT=2  port=1(B)  pin=0      different table entry
```

The ALT bits create separate PinName values for the same physical pin so PinMap tables can list multiple peripheral mappings (e.g., PB0 → TIM1 under `PB_0`, PB0 → TIM3 under `PB_0_ALT1`). PinMap tables need these ALT-encoded values, which means PinName must support integer arithmetic — and integer arithmetic is exactly what caused silent misconfiguration bugs (the "ALT trap": `pinmap_function()` returning the wrong AF with no compiler error).

Pin is a struct specifically to make that arithmetic impossible: `PA0 | ALT1` does not compile. User code names physical pins; it never manipulates ALT encodings. The HAL layer handles ALT resolution internally through `pinmap_function_for_peripheral()`.

The two types cannot be unified without giving up one of these properties — either the HAL loses ALT encoding support, or user code regains the ability to silently corrupt pin values. The explicit `toPinName()` conversion marks every crossing from user-land to HAL-land.

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

Config types are defined in `targets/config/ConfigTypes.h`. All pin fields use `Pin`.

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

When a pin supports multiple peripherals of the same type (e.g., PB0 → TIM1_CH2N and TIM3_CH3), use the peripheral-aware lookup to get the correct AF:

```cpp
// Get AF for a specific timer on a specific pin
uint32_t af = pinmap_function_for_peripheral(pn, TIM3, PinMap_TIM);

// Configure GPIO AF for a specific timer on a specific pin
pinmap_pinout_for_peripheral(pn, TIM3, PinMap_TIM);
```

Query functions in `pinmap.h`:

| Function | Purpose |
|----------|---------|
| `pinmap_peripheral(pn, map)` | Get peripheral instance for a pin |
| `pinmap_function_for_peripheral(pn, periph, map)` | Get AF for a specific peripheral |
| `pinmap_pinout_for_peripheral(pn, periph, map)` | Configure GPIO AF for a specific peripheral |
| `pin_in_pinmap(pn, map)` | Check if pin has any entry in table |

The `_for_peripheral` variants are used by `HardwareTimer::setMode()` and `timer.c::getTimerChannel()`.

#### PeripheralPins.c by Family

| Family | Path |
|--------|------|
| F411RE | `variants/STM32F4xx/F411R(C-E)T/PeripheralPins.c` |
| F405RG | `variants/STM32F4xx/F405RG/PeripheralPins.c` |
| F722RE | `variants/STM32F7xx/F722R(C-E)T/PeripheralPins.c` |
| G473RE | `variants/STM32G4xx/G473R(B-C-E)T/PeripheralPins.c` |
| H743VI | `variants/STM32H7xx/H742V(G-I)(H-T)_H743V(G-I)(H-T)/PeripheralPins.c` |

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

The standard `digitalWrite(Pin)` calls `digitalWriteFast(pin.toPinName(), val)` internally. The `toPinName()` conversion itself is zero-cost (constexpr shift+or), but each `digitalWriteFast()` call still performs two runtime lookups:

- `get_GPIO_Port()` is a macro that expands to an index into `GPIOPort[]` — an `extern GPIO_TypeDef*` array (`PortNames.c`), not const, not constexpr. The compiler cannot constant-fold this.
- `STM_LL_GPIO_PIN()` is a macro that expands to an index into `pin_map_ll[]` — an `extern const uint32_t` array (`pinmap.c`). Const but extern, so also a runtime load.

Total overhead per call: ~4–6 cycles on Cortex-M4 vs a direct `port->BSRR = pin_mask` write. Small and L1-cached, but not zero. For most code this is negligible. For timing-critical paths (bit-banging, high-frequency ISRs), see the caching pattern below.

### Caching Pattern for Performance-Sensitive Drivers

When a driver toggles a GPIO pin in a hot loop or ISR, it can cache the port pointer and LL pin mask at construction time to eliminate the per-call lookups. SoftwareSerial uses this approach for bit-banged UART (caching `_transmitPinPort` and `_transmitPinNumber` in its constructor initializer list). The general pattern:

```cpp
// Header — cache as member variables
class MyDriver {
    GPIO_TypeDef *port_;
    uint32_t ll_pin_;
public:
    void init(Pin pin) {
        PinName pn = pin.toPinName();
        port_   = get_GPIO_Port(STM_PORT(pn));   // resolved once
        ll_pin_ = STM_LL_GPIO_PIN(pn);           // resolved once
    }
    void writeFast(bool state) {
        if (state) LL_GPIO_SetOutputPin(port_, ll_pin_);    // direct BSRR write
        else       LL_GPIO_ResetOutputPin(port_, ll_pin_);
    }
};
```

Each `writeFast()` call compiles to a single store (~2–3 cycles). Use this pattern for bit-banging, chip-select toggling in tight SPI loops, or ISR-driven GPIO. It is unnecessary for init-time configuration or infrequent operations — use `digitalWrite()` for those.

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

---

## Future: GPIO Class

The caching pattern above works but is ad hoc — each driver that needs fast GPIO must declare its own port pointer and pin mask members and resolve them at init time. A future `GPIO` class would formalize this into a reusable type:

```cpp
class GPIO {
public:
    enum class Mode { INPUT, OUTPUT, INPUT_PULLUP, INPUT_PULLDOWN, ANALOG, OPEN_DRAIN };
    enum class Pull { NONE, UP, DOWN };
    enum class Speed { LOW, MEDIUM, HIGH, VERY_HIGH };

    void Init(Pin p, Mode m, Pull pull = Pull::NONE, Speed spd = Speed::HIGH);
    bool Read() const;          // LL_GPIO_IsInputPinSet(port_, ll_pin_)
    void Write(bool state);     // LL_GPIO_Set/ResetOutputPin(port_, ll_pin_)
    void Toggle();              // LL_GPIO_TogglePin(port_, ll_pin_)

private:
    GPIO_TypeDef* port_;        // Cached at Init()
    uint32_t ll_pin_;           // Cached at Init()
};
```

This replaces the manual caching pattern with a single object per pin:

```cpp
// Before: manual caching (2 members, LL boilerplate)
GPIO_TypeDef *_txPort;
uint32_t _txPin;

_txPort = get_GPIO_Port(STM_PORT(pin.toPinName()));
_txPin  = STM_LL_GPIO_PIN(pin.toPinName());

LL_GPIO_SetOutputPin(_txPort, _txPin);

// After: GPIO class (1 member, no LL includes needed)
GPIO _tx;

Pin gpio_pin = PA9;
_tx.Init(gpio_pin, GPIO::Mode::OUTPUT);

_tx.Write(true);   // same register write underneath
```

Complementary to `pinMode()`/`digitalWrite()`, not a replacement. The existing Arduino API continues to work unchanged.

**Status:** Deferred. Current `digitalWrite()` overhead (~4–6 cycles) is negligible for all validated use cases. Consider adding this class when two or more drivers independently implement the manual caching pattern (SoftwareSerial is the first).
