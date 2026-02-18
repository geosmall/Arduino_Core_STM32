# Betaflight to BoardConfig Converter

## Overview

Python tool that automatically converts native Betaflight target `config.h` files into Arduino STM32 BoardConfig headers with comprehensive validation.

**Status**: Production Ready

**Cross-Platform**: Works on Windows, macOS, and Linux with Python 3.7+

## Quick Start

### Requirements
- Python 3.7+
- Arduino Core STM32 (for PeripheralPins.c validation)

### Adding a New Target

1. Copy the target directory from `betaflight/src/config/configs/<TARGET>/` to `bf_configs/`
2. Run the converter:

```bash
python3 convert.py bf_configs/<TARGET>/
```

If the MCU family is not yet supported, the converter will print an error listing
supported MCUs. To add a new MCU family:

1. Find the matching `PeripheralPins.c` under `variants/STM32xxxx/`
2. Add the MCU type and variant path(s) to `MCU_TO_VARIANTS` in `convert.py`
3. Re-run the converter

### Usage

```bash
# Convert a single target directory
python3 convert.py bf_configs/JHEF411/

# Convert a single config.h file
python3 convert.py bf_configs/JHEF411/config.h

# Custom output filename
python3 convert.py bf_configs/JHEF411/ output/CUSTOM_NAME.h

# Convert all targets in bf_configs/
python3 convert.py --all

# Force generation even if validation fails
python3 convert.py bf_configs/JHEF411/ --force
```

### Example Output
```
Loading Betaflight config: bf_configs/JHEF411
  Board: JHEF411
  Manufacturer: JHEF
  MCU: STM32F411
Loading PeripheralPins.c: variants/STM32F4xx/F411C(C-E)(U-Y)/PeripheralPins.c
  Resolved 10 timer assignments

Validating configuration...
Validation passed
Validation Summary:
  Errors: 0
  Warnings: 0

Generating BoardConfig: output/JHEF-JHEF411.h
Successfully generated: output/JHEF-JHEF411.h
```

### Testing

```bash
# Run tests with unittest
python3 -m unittest discover -s tests -v
# Expected: 62 tests passing

# Or with pytest (if installed)
pytest -v
```

## Implementation Status

**Completed**: Full converter with validation
- Betaflight config.h parser (26 tests)
- PeripheralPins.c parser (15 tests)
- Configuration validator (8 tests)
- C++ code generator (13 tests)
- **Total: 62 tests, 100% passing**

---

## Architecture

### Parser -> Validator -> Generator Pipeline

1. **BetaflightConfig Parser** (`src/betaflight_config.py`)
   - Parses native Betaflight `config.h` files (`#define` format)
   - Extracts motors, servos, SPI buses, I2C, UARTs, ADC
   - Resolves `TIMER_PIN_MAP` occurrences against PeripheralPins.c
   - Pins are already in Arduino macro format (PA8, PB0)

2. **PeripheralPinMap Parser** (`src/peripheral_pins.py`)
   - Parses Arduino Core STM32 `PeripheralPins.c` files
   - Provides authoritative pin->peripheral mappings
   - Validates timer/AF, SPI, I2C, UART assignments

3. **ConfigValidator** (`src/validator.py`)
   - Cross-validates Betaflight config against PeripheralPins.c
   - Detects pin conflicts and invalid assignments
   - Groups motors by timer banks

4. **BoardConfigGenerator** (`src/code_generator.py`)
   - Generates C++ BoardConfig headers
   - Creates Storage, IMU, I2C, UART, ADC, Motor namespaces
   - Outputs compile-time configuration

### Config Format

The converter parses native Betaflight `config.h` files which use `#define` statements:

```c
#define FC_TARGET_MCU  STM32F411
#define BOARD_NAME     JHEF411
#define MOTOR1_PIN     PA8
#define SPI1_SDI_PIN   PA6        // SDI = MISO
#define SPI1_SDO_PIN   PA7        // SDO = MOSI
#define GYRO_1_SPI_INSTANCE SPI1
#define TIMER_PIN_MAP(...) ...    // Occurrence-based timer selection
```

Timer resolution uses the `TIMER_PIN_MAP` occurrence parameter as a 1-based index into
PeripheralPins.c's `PinMap_TIM` entries for each pin.

---

## Pin Format Architecture

### Two Pin Systems in STM32 Arduino Core

The STM32 Arduino Core uses two different pin identification systems:

1. **PinName Enums** (`PA_0`, `PB_15`, `PC_12`) - HAL/Low-level format
   - Defined in `cores/arduino/stm32/PinNames.h`
   - Used internally by STM32 HAL drivers
   - Enum values like `PA_0 = 0x00`, `PB_15 = 0x1F`

2. **Arduino Pin Macros** (`PA0`, `PB15`, `PC12`) - Arduino API format
   - Defined in variant headers (e.g., `variant_NUCLEO_F411RE.h`)
   - Integer constants: `#define PA0  0`, `#define PC12  17`
   - Used by all Arduino APIs and library constructors

### Why Arduino Macros Are Used

**All STM32 Arduino libraries expect `uint32_t` pin numbers (Arduino macros), NOT PinName enums:**

```cpp
// Library constructors accept uint32_t
SPIClass::SPIClass(uint32_t mosi, uint32_t miso, uint32_t sclk);
HardwareSerial::HardwareSerial(uint32_t rx, uint32_t tx);

// Internally, libraries convert to PinName enums using digitalPinToPinName()
SPIClass::SPIClass(uint32_t mosi, ...) {
  _spi.pin_mosi = digitalPinToPinName(mosi);  // uint32_t -> PinName enum
}
```

### Correct Usage in Generated Configs

**This converter generates Arduino macros (no underscore):**

```cpp
// CORRECT - Arduino macro format (this converter)
StorageConfig storage{StorageBackend::LITTLEFS, PB15, PB14, PB13, PB2, 8000000};

// WRONG - PinName enum format (would fail)
StorageConfig storage{StorageBackend::LITTLEFS, PB_15, PB_14, PB_13, PB_2, 8000000};
```

### ALT Pin Variants for Peripheral Selection

Some pins support multiple peripheral instances. ALT variants select the correct one:

**Timer/Motor Example - Motor 4 on JHEF411:**

```cpp
// PeripheralPins.c shows PB_0 has TWO timer mappings:
{PB_0,      TIM1, ..._AF1_TIM1, 2, 1}    // Default: TIM1_CH2N (AF1)
{PB_0_ALT1, TIM3, ..._AF2_TIM3, 3, 0}    // ALT1: TIM3_CH3 (AF2)

// TIMER_PIN_MAP specifies occurrence=2 for PB0 -> selects TIM3_CH3
// Generated config uses ALT variant:
{TIM3, PB0_ALT1, 3, 125, 250}  // Correct (TIM3_CH3)
```

The converter automatically handles ALT pin selection by cross-referencing
`TIMER_PIN_MAP` occurrences against PeripheralPins.c entries.

---

## Supported MCU Families

| MCU | Variant Path | Example Board |
|-----|-------------|---------------|
| STM32F411 | STM32F4xx/F411C(C-E)(U-Y) | JHEF411 |
| STM32F405 | STM32F4xx/F405RGT_F415RGT | REVO |
| STM32F7X2 | STM32F7xx/F722Z(C-E)T_F732ZET | NERO |
| STM32F745 | STM32F7xx/F74xZ(G-I) | - |
| STM32H743 | STM32H7xx/H742V...H753VI(H-T) | MATEKH743 |
| STM32G47X | STM32G4xx/G473C(B-C-E)U...G484CEU | BETAFPVG473 |

---

## Development Documentation

See [dev-docs/](dev-docs/) for detailed design documentation:

### Design Documents
- **RESEARCH.md** - Betaflight target format reference
- **CONVERTER_ANALYSIS.md** - Implementation design and architecture
- **MOTOR_CONFIG_DESIGN.md** - Motor->TimerPWM integration
- **IMPLEMENTATION_SUMMARY.md** - Project completion summary
- **COMPARISON.md** - Generated vs manual config validation

---

## Example: JHEF-JHEF411 (NOXE V3)

**Input:** `bf_configs/JHEF411/config.h` (native Betaflight target)

**Output:** `output/JHEF-JHEF411.h` - BoardConfig header with:
- Storage: SPI flash (SPI2) -> `StorageConfig`
- IMU: ICM42688P/MPU6000 (SPI1) -> `IMUConfig`
- I2C: Environmental sensors (I2C1) -> `I2CConfig`
- UARTs: 2 serial ports -> `UARTConfig`
- ADC: Battery monitoring -> `ADCConfig`
- LEDs: Status LEDs -> `LEDConfig`
- Servos: PWM servo outputs (50 Hz) -> `Servo` namespace (when present)
- Motors: 5 motors on 2 timer banks -> `Motor` namespace

**Validation:** All pins cross-validated against `PeripheralPins.c`

See `output/JHEF-JHEF411.h` for complete example.

---

## Usage Examples

### Basic Configuration Access

See `examples/basic_config_usage/` - Demonstrates how to read and use generated BoardConfig:
- Storage (SPI flash/SD card) configuration
- IMU sensor setup (SPI + interrupt)
- UART port configuration
- Battery monitoring (ADC)
- Status LED initialization
- Motor/servo frequency and pin information

**Key Pattern**:
```cpp
#include "../../output/JHEF-JHEF411.h"

void setup() {
  // Access storage config
  auto cs_pin = BoardConfig::storage.cs_pin;
  auto freq = BoardConfig::storage.freq_hz;

  // Access IMU config
  auto imu_cs = BoardConfig::imu.spi.cs_pin;
  auto imu_int = BoardConfig::imu.int_pin;

  // Initialize status LED
  pinMode(BoardConfig::status_leds.led1_pin, OUTPUT);
}
```

### Motor and Servo PWM Control

Generated BoardConfig headers include Motor and Servo namespaces organized by timer banks:

**Generated Structure** (from MTKS-MATEKH743.h):
```cpp
namespace BoardConfig {
  namespace Servo {
    static constexpr uint32_t frequency_hz = 50;  // 50 Hz for servos

    namespace TIM15_Bank {
      static inline TIM_TypeDef* const timer = TIM15;
      static constexpr Channel servo1 = {PE5, 1, 1000, 2000};  // TIM15_CH1
      static constexpr Channel servo2 = {PE6, 2, 1000, 2000};  // TIM15_CH2
    };
  };

  namespace Motor {
    static constexpr uint32_t frequency_hz = 1000;  // OneShot125

    namespace TIM3_Bank {
      static inline TIM_TypeDef* const timer = TIM3;
      static constexpr Channel motor1 = {PB0_ALT1, 3, 125, 250};  // TIM3_CH3
      static constexpr Channel motor2 = {PB1_ALT1, 4, 125, 250};  // TIM3_CH4
    };
  };
}
```

**Usage with TimerPWM Library** (see main repo's TimerPWM examples):
```cpp
#include <PWMOutputBank.h>
#include "output/MTKS-MATEKH743.h"

PWMOutputBank servo_pwm;

void setup() {
  // Initialize servo timer bank
  servo_pwm.Init(BoardConfig::Servo::TIM15_Bank::timer,
                 BoardConfig::Servo::frequency_hz);

  // Attach servo channels
  auto& servo1 = BoardConfig::Servo::TIM15_Bank::servo1;
  servo_pwm.AttachChannel(servo1.ch, servo1.pin,
                          servo1.min_us, servo1.max_us);
  servo_pwm.Start();
}

void loop() {
  servo_pwm.SetPulseWidth(servo1.ch, 1500); // Center position
}
```
