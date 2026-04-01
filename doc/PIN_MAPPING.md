# STM32 Pin Mapping Pitfalls

## The ALT Pin Trap: `pinmap_function()` Returns Wrong AF

### Problem

`pinmap_function(pin, PinMap_TIM)` returns the **first** match for a pin in `PeripheralPins.c`. When a pin has multiple timer functions, the second (and subsequent) functions are listed under ALT pin names (`PB_0_ALT1`, `PB_0_ALT2`, etc.) — separate PinName values that neither `Pin::toPinName()` nor the compatibility shim `digitalPinToPinName()` ever produce.

Example from `variants/STM32F4xx/F411R(C-E)T/PeripheralPins.c`:

```c
{PB_0,      TIM1, STM_PIN_DATA_EXT(..., GPIO_AF1_TIM1, 2, 1)}, // TIM1_CH2N  ← first match
{PB_0_ALT1, TIM3, STM_PIN_DATA_EXT(..., GPIO_AF2_TIM3, 3, 0)}, // TIM3_CH3   ← unreachable via PB_0
```

Calling `pinmap_function(PB0.toPinName(), PinMap_TIM)` gets AF1 (TIM1_CH2N), even when TIM3_CH3 was intended. The GPIO is configured for the wrong timer and produces no output.

### Symptom

Timer output appears dead on a pin that should work. No signal, no edges. The timer is running, DMA is transferring, but the pin's alternate function routes to a different timer than intended.

### Fix: Peripheral-Aware Lookup

Use `pinmap_function_for_peripheral()` or `pinmap_pinout_for_peripheral()` instead of the bare `pinmap_function()`. These match both the physical pin AND the target peripheral, skipping ALT boundaries automatically:

```cpp
// Get AF for a specific timer on a specific pin
PinName pn = pin.toPinName();
uint32_t function = pinmap_function_for_peripheral(pn, TIM3, PinMap_TIM);

// Configure GPIO AF for a specific timer on a specific pin
pinmap_pinout_for_peripheral(pn, TIM3, PinMap_TIM);
```

Internally these iterate the PinMap table using `STM_PORT()` / `STM_PIN()` to mask off ALT bits, so they match `PB_0_ALT1` entries when the peripheral matches — without the caller ever needing ALT-encoded PinName values.

Declared in `cores/arduino/stm32/pinmap.h`, implemented in `pinmap.c`.

### Where This Is Used

- `HardwareTimer::setMode()` — uses `pinmap_pinout_for_peripheral()` for GPIO AF setup and `pinmap_function_for_peripheral()` for complementary channel detection
- `timer.c::getTimerChannel()` — uses `pinmap_function_for_peripheral()` to resolve channel + AF for a specific timer instance

**Legacy manual iteration** (pre-M3): `DShot_ll.cpp::initGPIO()` still iterates PinMap_TIM directly. Can be migrated to the API above.

### How to Diagnose

1. Check `PeripheralPins.c` for the target variant — grep for the pin name (e.g., `PB_0`)
2. Look for `_ALT` variants — if the desired timer is under an ALT name, bare `pinmap_function()` won't find it
3. The first entry listed for the base pin name is what `pinmap_function()` returns
4. If using `pinmap_function_for_peripheral()`, the ALT issue is handled — verify the peripheral pointer is correct instead

### Files to Check

| Family | PeripheralPins.c path |
|--------|----------------------|
| F411RE | `variants/STM32F4xx/F411R(C-E)T/PeripheralPins.c` |
| F405RG | `variants/STM32F4xx/F405RG/PeripheralPins.c` |
| F722RE | `variants/STM32F7xx/F722R(C-E)T/PeripheralPins.c` |
| G473RE | `variants/STM32G4xx/G473R(B-C-E)T/PeripheralPins.c` |
| H743VI | `variants/STM32H7xx/H742V(G-I)(H-T)_H743V(G-I)(H-T)/PeripheralPins.c` |

Use `Glob` with `**/PeripheralPins.c` if unsure of the exact path for a variant.
