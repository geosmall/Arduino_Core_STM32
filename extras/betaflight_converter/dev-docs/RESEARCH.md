# Betaflight Native Target Config Format Reference

## Overview

Betaflight target configurations are C header files (`config.h`) located in per-target
directories under `betaflight/src/config/configs/<TARGET>/`. Each file uses `#define`
statements to declare pin assignments, peripheral instances, timer mappings, and
default settings.

The converter parses these files directly — pins are already in Arduino macro format
(`PA8`, `PB0`), so no pin format conversion is required.

## File Structure

### License Header and Pragma

Every config.h begins with the GPL license header and `#pragma once`:
```c
/*
 * This file is part of Betaflight.
 * ...
 */

#pragma once
```

### Board Identity

Three required defines identify the target:
```c
#define FC_TARGET_MCU     STM32F411
#define BOARD_NAME        JHEF411
#define MANUFACTURER_ID   JHEF
```

`FC_TARGET_MCU` determines which PeripheralPins.c variant to use for validation.

### Sensor and Feature Defines

`USE_*` defines declare which hardware drivers to include:
```c
#define USE_ACC
#define USE_ACC_SPI_MPU6000
#define USE_GYRO
#define USE_GYRO_SPI_MPU6000
#define USE_GYRO_SPI_ICM42688P
#define USE_ACC_SPI_ICM42688P
#define USE_BARO
#define USE_BARO_BMP280
#define USE_BARO_DPS310
#define USE_FLASH
#define USE_FLASH_W25Q128FV
#define USE_MAX7456
#define USE_SDCARD
```

The converter extracts gyro chip names from `USE_GYRO_SPI_*` defines for the generated
header comment.

---

## Pin Definitions

All pin assignments use `#define` with Arduino-format pin names (no conversion needed):

### Motor and Servo Outputs

```c
#define MOTOR1_PIN           PA8
#define MOTOR2_PIN           PA9
#define MOTOR3_PIN           PA10
#define MOTOR4_PIN           PB0
#define MOTOR5_PIN           PB4

#define SERVO1_PIN           PE5
#define SERVO2_PIN           PE6
```

Motor indices are 1-based. Servo defines are only present on boards with dedicated
servo outputs (e.g., MATEKH743).

### UART Pins

```c
#define UART1_TX_PIN         PB6
#define UART1_RX_PIN         PB7
#define UART2_TX_PIN         PA2
#define UART2_RX_PIN         PA3
```

Note: Betaflight uses `UART` prefix even for USARTs. The converter maps these to
`SERIAL_TX`/`SERIAL_RX` resource types internally.

### SPI Bus Pins

```c
#define SPI1_SCK_PIN         PA5
#define SPI1_SDI_PIN         PA6     // SDI = MISO
#define SPI1_SDO_PIN         PA7     // SDO = MOSI
#define SPI2_SCK_PIN         PB13
#define SPI2_SDI_PIN         PB14
#define SPI2_SDO_PIN         PB15
```

Betaflight uses SDI/SDO naming (Slave Data In/Out). The converter maps internally:
- `SDI` → `MISO` (Master In, Slave Out)
- `SDO` → `MOSI` (Master Out, Slave In)

### I2C Bus Pins

```c
#define I2C1_SCL_PIN         PB8
#define I2C1_SDA_PIN         PB9
#define I2C2_SCL_PIN         PB10    // Some boards have multiple I2C buses
#define I2C2_SDA_PIN         PB11
```

### Gyro/IMU Pins

```c
#define GYRO_1_CS_PIN        PA4
#define GYRO_1_EXTI_PIN      PB3
#define GYRO_2_CS_PIN        PC13    // Dual-gyro boards
#define GYRO_2_EXTI_PIN      PE15
```

### Storage Pins

```c
// SPI Flash
#define FLASH_CS_PIN         PB2

// SD Card (SPI mode)
#define SDCARD_SPI_CS_PIN    PA15
#define SDCARD_DETECT_PIN    PD2
```

### ADC Pins

```c
#define ADC_VBAT_PIN         PA0
#define ADC_CURR_PIN         PA1
#define ADC_RSSI_PIN         PB1
```

### Status LEDs

```c
#define LED0_PIN             PC13
#define LED1_PIN             PC14
```

LED indices are 0-based in config.h. The converter adds 1 internally (LED0 → LED index 1).

### Other Pins

```c
#define BEEPER_PIN           PC14
#define LED_STRIP_PIN        PA15
#define CAMERA_CONTROL_PIN   PB10
#define USB_DETECT_PIN       PC15
#define PINIO1_PIN           PB5
```

---

## Timer Pin Mapping

The `TIMER_PIN_MAPPING` macro defines timer assignments for all PWM-capable pins:

```c
#define TIMER_PIN_MAPPING \
    TIMER_PIN_MAP( 0, PA2 , 3, -1) \
    TIMER_PIN_MAP( 1, PA8 , 1,  1) \
    TIMER_PIN_MAP( 2, PA9 , 1,  1) \
    TIMER_PIN_MAP( 3, PA10, 1,  1) \
    TIMER_PIN_MAP( 4, PB0 , 2,  0) \
    TIMER_PIN_MAP( 5, PB4 , 1,  0)
```

### TIMER_PIN_MAP Parameters

`TIMER_PIN_MAP(index, pin, occurrence, dma_opt)`

| Parameter | Meaning |
|-----------|---------|
| `index` | Sequential index (0-based, for ordering) |
| `pin` | Pin name in Arduino format |
| `occurrence` | 1-based index into PeripheralPins.c PinMap_TIM entries for this pin |
| `dma_opt` | DMA option (-1 = no DMA) |

### Occurrence-Based Timer Resolution

The `occurrence` parameter is the key to timer assignment. It selects which timer
option to use for a given pin by indexing into the PeripheralPins.c `PinMap_TIM`
entries for that pin.

**Example: PB0 on STM32F411**

PeripheralPins.c lists these timer entries for PB_0:
```c
// Index 1: {PB_0,      TIM1, GPIO_AF1_TIM1, 2, 1}  → TIM1_CH2N (complementary)
// Index 1: {PB_0_ALT1, TIM3, GPIO_AF2_TIM3, 3, 0}  → TIM3_CH3
```

After filtering complementary channels, the non-complementary entries are indexed:
- Occurrence 1 → first non-complementary entry
- Occurrence 2 → second non-complementary entry → TIM3_CH3 (requires ALT1)

So `TIMER_PIN_MAP(4, PB0, 2, 0)` resolves to:
- Timer: TIM3
- Channel: 3
- Pin variant: PB0_ALT1 (Arduino macro format)

This is how the converter determines ALT pin variants automatically.

---

## Peripheral Instance Assignments

SPI instance defines link devices to specific SPI buses:

```c
#define GYRO_1_SPI_INSTANCE  SPI1
#define FLASH_SPI_INSTANCE   SPI2
#define SDCARD_SPI_INSTANCE  SPI3
#define MAX7456_SPI_INSTANCE SPI2
#define GYRO_2_SPI_INSTANCE  SPI4     // Dual-gyro boards
```

I2C instance defines (note the `I2CDEV_` prefix):
```c
#define MAG_I2C_INSTANCE     I2CDEV_1
#define BARO_I2C_INSTANCE    I2CDEV_1
#define BARO_I2C_INSTANCE    I2CDEV_2  // Some boards use separate bus
```

The converter maps SPI instances to settings keys:
- `GYRO_1_SPI_INSTANCE SPI1` → `gyro_1_spibus = '1'`
- `FLASH_SPI_INSTANCE SPI2` → `flash_spi_bus = '2'`
- `SDCARD_SPI_INSTANCE SPI3` → `sdcard_spi_bus = '3'`

---

## Default Settings

Configuration defaults are expressed as `#define` statements:

### Storage

```c
#define DEFAULT_BLACKBOX_DEVICE     BLACKBOX_DEVICE_FLASH   // SPI flash → LITTLEFS
#define DEFAULT_BLACKBOX_DEVICE     BLACKBOX_DEVICE_SDCARD  // SD card → SDFS
```

The converter maps:
- `BLACKBOX_DEVICE_FLASH` → `StorageBackend::LITTLEFS`
- `BLACKBOX_DEVICE_SDCARD` → `StorageBackend::SDFS`

### ADC Scaling

```c
#define DEFAULT_CURRENT_METER_SCALE 170
#define DEFAULT_CURRENT_METER_SOURCE CURRENT_METER_ADC
#define DEFAULT_VOLTAGE_METER_SOURCE VOLTAGE_METER_ADC
```

### Gyro Alignment

```c
#define GYRO_1_ALIGN CW180_DEG
#define GYRO_2_ALIGN CW90_DEG_FLIP
```

### Other Settings

```c
#define BEEPER_INVERTED
#define SYSTEM_HSE_MHZ 8
#define DEFAULT_DSHOT_BURST DSHOT_DMAR_ON
```

---

## Pin Naming Convention

### Betaflight Native Format

Betaflight config.h files use Arduino-compatible pin names directly:
- `PA8`, `PB0`, `PC13`, `PE5`, `PD12`

No conversion is needed — these match the Arduino pin macros used in generated
BoardConfig headers.

### Arduino STM32 Pin Systems

The STM32 Arduino Core has two pin identification systems:

1. **PinName Enums** (HAL/low-level): `PA_0`, `PB_15`, `PB_0_ALT1`
   - Defined in `PinNames.h`
   - Used in PeripheralPins.c mapping tables
   - Include ALT variants for alternate peripheral functions

2. **Arduino Pin Macros** (Arduino API): `PA0`, `PB15`, `PB0_ALT1`
   - Defined in variant headers
   - Used by all Arduino APIs and library constructors
   - Generated BoardConfig headers use this format

### ALT Pin Variants

Some pins support multiple peripheral instances. ALT variants select the correct one:

```c
// PeripheralPins.c for PB_0 on STM32F411:
{PB_0,      TIM1, GPIO_AF1_TIM1, 2, 1},  // Default: TIM1_CH2N
{PB_0_ALT1, TIM3, GPIO_AF2_TIM3, 3, 0},  // ALT1: TIM3_CH3

// Generated BoardConfig when occurrence=2 selects TIM3:
{TIM3, PB0_ALT1, 3, 125, 250}  // Arduino macro format with ALT suffix
```

Without the ALT suffix, the STM32 HAL would default to TIM1 for PB0, which is the
wrong timer for this motor output.

---

## Resource Type Reference

### Parsed by Converter

| Config.h Pattern | Internal Resource Type | Example |
|------------------|----------------------|---------|
| `MOTOR{n}_PIN` | `MOTOR` | `MOTOR1_PIN PA8` |
| `SERVO{n}_PIN` | `SERVO` | `SERVO1_PIN PE5` |
| `UART{n}_TX_PIN` | `SERIAL_TX` | `UART1_TX_PIN PB6` |
| `UART{n}_RX_PIN` | `SERIAL_RX` | `UART1_RX_PIN PB7` |
| `SPI{n}_SCK_PIN` | `SPI_SCK` | `SPI1_SCK_PIN PA5` |
| `SPI{n}_SDI_PIN` | `SPI_MISO` | `SPI1_SDI_PIN PA6` |
| `SPI{n}_SDO_PIN` | `SPI_MOSI` | `SPI1_SDO_PIN PA7` |
| `I2C{n}_SCL_PIN` | `I2C_SCL` | `I2C1_SCL_PIN PB8` |
| `I2C{n}_SDA_PIN` | `I2C_SDA` | `I2C1_SDA_PIN PB9` |
| `LED{n}_PIN` | `LED` (index+1) | `LED0_PIN PC13` |
| `ADC_VBAT_PIN` | `ADC_BATT` | `ADC_VBAT_PIN PA0` |
| `ADC_CURR_PIN` | `ADC_CURR` | `ADC_CURR_PIN PA1` |
| `FLASH_CS_PIN` | `FLASH_CS` | `FLASH_CS_PIN PB2` |
| `SDCARD_SPI_CS_PIN` | `SDCARD_CS` | `SDCARD_SPI_CS_PIN PA15` |
| `GYRO_{n}_CS_PIN` | `GYRO_CS` | `GYRO_1_CS_PIN PA4` |
| `GYRO_{n}_EXTI_PIN` | `GYRO_EXTI` | `GYRO_1_EXTI_PIN PB3` |
| `BEEPER_PIN` | `BEEPER` | `BEEPER_PIN PC14` |

### Not Parsed (Present but Unused)

These defines appear in config.h files but are not currently extracted by the converter:

| Define | Purpose |
|--------|---------|
| `RX_PPM_PIN` | PPM receiver input |
| `RX_PWM{n}_PIN` | PWM receiver channels |
| `LED_STRIP_PIN` | WS2812 addressable LED |
| `CAMERA_CONTROL_PIN` | Camera control output |
| `ESCSERIAL_PIN` | ESC serial telemetry |
| `USB_DETECT_PIN` | USB connection detection |
| `PINIO{n}_PIN` | Programmable I/O |
| `SDCARD_DETECT_PIN` | SD card detection |
| `MAX7456_SPI_CS_PIN` | OSD chip select |
| `ADC_RSSI_PIN` | Analog RSSI input |
| `INVERTER_PIN_UART{n}` | Serial inverter control |

---

## Settings Reference

### Parsed by Converter

| Config.h Define | Settings Key | Value Transform |
|-----------------|-------------|-----------------|
| `GYRO_1_SPI_INSTANCE SPI1` | `gyro_1_spibus` | Extract number: `'1'` |
| `FLASH_SPI_INSTANCE SPI2` | `flash_spi_bus` | Extract number: `'2'` |
| `SDCARD_SPI_INSTANCE SPI3` | `sdcard_spi_bus` | Extract number: `'3'` |
| `MAX7456_SPI_INSTANCE SPI2` | `max7456_spi_bus` | Extract number: `'2'` |
| `DEFAULT_BLACKBOX_DEVICE BLACKBOX_DEVICE_FLASH` | `blackbox_device` | `'SPIFLASH'` |
| `DEFAULT_BLACKBOX_DEVICE BLACKBOX_DEVICE_SDCARD` | `blackbox_device` | `'SDCARD'` |
| `DEFAULT_CURRENT_METER_SCALE 170` | `ibata_scale` | `'170'` |
| `GYRO_1_ALIGN CW180_DEG` | `gyro_1_sensor_align` | `'CW180'` |

### Not Parsed (Present but Unused)

| Define | Purpose |
|--------|---------|
| `DEFAULT_DSHOT_BURST` | DShot DMA burst mode |
| `DEFAULT_DSHOT_BITBANG` | DShot bitbang mode |
| `SYSTEM_HSE_MHZ` | External oscillator frequency |
| `BEEPER_INVERTED` | Beeper polarity |
| `BEEPER_PWM_HZ` | Beeper PWM frequency |
| `SDCARD_DETECT_INVERTED` | SD detect polarity |
| `SDIO_DEVICE` | SDIO peripheral instance |
| `SDIO_USE_4BIT` | SDIO bus width |
| `SERIALRX_UART` | Receiver UART port |
| `SERIALRX_PROVIDER` | Receiver protocol |
| `ADC{n}_DMA_OPT` | ADC DMA configuration |
| `SPI{n}_TX_DMA_OPT` | SPI TX DMA configuration |
| `SPI{n}_RX_DMA_OPT` | SPI RX DMA configuration |

---

## DMA Option Resolution

The `dma_opt` parameter in `TIMER_PIN_MAP` and the standalone `*_DMA_OPT` defines
specify which DMA stream/channel a peripheral should use. The meaning of the value
depends on the MCU family.

**Source:** `betaflight/src/platform/STM32/dma_reqmap_mcu.c` contains all mapping
tables and lookup functions. `betaflight/src/main/drivers/dma_reqmap.h` defines the
data structures.

### F4/F7 — Index into Per-Channel Options Array

On F4/F7, DMA streams have fixed peripheral-to-stream mappings (no DMAMUX). Each
timer channel has 1–3 valid DMA streams. `dma_opt` is an index into this per-channel
array.

**Betaflight's `dmaTimerMapping[]` table (F4/F7):**

```c
// Format: { timer, channel, { DMA(controller, stream, channel_select), ... } }
// dma_opt=0 selects [0], dma_opt=1 selects [1], etc.

{ TIM1, CH1, { DMA(2,6,0), DMA(2,1,6), DMA(2,3,6) } },  // 3 options
{ TIM1, CH2, { DMA(2,6,0), DMA(2,2,6) } },                // 2 options
{ TIM1, CH3, { DMA(2,6,0), DMA(2,6,6) } },                // 2 options
{ TIM1, CH4, { DMA(2,4,6) } },                            // 1 option

{ TIM2, CH1, { DMA(1,5,3) } },
{ TIM2, CH2, { DMA(1,6,3) } },
{ TIM2, CH3, { DMA(1,1,3) } },
{ TIM2, CH4, { DMA(1,7,3), DMA(1,6,3) } },

{ TIM3, CH1, { DMA(1,4,5) } },
{ TIM3, CH2, { DMA(1,5,5) } },
{ TIM3, CH3, { DMA(1,7,5) } },
{ TIM3, CH4, { DMA(1,2,5) } },

{ TIM4, CH1, { DMA(1,0,2) } },
{ TIM4, CH2, { DMA(1,3,2) } },
{ TIM4, CH3, { DMA(1,7,2) } },

{ TIM5, CH1, { DMA(1,2,6) } },
{ TIM5, CH2, { DMA(1,4,6) } },
{ TIM5, CH3, { DMA(1,0,6) } },
{ TIM5, CH4, { DMA(1,1,6), DMA(1,3,6) } },

{ TIM8, CH1, { DMA(2,2,0), DMA(2,2,7) } },
{ TIM8, CH2, { DMA(2,2,0), DMA(2,3,7) } },
{ TIM8, CH3, { DMA(2,2,0), DMA(2,4,7) } },
{ TIM8, CH4, { DMA(2,7,7) } },
```

**Note on TIM1 option 0:** For TIM1 CH1–CH3, `dma_opt=0` is `DMA(2,6,0)` — DMA2
Stream 6 with Channel Select 0, which is the `TIM1_UP` DMA request. This is the
stream used for DMAR burst mode (all channels share one stream via `TIMx->DMAR`).
Options 1+ are per-channel dedicated streams. A board author choosing `dma_opt=1`
is explicitly selecting per-channel DMA over burst.

**Example: JHEF411 (F411)**

```c
TIMER_PIN_MAP( 1, PA8 , 1, 1)   // TIM1_CH1, dma_opt=1 → DMA(2,1,6) = DMA2 Stream 1, Ch 6
TIMER_PIN_MAP( 2, PA9 , 1, 1)   // TIM1_CH2, dma_opt=1 → DMA(2,2,6) = DMA2 Stream 2, Ch 6
TIMER_PIN_MAP( 3, PA10, 1, 1)   // TIM1_CH3, dma_opt=1 → DMA(2,6,6) = DMA2 Stream 6, Ch 6
TIMER_PIN_MAP( 4, PB0 , 2, 0)   // TIM3_CH3, dma_opt=0 → DMA(1,7,5) = DMA1 Stream 7, Ch 5
TIMER_PIN_MAP( 5, PB4 , 1, 0)   // TIM3_CH1, dma_opt=0 → DMA(1,4,5) = DMA1 Stream 4, Ch 5
```

Translation requires the `dmaTimerMapping[]` table: look up `(timer, channel)`, then
index by `dma_opt` to get `(DMA controller, stream, channel_select)`.

### G4/H7 — Flat Index into All DMA Streams

On G4/H7, DMAMUX hardware allows any DMA stream to serve any peripheral. `dma_opt`
is a direct index into a flat array of all available streams:

```c
// G4: 16 channels (DMA1 has 8, DMA2 has 8), 1-based channel numbering
dmaChannelSpec[0]  = DMA1_Channel1    dmaChannelSpec[8]  = DMA2_Channel1
dmaChannelSpec[1]  = DMA1_Channel2    dmaChannelSpec[9]  = DMA2_Channel2
...                                   ...
dmaChannelSpec[7]  = DMA1_Channel8    dmaChannelSpec[15] = DMA2_Channel8

// H7: 16 streams (DMA1 has 8, DMA2 has 8), 0-based stream numbering
dmaChannelSpec[0]  = DMA1_Stream0     dmaChannelSpec[8]  = DMA2_Stream0
dmaChannelSpec[1]  = DMA1_Stream1     dmaChannelSpec[9]  = DMA2_Stream1
...                                   ...
dmaChannelSpec[7]  = DMA1_Stream7     dmaChannelSpec[15] = DMA2_Stream7
```

Translation is arithmetic:
- **G4:** `DMA controller = (dma_opt / 8) + 1`, `channel = (dma_opt % 8) + 1`
- **H7:** `DMA controller = (dma_opt / 8) + 1`, `stream = dma_opt % 8`

Which peripheral request to route through DMAMUX is determined separately — BF looks
up the timer channel in `dmaTimerMapping[]` to get the `DMA_REQUEST_TIMx_CHy` constant,
then writes it into the chosen stream's DMAMUX configuration via `dmaSetupRequest()`.
The converter doesn't need the request ID — our DShot library resolves it from the
timer/channel at runtime.

**Example: MATEKH743 (H743)**

```c
TIMER_PIN_MAP( 0, PB0 , 2, 0)   // dma_opt=0  → DMA1 Stream 0
TIMER_PIN_MAP( 1, PB1 , 2, 1)   // dma_opt=1  → DMA1 Stream 1
TIMER_PIN_MAP( 2, PA0 , 2, 2)   // dma_opt=2  → DMA1 Stream 2
TIMER_PIN_MAP( 3, PA1 , 2, 3)   // dma_opt=3  → DMA1 Stream 3
TIMER_PIN_MAP( 4, PA2 , 2, 4)   // dma_opt=4  → DMA1 Stream 4
TIMER_PIN_MAP( 5, PA3 , 2, 5)   // dma_opt=5  → DMA1 Stream 5
TIMER_PIN_MAP( 6, PD12, 1, 6)   // dma_opt=6  → DMA1 Stream 6
TIMER_PIN_MAP( 7, PD13, 1, 7)   // dma_opt=7  → DMA1 Stream 7
TIMER_PIN_MAP(10, PA8 , 1, 8)   // dma_opt=8  → DMA2 Stream 0
```

Standalone DMA option defines use the same flat index:
```c
#define ADC1_DMA_OPT        9    // → DMA2 Stream 1
#define ADC3_DMA_OPT        10   // → DMA2 Stream 2
#define TIMUP3_DMA_OPT      11   // → DMA2 Stream 3
#define TIMUP4_DMA_OPT      12   // → DMA2 Stream 4
#define TIMUP5_DMA_OPT      13   // → DMA2 Stream 5
```

This shows how H7 board authors allocate the full DMA budget: motors get DMA1
streams 0–7, then ADC and timer-UP DMA spill into DMA2.

### DSHOT_BURST Define

```c
#define DEFAULT_DSHOT_BURST DSHOT_DMAR_ON    // Force DMAR burst mode
#define DEFAULT_DSHOT_BURST DSHOT_DMAR_OFF   // Force per-channel DMA
// (absent) → auto-detect based on timer DMA conflicts
```

When `DSHOT_DMAR_ON`, BF uses the `TIM_UP` DMA stream (dma_opt=0 for TIM1/TIM8 on
F4/F7) to drive all channels on a timer via `TIMx->DMAR`. On G4/H7, the TIMUP DMA
options (`TIMUP3_DMA_OPT`, etc.) specify which stream carries the burst. This define
is currently in the "Not Parsed" section — it will become relevant when the converter
emits DMA fields in BoardConfig.

### Converter Implementation Notes

To translate `dma_opt` to `DMAResource` fields in generated BoardConfig:

**F4/F7:** Embed the `dmaTimerMapping[]` table (~25 entries) in the converter. Given
a motor's resolved `(timer, channel)` and its `dma_opt` value, index into the table
to get `(DMA controller, stream, channel_select)`. If `dma_opt=-1`, omit the DMA
field (runtime discovery).

**G4/H7:** Pure arithmetic — no table needed. `dma_opt` directly encodes the
stream index. If `dma_opt=-1`, omit the DMA field.

**All families:** Motors with `dma_opt=-1` get no DMA override in BoardConfig and
fall back to the DShot library's runtime discovery (greedy scan on G4/H7, table
lookup on F4/F7).

---

## Complete Example: JHEF411 config.h

```c
#pragma once

#define FC_TARGET_MCU     STM32F411
#define BOARD_NAME        JHEF411
#define MANUFACTURER_ID   JHEF

#define USE_GYRO_SPI_MPU6000
#define USE_ACC_SPI_MPU6000
#define USE_GYRO_SPI_ICM42688P
#define USE_ACC_SPI_ICM42688P
#define USE_BARO_BMP280
#define USE_BARO_DPS310
#define USE_FLASH
#define USE_FLASH_W25Q128FV
#define USE_MAX7456

#define MOTOR1_PIN           PA8
#define MOTOR2_PIN           PA9
#define MOTOR3_PIN           PA10
#define MOTOR4_PIN           PB0
#define MOTOR5_PIN           PB4
#define UART1_TX_PIN         PB6
#define UART2_TX_PIN         PA2
#define UART1_RX_PIN         PB7
#define UART2_RX_PIN         PA3
#define I2C1_SCL_PIN         PB8
#define I2C1_SDA_PIN         PB9
#define LED0_PIN             PC13
#define SPI1_SCK_PIN         PA5
#define SPI2_SCK_PIN         PB13
#define SPI1_SDI_PIN         PA6
#define SPI2_SDI_PIN         PB14
#define SPI1_SDO_PIN         PA7
#define SPI2_SDO_PIN         PB15
#define ADC_VBAT_PIN         PA0
#define ADC_CURR_PIN         PA1
#define FLASH_CS_PIN         PB2
#define GYRO_1_EXTI_PIN      PB3
#define GYRO_1_CS_PIN        PA4

#define TIMER_PIN_MAPPING \
    TIMER_PIN_MAP( 0, PA2 , 3, -1) \
    TIMER_PIN_MAP( 1, PA8 , 1,  1) \
    TIMER_PIN_MAP( 2, PA9 , 1,  1) \
    TIMER_PIN_MAP( 3, PA10, 1,  1) \
    TIMER_PIN_MAP( 4, PB0 , 2,  0) \
    TIMER_PIN_MAP( 5, PB4 , 1,  0)

#define DEFAULT_BLACKBOX_DEVICE     BLACKBOX_DEVICE_FLASH
#define DEFAULT_CURRENT_METER_SCALE 170
#define FLASH_SPI_INSTANCE SPI2
#define GYRO_1_SPI_INSTANCE SPI1
#define GYRO_1_ALIGN CW180_DEG
```

### Converter Output

From the above config.h, the converter generates `output/JHEF-JHEF411.h`:

```cpp
namespace BoardConfig {
  // Storage: W25Q128FV SPI flash on SPI2
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB15, PB14, PB13, PB2, 8000000};

  // IMU: MPU6000, ICM42688P on SPI1
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 8000000};
  static constexpr IMUConfig imu{imu_spi, PB3, 1000000};

  // I2C1: Environmental sensors
  static constexpr I2CConfig sensors{PB9, PB8, 400000};

  // UARTs
  static constexpr UARTConfig uart1{PB6, PB7, 115200};
  static constexpr UARTConfig uart2{PA2, PA3, 115200};

  // ADC: Battery voltage and current monitoring
  static constexpr ADCConfig battery{PA0, PA1, 110, 170};

  // Status LEDs
  static constexpr LEDConfig status_leds{PC13};

  // Motors: ONESHOT125 protocol (125-250 µs)
  namespace Motor {
    static constexpr uint32_t frequency_hz = 2000;
    static constexpr MotorConfig motors[] = {
      {TIM1, PA8, 1, 125, 250},       // Motor 1: TIM1_CH1
      {TIM1, PA9, 2, 125, 250},       // Motor 2: TIM1_CH2
      {TIM1, PA10, 3, 125, 250},      // Motor 3: TIM1_CH3
      {TIM3, PB0_ALT1, 3, 125, 250},  // Motor 4: TIM3_CH3
      {TIM3, PB4, 1, 125, 250},       // Motor 5: TIM3_CH1
    };
    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}
```

---

## References

- Betaflight source: `betaflight/src/config/configs/<TARGET>/config.h`
- Creating Configurations: https://betaflight.com/docs/development/manufacturer/creating-configuration
- PeripheralPins.c: `Arduino_Core_STM32/variants/STM32xxxx/<variant>/PeripheralPins.c`
