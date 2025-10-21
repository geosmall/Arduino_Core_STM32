# Betaflight Configuration Format Research

## Overview

Betaflight uses unified target configuration files (`.config`) to define flight controller hardware. These files contain pin mappings, peripheral configurations, and system settings.

**Note**: Unified Targets are being deprecated in favor of `config.h` files in Betaflight 4.5.0+, but the unified target format is still widely used and documented.

## Configuration File Structure

### Header
```
# Betaflight / STM32F405 (S405) 4.x.x [build date] / [build hash]
board_name <BOARD_NAME>
manufacturer_id <MANUFACTURER_ID>
```

### Resource Definitions
Format: `resource <RESOURCE_TYPE> <INDEX> <PIN>`

Example:
```
resource MOTOR 1 B04
resource SERIAL_TX 1 A09
resource FLASH_CS 1 C13
resource GYRO_CS 1 A04
resource GYRO_EXTI 1 C04
```

### Timer Mappings
Format: `timer <PIN> <ALTERNATE_FUNCTION>`

Example:
```
timer B04 AF2
timer A03 AF3
```

Timer format can also include full details:
```
# TIM2 CH2 (AF1) on PB03
timer B03 AF1
```

### DMA Configurations
Format: `dma <PERIPHERAL> <INDEX> <DMA_STREAM> <DMA_CHANNEL>`

Example:
```
dma ADC 1 1
dma pin B04 0
```

### Feature Flags
```
feature RX_SERIAL
feature SOFTSERIAL
feature OSD
```

### Driver Definitions
```
#define USE_GYRO
#define USE_GYRO_SPI_MPU6000
#define USE_GYRO_SPI_ICM42688P
#define USE_ACC_SPI_MPU6000
#define USE_FLASH
#define USE_FLASH_M25P16
#define USE_MAX7456
```

### Serial Port Functions

Serial ports have function codes that define their purpose:
- `0` = Disabled
- `1` = MSP (Configurator/CLI)
- `2` = GPS
- `4` = TELEMETRY_FRSKY
- `8` = TELEMETRY_HOTT
- `16` = TELEMETRY_LTM
- `32` = TELEMETRY_SMARTPORT
- `64` = RX_SERIAL (receiver input)
- `128` = BLACKBOX
- `256` = TELEMETRY_MAVLINK
- `512` = ESC_SENSOR
- `1024` = TBS_SMARTAUDIO
- `2048` = IRC_TRAMP
- `4096` = RUNCAM_DEVICE_CONTROL
- `8192` = LIDAR_TF

Format: `serial <port> <function> <baud_rx> <baud_tx> <baud_min> <baud_max>`

Example:
```
serial 1 64 115200 57600 0 115200    # UART1 = RX_SERIAL (SBUS)
serial 2 32 57600 115200 0 115200    # UART2 = TELEMETRY_SMARTPORT
serial 3 1 115200 115200 0 115200    # UART3 = MSP (Configurator)
```

### Configuration Settings
Format: `set <parameter> = <value>`

Example:
```
set blackbox_device = SPIFLASH
set flash_spi_bus = 2
set gyro_1_bustype = SPI
set gyro_1_spibus = 1
set gyro_1_sensor_align = CW0
set serialrx_provider = SBUS
set mag_bustype = I2C
set mag_i2c_device = 1
set baro_bustype = I2C
set sdcard_mode = SPI
set sdcard_spi_bus = 3
```

## Complete Resource Type List

### Motor/Servo Outputs
- `MOTOR` - ESC/motor outputs (1-8+)
- `SERVO` - Servo outputs (1-8+)

### Serial Communication
- `SERIAL_TX` - UART transmit pins (1-8)
- `SERIAL_RX` - UART receive pins (1-8)
- `SOFTSERIAL_TX` - Software serial TX (11-12)
- `SOFTSERIAL_RX` - Software serial RX (11-12)

### SPI Bus
- `SPI_SCK` - SPI clock
- `SPI_MISO` (or `SPI_SDI`) - SPI master in, slave out
- `SPI_MOSI` (or `SPI_SDO`) - SPI master out, slave in

### I2C Bus
- `I2C_SCL` - I2C clock
- `I2C_SDA` - I2C data

### Storage
- `FLASH_CS` - SPI flash chip select
- `SDCARD_CS` - SD card chip select (SPI mode)
- `SDCARD_DETECT` - SD card detection pin

### Sensors
- `GYRO_CS` - Gyro/IMU chip select (SPI)
- `GYRO_EXTI` - Gyro external interrupt (data ready)
- `BARO_CS` - Barometer chip select
- `BARO_EXTI` - Barometer external interrupt

### OSD
- `OSD_CS` - OSD chip select (MAX7456, etc)

### Input
- `PPM` - PPM receiver input
- `PWM` - PWM input channels (1-8)
- `RX_BIND` - Receiver bind button
- `RX_SPI_CS` - SPI receiver chip select
- `RX_SPI_EXTI` - SPI receiver interrupt

### ADC (Analog)
- `ADC_BATT` - Battery voltage sensor
- `ADC_CURR` - Current sensor
- `ADC_RSSI` - Analog RSSI input
- `ADC_EXT` - External ADC input

### Indicators/Control
- `BEEPER` - Buzzer/beeper
- `LED` - Status LEDs (1-3)
- `LED_STRIP` - WS2812 LED strip
- `CAMERA_CONTROL` - Camera control output

### Receivers
- `RX_SPI_CS` - SPI receiver chip select
- `RX_SPI_EXTI` - SPI receiver external interrupt (DIO1)
- `RX_SPI_BIND` - SPI receiver bind button
- `RX_SPI_LED` - SPI receiver status LED
- `RX_SPI_EXPRESSLRS_RESET` - ExpressLRS reset pin
- `RX_SPI_EXPRESSLRS_BUSY` - ExpressLRS busy pin
- `RX_SPI_CC2500_TX_EN` - CC2500 TX enable
- `RX_SPI_CC2500_LNA_EN` - CC2500 LNA enable
- `RX_SPI_CC2500_ANT_SEL` - CC2500 antenna select

### Other
- `INVERTER` - Serial inverter control
- `ESCSERIAL` - ESC serial communication
- `PINIO` - Programmable I/O pins
- `USB_DETECT` - USB connection detection
- `SONAR_TRIGGER` - Ultrasonic sonar trigger
- `SONAR_ECHO` - Ultrasonic sonar echo

## Key Configuration Parameters

### Storage Configuration
```
set blackbox_device = SPIFLASH      # or SDCARD, SERIAL, NONE
set flash_spi_bus = 2                # SPI bus number for flash
set sdcard_mode = SPI                # or SDIO
set sdcard_spi_bus = 3               # SPI bus for SD card
```

### IMU/Gyro Configuration
```
set gyro_1_bustype = SPI             # or I2C
set gyro_1_spibus = 1                # SPI bus number
set gyro_1_i2cBus = 1                # I2C bus number
set gyro_1_i2c_address = 104         # I2C address
set gyro_1_sensor_align = CW0        # Sensor alignment
set gyro_2_spibus = 2                # Secondary gyro (if present)
```

### Magnetometer Configuration
```
set mag_bustype = I2C                # or SPI
set mag_i2c_device = 1               # I2C bus number
set mag_hardware = AUTO              # or QMC5883, HMC5883, LIS3MDL, etc
```

### Barometer Configuration
```
set baro_bustype = I2C               # or SPI
set baro_i2c_device = 1              # I2C bus number
set baro_hardware = AUTO             # or BMP280, DPS310, MS5611, etc
```

### Serial Configuration
```
set serialrx_provider = SBUS         # or CRSF, IBUS, DSM, etc
set serialrx_inverted = OFF          # Signal inversion
```

### OSD Configuration
```
set max7456_spi_bus = 2              # OSD chip SPI bus
```

### Current/Voltage Sensing
```
set current_meter = ADC              # or ESC, VIRTUAL, NONE
set battery_meter = ADC              # or ESC, NONE
set ibata_scale = 400                # Current sensor scale
set vbat_scale = 110                 # Voltage divider scale
```

### Receiver Configuration

#### Serial Receivers (SBUS, CRSF, ELRS UART)
```
# Serial port assignment (Ports tab in configurator)
serial <port> <functions> <baud> ...
# Example: serial 1 64 115200 ... (64 = RX_SERIAL function)

set serialrx_provider = CRSF         # or SBUS, IBUS, DSM, FPORT, etc
set serialrx_uart = SERIAL_PORT_USART1  # Which UART for receiver
set serialrx_inverted = OFF          # Signal inversion
set serialrx_halfduplex = OFF        # Half-duplex mode (FPORT)
```

#### SPI Receivers (ExpressLRS, FrSky, etc)
```
# Resource mappings
resource RX_SPI_CS 1 <pin>           # Chip select
resource RX_SPI_EXTI 1 <pin>         # External interrupt (DIO1 for ELRS)
resource RX_SPI_BIND 1 <pin>         # Bind button
resource RX_SPI_LED 1 <pin>          # Status LED
resource RX_SPI_EXPRESSLRS_RESET 1 <pin>  # ELRS reset pin
resource RX_SPI_EXPRESSLRS_BUSY 1 <pin>   # ELRS busy pin

# Configuration
set rx_spi_protocol = EXPRESSLRS     # or FRSKY_D, FRSKY_X, etc
set rx_spi_bus = 3                   # SPI bus number
set rx_spi_led_inversion = ON        # LED polarity

# CC2500-specific (FrSky)
resource RX_SPI_CC2500_TX_EN 1 <pin>
resource RX_SPI_CC2500_LNA_EN 1 <pin>
resource RX_SPI_CC2500_ANT_SEL 1 <pin>
set cc2500_spi_chip_detect = OFF

# ExpressLRS binding
set expresslrs_uid = 0,1,2,3,4,5     # UID from binding phrase
```

#### Receiver Features
```
feature RX_SERIAL                    # Enable serial receiver
feature RX_SPI                       # Enable SPI receiver
feature RX_PPM                       # Enable PPM receiver
feature RX_PARALLEL_PWM              # Enable PWM receiver
```

## Pin Naming Convention

### Betaflight Format
Betaflight uses short pin names:
- `A09` = PA9 (Port A, pin 9)
- `B04` = PB4 (Port B, pin 4)
- `C13` = PC13 (Port C, pin 13)
- `E11` = PE11 (Port E, pin 11)

### Arduino STM32 Pin Formats

**Two distinct formats exist:**

1. **PinName Enums** (HAL/low-level): `PA_0`, `PB_15`, `PB_0_ALT1`
   - Used in PeripheralPins.c mapping tables
   - Includes ALT variants for alternate peripheral functions
   - Underscore separators: `P{port}_{pin}[_ALT{n}]`

2. **Arduino Pin Macros** (Arduino API): `PA0`, `PB15`, `PB0_ALT1`
   - Used in sketches and BoardConfig headers
   - Can include ALT suffix when alternate timer mapping needed
   - No underscore before pin number: `P{port}{pin}[_ALT{n}]`

### ALT Pin Variants (CRITICAL)

Some pins support multiple peripheral functions on different timers. When a pin's default timer mapping conflicts with Betaflight's assignment, an ALT variant must be used.

**Example - Motor 4 on JHEF411:**
```
# Betaflight config
resource MOTOR 4 B00
timer B00 AF2         # Requests TIM3_CH3

# PeripheralPins.c shows TWO mappings for PB_0:
{PB_0, TIM1, ..._AF1_TIM1, 2, 1}    # Default: TIM1_CH2N (AF1)
{PB_0_ALT1, TIM3, ..._AF2_TIM3, 3, 0}  # ALT1: TIM3_CH3 (AF2)

# Generated config MUST use ALT variant:
static constexpr Channel motor4 = {PB0_ALT1, 3, ...};  // ✅ Correct
// NOT: {PB0, 3, ...};  // ❌ Wrong - would use TIM1_CH2N instead
```

**Converter Requirements:**
1. Parse Betaflight timer assignments: `timer {pin} AF{n}`
2. Cross-validate against PeripheralPins.c timer mappings
3. If base pin doesn't support requested timer/AF, search for ALT variant
4. Generate Arduino format with ALT suffix when needed: `PB0_ALT1`

## Timer and DMA Requirements

### Motor Outputs
- Each motor requires:
  - A pin with timer capability
  - Timer channel assignment
  - DMA channel (for DShot protocol)

### LED Strip
- Requires dedicated timer + DMA channel
- Usually mapped to avoid conflicts

### DMA Conflicts
- DShot cannot share DMA channels
- Multiple motors on same timer OK if using different channels
- Resource remapping may be needed to resolve conflicts

## Example Configuration Files

### STM32F411 with SPI Flash (JHEF-JHEF411 / NOXE V3)
```
# Betaflight / STM32F411 (S411) 4.2.0 Jun 14 2020 / 03:04:43 (8f2d21460) MSP API: 1.43

board_name JHEF411
manufacturer_id JHEF

# Supported IMU sensors
#define USE_ACC
#define USE_ACC_SPI_MPU6000
#define USE_GYRO
#define USE_GYRO_SPI_MPU6000
#define USE_GYRO_SPI_ICM42688P
#define USE_ACC_SPI_ICM42688P

# Barometer support
#define USE_BARO
#define USE_BARO_BMP280
#define USE_BARO_DPS310

# Storage and OSD
#define USE_FLASH
#define USE_FLASH_W25Q128FV
#define USE_MAX7456

# Motor outputs (5 motors supported)
resource MOTOR 1 A08
resource MOTOR 2 A09
resource MOTOR 3 A10
resource MOTOR 4 B00
resource MOTOR 5 B04

# Serial ports
resource SERIAL_TX 1 B06
resource SERIAL_TX 2 A02
resource SERIAL_RX 1 B07
resource SERIAL_RX 2 A03

# I2C bus (for baro/mag)
resource I2C_SCL 1 B08
resource I2C_SDA 1 B09

# SPI buses
resource SPI_SCK 1 A05
resource SPI_SCK 2 B13
resource SPI_MISO 1 A06
resource SPI_MISO 2 B14
resource SPI_MOSI 1 A07
resource SPI_MOSI 2 B15

# Storage and peripherals
resource FLASH_CS 1 B02      # W25Q128FV SPI flash (SPI2)
resource OSD_CS 1 B12        # MAX7456 OSD (SPI2)
resource GYRO_CS 1 A04       # IMU chip select (SPI1)
resource GYRO_EXTI 1 B03     # IMU interrupt

# ADC inputs
resource ADC_BATT 1 A00      # Battery voltage
resource ADC_CURR 1 A01      # Current sensor
resource ADC_RSSI 1 B01      # RSSI input

# Other
resource BEEPER 1 C14
resource LED 1 C13
resource LED_STRIP 1 A15
resource CAMERA_CONTROL 1 B10
resource PINIO 1 B05
resource USB_DETECT 1 C15

# Timer assignments with alternate functions
timer A08 AF1  # TIM1 CH1 - MOTOR 1
timer A09 AF1  # TIM1 CH2 - MOTOR 2
timer A10 AF1  # TIM1 CH3 - MOTOR 3
timer B00 AF2  # TIM3 CH3 - MOTOR 4
timer B04 AF2  # TIM3 CH1 - MOTOR 5
timer A15 AF1  # TIM2 CH1 - LED_STRIP

# DMA assignments
dma pin A08 1  # DMA2 Stream 1 Channel 6
dma pin A09 1  # DMA2 Stream 2 Channel 6
dma pin A10 1  # DMA2 Stream 6 Channel 6
dma pin B00 0  # DMA1 Stream 7 Channel 5
dma pin B04 0  # DMA1 Stream 4 Channel 5
dma pin A15 0  # DMA1 Stream 5 Channel 3

# Features
feature OSD
feature RX_SERIAL

# Serial configuration
serial 1 64 115200 57600 0 115200  # UART1 = RX_SERIAL (SBUS)

# System settings
set mag_bustype = I2C
set mag_i2c_device = 1
set baro_bustype = I2C
set baro_i2c_device = 1
set blackbox_device = SPIFLASH
set flash_spi_bus = 2
set max7456_spi_bus = 2
set gyro_1_bustype = SPI
set gyro_1_spibus = 1
set gyro_1_sensor_align = CW180
set gyro_1_align_yaw = 1800
set dshot_burst = ON
set motor_pwm_protocol = DSHOT300
set current_meter = ADC
set battery_meter = ADC
set ibata_scale = 170
set beeper_inversion = ON
set beeper_od = OFF
set system_hse_mhz = 8
```

### STM32F722 with SD Card
```
board_name MATEKF722SE
manufacturer_id MTKS

resource SDCARD_CS 1 A15

#define USE_SDCARD

set blackbox_device = SDCARD
set sdcard_mode = SPI
set sdcard_spi_bus = 3
```

### Dual Gyro Configuration
```
resource GYRO_CS 1 A04
resource GYRO_CS 2 B12
resource GYRO_EXTI 1 C04
resource GYRO_EXTI 2 D15

#define USE_GYRO_SPI_MPU6000
#define USE_GYRO_SPI_ICM42688P

set gyro_1_bustype = SPI
set gyro_1_spibus = 1
set gyro_1_sensor_align = CW0
set gyro_2_bustype = SPI
set gyro_2_spibus = 3
set gyro_2_sensor_align = CW270
```

### SPI Receiver (FrSky X - MATEKF411RX)
```
board_name MATEKF411RX
manufacturer_id MTKS

resource RX_SPI_CS 1 A15
resource RX_SPI_EXTI 1 C14
resource RX_SPI_BIND 1 B02
resource RX_SPI_LED 1 B09
resource RX_SPI_CC2500_TX_EN 1 A08
resource RX_SPI_CC2500_LNA_EN 1 A13
resource RX_SPI_CC2500_ANT_SEL 1 A14

#define USE_RX_SPI
#define USE_RX_FRSKY_SPI_X
#define USE_RX_CC2500_SPI_PA_LNA

set rx_spi_protocol = FRSKY_X
set rx_spi_bus = 3
set rx_spi_led_inversion = ON
set cc2500_spi_chip_detect = OFF
```

### SPI Receiver (ExpressLRS)
```
resource RX_SPI_CS 1 C15
resource RX_SPI_EXTI 1 C13
resource RX_SPI_EXPRESSLRS_BUSY 1 C14
resource RX_SPI_EXPRESSLRS_RESET 1 <pin>

#define USE_RX_SPI
#define USE_RX_EXPRESSLRS

set rx_spi_protocol = EXPRESSLRS
set rx_spi_bus = 3
set expresslrs_uid = 0,1,2,3,4,5
```

### Serial Receiver with Telemetry Examples

#### SBUS (FrSky) - Unidirectional
```
# RX only, no telemetry
resource SERIAL_RX 1 A10             # SBUS receiver input

serial 1 64 115200 57600 0 115200   # Function 64 = RX_SERIAL

feature RX_SERIAL

set serialrx_provider = SBUS
set serialrx_uart = SERIAL_PORT_USART1
set serialrx_inverted = ON           # SBUS is inverted
```

#### SBUS + SmartPort (FrSky) - Bidirectional Telemetry
```
# SBUS on UART1 RX, SmartPort telemetry on UART2 TX
resource SERIAL_RX 1 A10             # SBUS receiver input
resource SERIAL_TX 2 A02             # SmartPort telemetry output

serial 1 64 115200 57600 0 115200   # UART1 = RX_SERIAL (SBUS)
serial 2 32 57600 115200 0 115200   # UART2 = TELEMETRY_SMARTPORT (function 32)

feature RX_SERIAL
feature TELEMETRY

set serialrx_provider = SBUS
set serialrx_uart = SERIAL_PORT_USART1
set serialrx_inverted = ON           # SBUS is inverted
set tlm_inverted = ON                # SmartPort is inverted
```

#### IBUS (FlySky) - Bidirectional with Telemetry
```
# Single UART, RX for control, TX for telemetry (full duplex)
resource SERIAL_TX 1 A09             # IBUS telemetry output
resource SERIAL_RX 1 A10             # IBUS receiver input

serial 1 64 115200 57600 0 115200   # Function 64 = RX_SERIAL

feature RX_SERIAL
feature TELEMETRY

set serialrx_provider = IBUS
set serialrx_uart = SERIAL_PORT_USART1
set serialrx_inverted = OFF          # IBUS is not inverted
set serialrx_halfduplex = OFF        # Uses both TX and RX pins
```

## Mapping to Your BoardConfig System

### JHEF411 → NOXE_V3 Actual Mapping

**Current NOXE_V3 BoardConfig** (`targets/NOXE_V3.h`):
```cpp
namespace BoardConfig {
  // Storage: High-speed SPI flash for configuration and logging (LittleFS backend)
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB15, PB14, PB13, PB12, 8000000};

  // IMU: High-performance IMU for flight control
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 8000000, CS_Mode::HARDWARE};
  static constexpr IMUConfig imu{imu_spi, PD2, 1000000};

  // GPS: High-precision GPS module
  static constexpr UARTConfig gps{PC6, PC7, 115200};

  // I2C: Environmental sensors
  static constexpr I2CConfig sensors{PB6, PB7, 400000};
}
```

**JHEF411 Betaflight Config vs Hardware Wiring Diagram**:

From the official JHEMCU F4 NOXE V3 wiring diagram, here are the actual pin assignments:

**Verified Hardware Pins** (from wiring diagram):
- **Motors (M1-M5)**: Labeled on board, timers from Betaflight config
  - M1: PA08 (TIM1_CH1)
  - M2: PA09 (TIM1_CH2)
  - M3: PA10 (TIM1_CH3)
  - M4: PB00 (TIM3_CH3)
  - M5: PB04 (TIM3_CH1)
- **UART1**: TX=PB06, RX=PB07 (labeled on board)
- **UART2**: TX=PA02, RX=PA03 (labeled on board)
- **I2C1**: SCL=PB08, SDA=PB09 (labeled "SCL/SDA Cap/GND")
- **SPI Flash**: CS=PB02 (from Betaflight config)
- **IMU (ICM42688P/MPU6000)**: CS=PA04, INT=PB03
- **OSD (MAX7456)**: CS=PB12 (likely, on SPI2)
- **LED Strip**: PA15
- **Beeper**: PC14
- **Camera Control**: PB10
- **ADC**: VBAT=PA00, CURR=PA01, RSSI=PB01
- **5.8GHz VTX Control**: Visible in diagram (TX2/RX2 or separate pins)

**NOXE_V3 BoardConfig Discrepancies**:

| Component | targets/NOXE_V3.h | JHEF411 Hardware | Status | Action Needed |
|-----------|-------------------|------------------|--------|---------------|
| **Storage SPI** | PB15/PB14/PB13/PB12 | PB15/PB14/PB13/PB02 | ❌ **CS WRONG** | Change storage CS: PB12→PB02 |
| **IMU SPI** | PA7/PA6/PA5/PA4 | PA7/PA6/PA5/PA4 | ✅ Match | OK |
| **IMU Interrupt** | PD2 | PB03 | ❌ **WRONG** | Change imu int_pin: PD2→PB03 |
| **I2C Sensors** | PB6/PB7 | PB08/PB09 | ❌ **WRONG** | Change I2C: PB6/PB7→PB08/PB09 |
| **GPS UART** | PC6/PC7 | Not available | ❌ **INVALID** | Remove or map to UART1/2 |

**CRITICAL FINDINGS**:
1. **FLASH_CS is WRONG**: targets/NOXE_V3.h uses PB12, but hardware uses **PB02**
   - PB12 is actually the **OSD_CS** pin
   - This would cause SPI flash to fail if OSD is present
2. **GYRO_EXTI is WRONG**: PD2 doesn't exist on this board, should be **PB03**
3. **I2C pins are WRONG**: Should be PB08/PB09 (hardware labeled)
4. **GPS pins don't exist**: PC6/PC7 not available on this F411 board

**Corrected NOXE_V3 Configuration**:
```cpp
namespace BoardConfig {
  // Storage: SPI2 flash (W25Q128FV) - CORRECTED CS PIN
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB15, PB14, PB13, PB02, 8000000};

  // IMU: SPI1 (ICM42688P/MPU6000) - CORRECTED INTERRUPT PIN
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 8000000, CS_Mode::HARDWARE};
  static constexpr IMUConfig imu{imu_spi, PB03, 1000000};

  // I2C: Barometer/Magnetometer - CORRECTED PINS
  static constexpr I2CConfig sensors{PB08, PB09, 400000};

  // UART1: Primary serial (RX receiver or MSP)
  static constexpr UARTConfig uart1{PB06, PB07, 115200};

  // UART2: Secondary serial (GPS, telemetry, or VTX control)
  static constexpr UARTConfig uart2{PA02, PA03, 115200};
}
```

### Generic Mapping Templates

#### Storage Mapping
| Betaflight | Your BoardConfig |
|------------|------------------|
| `resource FLASH_CS 1 <pin>` | `Storage::cs_pin` |
| `set flash_spi_bus = <n>` | SPI bus selection |
| `#define USE_FLASH_<chip>` | Chip type for LittleFS |
| `set blackbox_device = SPIFLASH` | `StorageBackend::LITTLEFS` |
| `resource SDCARD_CS 1 <pin>` | `Storage::cs_pin` |
| `set blackbox_device = SDCARD` | `StorageBackend::SDFS` |

#### IMU Mapping
| Betaflight | Your BoardConfig |
|------------|------------------|
| `resource GYRO_CS 1 <pin>` | `IMU::cs_pin` |
| `resource GYRO_EXTI 1 <pin>` | `IMU::int_pin` |
| `set gyro_1_spibus = <n>` | SPI bus selection |
| `#define USE_GYRO_SPI_ICM42688P` | Chip type detection |

#### Motor/ESC Mapping
| Betaflight | Your BoardConfig |
|------------|------------------|
| `resource MOTOR <n> <pin>` | `ESC::esc<n>.pin` |
| `timer <pin> AF<n>` | Timer bank assignment |
| DMA analysis | Channel validation |

#### Servo Mapping
| Betaflight | Your BoardConfig |
|------------|------------------|
| `resource SERVO <n> <pin>` | `Servo::pwm_output.pin` |
| `timer <pin> AF<n>` | Timer bank assignment |

#### Receiver Mapping
| Betaflight | Your BoardConfig | Notes |
|------------|------------------|-------|
| `resource SERIAL_RX <n> <pin>` | `Receiver::uart_rx_pin` | Serial RX input |
| `resource SERIAL_TX <n> <pin>` | `Receiver::uart_tx_pin` | Telemetry output |
| `set serialrx_provider = SBUS` | `Receiver::protocol` | SBUS, IBUS, etc |
| `set serialrx_uart = SERIAL_PORT_USART1` | `Receiver::uart_num` | UART number |
| `set serialrx_inverted = ON` | `Receiver::inverted` | Signal inversion |
| `serial <n> 64 ...` | UART RX enable | Function 64 = RX_SERIAL |
| `serial <n> 32 ...` | UART telemetry | Function 32 = TELEMETRY_SMARTPORT |
| `feature TELEMETRY` | `Receiver::telemetry_enabled` | Enable telemetry |
| `set tlm_inverted = ON` | `Receiver::tlm_inverted` | Telemetry inversion |

## Research Completion Summary

### What We've Learned

✅ **Betaflight Config Format** - Complete understanding of resource/timer/dma/set syntax
✅ **Pin Mappings** - JHEF411 verified against NOXE V3 hardware wiring diagram
✅ **Critical Bug Fixes** - Corrected NOXE_V3.h (FLASH_CS, IMU_INT, I2C pins)
✅ **madflight Converter** - Analyzed implementation, mapping strategy, limitations

### Key Insights

**Simple parsing works**: Line-by-line regex sufficient (no complex state machine needed)
**Two-stage vs compile-time**: madflight uses runtime parsing, we want compile-time validation
**Resource coverage**: madflight ignores FLASH_CS, LED_STRIP, OSD_CS - we should map these
**Bus indexing**: Always convert Betaflight 1-based to 0-based
**Ambiguity handling**: Multiple sensors require user selection or policy (first/last)

### Next Steps Recommendation

**Option A - Implement Minimal Converter** (Recommended):
1. Design ConfigTypes extensions (MotorConfig, ADCConfig, ReceiverConfig)
2. Write Python parser for subset: MOTOR, SERIAL, ADC, LED_STRIP
3. Generate JHEF411 BoardConfig and compare to corrected NOXE_V3.h
4. Iterate and expand

**Option B - Design-First Approach**:
1. Complete ConfigTypes design document
2. Define full mapping specification
3. Write comprehensive parser with validation
4. Test against 10+ board configs

**Option C - Research More Boards**:
1. Collect F405/F411/F722/H743 configs from unified-targets
2. Identify common patterns and edge cases
3. Design converter to handle variations
4. Implement with test suite

**Recommended**: Start with **Option A** - Learn by building incrementally on known-good JHEF411 example.

## References

- Betaflight Unified Targets: https://github.com/betaflight/unified-targets
- Resource Remapping Guide: https://betaflight.com/docs/wiki/guides/current/Resource-remapping
- Creating Configurations: https://betaflight.com/docs/development/manufacturer/creating-configuration
- madflight Converter: https://github.com/qqqlab/madflight/tree/main/extras/betaflight_target_converter
- madflight Converter Analysis: See `CONVERTER_ANALYSIS.md` in this directory
