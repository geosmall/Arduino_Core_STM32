# STM32 Pin Mapping Pitfalls

## The ALT Pin Trap: `pinmap_function()` Returns Wrong AF

### Problem

`pinmap_function(pin, PinMap_TIM)` returns the **first** match for a pin in `PeripheralPins.c`. When a pin has multiple timer functions, the second (and subsequent) functions are listed under ALT pin names (`PB_0_ALT1`, `PB_0_ALT2`, etc.) — separate PinName values that `digitalPinToPinName()` never returns.

Example from `variants/STM32F4xx/F411R(C-E)T/PeripheralPins.c`:

```c
{PB_0,      TIM1, STM_PIN_DATA_EXT(..., GPIO_AF1_TIM1, 2, 1)}, // TIM1_CH2N  ← first match
{PB_0_ALT1, TIM3, STM_PIN_DATA_EXT(..., GPIO_AF2_TIM3, 3, 0)}, // TIM3_CH3   ← unreachable via PB_0
```

Any code that does `pinmap_function(digitalPinToPinName(PB0), PinMap_TIM)` gets AF1 (TIM1_CH2N), even when it intended TIM3_CH3. The GPIO is configured for the wrong timer and produces no output.

### Symptom

Timer output appears dead on a pin that should work. No signal, no edges. The timer is running, DMA is transferring, but the pin's alternate function routes to a different timer than intended.

### Fix Pattern

When you know which timer you want, iterate `PinMap_TIM` matching both the physical pin AND the timer peripheral:

```cpp
const PinMap *map = PinMap_TIM;
uint32_t function = 0;
while (map->pin != NC) {
    if (map->peripheral == timer &&
        STM_PORT(map->pin) == STM_PORT(pin_name) &&
        STM_PIN(map->pin) == STM_PIN(pin_name)) {
        function = map->function;
        break;
    }
    map++;
}
```

`STM_PORT()` and `STM_PIN()` mask off the ALT bits, so this matches both `PB_0` and `PB_0_ALT1` entries for the same physical pin.

### Where This Is Already Fixed

- `DShot_ll.cpp::initGPIO()` — iterates PinMap_TIM matching pin + timer

### How to Diagnose

1. Check `PeripheralPins.c` for the target variant — grep for the pin name (e.g., `PB_0`)
2. Look for `_ALT` variants — if the desired timer is under an ALT name, `pinmap_function()` won't find it
3. The first entry listed for the base pin name is what `pinmap_function()` returns

### Files to Check

| Family | PeripheralPins.c path |
|--------|----------------------|
| F411RE | `variants/STM32F4xx/F411R(C-E)T/PeripheralPins.c` |
| F405RG | `variants/STM32F4xx/F405RG/PeripheralPins.c` |
| F722RE | `variants/STM32F7xx/F722R(C-E)T/PeripheralPins.c` |
| G473RE | `variants/STM32G4xx/G473R(B-C-E)T/PeripheralPins.c` |
| H743VI | `variants/STM32H7xx/H742V(G-I)(H-T)_H743V(G-I)(H-T)/PeripheralPins.c` |

Use `Glob` with `**/PeripheralPins.c` if unsure of the exact path for a variant.
