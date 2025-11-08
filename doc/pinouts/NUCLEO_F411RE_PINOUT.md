# NUCLEO-F411RE Pinout Reference

Complete pin mapping for STM32 NUCLEO-F411RE development board.

## Arduino Connector Pins

### CN6 - Left Arduino Shield Connector (Power)
Located between CN7 morpho header and Arduino connectors, provides power and I2C:

| Pin | Function | Notes |
|-----|----------|-------|
| 1 | NC | Not connected |
| 2 | IOREF | I/O reference voltage (3.3V) |
| 3 | RESET | System reset |
| 4 | +3.3V | 3.3V regulated power |
| 5 | +5V | 5V power |
| 6 | GND | Ground |
| 7 | GND | Ground |
| 8 | VIN | External power input |

**Bottom connectors**: CN8 (1×6, A0-A5) and CN9 (1×8, D0-D7)
**Right side connector**: CN5 (1×10, D8-D15 plus SPI/PWM)
**Left side connector**: CN6 (1×8, Power)

### CN8 - Left Arduino Header (Analog)
**Connector**: 1×6 female socket

| Arduino | STM32 Pin | CN8 Pin | Notes |
|---------|-----------|---------|-------|
| A0      | PA0       | 1       | ADC1_IN0, TIM2_CH1, TIM5_CH1 |
| A1      | PA1       | 2       | ADC1_IN1, TIM2_CH2, TIM5_CH2 |
| A2      | PA4       | 3       | ADC1_IN4, DAC_OUT1 |
| A3      | PB0       | 4       | ADC1_IN8, TIM1_CH2N, TIM3_CH3 |
| A4      | PC1       | 5       | ADC1_IN11 |
| A5      | PC0       | 6       | ADC1_IN10 |

### CN5 - Digital Pins D8-D15
**Connector**: 1×10 female socket
**Location**: Right side, between CN10 morpho header and bottom Arduino connectors
**Pin numbering**: Top to bottom (10 to 1)

| Arduino | STM32 Pin | CN5 Pin | Notes |
|---------|-----------|---------|-------|
| D15/SCL | PB8       | 10      | I2C1_SCL, TIM4_CH3, TIM10_CH1 |
| D14/SDA | PB9       | 9       | I2C1_SDA, TIM4_CH4, SPI2_NSS |
| AREF    | -         | 8       | Analog reference (not connected) |
| GND     | -         | 7       | Ground |
| D13     | PA5       | 6       | SPI1_SCK, TIM2_CH1, ADC1_IN5, DAC_OUT2 |
| D12     | PA6       | 5       | SPI1_MISO, TIM1_CH_BKIN, TIM3_CH1, ADC1_IN6 |
| D11     | PA7       | 4       | SPI1_MOSI, TIM1_CH1N, TIM3_CH2, ADC1_IN7 |
| D10     | PB6       | 3       | I2C1_SCL, TIM4_CH1 (PWM), USART1_TX |
| D9      | PC7       | 2       | TIM3_CH2, TIM8_CH2 (PWM) |
| D8      | PA9       | 1       | USART1_TX, TIM1_CH2 |

### CN9 - Digital Pins D0-D7
**Connector**: 1×8 female socket
**Pin numbering**: Top to bottom (8 to 1)

| Arduino | STM32 Pin | CN9 Pin | Notes |
|---------|-----------|---------|-------|
| D7      | PA8       | 8       | TIM1_CH1, I2C3_SCL |
| D6      | PB10      | 7       | I2C2_SCL, TIM2_CH3 (PWM) |
| D5      | PB4       | 6       | SPI1_MISO, TIM3_CH1 (PWM) |
| D4      | PB5       | 5       | SPI1_MOSI, TIM3_CH2 |
| D3      | PB3       | 4       | SPI1_SCK, TIM2_CH2 (PWM) |
| D2      | PA10      | 3       | USART1_RX, TIM1_CH3 |
| D1      | PA2       | 2       | USART2_TX, ADC1_IN2, TIM2_CH3, TIM5_CH3 |
| D0      | PA3       | 1       | USART2_RX, ADC1_IN3, TIM2_CH4, TIM5_CH4 |

## Morpho Connector Pins

**Note**: CN7 and CN10 are 2×19 pin headers (38 pins each). Odd-numbered pins are on one row, even-numbered pins on the other row.

### CN7 - Left Morpho Header (2×19 pins)

| Odd Pin | STM32 Pin | Function | Even Pin | STM32 Pin | Function |
|---------|-----------|----------|----------|-----------|----------|
| 1  | PC10  | SPI3_SCK, USART3_TX, SDIO_D2 | 2  | PC11  | SPI3_MISO, USART3_RX, SDIO_D3 |
| 3  | PC12  | SPI3_MOSI, USART3_CK, SDIO_CK | 4  | PD2   | USART5_RX, TIM3_ETR, SDIO_CMD |
| 5  | VDD   | +3.3V Power | 6  | E5V   | External 5V power |
| 7  | BOOT0 | Boot mode select | 8  | GND   | Ground |
| 9  | NC    | Not connected | 10 | NC    | Not connected |
| 11 | NC    | Not connected | 12 | IOREF | I/O reference voltage |
| 13 | PA13  | SWDIO (Debug) | 14 | RESET | System reset |
| 15 | PA14  | SWCLK (Debug) | 16 | +3V3  | +3.3V Power |
| 17 | PA15  | SPI1_NSS, SPI3_NSS, TIM2_CH1 | 18 | +5V   | +5V Power |
| 19 | GND   | Ground | 20 | GND   | Ground |
| 21 | PB7   | I2C1_SDA, USART1_RX, TIM4_CH2 | 22 | GND   | Ground |
| 23 | PC13  | RTC_OUT, User button | 24 | VIN   | External power input |
| 25 | PC14  | OSC32_IN (RTC crystal) | 26 | NC    | Not connected |
| 27 | PC15  | OSC32_OUT (RTC crystal) | 28 | PA0   | A0 (see CN8) |
| 29 | PH0   | OSC_IN (External oscillator) | 30 | PA1   | A1 (see CN8) |
| 31 | PH1   | OSC_OUT (External oscillator) | 32 | PA4   | A2 (see CN8) |
| 33 | VBAT  | Battery backup power | 34 | PB0   | A3 (see CN8) |
| 35 | PC2   | ADC1_IN12, SPI2_MISO | 36 | PC1   | A4 (see CN8) |
| 37 | PC3   | ADC1_IN13, SPI2_MOSI | 38 | PC0   | A5 (see CN8) |

### CN10 - Right Morpho Header (2×19 pins)

| Odd Pin | STM32 Pin | Function | Even Pin | STM32 Pin | Function |
|---------|-----------|----------|----------|-----------|----------|
| 1  | PC9  | TIM3_CH4, TIM8_CH4, I2C3_SDA, SDIO_D1 | 2  | PC8  | TIM3_CH3, TIM8_CH3, SDIO_D0 |
| 3  | PB8  | D15 (see CN9) | 4  | PC6  | TIM3_CH1, TIM8_CH1, USART6_TX, I2S2_MCK |
| 5  | PB9  | D14 (see CN9) | 6  | PC5  | ADC1_IN15 |
| 7  | AVDD | Analog power supply | 8  | U5V  | USB 5V power |
| 9  | GND  | Ground | 10 | NC   | Not connected |
| 11 | PA5  | D13 (see CN9) | 12 | PA12 | USB_DP, TIM1_ETR, USART1_RTS |
| 13 | PA6  | D12 (see CN9) | 14 | PA11 | USB_DM, TIM1_CH4, USART1_CTS |
| 15 | PA7  | D11 (see CN9) | 16 | PB12 | SPI2_NSS, I2C2_SMBA, TIM1_BKIN, USART3_CK |
| 17 | PB6  | D10 (see CN9) | 18 | NC   | Not connected |
| 19 | PC7  | D9 (see CN9) | 20 | GND  | Ground |
| 21 | PA9  | D8 (see CN9) | 22 | PB2  | Boot1 |
| 23 | PA8  | D7 (see CN9) | 24 | PB1  | ADC1_IN9, TIM1_CH3N, TIM3_CH4 |
| 25 | PB10 | D6 (see CN9) | 26 | PB15 | SPI2_MOSI, TIM1_CH3N |
| 27 | PB4  | D5 (see CN9) | 28 | PB14 | SPI2_MISO, TIM1_CH2N |
| 29 | PB5  | D4 (see CN9) | 30 | PB13 | SPI2_SCK, TIM1_CH1N |
| 31 | PB3  | D3 (see CN9) | 32 | AGND | Analog ground |
| 33 | PA10 | D2 (see CN9) | 34 | PC4  | ADC1_IN14 |
| 35 | PA2  | D1 (see CN9) | 36 | NC   | Not connected |
| 37 | PA3  | D0 (see CN9) | 38 | NC   | Not connected |

## Special Function Pins

### Debug/Programming
- **SWDIO**: PA13 (CN7-13) - Serial Wire Debug Data
- **SWCLK**: PA14 (CN7-15) - Serial Wire Debug Clock
- **RESET**: CN6-14 - System reset
- **BOOT0**: CN7-7 - Boot mode selection

### User Interface
- **User Button**: PC13 (Blue button, active low)
- **LED (LD2)**: PA5 (Green LED, connected to D13)

### Oscillators
- **HSE Crystal**: PH0/PH1 (8 MHz external oscillator)
- **LSE Crystal**: PC14/PC15 (32.768 kHz RTC crystal)

### USB
- **USB_DM**: PA11 (CN5-14) - USB Data Minus
- **USB_DP**: PA12 (CN5-12) - USB Data Plus

## Timer Channel Availability

### Timer 1 (Advanced Control)
- CH1: PA8 (D7)
- CH2: PA9 (D8)
- CH3: PA10 (D2)
- CH4: PA11 (USB_DM)
- CH1N: PA7 (D11), PB13
- CH2N: PB0 (A3), PB14
- CH3N: PB1, PB15

### Timer 2 (General Purpose, 32-bit)
- CH1: PA0 (A0), PA5 (D13), PA15
- CH2: PA1 (A1), PB3 (D3)
- CH3: PA2 (D1), PB10 (D6)
- CH4: PA3 (D0)

### Timer 3 (General Purpose)
- CH1: PA6 (D12), PB4 (D5), PC6
- CH2: PA7_ALT1 (D11), PB5 (D4), PC7 (D9)
- CH3: PB0_ALT1 (A3), PC8
- CH4: PB1_ALT1, PC9

### Timer 4 (General Purpose)
- CH1: PB6 (D10)
- CH2: PB7
- CH3: PB8 (D15)
- CH4: PB9 (D14)

### Timer 5 (General Purpose, 32-bit)
- CH1: PA0 (A0)
- CH2: PA1 (A1)
- CH3: PA2 (D1)
- CH4: PA3 (D0)

### Timer 8 (Advanced Control)
- CH1: PC6
- CH2: PC7 (D9)
- CH3: PC8
- CH4: PC9

### Timer 10 (General Purpose)
- CH1: PB8 (D15)

## SPI Buses

### SPI1 (Primary Arduino SPI)
- **SCK**: PA5 (D13), PB3 (D3)
- **MISO**: PA6 (D12), PB4 (D5)
- **MOSI**: PA7 (D11), PB5 (D4)
- **NSS**: PA4 (A2), PA15

### SPI2
- **SCK**: PB13
- **MISO**: PB14, PC2
- **MOSI**: PB15, PC3
- **NSS**: PB9 (D14), PB12

### SPI3
- **SCK**: PC10
- **MISO**: PC11
- **MOSI**: PC12

## I2C Buses

### I2C1 (Primary Arduino I2C)
- **SCL**: PB6 (D10), PB8 (D15)
- **SDA**: PB7, PB9 (D14)

### I2C2
- **SCL**: PB10 (D6)
- **SDA**: PB3 (D3)

### I2C3
- **SCL**: PA8 (D7)
- **SDA**: PC9

## UART/USART

### USART1
- **TX**: PA9 (D8), PB6 (D10)
- **RX**: PA10 (D2), PB7

### USART2 (Connected to ST-LINK VCP)
- **TX**: PA2 (D1)
- **RX**: PA3 (D0)

### USART3
- **TX**: PC10 (CN7-1)
- **RX**: PC11 (CN7-2)

### USART6
- **TX**: PC6
- **RX**: PC7 (D9)

## ADC Channels

All ADC channels are on ADC1:

| Arduino Pin | STM32 Pin | ADC Channel |
|-------------|-----------|-------------|
| A0          | PA0       | IN0         |
| A1          | PA1       | IN1         |
| D1          | PA2       | IN2         |
| D0          | PA3       | IN3         |
| A2          | PA4       | IN4         |
| D13         | PA5       | IN5         |
| D12         | PA6       | IN6         |
| D11         | PA7       | IN7         |
| A3          | PB0       | IN8         |
| -           | PB1       | IN9         |
| A5          | PC0       | IN10        |
| A4          | PC1       | IN11        |
| -           | PC2       | IN12        |
| -           | PC3       | IN13        |
| -           | PC4       | IN14        |
| -           | PC5       | IN15        |

## DAC Channels

| STM32 Pin | Arduino Pin | DAC Channel |
|-----------|-------------|-------------|
| PA4       | A2          | OUT1        |
| PA5       | D13         | OUT2        |

## Notes

1. **Pin Conflicts**: Some pins have multiple functions. Check PeripheralPins.c for ALT variants.
2. **ALT Variants Required**: Some timer channels REQUIRE ALT variants (e.g., PB0_ALT1 for TIM3_CH3, PA7_ALT1 for TIM3_CH2). Using the base pin (PB0, PA7) will select the default timer mapping (TIM1_CH2N, TIM1_CH1N respectively).
3. **PWM Channels**: D9 and D11 both use TIM3_CH2 - cannot be used simultaneously for PWM.
4. **ST-LINK UART**: USART2 (PA2/PA3) is connected to ST-LINK for USB serial communication.
5. **LED Pin**: PA5 (D13) has an onboard LED - may affect ADC/DAC readings.
6. **User Button**: PC13 has an onboard button and RTC output - use carefully.
7. **USB Pins**: PA11/PA12 are dedicated to USB - avoid using for other purposes if USB is needed.

## References

- STM32F411RE Datasheet: [DS9716](https://www.st.com/resource/en/datasheet/stm32f411re.pdf)
- NUCLEO-F411RE User Manual: [UM1724](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- Arduino Core STM32 Variant: `Arduino_Core_STM32/variants/STM32F4xx/F411R(C-E)T/PeripheralPins.c`
