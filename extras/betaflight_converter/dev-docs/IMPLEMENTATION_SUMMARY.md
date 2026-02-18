# Betaflight Target Config Converter - Implementation Summary

## Project Status: ✅ COMPLETE

Python-based converter that transforms native Betaflight target `config.h` files into Arduino STM32 BoardConfig headers with PeripheralPins.c cross-validation.

## Implementation Metrics

- **Source Code:** ~1950 lines (Python)
- **Test Code:** ~920 lines (Python)
- **Total Tests:** 62 tests (100% passing)
- **Test Coverage:** All components (parser, peripheral pins, validator, generator)
- **Supported Targets:** 6 boards across 5 MCU families
- **Validation Method:** Arduino Core STM32 PeripheralPins.c cross-validation

## Architecture

### 1. PeripheralPins Parser (`src/peripheral_pins.py`, 412 lines)
**Purpose:** Parse Arduino Core STM32 PeripheralPins.c files for validation

**Key Features:**
- Extracts timer/AF/channel mappings from PinMap_TIM
- Extracts SPI/I2C/UART bus assignments
- Handles ALT pin variants (PB_0_ALT1, etc.)
- Provides validation methods for pin assignments

**Tests:** 15 tests covering:
- Timer/SPI/I2C/UART parsing
- JHEF411 hardware validation
- Bus validation methods

### 2. Betaflight Config Parser (`src/betaflight_config.py`, 485 lines)
**Purpose:** Parse native Betaflight `config.h` files (`#define` format)

**Key Features:**
- Parses `#define` statements for pins, settings, and board metadata
- Extracts `TIMER_PIN_MAP(index, pin, occurrence, dma)` entries
- Resolves timer occurrences against PeripheralPins.c entries
- Maps SDI/SDO naming to MISO/MOSI internally
- Pins stored in native Arduino format (PA8, PB0) — no conversion needed

**Parsing Examples:**
```c
#define FC_TARGET_MCU     STM32F411     → mcu_type = 'STM32F411'
#define BOARD_NAME        JHEF411       → board_name = 'JHEF411'
#define MOTOR1_PIN        PA8           → ResourcePin('MOTOR', 1, 'PA8')
#define SPI1_SDI_PIN      PA6           → ResourcePin('SPI_MISO', 1, 'PA6')
#define GYRO_1_SPI_INSTANCE SPI1        → settings['gyro_1_spibus'] = '1'
TIMER_PIN_MAP(4, PB0, 2, 0)            → timer_pin_map['PB0'] = 2
```

**Tests:** 26 tests covering:
- `#define` parsing for all resource types
- Timer occurrence resolution (3 dedicated tests)
- Multi-target validation (MATEKH743, NERO, REVO)
- Pin format identity (no conversion)
- Settings mapping (SPI instances, scales, protocols)

### 3. Configuration Validator (`src/validator.py`, 397 lines)
**Purpose:** Cross-validate Betaflight config against PeripheralPins.c

**Key Features:**
- Validates motor timer/AF assignments
- Validates SPI/I2C/UART bus pin assignments
- Groups motors by timer banks
- Generates validation summary with errors/warnings

**Tests:** 8 tests covering:
- Complete validation workflow
- Motor validation and grouping
- Bus validation (SPI, I2C, UART)
- Error-free JHEF411 validation

### 4. Code Generator (`src/code_generator.py`, 448 lines)
**Purpose:** Generate C++ BoardConfig header files

**Key Features:**
- Generates header with board metadata and gyro types
- StorageConfig (SPI flash with LITTLEFS or SD card with SDFS)
- IMUConfig with SPI and interrupt pin
- I2CConfig for environmental sensors
- UARTConfig for serial ports
- ADCConfig for battery monitoring
- LEDConfig for status LEDs
- RCReceiverConfig for RC input
- Motor namespace with flat `motors[]` array and `num_motors`
- Servo namespace with flat `servos[]` array and `num_servos`

**Output Format:**
```cpp
namespace BoardConfig {
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB15, PB14, PB13, PB2, 8000000};
  static constexpr IMUConfig imu{imu_spi, PB3, 1000000};
  static constexpr UARTConfig uart1{PB6, PB7, 115200};

  namespace Motor {
    static constexpr uint32_t frequency_hz = 2000;
    static constexpr MotorConfig motors[] = {
      {TIM1, PA8, 1, 125, 250},       // Motor 1: TIM1_CH1
      {TIM3, PB0_ALT1, 3, 125, 250},  // Motor 4: TIM3_CH3
    };
    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}
```

**Tests:** 13 tests covering:
- Complete header generation
- Individual peripheral generation (storage, IMU, I2C, UART, ADC, motors, servos)
- C++ syntax validation
- File saving

### 5. Main Converter (`convert.py`, 208 lines)
**Purpose:** Command-line converter tool

**Usage:**
```bash
python3 convert.py bf_configs/JHEF411/              # Convert single target directory
python3 convert.py bf_configs/JHEF411/config.h      # Convert single config.h file
python3 convert.py bf_configs/JHEF411/ output/X.h   # Custom output path
python3 convert.py --all                             # Convert all targets in bf_configs/
python3 convert.py --all --force                     # Skip validation errors
```

**Workflow:**
1. Load Betaflight config.h (accepts directory or file path)
2. Auto-detect MCU variant and locate PeripheralPins.c
3. Resolve timer occurrences against PeripheralPins.c
4. Validate complete configuration
5. Generate BoardConfig header (named `MANUFACTURER-BOARD.h`)
6. Report validation summary

## Supported MCU Families

| MCU | Variant Path | Example Board |
|-----|-------------|---------------|
| STM32F411 | STM32F4xx/F411C(C-E)(U-Y) | JHEF411 |
| STM32F405 | STM32F4xx/F405RGT_F415RGT | REVO |
| STM32F7X2 | STM32F7xx/F722Z(C-E)T_F732ZET | NERO |
| STM32H743 | STM32H7xx/H742V...H753VI(H-T) | MATEKH743 |
| STM32G47X | STM32G4xx/G473C(B-C-E)U...G484CEU | BETAFPVG473 |

## Validation Results

### All 6 Targets ✅

```bash
python3 convert.py --all
# ✅ JHEF411 (STM32F411) - 5 motors, 0 servos
# ✅ REVO (STM32F405) - 6 motors, 0 servos
# ✅ NERO (STM32F7X2) - 8 motors, 0 servos
# ✅ MATEKH743 (STM32H743) - 8 motors, 2 servos
# ✅ BLACKPILL_F411CE (STM32F411) - 4 motors, 0 servos
# ✅ BETAFPVG473 (STM32G47X) - 4 motors, 0 servos
```

All targets: 0 errors, 0 warnings.

## Key Technical Solutions

### 1. Timer Occurrence Resolution
**Problem:** Betaflight `TIMER_PIN_MAP(index, pin, occurrence, dma)` uses an occurrence-based index to select which timer option a pin uses.

**Solution:** The occurrence value is a 1-based index into PeripheralPins.c's `PinMap_TIM` entries for that pin (filtered to non-complementary channels):
```python
def resolve_timers(self, pinmap):
    for pin, occurrence in self.timer_pin_map.items():
        entries = pinmap.get_timer_entries(pin)  # All PinMap_TIM entries for pin
        non_complementary = [e for e in entries if not e.complementary]
        selected = non_complementary[occurrence - 1]  # 1-based index
        self.timers[pin] = TimerAssignment(pin, selected.af, selected.timer, selected.channel)
```

### 2. ALT Pin Variant Selection
**Problem:** When occurrence > 1, the selected PeripheralPins.c entry uses an ALT variant (e.g., `PB_0_ALT1` for TIM3 instead of default `PB_0` for TIM1).

**Solution:** The converter preserves the ALT suffix from PeripheralPins.c and converts to Arduino macro format:
- `PB_0_ALT1` (PinName) → `PB0_ALT1` (Arduino macro)
- Result: `{TIM3, PB0_ALT1, 3, 125, 250}` instead of incorrect `{TIM3, PB0, 3, 125, 250}`

### 3. ConfigTypes.h Include Path
**Solution:** Relative path from `output/` to canonical source:
```cpp
#include "../../../targets/config/ConfigTypes.h"
```
Three levels up from `extras/betaflight_converter/output/` reaches the Arduino Core root.

### 4. Automatic MCU Variant Detection
**Solution:** `MCU_TO_VARIANTS` dict maps Betaflight MCU types to variant path candidates (tried in order):
```python
MCU_TO_VARIANTS = {
    'STM32F411': ['STM32F4xx/F411C(C-E)(U-Y)'],
    'STM32F405': ['STM32F4xx/F405RGT_F415RGT', ...],
    'STM32H743': ['STM32H7xx/H742V...', ...],
    'STM32G47X': ['STM32G4xx/G473C(B-C-E)U...', ...],
}
```

### 5. SDI/SDO to MISO/MOSI Mapping
**Problem:** Betaflight uses `SPI1_SDI_PIN` / `SPI1_SDO_PIN` naming.

**Solution:** Mapped internally: SDI → MISO, SDO → MOSI. Stored as `SPI_MISO` / `SPI_MOSI` resource types for validator/generator compatibility.

## Test Suite

**Total Tests:** 62 (100% passing)

| Module | Tests | Coverage |
|--------|-------|----------|
| PeripheralPins parser | 15 | Timer, SPI, I2C, UART parsing + validation |
| Betaflight config parser | 26 | #define parsing, timer resolution, multi-target |
| Validator | 8 | Motor/SPI/I2C/UART validation, grouping |
| Code generator | 13 | All peripherals, servos, C++ syntax, file I/O |

```bash
python3 -m unittest discover -s tests -v
# Ran 62 tests in 0.013s — OK
```

## File Structure

### Source (`src/`)
| File | Lines | Purpose |
|------|-------|---------|
| `peripheral_pins.py` | 412 | PeripheralPins.c parser |
| `betaflight_config.py` | 485 | Betaflight config.h parser |
| `validator.py` | 397 | Configuration cross-validator |
| `code_generator.py` | 448 | C++ BoardConfig generator |

### Tests (`tests/`)
| File | Lines | Tests |
|------|-------|-------|
| `test_peripheral_pins.py` | 208 | 15 |
| `test_betaflight_config.py` | 277 | 26 |
| `test_validator.py` | 167 | 8 |
| `test_code_generator.py` | 264 | 13 |

### Input (`bf_configs/`)
6 native Betaflight target directories, each containing `config.h`.

### Output (`output/`)
6 generated BoardConfig headers: JHEF-JHEF411.h, OPEN-REVO.h, BKMN-NERO.h, MTKS-MATEKH743.h, WACT-BLACKPILL_F411CE.h, BEFH-BETAFPVG473.h.

### Documentation (`dev-docs/`)
- `RESEARCH.md` — Betaflight target format reference
- `MOTOR_CONFIG_DESIGN.md` — Motor→TimerPWM integration design
- `COMPARISON.md` — Generated vs input side-by-side analysis
- `IMPLEMENTATION_SUMMARY.md` — This document
