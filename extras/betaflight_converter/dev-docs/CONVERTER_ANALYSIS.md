# Betaflight Converter Implementation Analysis

## Overview

Analysis of madflight's `betaflight_target_converter.py` to inform our own converter implementation for the Arduino STM32 BoardConfig system.

**Source**: https://github.com/qqqlab/madflight/blob/main/extras/betaflight_target_converter/betaflight_target_converter.py

## madflight Converter Architecture

### Input/Output Flow

```
Betaflight .config files
         ↓
betaflight_target_converter.py (Python)
         ↓
C++ header files with R"" raw string literals
         ↓
madflight runtime parser
```

**Key Insight**: madflight uses a **two-stage approach**:
1. **Python converter** (offline): Betaflight config → C++ header with string data
2. **Runtime parser** (on-device): Parse string data into runtime structures

### Parsing Strategy

**Simple line-by-line regex parsing**:
```python
# Split line by spaces, extract components
p = (line + "   ").split(" ")

# Pattern matching on first token
if p[0] == "#define":
    # Extract sensor type from USE_GYRO_SPI_*
elif p[0] == "resource":
    # resource MOTOR 1 A08 → pin_out0 PA8
elif p[0] == "set":
    # set gyro_1_spibus = 1 → imu_spi_bus 0
```

**No complex state machine** - just pattern matching per line.

### Resource Mapping (Betaflight → madflight)

| Betaflight Resource | madflight Output | Notes |
|---------------------|------------------|-------|
| `MOTOR <n> <pin>` | `pin_out<n-1> P<pin>` | 0-based indexing |
| `SERIAL_TX <n> <pin>` | `pin_ser<n-1>_tx P<pin>` | 0-based bus |
| `SERIAL_RX <n> <pin>` | `pin_ser<n-1>_rx P<pin>` | 0-based bus |
| `SPI_SCK <n> <pin>` | `pin_spi<n-1>_sclk P<pin>` | 0-based bus |
| `SPI_MISO <n> <pin>` | `pin_spi<n-1>_miso P<pin>` | 0-based bus |
| `SPI_MOSI <n> <pin>` | `pin_spi<n-1>_mosi P<pin>` | 0-based bus |
| `I2C_SCL <n> <pin>` | `pin_i2c<n-1>_scl P<pin>` | 0-based bus |
| `I2C_SDA <n> <pin>` | `pin_i2c<n-1>_sda P<pin>` | 0-based bus |
| `GYRO_CS 1 <pin>` | `pin_imu_cs P<pin>` | Only first instance |
| `GYRO_EXTI 1 <pin>` | `pin_imu_int P<pin>` | Only first instance |
| `ADC_BATT 1 <pin>` | `pin_bat_v P<pin>` + `bat_gizmo ADC` | |
| `ADC_CURR 1 <pin>` | `pin_bat_i P<pin>` + `bat_gizmo ADC` | |
| `SDCARD_CS 1 <pin>` | `pin_bbx_cs P<pin>` + `bbx_gizmo SDSPI` | |
| `LED 1 <pin>` | `pin_led P<pin>` | Only first LED |
| `PPM 1 <pin>` | `pin_rcl_ppm P<pin>` | Only first PPM |

**Resources NOT mapped** (commented out):
- `BEEPER`
- `PWM` (individual PWM inputs)
- `LED_STRIP`
- `CAMERA_CONTROL`
- `PINIO`
- `FLASH_CS` (should be mapped to blackbox!)
- `OSD_CS`
- `USB_DETECT`

### Pin Name Conversion

**Betaflight**: `A09`, `B04`, `C13` (leading zero on single digits)
**madflight**: `PA9`, `PB4`, `PC13` (standard STM32 format)

```python
pin = "P" + re.sub(r"0([0-9])", r"\1", p[3])  # Convert C01 → PC1
```

### Sensor Detection (#define → gizmo)

```python
if nme.startswith("USE_GYRO_SPI_"):
    val = nme.replace("USE_GYRO_SPI_", "")  # ICM42688P, MPU6000, etc
    defines.append("imu_gizmo " + val)
elif nme.startswith("USE_BARO_"):
    val = nme.replace("USE_BARO_", "")      # BMP280, DPS310, etc
    defines.append("bar_gizmo " + val)
elif nme.startswith("USE_MAG_"):
    val = nme.replace("USE_MAG_", "")       # HMC5883, QMC5883, etc
    defines.append("mag_gizmo " + val)
```

**Issue**: Multiple sensors defined → madflight picks **last one** in file.
**Solution**: User must comment out unused sensors.

### Bus Configuration (set commands)

```python
if n == "gyro_1_spibus":
    bus = str(toInt(v) - 1)  # Convert 1-based to 0-based
    defines.append("imu_spi_bus " + bus)
elif n == "gyro_1_sensor_align":
    defines.append("imu_align " + v)  # CW0, CW90, CW180, CW270
```

**Bus indexing**: Betaflight uses 1-based, madflight uses 0-based.

### Example Conversion: JHEF411

**Input** (Betaflight):
```
resource MOTOR 1 A08
resource GYRO_CS 1 A04
resource GYRO_EXTI 1 B03
set gyro_1_spibus = 1
set gyro_1_sensor_align = CW180
#define USE_GYRO_SPI_ICM42688P
```

**Output** (madflight):
```cpp
const char madflight_board[] = R""(
imu_bus_type SPI
pin_out0 PA8        // resource MOTOR 1 A08
pin_imu_cs PA4      // resource GYRO_CS 1 A04
pin_imu_int PB3     // resource GYRO_EXTI 1 B03
imu_spi_bus 0       // set gyro_1_spibus = 1
imu_align CW180     // set gyro_1_sensor_align = CW180
imu_gizmo ICM42688P // #define USE_GYRO_SPI_ICM42688P
)"";
```

## Comparison: madflight vs Our BoardConfig System

| Aspect | madflight | Our BoardConfig | Notes |
|--------|-----------|-----------------|-------|
| **Architecture** | Two-stage (offline + runtime) | Compile-time only | We want full compile-time validation |
| **Output Format** | Raw string literals | C++ constexpr structs | Type-safe, zero runtime overhead |
| **Pin Format** | String parsing | Arduino pin macros with ALT | Direct hardware mapping, ALT variants for timer conflicts |
| **Bus Selection** | Runtime string parsing | Compile-time pin deduction | Our SPI/I2C uses actual pins |
| **Sensor Detection** | Multiple gizmos, pick last | Single chip type | We detect at runtime via SPI |
| **Validation** | Runtime errors | Compile-time + PeripheralPins.c | Catch misconfigurations early, validate timer/AF |

## Our Converter Design Requirements

### Input
- Betaflight `.config` files (same as madflight)
- Optional: User overrides for ambiguous cases

### Output
- C++ header file with `namespace BoardConfig`
- Compile-time `constexpr` structures
- Full type safety and validation

### Mapping Strategy

**Storage (FLASH_CS / SDCARD_CS)**:
```cpp
// Betaflight:
// resource FLASH_CS 1 B02
// set flash_spi_bus = 2
// set blackbox_device = SPIFLASH

// Our output:
static constexpr StorageConfig storage{
  StorageBackend::LITTLEFS,  // from blackbox_device
  PB15, PB14, PB13,          // SPI2 pins from resource SPI_MOSI/MISO/SCK 2
  PB02,                      // CS from FLASH_CS
  8000000                    // Default 8 MHz
};
```

**IMU (GYRO_CS / GYRO_EXTI)**:
```cpp
// Betaflight:
// resource GYRO_CS 1 A04
// resource GYRO_EXTI 1 B03
// set gyro_1_spibus = 1

// Our output:
static constexpr SPIConfig imu_spi{
  PA7, PA6, PA5,             // SPI1 pins from resource SPI_MOSI/MISO/SCK 1
  PA4,                       // CS from GYRO_CS
  8000000,                   // Default 8 MHz
  CS_Mode::HARDWARE
};
static constexpr IMUConfig imu{
  imu_spi,
  PB03,                      // INT from GYRO_EXTI
  1000000                    // 1 MHz for init (MPU-6000 pattern)
};
```

**Motors (MOTOR resources + timer)**:
```cpp
// Betaflight:
// resource MOTOR 1 A08
// timer A08 AF1  # TIM1 CH1
// dma pin A08 1  # DMA2 Stream 1 Channel 6

// Our output (NEW types needed):
namespace Motor {
  static constexpr TIM_TypeDef* timer = TIM1;
  static constexpr uint32_t frequency_hz = 1000;  // From motor_pwm_protocol

  static constexpr PWMOutputConfig motor1{1, PA08, 1000, 2000};
  static constexpr PWMOutputConfig motor2{2, PA09, 1000, 2000};
  // ... up to motor5
}
```

**UARTs (SERIAL_TX / SERIAL_RX)**:
```cpp
// Betaflight:
// resource SERIAL_TX 1 B06
// resource SERIAL_RX 1 B07

// Our output:
static constexpr UARTConfig uart1{PB06, PB07, 115200};
static constexpr UARTConfig uart2{PA02, PA03, 115200};
```

**I2C (I2C_SCL / I2C_SDA)**:
```cpp
// Betaflight:
// resource I2C_SCL 1 B08
// resource I2C_SDA 1 B09

// Our output:
static constexpr I2CConfig sensors{PB08, PB09, 400000};
```

**ADC (ADC_BATT / ADC_CURR)**:
```cpp
// Betaflight:
// resource ADC_BATT 1 A00
// resource ADC_CURR 1 A01
// set ibata_scale = 170
// set vbat_scale = 110

// Our output (NEW type needed):
static constexpr ADCConfig battery{
  PA00,      // voltage pin
  PA01,      // current pin
  110,       // vbat_scale
  170        // ibata_scale
};
```

## New ConfigTypes Needed

### MotorConfig / PWMOutputConfig
```cpp
struct PWMOutputConfig {
  uint8_t channel;
  uint32_t pin;
  uint16_t min_us;
  uint16_t max_us;
};

namespace Motor {
  TIM_TypeDef* timer;
  uint32_t frequency_hz;
  PWMOutputConfig motor1;
  PWMOutputConfig motor2;
  // ...
}
```

### ADCConfig
```cpp
struct ADCConfig {
  uint32_t voltage_pin;
  uint32_t current_pin;
  uint16_t voltage_scale;   // vbat_scale
  uint16_t current_scale;   // ibata_scale
};
```

### ReceiverConfig
```cpp
enum class ReceiverProtocol {
  SBUS,
  CRSF,
  IBUS,
  FPORT,
  EXPRESSLRS_UART,
  EXPRESSLRS_SPI
};

struct ReceiverConfig {
  ReceiverProtocol protocol;
  UARTConfig uart;          // For serial receivers
  SPIConfig spi;            // For SPI receivers
  bool inverted;
  bool telemetry_enabled;
};
```

### LEDConfig
```cpp
struct LEDConfig {
  uint32_t status_led_pin;
  uint32_t strip_pin;       // WS2812 LED strip
  TIM_TypeDef* strip_timer; // Timer for LED strip DMA
};
```

## PeripheralPins.c Validation (CRITICAL DISCOVERY)

### Overview

The Arduino Core STM32 variant files contain **complete pin→peripheral mapping data** that can be used to validate Betaflight configurations!

**Location**: `Arduino_Core_STM32/variants/STM32F4xx/F411C(C-E)(U-Y)/PeripheralPins.c`

### Available Validation Data

#### 1. Timer Mappings (PinMap_TIM)

**Format**:
```c
{PB_4, TIM3, STM_PIN_DATA_EXT(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF2_TIM3, 1, 0)}, // TIM3_CH1
//Pin  Timer                                   AF            Channel^
```

**Validation capability**:
- Verify pin supports specified timer (PB4 → TIM3 ✅)
- Verify alternate function (AF2 ✅)
- Extract timer channel (CH1)
- Detect invalid combinations

**Example validation**:
```python
# Betaflight: timer B04 AF2
bf_pin = "B04"  # → PB_4
bf_timer = "TIM3"  # Extracted from AF2 + context
bf_af = 2

# Lookup in PinMap_TIM
entry = pinmap_tim[(PB_4, TIM3, AF2)]
# → {'channel': 1, 'comment': 'TIM3_CH1'}
```

#### 2. SPI Mappings

**PinMap_SPI_MOSI**:
```c
{PA_7, SPI1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF5_SPI1)},
{PB_5, SPI1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF5_SPI1)},
{PB_15, SPI2, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF5_SPI2)},
```

**PinMap_SPI_MISO**, **PinMap_SPI_SCLK** - Similar format

**Validation capability**:
- Verify SPI bus pins (PA7/PA6/PA5 = SPI1 ✅)
- Detect pin→bus mismatches
- Group pins by SPI bus number

#### 3. I2C Mappings

**PinMap_I2C_SDA**:
```c
{PB_7, I2C1, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF4_I2C1)},
{PB_9, I2C1, STM_PIN_DATA(STM_MODE_AF_OD, GPIO_NOPULL, GPIO_AF4_I2C1)},
```

**PinMap_I2C_SCL** - Similar format

**Validation capability**:
- Verify I2C bus pins (PB8/PB9 = I2C1 ✅)
- Ensure SCL/SDA on same bus

#### 4. UART Mappings

**PinMap_UART_TX**:
```c
{PA_9, USART1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF7_USART1)},
{PB_6, USART1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF7_USART1)},
```

**PinMap_UART_RX** - Similar format

**Validation capability**:
- Verify UART pins (PB6/PB7 = USART1 ✅)
- Ensure TX/RX on same UART

### MCU Detection

Betaflight configs specify MCU in header:
```
# Betaflight / STM32F411 (S411) 4.2.0 Jun 14 2020 / 03:04:43 (8f2d21460)
```

**Mapping to PeripheralPins.c**:
- `STM32F411` → `variants/STM32F4xx/F411C(C-E)(U-Y)/PeripheralPins.c`
- `STM32F405` → `variants/STM32F4xx/F405RGTx/PeripheralPins.c`
- `STM32F722` → `variants/STM32F7xx/F722R(C-E)T/PeripheralPins.c`
- `STM32H743` → `variants/STM32H7xx/H743ZITx/PeripheralPins.c`

### Parser Implementation

```python
class PeripheralPinMap:
    def __init__(self, mcu_type):
        self.pinmap_tim = {}    # (pin, timer, af) → channel
        self.pinmap_spi = {}    # (pin, spi_bus) → signal_type
        self.pinmap_i2c = {}    # (pin, i2c_bus) → signal_type
        self.pinmap_uart = {}   # (pin, uart_num) → signal_type

        self.parse_peripheral_pins(mcu_type)

    def parse_peripheral_pins(self, mcu_type):
        # Find correct PeripheralPins.c file
        variant_path = self.find_variant(mcu_type)

        # Parse PinMap_TIM array
        # {PB_4, TIM3, STM_PIN_DATA_EXT(..., GPIO_AF2_TIM3, 1, 0)}
        # Extract: pin=PB_4, timer=TIM3, af=2, channel=1

        # Parse PinMap_SPI_MOSI/MISO/SCLK arrays
        # Parse PinMap_I2C_SDA/SCL arrays
        # Parse PinMap_UART_TX/RX arrays

    def validate_timer(self, pin, timer, af):
        """Validate timer assignment from Betaflight config"""
        key = (pin, timer, af)
        if key not in self.pinmap_tim:
            raise ValidationError(f"Invalid: {pin} does not support {timer} on AF{af}")
        return self.pinmap_tim[key]['channel']

    def validate_spi_bus(self, mosi, miso, sclk, bus_num):
        """Validate SPI pins belong to same bus"""
        if self.pinmap_spi[(mosi, 'MOSI')] != f"SPI{bus_num}":
            raise ValidationError(f"MOSI pin {mosi} not on SPI{bus_num}")
        # Similar for MISO, SCLK
```

### Validation Workflow

```python
def convert_betaflight_config(bf_config_path):
    # 1. Parse Betaflight config
    bf = BetaflightConfig(bf_config_path)
    bf.parse()

    # 2. Load PeripheralPins.c for target MCU
    mcu = bf.get_mcu_type()  # "STM32F411"
    pinmap = PeripheralPinMap(mcu)

    # 3. Validate timer assignments
    for motor in bf.motors:
        pin = motor.pin
        timer_info = bf.timers[pin]

        channel = pinmap.validate_timer(
            pin=pin,
            timer=timer_info.timer,
            af=timer_info.af
        )

        motor.channel = channel  # Store validated channel

    # 4. Validate SPI buses
    for bus_num, pins in bf.spi_buses.items():
        pinmap.validate_spi_bus(
            mosi=pins.mosi,
            miso=pins.miso,
            sclk=pins.sclk,
            bus_num=bus_num
        )

    # 5. Generate BoardConfig with validated data
    generate_board_config(bf, pinmap)
```

### Benefits

✅ **Authoritative data source** - Same files used by STM32 Arduino Core
✅ **MCU-specific validation** - Correct for F411, F405, F722, H743, etc.
✅ **Comprehensive coverage** - TIM, SPI, I2C, UART, ADC all included
✅ **Upstream maintained** - STMicroelectronics CubeMX database
✅ **No manual database** - Don't need to parse datasheets manually

## Converter Implementation Approach

### Phase 1: Parser (Python)
```python
class BetaflightConfig:
    def __init__(self, filepath):
        self.resources = {}    # resource type → [(index, pin)]
        self.settings = {}     # setting name → value
        self.defines = []      # #define statements
        self.timers = {}       # pin → (timer, AF)
        self.dma = {}          # pin → DMA info

    def parse(self):
        # Line-by-line parsing like madflight
        pass

    def get_spi_pins(self, bus_num):
        # Returns (mosi, miso, sclk) for SPI bus
        pass

    def get_storage_config(self):
        # Returns StorageConfig parameters
        pass
```

### Phase 2: Code Generator (Python)
```python
class BoardConfigGenerator:
    def __init__(self, bf_config):
        self.bf = bf_config

    def generate_header(self, board_name):
        # Generate C++ header with namespace BoardConfig
        pass

    def generate_storage(self):
        # Generate StorageConfig from FLASH_CS/SDCARD_CS
        pass

    def generate_imu(self):
        # Generate IMUConfig from GYRO_CS/GYRO_EXTI
        pass
```

### Phase 3: Validation
- Pin conflict detection (same pin used twice)
- Timer conflict detection (motors on incompatible timers)
- DMA conflict detection (DShot requirements)
- Bus validation (pins belong to correct SPI/I2C peripheral)

## Next Steps

1. **Design ConfigTypes extensions** - Add Motor, ADC, Receiver, LED config types
2. **Write minimal parser** - Parse resources, settings, timers (subset first)
3. **Generate JHEF411 config** - Test with known working board
4. **Validate against hardware** - Compare generated vs hand-written NOXE_V3.h
5. **Expand to full parser** - Add all resource types, validation
6. **Test with multiple boards** - F405, F411, F722, H743 variants

## Key Lessons from madflight

✅ **Keep parsing simple** - Line-by-line regex is sufficient
✅ **Handle ambiguity** - Multiple sensors → require user selection
✅ **Preserve original** - Embed source .config in comments for reference
✅ **Bus indexing** - Always convert Betaflight 1-based to 0-based
❌ **Avoid runtime parsing** - We want compile-time validation instead
❌ **Don't ignore resources** - Map FLASH_CS, OSD_CS, LED_STRIP, etc.
