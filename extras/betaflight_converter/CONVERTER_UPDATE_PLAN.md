# Plan: Update Betaflight Converter for Native Config Format

## Context

The betaflight_converter currently parses "unified target" `.config` files (sourced from
madflight, not actually Betaflight's native format). Real Betaflight configs are C header
files (`config.h`) with `#define` statements, located in per-target directories under
`betaflight/src/config/configs/`. Four real Betaflight target directories have been copied
to `bf_configs/` and are ready for use.

**Goal**: Rewrite the parser to consume native Betaflight `config.h` files so that adding
new targets is as simple as copying a target folder from the Betaflight repo and re-running
the converter.

## Format Differences

| Aspect | Old `.config` (madflight) | New `config.h` (Betaflight) |
|--------|---------------------------|----------------------------|
| Pin defines | `resource MOTOR 1 A08` | `#define MOTOR1_PIN PA8` |
| Pin format | `A08` (needs P prefix, strip leading zero) | `PA8` (already Arduino format) |
| Timer map | `timer A08 AF1` + comment `# pin A08: TIM1 CH1` | `TIMER_PIN_MAP(1, PA8, 1, 1)` — occurrence-based |
| SPI bus | `set gyro_1_spibus = 1` | `#define GYRO_1_SPI_INSTANCE SPI1` |
| SPI pins | `resource SPI_MISO 1 A06` | `#define SPI1_SDI_PIN PA6` (SDI=MISO, SDO=MOSI) |
| Settings | `set ibata_scale = 170` | `#define DEFAULT_CURRENT_METER_SCALE 170` |
| Storage | `set blackbox_device = SPIFLASH` | `#define DEFAULT_BLACKBOX_DEVICE BLACKBOX_DEVICE_FLASH` |
| Board info | `board_name JHEF411` | `#define BOARD_NAME JHEF411` |
| MCU | Header comment `STM32F411` | `#define FC_TARGET_MCU STM32F411` |

### Timer Occurrence Resolution (Critical)

The `TIMER_PIN_MAP(index, pin, occurrence, dma)` "occurrence" is a 1-based index into the
timer options for that pin. **Verified**: this ordering matches PeripheralPins.c's PinMap_TIM
ordering across F411, F405, F7X2, and H743:

- PB0 occurrence=2 → 2nd entry in PinMap_TIM for PB_0 → TIM3_CH3 (confirmed all families)
- PA3 occurrence=1 → 1st entry → TIM2_CH4; occurrence=2 → TIM5_CH4 (confirmed F405)
- PA0 occurrence=2 → TIM5_CH1 (confirmed all families)

## Architecture (unchanged modules marked)

```
convert.py          ← UPDATE: accept config.h/directory, new output naming
src/
  betaflight_config.py  ← REWRITE: parse #define format, occurrence-based timers
  peripheral_pins.py    ← UNCHANGED
  validator.py          ← MINOR: pin format is already Arduino (no conversion needed)
  code_generator.py     ← UNCHANGED
```

**Key design decision**: The rewritten `BetaflightConfig` preserves the same public API
(ResourcePin, TimerAssignment, settings dict, get_motors(), get_spi_pins(), etc.) so that
`validator.py` and `code_generator.py` require minimal changes.

## Implementation Steps

### Step 1: Rewrite `betaflight_config.py`

**File**: `Arduino_Core_STM32/extras/betaflight_converter/src/betaflight_config.py`

Replace the `.config` parser with a `config.h` parser. Same class name, same public API.

**Parsing changes**:

| What | Regex pattern | Example |
|------|---------------|---------|
| MCU | `#define FC_TARGET_MCU\s+(\w+)` | `STM32F411` |
| Board | `#define BOARD_NAME\s+(\w+)` | `JHEF411` |
| Manufacturer | `#define MANUFACTURER_ID\s+(\w+)` | `JHEF` |
| Motors | `#define MOTOR(\d+)_PIN\s+(P\w+)` | `MOTOR1_PIN PA8` |
| Servos | `#define SERVO(\d+)_PIN\s+(P\w+)` | `SERVO1_PIN PE5` |
| UART TX | `#define UART(\d+)_TX_PIN\s+(P\w+)` | `UART1_TX_PIN PB6` |
| UART RX | `#define UART(\d+)_RX_PIN\s+(P\w+)` | `UART1_RX_PIN PB7` |
| SPI SCK | `#define SPI(\d+)_SCK_PIN\s+(P\w+)` | `SPI1_SCK_PIN PA5` |
| SPI SDI(MISO) | `#define SPI(\d+)_SDI_PIN\s+(P\w+)` | `SPI1_SDI_PIN PA6` |
| SPI SDO(MOSI) | `#define SPI(\d+)_SDO_PIN\s+(P\w+)` | `SPI1_SDO_PIN PA7` |
| I2C SCL | `#define I2C(\d+)_SCL_PIN\s+(P\w+)` | `I2C1_SCL_PIN PB8` |
| I2C SDA | `#define I2C(\d+)_SDA_PIN\s+(P\w+)` | `I2C1_SDA_PIN PB9` |
| LEDs | `#define LED(\d+)_PIN\s+(P\w+)` | `LED0_PIN PC13` (0-based→1-based) |
| ADC VBAT | `#define ADC_VBAT_PIN\s+(P\w+)` | `ADC_VBAT_PIN PA0` |
| ADC Current | `#define ADC_CURR_PIN\s+(P\w+)` | `ADC_CURR_PIN PA1` |
| Gyro CS | `#define GYRO_(\d+)_CS_PIN\s+(P\w+)` | `GYRO_1_CS_PIN PA4` |
| Gyro EXTI | `#define GYRO_(\d+)_EXTI_PIN\s+(P\w+)` | `GYRO_1_EXTI_PIN PB3` |
| Flash CS | `#define FLASH_CS_PIN\s+(P\w+)` | `FLASH_CS_PIN PB2` |
| SD CS | `#define SDCARD_SPI_CS_PIN\s+(P\w+)` | `SDCARD_SPI_CS_PIN PA15` |
| Timer map | `TIMER_PIN_MAP\(\s*(\d+),\s*(P\w+)\s*,\s*(\d+),\s*(-?\d+)\)` | multi-line macro |
| SPI instance | `#define (\w+)_SPI_INSTANCE\s+SPI(\d+)` | `GYRO_1_SPI_INSTANCE SPI1` |
| Current scale | `#define DEFAULT_CURRENT_METER_SCALE\s+(\d+)` | `170` |
| Blackbox | `#define DEFAULT_BLACKBOX_DEVICE\s+BLACKBOX_DEVICE_(\w+)` | `FLASH`/`SDCARD` |
| USE_* defines | `#define (USE_\w+)` | `USE_GYRO_SPI_MPU6000` |

**ResourcePin storage**: Pins stored in Arduino format directly (PA8, PB0). The `pin` field
uses Arduino macro format. `convert_pin_format()` becomes identity (returns input unchanged).

**Resource mapping from config.h to internal types** (for validator/generator compatibility):

| Config.h define | Internal resource type | Index |
|-----------------|----------------------|-------|
| `MOTOR1_PIN PA8` | `MOTOR` index=1 | from regex group |
| `UART1_TX_PIN PB6` | `SERIAL_TX` index=1 | from regex group |
| `UART1_RX_PIN PB7` | `SERIAL_RX` index=1 | from regex group |
| `SPI1_SCK_PIN PA5` | `SPI_SCK` index=1 | from regex group |
| `SPI1_SDI_PIN PA6` | `SPI_MISO` index=1 | SDI→MISO mapping |
| `SPI1_SDO_PIN PA7` | `SPI_MOSI` index=1 | SDO→MOSI mapping |
| `I2C1_SCL_PIN PB8` | `I2C_SCL` index=1 | from regex group |
| `I2C1_SDA_PIN PB9` | `I2C_SDA` index=1 | from regex group |
| `LED0_PIN PC13` | `LED` index=1 | 0-based→1-based |
| `ADC_VBAT_PIN PA0` | `ADC_BATT` index=1 | single instance |
| `ADC_CURR_PIN PA1` | `ADC_CURR` index=1 | single instance |
| `GYRO_1_CS_PIN PA4` | `GYRO_CS` index=1 | from regex group |
| `GYRO_1_EXTI_PIN PB3` | `GYRO_EXTI` index=1 | from regex group |
| `FLASH_CS_PIN PB2` | `FLASH_CS` index=1 | single instance |
| `SDCARD_SPI_CS_PIN PA15` | `SDCARD_CS` index=1 | single instance |

**Settings mapping** (populate `self.settings` dict for validator/generator compatibility):

| Config.h define | Settings key | Value transform |
|-----------------|-------------|-----------------|
| `GYRO_1_SPI_INSTANCE SPI1` | `gyro_1_spibus` | Extract number: `'1'` |
| `FLASH_SPI_INSTANCE SPI2` | `flash_spi_bus` | Extract number: `'2'` |
| `SDCARD_SPI_INSTANCE SPI3` | `sdcard_spi_bus` | Extract number: `'3'` |
| `MAX7456_SPI_INSTANCE SPI2` | `max7456_spi_bus` | Extract number: `'2'` |
| `DEFAULT_BLACKBOX_DEVICE BLACKBOX_DEVICE_FLASH` | `blackbox_device` | `'SPIFLASH'` |
| `DEFAULT_BLACKBOX_DEVICE BLACKBOX_DEVICE_SDCARD` | `blackbox_device` | `'SDCARD'` |
| `DEFAULT_CURRENT_METER_SCALE 170` | `ibata_scale` | `'170'` |
| `GYRO_1_ALIGN CW180_DEG` | `gyro_1_sensor_align` | `'CW180'` |

**Timer resolution**: Add `resolve_timers(pinmap: PeripheralPinMap)` method:
1. For each pin in `self.timer_pin_map` (raw TIMER_PIN_MAP data: `{pin: occurrence}`):
2. Get all TimerPin entries from `pinmap.timer_pins` for that base pin
3. Filter to non-complementary entries (channel, not CHxN) — same pin order
4. Select the Nth entry (occurrence is 1-based)
5. Create `TimerAssignment(pin=pin, af=tp.af, timer=tp.timer, channel=tp.channel)`
6. Store in `self.timers[pin]` (keyed by Arduino format pin name)

### Step 2: Minor update to `validator.py`

**File**: `Arduino_Core_STM32/extras/betaflight_converter/src/validator.py`

Since pins are now stored in Arduino format directly, `convert_pin_format()` is a no-op.
The validator calls `self.bf_config.convert_pin_format(pin_bf)` which returns the pin
unchanged. The timer dict keys are now in Arduino format (e.g., `'PA8'` not `'A08'`).

Only change needed: The validator accesses `self.bf_config.timers[pin_bf]` — since `pin_bf`
is now the same as `pin_arduino`, this still works. No code changes required if
`convert_pin_format()` is identity and timer keys match the resource pin format.

### Step 3: Update `convert.py`

**File**: `Arduino_Core_STM32/extras/betaflight_converter/convert.py`

Changes:
1. Accept a target directory (e.g., `bf_configs/JHEF411/`) or direct `config.h` path
2. If directory given, look for `config.h` inside it
3. After loading both BetaflightConfig and PeripheralPinMap, call
   `bf_config.resolve_timers(pinmap)` to populate timer assignments
4. Derive output filename from `{manufacturer_id}-{board_name}.h`
   (e.g., `JHEF-JHEF411.h`)
5. Add `--all` flag to convert all targets in `bf_configs/`
6. Update usage/help strings

### Step 4: Update tests

**File**: `Arduino_Core_STM32/extras/betaflight_converter/tests/test_betaflight_config.py`

Rewrite tests to use `bf_configs/JHEF411/config.h` instead of `data/JHEF-JHEF411.config`.
Update expected values:
- Pin format: `'A08'` → `'PA8'`, `'B04'` → `'PB4'`, etc.
- Timer keys: `'A08'` → `'PA8'`
- Settings keys remain the same (mapped internally)
- Add tests for timer occurrence resolution (requires PeripheralPinMap fixture)

### Step 5: Clean up old format

- Remove `data/` directory (old `.config` files)
- Update `output/` - regenerate all 4 board headers from `bf_configs/`
- Verify generated output matches expected (same pin assignments, timer mappings)

## Critical Files

| File | Action |
|------|--------|
| `src/betaflight_config.py` | Rewrite |
| `convert.py` | Update |
| `tests/test_betaflight_config.py` | Rewrite |
| `tests/test_validator.py` | Update pin format expectations |
| `tests/test_code_generator.py` | Update pin format expectations |
| `src/peripheral_pins.py` | Unchanged |
| `src/validator.py` | Minimal (verify no changes needed) |
| `src/code_generator.py` | Unchanged |
| `bf_configs/*/config.h` | Input (already present) |
| `data/` | Remove |

## Verification

1. **Unit tests**: `cd extras/betaflight_converter && python3 -m pytest tests/ -v`
2. **Convert all 4 targets**:
   ```bash
   cd Arduino_Core_STM32/extras/betaflight_converter
   python3 convert.py bf_configs/JHEF411/
   python3 convert.py bf_configs/REVO/
   python3 convert.py bf_configs/NERO/
   python3 convert.py bf_configs/MATEKH743/
   ```
3. **Diff generated output** against existing `output/*.h` files — pin assignments and timer
   mappings should match (only timestamp changes)
4. **Compile test**: Build a sketch that includes a generated BoardConfig header to verify
   C++ syntax is correct
