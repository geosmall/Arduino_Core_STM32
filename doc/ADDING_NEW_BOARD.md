# Adding a New Board (Pin Refactor)

How to add a board to the refactored core. The refactored core uses the `Pin` struct instead of D-number macros, so variant headers are much simpler — no `digitalPin[]` arrays, no `#define PA5 13` macros, no ALT pin defines.

## Prerequisites

The variant directory for your chip package must already exist with these Layer 1/2 files (they ship with the core and are never modified):

- `PeripheralPins.c` — generated PinMap arrays (SPI, I2C, UART, TIM, ADC, etc.)
- `PinNamesVar.h` — ALT pin PinName definitions (PA_0_ALT1, etc.)
- `ldscript.ld` — linker script with flash/RAM sizes

Find the right directory under `variants/STM32Fxxx/` or `variants/STM32Gxxx/` by matching your chip's part number pattern.

## Step 1: boards.txt Entry

Add a pnum entry under the appropriate board group. Copy from the old `boards.txt` (in the reference clone) or from `boards_entry.txt` in the variant directory.

```
# NUCLEO_G474RE board
Nucleo_64.menu.pnum.NUCLEO_G474RE=Nucleo G474RE
Nucleo_64.menu.pnum.NUCLEO_G474RE.node=NODE_G474RE
Nucleo_64.menu.pnum.NUCLEO_G474RE.upload.maximum_size=524288
Nucleo_64.menu.pnum.NUCLEO_G474RE.upload.maximum_data_size=131072
Nucleo_64.menu.pnum.NUCLEO_G474RE.build.mcu=cortex-m4
Nucleo_64.menu.pnum.NUCLEO_G474RE.build.fpu=-mfpu=fpv4-sp-d16
Nucleo_64.menu.pnum.NUCLEO_G474RE.build.float-abi=-mfloat-abi=hard
Nucleo_64.menu.pnum.NUCLEO_G474RE.build.board=NUCLEO_G474RE
Nucleo_64.menu.pnum.NUCLEO_G474RE.build.series=STM32G4xx
Nucleo_64.menu.pnum.NUCLEO_G474RE.build.product_line=STM32G474xx
Nucleo_64.menu.pnum.NUCLEO_G474RE.build.variant=STM32G4xx/G473R(B-C-E)T_G474R(B-C-E)T_G483RET_G484RET
```

Key fields:
- **`build.board`** — becomes `ARDUINO_NUCLEO_G474RE` (the `#if defined(...)` guard in variant .cpp)
- **`build.series`** — selects HAL family headers (STM32F4xx, STM32G4xx, etc.)
- **`build.product_line`** — selects specific chip CMSIS headers
- **`build.variant`** — path under `variants/` to the chip package directory

The group header (e.g., `Nucleo_64`) must already exist in boards.txt with its build defaults and upload methods. The pnum entry inherits from it.

## Step 2: Variant Header

Create `variant_YOURBOARD.h` in the variant directory. This replaces the old 200+ line header with a minimal one — no D-number macros, no ALT defines, no pin arrays.

```cpp
#pragma once

// On-board LED
#ifndef LED_BUILTIN
  #define LED_BUILTIN             PA5
#endif

// On-board user button
#ifndef USER_BTN
  #define USER_BTN                PC13
#endif

// Timer definitions — pick timers not used by PWM/motor outputs
#ifndef TIMER_TONE
  #define TIMER_TONE              TIM6    // Basic timer (no GPIO), safe default
#endif
#ifndef TIMER_SERVO
  #define TIMER_SERVO             TIM7    // Basic timer (no GPIO), safe default
#endif

// SPI Definitions — default SPI pins (usually SPI1 on Arduino connector)
#ifndef PIN_SPI_MOSI
  #define PIN_SPI_MOSI            PA7
#endif
#ifndef PIN_SPI_MISO
  #define PIN_SPI_MISO            PA6
#endif
#ifndef PIN_SPI_SCK
  #define PIN_SPI_SCK             PA5
#endif

// UART Definitions
#ifndef SERIAL_UART_INSTANCE
  #define SERIAL_UART_INSTANCE    101     // LPUART1 for G4 Nucleo; use 2 for F4 (USART2)
#endif

// Default pin used for generic 'Serial' instance
// NOTE: Must be PinName values (PA_2, not PA2) — uart.c is a C file that
// uses these via digitalPinToPinName(), which needs a PinName enum, not
// a Pin constexpr
#ifndef PIN_SERIAL_RX
  #define PIN_SERIAL_RX           PA_3
#endif
#ifndef PIN_SERIAL_TX
  #define PIN_SERIAL_TX           PA_2
#endif

// Required by core — count of physical pins on the package
#ifndef NUM_DIGITAL_PINS
  #define NUM_DIGITAL_PINS        51
#endif
#ifndef NUM_ANALOG_INPUTS
  #define NUM_ANALOG_INPUTS       13
#endif

// Extra HAL modules (chip-specific, check the old variant header)
#if !defined(HAL_DAC_MODULE_DISABLED)
  #define HAL_DAC_MODULE_ENABLED
#endif

#ifdef __cplusplus
  #ifndef SERIAL_PORT_MONITOR
    #define SERIAL_PORT_MONITOR   Serial
  #endif
  #ifndef SERIAL_PORT_HARDWARE
    #define SERIAL_PORT_HARDWARE  Serial
  #endif
#endif
```

### Important rules

- **Cannot include Pin.h.** The variant header is pulled in early via the HAL conf chain. Pin names like `PA5` are macros here — they resolve to `constexpr Pin` values later when user code includes `Arduino.h`.
- **PIN_SERIAL_RX/TX use PinName values** (`PA_3`, not `PA3`). `uart.c` is a C file that passes these through `digitalPinToPinName()` — Pin constexprs don't exist in C.
- **PIN_SPI_MOSI/MISO/SCK use Pin-style names** (`PA7`, not `PA_7`). These are consumed by `SPI.cpp` which calls `.toPinName()`.
- **All defines are `#ifndef` guarded** so BoardConfig targets or sketches can override.

### Where to find the right values

| Define | Source |
|--------|--------|
| LED_BUILTIN, USER_BTN | Board schematic or old variant header |
| TIMER_TONE, TIMER_SERVO | Old variant header. Prefer basic timers (TIM6/TIM7) that have no GPIO |
| SERIAL_UART_INSTANCE | Old variant header. Nucleo F4: `2` (USART2). Nucleo G4: `101` (LPUART1) |
| PIN_SERIAL_RX/TX | Old variant header — pins connected to ST-Link VCOM |
| PIN_SPI_* | Datasheet or `PeripheralPins.c` — find SPI1 entries matching Arduino header pins |
| NUM_DIGITAL_PINS | Old variant header |
| NUM_ANALOG_INPUTS | Old variant header |
| HAL modules | Old variant header — copy any `HAL_*_MODULE_ENABLED` defines |

## Step 3: Variant .cpp

Create `variant_YOURBOARD.cpp` — just `SystemClock_Config()`. Copy from the old variant .cpp but remove the `digitalPin[]` and `analogInputPin[]` arrays.

```cpp
#if defined(ARDUINO_NUCLEO_G474RE)

#include "stm32_def.h"

#ifdef __cplusplus
extern "C" {
#endif

WEAK void SystemClock_Config(void)
{
  // ... clock config from old variant or STM32CubeMX ...
}

#ifdef __cplusplus
}
#endif

#endif /* ARDUINO_NUCLEO_G474RE */
```

### Important rules

- **Include `stm32_def.h`**, not `pins_arduino.h`. The .cpp only needs HAL types for clock config. Including `pins_arduino.h` pulls in `_Static_assert` checks that fail before the HAL headers are loaded.
- **Guard with `ARDUINO_YOURBOARD`** — this is `ARDUINO_` + `build.board` from boards.txt.
- **Keep `WEAK`** — allows sketches to override clock config.
- **Delete `digitalPin[]` and `analogInputPin[]`** — these are not used in the refactored core.

## Step 4: Verify

```bash
# Compile a basic sketch
arduino-cli compile \
  --fqbn STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_G474RE \
  sketches/PinWiggle

# Check for warnings
arduino-cli compile --warnings all \
  --fqbn STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_G474RE \
  sketches/PinWiggle
```

Zero warnings, zero errors before proceeding to hardware test.

## What you DON'T need to create

These already exist in the variant directory and are unchanged:

- `PeripheralPins.c` — pin-to-peripheral mappings (Layer 1, auto-generated)
- `PinNamesVar.h` — ALT PinName definitions (Layer 1)
- `ldscript.ld` — linker memory layout (Layer 1)
- `variant_generic.h/.cpp` — unused by named boards but kept as reference

## Checklist

- [ ] `boards.txt` pnum entry added under correct group
- [ ] `variant_YOURBOARD.h` created with Pin-style defines
- [ ] `variant_YOURBOARD.cpp` created with `SystemClock_Config()` only
- [ ] `variant_YOURBOARD.cpp` includes `stm32_def.h` (NOT `pins_arduino.h`)
- [ ] Compiles with zero warnings
- [ ] Hardware tested (LED blink, Serial output, SPI if applicable)
