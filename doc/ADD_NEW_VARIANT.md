# Adding a New Board Variant

How to add a new STM32 board to the refactored core. This guide assumes the chip package's variant directory already exists with Layer 1 files.

---

## Prerequisites

Identify your board's chip package directory under `variants/`. For example, an F722-based board uses `variants/STM32F7xx/F722R(C-E)T_F730R8T_F732RET/`.

The directory must already contain these Layer 1 files (ST vendor data, never modified):

| File | Purpose |
|------|---------|
| `PeripheralPins.c` | Pin-to-peripheral mapping tables |
| `PinNamesVar.h` | ALT pin definitions for the chip package |
| `ldscript.ld` | Linker script (flash/RAM layout) |

If these don't exist, the chip package hasn't been added yet — that's a larger task outside this guide.

---

## Steps

### 1. Create `variant_MFGR_BOARD.h`

Bare `#define` macros only. **Do not `#include` any header** — this file is included from the HAL conf chain before Pin.h is available. Pin-style names (`PA5`, not `PA_5`) resolve to Pin constexpr values later when evaluated in user code.

Required defines:

```cpp
#pragma once

// On-board LED and button (use PNUM_NOT_DEFINED if absent)
#define LED_BUILTIN             PA5
#define USER_BTN                PC13

// Timer assignments (TIM6/TIM7 preferred when available)
#define TIMER_TONE              TIM10
#define TIMER_SERVO             TIM11

// Default SPI bus
#define PIN_SPI_MOSI            PA7
#define PIN_SPI_MISO            PA6
#define PIN_SPI_SCK             PA5

// Debug UART
#define SERIAL_UART_INSTANCE    2
#define PIN_SERIAL_RX           PA3
#define PIN_SERIAL_TX           PA2

// Default I2C bus
#define PIN_WIRE_SDA            PB9
#define PIN_WIRE_SCL            PB8

// HSE crystal (omit for HSI-only boards)
#define HSE_VALUE               8000000U

// Required by core
#define NUM_DIGITAL_PINS        52
#define NUM_ANALOG_INPUTS       16

// UF2 bootloader (if applicable)
#define BOOTUF2_DBL_TAP_MAGIC   0xF01669EFUL
#define BOOTUF2_DBL_TAP_ADDR    0x2001FFF0UL

// Extra HAL modules (if needed beyond defaults)
// #define HAL_DAC_MODULE_ENABLED

// C++ only
#ifdef __cplusplus
  #define SERIAL_PORT_MONITOR   Serial
  #define SERIAL_PORT_HARDWARE  Serial
#endif
```

**Do not** define D-number macros (`#define PA0 47`), ALT pins, or `digitalPin[]` arrays.

### 2. Create `variant_MFGR_BOARD.cpp`

Only `SystemClock_Config()` — no pin arrays:

```cpp
#if defined(ARDUINO_MFGR_BOARD)

#include "pins_arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

WEAK void SystemClock_Config(void)
{
  // PLL configuration — copy from datasheet, CubeMX, or old core's
  // variant .cpp / clock_*.c file. Verify PLL math matches HSE_VALUE.
}

#ifdef __cplusplus
}
#endif

#endif /* ARDUINO_MFGR_BOARD */
```

### 3. Remove old clock file (if present)

Delete any `clock_MFGR_BOARD.c` in the variant directory — these are old-core files superseded by the new .cpp. (Their `#if defined(ARDUINO_MFGR_BOARD)` guards would match and fail to compile.)

Note: `generic_clock.c` files are fine to leave — their `#if defined(ARDUINO_GENERIC_*)` guards never match our boards.

### 4. Add `boards.txt` pnum entry

Place in the correct group header (`Nucleo_144`, `Nucleo_64`, `Nucleo_32`, `GenF4`, `GenF7`, `GenG4`, `GenH7`, `FlightCtr`):

```
FlightCtr.menu.pnum.MFGR_BOARD=Board Name (Manufacturer)
FlightCtr.menu.pnum.MFGR_BOARD.upload.maximum_size=458752
FlightCtr.menu.pnum.MFGR_BOARD.upload.maximum_data_size=262144
FlightCtr.menu.pnum.MFGR_BOARD.build.mcu=cortex-m7
FlightCtr.menu.pnum.MFGR_BOARD.build.series=STM32F7xx
FlightCtr.menu.pnum.MFGR_BOARD.build.board=MFGR_BOARD
FlightCtr.menu.pnum.MFGR_BOARD.build.product_line=STM32F722xx
FlightCtr.menu.pnum.MFGR_BOARD.build.variant=STM32F7xx/F722R(C-E)T_F730R8T_F732RET
FlightCtr.menu.pnum.MFGR_BOARD.build.variant_h=variant_MFGR_BOARD.h
```

Key fields:
- `upload.maximum_size` / `maximum_data_size` — flash and RAM from datasheet (subtract bootloader region if UF2)
- `build.variant` — path to the chip package directory (relative to `variants/`)
- `build.variant_h` — your variant header filename
- `build.board` — must match the `ARDUINO_MFGR_BOARD` guard in your .cpp
- H7 boards: add `build.st_extra_flags=-DCORE_CM7`

For UF2 bootloader boards, also add:
```
FlightCtr.menu.pnum.MFGR_BOARD.build.uf2_family=0x...
FlightCtr.menu.pnum.MFGR_BOARD.build.uf2_flash_offset=0x10000
FlightCtr.menu.pnum.MFGR_BOARD.build.bootloader_bin=bootuf2-board-v2.0.0.bin
```

### 5. Create BoardConfig target (flight controller boards only)

`targets/MFGR-BOARD.h` — compile-time pin configuration using Pin constants:

```cpp
#pragma once
#include "config/ConfigTypes.h"

namespace BoardConfig {
  static constexpr StorageConfig storage{...};
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PC4, 8000000};
  static constexpr IMUConfig imu{imu_spi, PB2, 1000000};
  // ...
}
```

Config types are defined in `targets/config/ConfigTypes.h`. Dev boards (Nucleo, WeAct, etc.) used for generic testing don't need BoardConfig targets.

### 6. Verify

```bash
# Compile-check
./ci/build.sh sketches/PinWiggle <FQBN>

# HIL test (if hardware available)
./ci/saflash.sh tests/<test_sketch> <FQBN>
```

---

## Reference

- Existing variant example: `variants/STM32F4xx/F411R(C-E)T/variant_NUCLEO_F411RE.h`
- BoardConfig example: `targets/BKMN-NERO.h`
- Pin system docs: `doc/PIN_USE.md`
- Unused variant directories contain `variant_generic.h/.cpp` with old-style pin definitions. These won't compile against the refactored core but are useful as reference for pin mappings and clock config.
