# Betaflight Config vs Generated Output Comparison

## JHEF-JHEF411 (NOXE V3 Flight Controller)

**Date**: 2026-02-18
**Generator Version**: betaflight_target_converter.py
**MCU**: STM32F411
**Source**: `bf_configs/JHEF411/config.h` (native Betaflight target)
**Output**: `output/JHEF-JHEF411.h`

---

## Board Identification

### Betaflight Config
```c
#define FC_TARGET_MCU     STM32F411
#define BOARD_NAME        JHEF411
#define MANUFACTURER_ID   JHEF
```

### Generated Output
```cpp
// Board: JHEF411
// Manufacturer: JHEF
// MCU: STM32F411
// Gyro: MPU6000, ICM42688P
namespace BoardConfig {
```

**Status**: ✅ Correctly extracted board metadata and gyro types from `USE_GYRO_SPI_*` defines

---

## Storage Configuration

### Betaflight Config
```c
#define USE_FLASH_W25Q128FV
#define FLASH_CS_PIN         PB2
#define SPI2_SCK_PIN         PB13
#define SPI2_SDI_PIN         PB14
#define SPI2_SDO_PIN         PB15
#define FLASH_SPI_INSTANCE SPI2
#define DEFAULT_BLACKBOX_DEVICE     BLACKBOX_DEVICE_FLASH
```

### Generated Output
```cpp
// Storage: W25Q128FV SPI flash on SPI2
static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB15, PB14, PB13, PB2, 8000000};
```

**Mapping**:
- Flash chip: W25Q128FV (from `USE_FLASH_W25Q128FV`) ✅
- SPI bus: SPI2 (from `FLASH_SPI_INSTANCE`) ✅
- CS pin: PB2 (from `FLASH_CS_PIN`) ✅
- MOSI: PB15 (from `SPI2_SDO_PIN`, SDO=MOSI) ✅
- MISO: PB14 (from `SPI2_SDI_PIN`, SDI=MISO) ✅
- SCLK: PB13 (from `SPI2_SCK_PIN`) ✅
- Frequency: 8 MHz (default) ✅
- Backend: LITTLEFS (inferred from `BLACKBOX_DEVICE_FLASH`) ✅

**Status**: ✅ Complete mapping with native pin format

---

## IMU Configuration

### Betaflight Config
```c
#define USE_GYRO_SPI_MPU6000
#define USE_ACC_SPI_MPU6000
#define USE_GYRO_SPI_ICM42688P
#define USE_ACC_SPI_ICM42688P
#define GYRO_1_CS_PIN        PA4
#define GYRO_1_EXTI_PIN      PB3
#define SPI1_SCK_PIN         PA5
#define SPI1_SDI_PIN         PA6
#define SPI1_SDO_PIN         PA7
#define GYRO_1_SPI_INSTANCE SPI1
```

### Generated Output
```cpp
// IMU: MPU6000, ICM42688P on SPI1
static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 8000000};
static constexpr IMUConfig imu{imu_spi, PB3, 1000000};
```

**Mapping**:
- Gyro types: MPU6000, ICM42688P (from `USE_GYRO_SPI_*` defines) ✅
- SPI bus: SPI1 (from `GYRO_1_SPI_INSTANCE`) ✅
- CS pin: PA4 (from `GYRO_1_CS_PIN`) ✅
- MOSI: PA7 (from `SPI1_SDO_PIN`) ✅
- MISO: PA6 (from `SPI1_SDI_PIN`) ✅
- SCLK: PA5 (from `SPI1_SCK_PIN`) ✅
- Interrupt: PB3 (from `GYRO_1_EXTI_PIN`) ✅
- Frequency: 8 MHz runtime, 1 MHz setup ✅

**Status**: ✅ Complete IMU configuration with dual gyro support

---

## I2C Sensors

### Betaflight Config
```c
#define USE_BARO_BMP280
#define USE_BARO_DPS310
#define I2C1_SCL_PIN         PB8
#define I2C1_SDA_PIN         PB9
#define MAG_I2C_INSTANCE I2CDEV_1
#define BARO_I2C_INSTANCE I2CDEV_1
```

### Generated Output
```cpp
// I2C1: Environmental sensors
static constexpr I2CConfig sensors{PB9, PB8, 400000};
```

**Mapping**:
- I2C bus: I2C1 (from `MAG_I2C_INSTANCE`, `BARO_I2C_INSTANCE`) ✅
- SDA: PB9 (from `I2C1_SDA_PIN`) ✅
- SCL: PB8 (from `I2C1_SCL_PIN`) ✅
- Frequency: 400 kHz (default) ✅

**Status**: ✅ Correct I2C configuration

---

## UART Configuration

### Betaflight Config
```c
#define UART1_TX_PIN         PB6
#define UART1_RX_PIN         PB7
#define UART2_TX_PIN         PA2
#define UART2_RX_PIN         PA3
```

### Generated Output
```cpp
// USART1: Serial port
static constexpr UARTConfig uart1{PB6, PB7, 115200};

// USART2: Serial port
static constexpr UARTConfig uart2{PA2, PA3, 115200};
```

**Mapping**:

**UART1**:
- TX: PB6 ✅
- RX: PB7 ✅
- Baud: 115200 (default) ✅

**UART2**:
- TX: PA2 ✅
- RX: PA3 ✅
- Baud: 115200 (default) ✅

**Status**: ✅ Both UARTs correctly mapped

---

## ADC Configuration

### Betaflight Config
```c
#define ADC_VBAT_PIN         PA0
#define ADC_CURR_PIN         PA1
#define DEFAULT_CURRENT_METER_SCALE 170
```

### Generated Output
```cpp
// ADC: Battery voltage and current monitoring
static constexpr ADCConfig battery{PA0, PA1, 110, 170};
```

**Mapping**:
- VBAT pin: PA0 ✅
- CURR pin: PA1 ✅
- VBAT scale: 110 (default) ✅
- CURR scale: 170 (from `DEFAULT_CURRENT_METER_SCALE`) ✅

**Status**: ✅ ADC configuration with extracted current scale

---

## Status LEDs

### Betaflight Config
```c
#define LED0_PIN             PC13
```

### Generated Output
```cpp
// Status LEDs
static constexpr LEDConfig status_leds{PC13};
```

**Mapping**:
- LED pin: PC13 (from `LED0_PIN`, 0-based index converted to 1-based) ✅

**Status**: ✅ LED configuration mapped

---

## Motor Configuration

### Betaflight Config
```c
#define MOTOR1_PIN           PA8
#define MOTOR2_PIN           PA9
#define MOTOR3_PIN           PA10
#define MOTOR4_PIN           PB0
#define MOTOR5_PIN           PB4

#define TIMER_PIN_MAPPING \
    TIMER_PIN_MAP( 0, PA2 , 3, -1) \
    TIMER_PIN_MAP( 1, PA8 , 1,  1) \
    TIMER_PIN_MAP( 2, PA9 , 1,  1) \
    TIMER_PIN_MAP( 3, PA10, 1,  1) \
    TIMER_PIN_MAP( 4, PB0 , 2,  0) \
    TIMER_PIN_MAP( 5, PB4 , 1,  0) \
    ...
```

### Generated Output
```cpp
// Motors: ONESHOT125 protocol (125-250 µs)
namespace Motor {
  static constexpr uint32_t frequency_hz = 2000;

  struct MotorConfig {
    TIM_TypeDef* timer;
    uint32_t pin;
    uint32_t channel;
    uint32_t min_us;
    uint32_t max_us;
  };

  // Motor array - hardware timer assignments from Betaflight config
  static constexpr MotorConfig motors[] = {
    {TIM1, PA8, 1, 125, 250},  // Motor 1: TIM1_CH1
    {TIM1, PA9, 2, 125, 250},  // Motor 2: TIM1_CH2
    {TIM1, PA10, 3, 125, 250},  // Motor 3: TIM1_CH3
    {TIM3, PB0_ALT1, 3, 125, 250},  // Motor 4: TIM3_CH3
    {TIM3, PB4, 1, 125, 250},  // Motor 5: TIM3_CH1
  };

  static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
};
```

**Mapping**:

**Motor 1** (TIM1_CH1):
- Pin: PA8 (from `MOTOR1_PIN`) ✅
- Timer: TIM1 (occurrence=1 in `TIMER_PIN_MAP`) ✅
- Channel: 1 ✅

**Motor 2** (TIM1_CH2):
- Pin: PA9 (from `MOTOR2_PIN`) ✅
- Timer: TIM1 (occurrence=1) ✅
- Channel: 2 ✅

**Motor 3** (TIM1_CH3):
- Pin: PA10 (from `MOTOR3_PIN`) ✅
- Timer: TIM1 (occurrence=1) ✅
- Channel: 3 ✅

**Motor 4** (TIM3_CH3):
- Pin: **PB0_ALT1** (from `MOTOR4_PIN PB0`, occurrence=2 selects ALT1) ✅
- Timer: TIM3 ✅
- Channel: 3 ✅
- **Note**: ALT1 variant required — PB0 default maps to TIM1_CH2N, ALT1 maps to TIM3_CH3

**Motor 5** (TIM3_CH1):
- Pin: PB4 (from `MOTOR5_PIN`) ✅
- Timer: TIM3 (occurrence=1) ✅
- Channel: 1 ✅

**Protocol**: OneShot125 (125-250 µs pulse, 2000 Hz)

**Status**: ✅ All 5 motors correctly mapped with ALT variant for motor 4

---

## ALT Variant Handling — Key Feature

### The PB0 Motor 4 Case Study

**Betaflight config.h**:
```c
#define MOTOR4_PIN           PB0
// TIMER_PIN_MAP( 4, PB0 , 2,  0)   ← occurrence=2
```

**PeripheralPins.c** for STM32F411 shows PB_0 has two timer entries:
```c
{PB_0,      TIM1, GPIO_AF1_TIM1, 2, 1},  // Entry 1 (default): TIM1_CH2N
{PB_0_ALT1, TIM3, GPIO_AF2_TIM3, 3, 0},  // Entry 2 (ALT1): TIM3_CH3
```

**Timer Occurrence Resolution**:
1. `TIMER_PIN_MAP(4, PB0, 2, 0)` specifies occurrence=2 for PB0
2. Converter looks up PB0 in PeripheralPins.c PinMap_TIM entries
3. Filters to non-complementary channels only
4. Selects 2nd entry → TIM3_CH3 with PB_0_ALT1
5. Generates `PB0_ALT1` in Arduino macro format

**Generated Output**:
```cpp
{TIM3, PB0_ALT1, 3, 125, 250},  // Motor 4: TIM3_CH3
```

**Why ALT1 is Required**:
- Without ALT suffix: `PB0` → defaults to TIM1_CH2N (wrong timer)
- With ALT suffix: `PB0_ALT1` → correctly maps to TIM3_CH3

---

## Second Board: MTKS-MATEKH743 (Servo + Multi-Motor)

**Source**: `bf_configs/MATEKH743/config.h`
**Output**: `output/MTKS-MATEKH743.h`

This board demonstrates servo outputs, 8 motors, and multiple ALT variants.

### Servo Configuration

**Betaflight Config**:
```c
#define SERVO1_PIN           PE5
#define SERVO2_PIN           PE6
// TIMER_PIN_MAP( 8, PE5 , 1, -1)   ← occurrence=1
// TIMER_PIN_MAP( 9, PE6 , 1, -1)   ← occurrence=1
```

**Generated Output**:
```cpp
namespace Servo {
  static constexpr uint32_t frequency_hz = 50;

  static constexpr ServoConfig servos[] = {
    {TIM15, PE5, 1, 1000, 2000},  // Servo 1: TIM15_CH1
    {TIM15, PE6, 2, 1000, 2000},  // Servo 2: TIM15_CH2
  };

  static constexpr int num_servos = sizeof(servos) / sizeof(servos[0]);
};
```

**Status**: ✅ Servos correctly separated from motors with 50 Hz frequency and 1000-2000 µs range

### Motor Configuration (8 motors, 3 timer banks)

**Betaflight Config**:
```c
#define MOTOR1_PIN           PB0
#define MOTOR2_PIN           PB1
#define MOTOR3_PIN           PA0
#define MOTOR4_PIN           PA1
#define MOTOR5_PIN           PA2
#define MOTOR6_PIN           PA3
#define MOTOR7_PIN           PD12
#define MOTOR8_PIN           PD13
// All motor pins have occurrence=2 in TIMER_PIN_MAP (except PD12/PD13 which are occurrence=1)
```

**Generated Output**:
```cpp
namespace Motor {
  static constexpr uint32_t frequency_hz = 2000;

  static constexpr MotorConfig motors[] = {
    {TIM3, PB0_ALT1, 3, 125, 250},  // Motor 1: TIM3_CH3
    {TIM3, PB1_ALT1, 4, 125, 250},  // Motor 2: TIM3_CH4
    {TIM5, PA0_ALT1, 1, 125, 250},  // Motor 3: TIM5_CH1
    {TIM5, PA1_ALT1, 2, 125, 250},  // Motor 4: TIM5_CH2
    {TIM5, PA2_ALT1, 3, 125, 250},  // Motor 5: TIM5_CH3
    {TIM5, PA3_ALT1, 4, 125, 250},  // Motor 6: TIM5_CH4
    {TIM4, PD12, 1, 125, 250},      // Motor 7: TIM4_CH1
    {TIM4, PD13, 2, 125, 250},      // Motor 8: TIM4_CH2
  };

  static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
};
```

**ALT Variants**: Motors 1-6 all use ALT1 variants (occurrence=2 selects the second timer option for each pin). Motors 7-8 use default pins (occurrence=1).

**Status**: ✅ All 8 motors correctly mapped across TIM3, TIM5, TIM4

---

## Validation Results

### PeripheralPins.c Validation (JHEF411)
```
Loading PeripheralPins.c: variants/STM32F4xx/F411C(C-E)(U-Y)/PeripheralPins.c

Validating configuration...
Validation passed
Validation Summary:
  Errors: 0
  Warnings: 0
```

**Validated Components**:
- ✅ Storage SPI2: All pins exist and map to SPI2
- ✅ IMU SPI1: All pins exist and map to SPI1
- ✅ I2C1: Both pins exist and map to I2C1
- ✅ UART1/UART2: All TX/RX pins exist and map to correct UARTs
- ✅ ADC: Both pins exist and support ADC
- ✅ Motors: All 5 timer assignments validated against PinMap_TIM
- ✅ Motor 4 ALT variant: PB0_ALT1 correctly maps to TIM3_CH3

---

## Summary

### JHEF411 Conversion Accuracy

| Component | Pins | Timer/Bus | ALT Variants | Status |
|-----------|------|-----------|--------------|--------|
| Storage | 4 | SPI2 | N/A | ✅ 100% |
| IMU | 5 | SPI1 | N/A | ✅ 100% |
| I2C | 2 | I2C1 | N/A | ✅ 100% |
| UART1 | 2 | USART1 | N/A | ✅ 100% |
| UART2 | 2 | USART2 | N/A | ✅ 100% |
| ADC | 2 | ADC1 | N/A | ✅ 100% |
| LEDs | 1 | N/A | N/A | ✅ 100% |
| Motors | 5 | TIM1/TIM3 | 1 ALT | ✅ 100% |

**Overall Accuracy**: ✅ **100%** (23 pins correctly mapped)

### Key Features Demonstrated

1. ✅ **Native config.h parsing**: Direct `#define` extraction (no format conversion needed)
2. ✅ **Timer occurrence resolution**: `TIMER_PIN_MAP` occurrence maps to PeripheralPins.c entry index
3. ✅ **ALT variant selection**: Occurrence > 1 selects ALT pin variants automatically
4. ✅ **Flat motor/servo arrays**: `motors[]` and `servos[]` with `num_motors`/`num_servos` counts
5. ✅ **Multi-SPI bus support**: SPI1 (IMU), SPI2 (Flash)
6. ✅ **Servo separation**: Servos at 50 Hz / 1000-2000 µs, motors at 2000 Hz / 125-250 µs
7. ✅ **Scale extraction**: Current sensor scale from `DEFAULT_CURRENT_METER_SCALE`
8. ✅ **PeripheralPins.c validation**: All pins cross-validated against Arduino Core

### Validation Status

- **Errors**: 0
- **Warnings**: 0
- **Pin accuracy**: 100%
- **Timer validation**: 100%
- **ALT variants**: Correctly handled
