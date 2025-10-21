# Arduino Core STM32 Pin Usage and Encoding Guide

This document provides an analysis of how the STM32 Arduino Core manages, encodes, and converts between different GPIO pin naming conventions.

**TLDR**: Arduino pin macros provide **hardware clarity** while maintaining **full compatibility** with existing Arduino code, avoiding mass changes while still providing **self-documenting** and **hardware-focused** board configurations.

## Pin Encoding Architecture Overview

The STM32 Arduino Core uses a three-layer pin encoding system that bridges Arduino compatibility with STM32 hardware specificity. Understanding this system is crucial because the same physical pin can be referenced in **three different ways**, which creates confusion:

**Example: The same physical pin PC12 can be referenced as:**
1. **Arduino Pin Number**: `17` (integer - varies by board variant)
2. **Arduino Pin Macro**: `PC12` (maps to the integer above) ← **We use this**
3. **Core PinName Constant**: `PC_12` (enum value from PinNames.h)

### 1. Core PinName Encoding (`PinNames.h`)

**Format**: `(PortX << 4) + pin_number`

```cpp
// Examples from PinNames.h:40-55
PA_4  = (PortA << 4) + 0x04  // = 0x04 (assuming PortA = 0)
PC_12 = (PortC << 4) + 0x0C  // = 0x2C (assuming PortC = 2)
PD_2  = (PortD << 4) + 0x02  // = 0x32 (assuming PortD = 3)
```

**Key Properties**:
- **Universal**: Same across all STM32 families
- **Hardware Direct**: Directly encodes GPIO port and pin number
- **Bit Packed**: Upper nibble = port, lower nibble = pin number

### 2. Arduino Pin Number Mapping (`variant.h`)

**Format**: Sequential integers defined per board variant

```cpp
// Example from NUCLEO_F411RE variant:19-31
#define PA3     PIN_A13  // Different boards = different numbers
#define PA10    2        // Arduino pin 2 maps to PA10
#define PB3     3        // Arduino pin 3 maps to PB3
#define PC12    17       // Arduino pin 17 maps to PC12
#define PD2     31       // Arduino pin 31 maps to PD2
```

**Key Properties**:
- **Board Specific**: Same physical pin has different numbers on different boards
- **Arduino Compatible**: Works directly with all Arduino functions
- **Sequential**: Numbered 0 to NUM_DIGITAL_PINS-1

### 3. Extended Pin Configuration Encoding

**32-bit Encoding Structure** (from `PinNamesTypes.h:20-31`):

```
Bits [31:23] - Reserved
Bit  [22]    - Analog channel bank B
Bit  [21]    - Analog ADC control
Bit  [20]    - Inverted (Analog/Timer specific)
Bits [19:15] - Channel (Analog/Timer specific)
Bits [14:8]  - Alternate Function number (AFRL/AFRH)
Bits [7:6]   - Speed config (OSPEEDR) - Reserved
Bits [5:4]   - Pull-up/Pull-down (PUPDR): No Pull/Pull-up/Pull-Down
Bit  [3]     - Output Push-Pull/Open Drain (OTYPER)
Bits [2:0]   - Function (MODER): Input/Output/Alt/Analog
```

## Critical Macros and Functions

### Pin Decomposition Macros

```cpp
// From PinNamesTypes.h:120-128
#define STM_PORT(X)     (((uint32_t)(X) >> 4) & 0xF)  // Extract port number
#define STM_PIN(X)      ((uint32_t)(X) & 0xF)         // Extract pin number
#define STM_GPIO_PIN(X) ((uint16_t)(1 << STM_PIN(X))) // Convert to GPIO bitmask
```

**Usage Example**:
```cpp
PinName pin = PC_12;     // = (PortC << 4) + 12 = 0x2C
uint32_t port = STM_PORT(pin);    // = 2 (PortC)
uint32_t pin_num = STM_PIN(pin);  // = 12
uint16_t gpio_mask = STM_GPIO_PIN(pin); // = (1 << 12) = 0x1000
```

### Pin Conversion Functions

#### Arduino Pin Number ↔ PinName
```cpp
// From pins_arduino.h:94-107
#define digitalPinToPinName(p) ((((uint32_t)(p) & PNUM_MASK) < NUM_DIGITAL_PINS) ? \
    (PinName)(digitalPin[(uint32_t)(p) & PNUM_MASK] | ((p) & ALTX_MASK)) : NC)

uint32_t pinNametoDigitalPin(PinName p);  // Function implementation
```

#### Arduino Pin Number → GPIO Hardware
```cpp
// From pins_arduino.h:139-142
#define digitalPinToPort(p)     (get_GPIO_Port(STM_PORT(digitalPinToPinName(p))))
#define digitalPinToBitMask(p)  (STM_GPIO_PIN(digitalPinToPinName(p)))
```

## GPIO Port Driver Integration

### Fast GPIO Operations

The core provides optimized GPIO operations using direct PinName access:

```cpp
// From digital_io.h:94-120
static inline void digitalWriteFast(PinName pn, uint32_t ulVal) {
    digital_io_write(get_GPIO_Port(STM_PORT(pn)), STM_LL_GPIO_PIN(pn), ulVal);
}

static inline int digitalReadFast(PinName pn) {
    return digital_io_read(get_GPIO_Port(STM_PORT(pn)), STM_LL_GPIO_PIN(pn)) ? HIGH : LOW;
}

static inline void digitalToggleFast(PinName pn) {
    digital_io_toggle(get_GPIO_Port(STM_PORT(pn)), STM_LL_GPIO_PIN(pn));
}
```

### Hardware Register Access

```cpp
// Low-level GPIO operations use STM32 LL drivers:
static inline void digital_io_write(GPIO_TypeDef *port, uint32_t pin, uint32_t val) {
    if (val) {
        LL_GPIO_SetOutputPin(port, pin);    // Set bit in BSRR register
    } else {
        LL_GPIO_ResetOutputPin(port, pin);  // Reset bit in BSRR register
    }
}

static inline uint32_t digital_io_read(GPIO_TypeDef *port, uint32_t pin) {
    return LL_GPIO_IsInputPinSet(port, pin); // Read IDR register
}
```

## Pin Conversion Workflows

### 1. Arduino Pin Macro → Hardware Registers

**Example**: `PA4` in board configuration → GPIO hardware

```cpp
// Step 1: Arduino pin macro resolves to pin number (variant-specific)
#define PA4  PIN_A2  // NUCLEO_F411RE: PA4 = pin 72 (PIN_A2)

// Step 2: Arduino pinMode/digitalWrite converts to PinName
PinName pn = digitalPinToPinName(PA4);  // Lookup in digitalPin[] array

// Step 3: Extract hardware details
uint32_t port_num = STM_PORT(pn);       // Extract port number (0 for PortA)
uint32_t pin_num = STM_PIN(pn);         // Extract pin number (4)
GPIO_TypeDef *port = get_GPIO_Port(port_num);  // Get GPIOA base address
uint16_t gpio_pin = STM_GPIO_PIN(pn);   // Convert to bitmask (1<<4 = 0x10)

// Step 4: Hardware register access
LL_GPIO_SetOutputPin(port, gpio_pin);   // Set GPIOA->BSRR bit 4
```

### 2. Direct PinName → Hardware (Fast Path)

**Example**: `PC_12` direct usage

```cpp
PinName pin = PC_12;  // Direct PinName constant

// Direct hardware access (bypasses Arduino pin number lookup)
digitalWriteFast(pin, HIGH);
// Expands to: LL_GPIO_SetOutputPin(GPIOC, (1<<12));
```

### 3. Raw Pin Number → Hardware (Legacy Support)

**Example**: Numeric pin usage

```cpp
digitalWrite(17, HIGH);  // NUCLEO_F411RE: pin 17 = PC12

// Conversion chain:
// 17 → digitalPinToPinName(17) → digitalPin[17] → PC_12
// PC_12 → STM_PORT(PC_12)=2, STM_PIN(PC_12)=12
// → get_GPIO_Port(2)=GPIOC, (1<<12)=0x1000
// → LL_GPIO_SetOutputPin(GPIOC, 0x1000)
```

## Board Variant Pin Mapping

### NUCLEO_F411RE Example

```cpp
// Physical pin PC12 mappings:
PinName core_name = PC_12;              // Core constant = 0x2C
#define PC12 17                         // Arduino pin macro = 17
extern const PinName digitalPin[52];    // digitalPin[17] = PC_12
```

**Conversion Verification**:
```cpp
// All these refer to the same physical pin PC12:
digitalWrite(17, HIGH);           // Arduino pin number
digitalWrite(PC12, HIGH);         // Arduino pin macro
digitalWriteFast(PC_12, HIGH);    // Direct PinName
```

## Advanced Features

### Alternate Function Support

```cpp
// Alternate function encoding (from PinNames.h:12-21)
#define ALT1  0x100
#define ALT2  0x200
// ...
#define ALTX_MASK  0x700

// Usage in variant definitions:
#define PA7_ALT1  (PA7 | ALT1)    // PA7 with alternate function 1
```

### Analog Pin Integration

```cpp
// Analog pins are also digital pins (pins_arduino.h:95-105)
#define analogInputToDigitalPin(p) ((((uint32_t)(p) & PNUM_MASK) < NUM_ANALOG_INPUTS) ? \
    analogInputPin[(uint32_t)(p) & PNUM_MASK] | ((uint32_t)(p) & ALTX_MASK) : NC)
```

### Peripheral Capability Detection

```cpp
// Capability checking macros (pins_arduino.h:128-136)
#define digitalPinHasPWM(p)    (pin_in_pinmap(digitalPinToPinName(p), PinMap_TIM))
#define digitalPinHasSPI(p)    (pin_in_pinmap(digitalPinToPinName(p), PinMap_SPI_MOSI) || \
                                pin_in_pinmap(digitalPinToPinName(p), PinMap_SPI_MISO) || \
                                pin_in_pinmap(digitalPinToPinName(p), PinMap_SPI_SCLK))
#define digitalPinHasI2C(p)    (pin_in_pinmap(digitalPinToPinName(p), PinMap_I2C_SDA) || \
                                pin_in_pinmap(digitalPinToPinName(p), PinMap_I2C_SCL))
```

## Performance Considerations

### Fast vs Standard GPIO Operations

| Operation | Arduino Standard | STM32 Fast | Performance Gain |
|-----------|------------------|------------|------------------|
| **digitalWrite(pin, val)** | Pin lookup + conversion | Direct register access | ~3-5x faster |
| **digitalRead(pin)** | Pin lookup + conversion | Direct register access | ~3-5x faster |
| **Array lookup cost** | digitalPin[pin] array access | None | Eliminates lookup |
| **Validation overhead** | Pin bounds checking | None | Eliminates validation |

### Optimal Usage Patterns

```cpp
// SLOWER: Arduino pin number (requires lookup)
digitalWrite(17, HIGH);

// FASTER: Arduino pin macro (still requires lookup, but clearer)
digitalWrite(PC12, HIGH);

// FASTEST: Direct PinName (no lookup, direct hardware access)
digitalWriteFast(PC_12, HIGH);
```

## Integration with Board Configuration System

### Why Arduino Pin Macros Are Optimal

Based on our analysis, Arduino pin macros (`PA4`, `PC12`, etc.) provide the best balance:

1. **Hardware Clarity**: Directly show physical STM32 pins
2. **Arduino Compatibility**: Work with all Arduino functions without conversion
3. **Board Portability**: Same macro name works across board variants
4. **Zero Overhead**: Compile-time constants with no runtime cost
5. **Self-Documenting**: Code clearly shows which hardware pins are used

### Comparison of Pin Naming Approaches

| Aspect | Raw Pin Numbers | Arduino Pin Macros | Core PinNames |
|--------|-----------------|-------------------|---------------|
| **Hardware Clarity** | `{17, 30, 16, 31}` ❌ | `{PC12, PC11, PC10, PD2}` ✅ | `{PC_12, PC_11, PC_10, PD_2}` ✅ |
| **Arduino Compatibility** | Works but unclear ❌ | Perfect compatibility ✅ | Requires conversion ❌ |
| **Code Changes Required** | None, but confusing ❌ | Zero breaking changes ✅ | Conversion functions needed ❌ |
| **STM32 Documentation Match** | Requires lookup ❌ | Direct match ✅ | Close match ✅ |
| **Board Variant Portability** | Variant-specific ❌ | Works across variants ✅ | Universal ✅ |
| **Self-Documenting** | Numbers mean nothing ❌ | Shows actual hardware ✅ | Shows actual hardware ✅ |

### Board Configuration Examples

#### Clear Hardware Configuration
```cpp
// BAD: Raw pin numbers - unclear and board-variant dependent
static constexpr SPIConfig storage{17, 30, 16, 31, 1000000, 8000000};
// What pins are these? Must check variant file.

// GOOD: Arduino pin macros - clear hardware pins, Arduino compatible
static constexpr SPIConfig storage{PC12, PC11, PC10, PD2, 1000000, 8000000};
// Clear which physical STM32 pins are used, works with all Arduino functions
```

#### Complete Board Configuration
```cpp
// targets/NUCLEO_F411RE.h
namespace BoardConfig {
    // Clear hardware mapping using Arduino pin macros
    static constexpr SPIConfig storage{
        .mosi_pin = PC12,    // Arduino macro → variant-specific pin number
        .miso_pin = PC11,    // Automatically resolved at compile time
        .sclk_pin = PC10,    // Works with all Arduino functions
        .cs_pin = PD2,       // Self-documenting hardware configuration
        .speed_low = 1000000,
        .speed_high = 8000000
    };

    // IMU: Accelerometer/Gyroscope via SPI
    static constexpr SPIConfig imu{PA7, PA6, PA5, PA4, 1000000, 8000000};
    //                             MOSI MISO SCLK CS
}

// Usage - no conversion needed:
SPIClass configuredSPI(
    BoardConfig::storage.mosi_pin,    // PC12 → 17 (NUCLEO_F411RE)
    BoardConfig::storage.miso_pin,    // PC11 → 30 (NUCLEO_F411RE)
    BoardConfig::storage.sclk_pin     // PC10 → 16 (NUCLEO_F411RE)
);

// Works directly with Arduino functions - no conversion needed
pinMode(BoardConfig::storage.cs_pin, OUTPUT);     // PD2
digitalWrite(BoardConfig::storage.cs_pin, HIGH);  // PD2
```

#### Board Variant Compatibility
```cpp
// Arduino pin macros are defined consistently across board variants:
// NUCLEO_F411RE: #define PC12 17
// BlackPill_F411: #define PC12 52  (different number, same macro name)

// Our config works on both:
static constexpr SPIConfig storage{PC12, PC11, PC10, PD2, 1000000, 8000000};
// Automatically gets correct pin numbers for each board variant
```

This approach maintains hardware clarity while providing seamless Arduino compatibility and optimal performance characteristics.

## Pin Definition Locations and Build Order

### Where Pin Macros and Arduino Pin Numbers Are Defined

#### 1. Pin Macros Definition
**Location**: `variants/STM32F4xx/F411R(C-E)T/variant_NUCLEO_F411RE.h:19-75`

Arduino pin macros are defined in the board variant header file:
```cpp
// STM32 pins number section
#define PA3     PIN_A13
#define PA10    2
#define PB3     3
#define PC12    17       // Pin macro → Arduino pin number
#define PD2     31
// ... etc for all board pins
```

#### 2. Arduino Pin Numbers Array
**Location**: `variants/STM32F4xx/F411R(C-E)T/variant_NUCLEO_F411RE.cpp:18-76`

The `digitalPin[]` array maps Arduino pin numbers back to PinName constants:
```cpp
const PinName digitalPin[] = {
  PA_3,  //D0/A13
  PA_2,  //D1/A14
  PA_10, //D2
  PB_3,  //D3
  // ...
  PC_12, //D17 ← digitalPin[17] = PC_12
  // ...
  PD_2,  //D31 ← digitalPin[31] = PD_2
};
```

### Build Process Definition Order

The pin definitions are processed in this order during compilation:

1. **Core PinNames** (`cores/arduino/stm32/PinNames.h:40-280`):
   ```cpp
   PC_12 = (PortC << 4) + 0x0C  // = 0x2C, hardware encoding
   PD_2  = (PortD << 4) + 0x02  // = 0x32, hardware encoding
   ```

2. **Variant Pin Macros** (`variants/.../variant_BOARD.h`):
   ```cpp
   #define PC12 17   // Arduino pin macro → pin number
   #define PD2  31   // Arduino pin macro → pin number
   ```

3. **digitalPin Array** (`variants/.../variant_BOARD.cpp`):
   ```cpp
   const PinName digitalPin[] = {
       // ... array index 17:
       PC_12,  // digitalPin[17] = PC_12 (creates the reverse lookup)
       // ... array index 31:
       PD_2,   // digitalPin[31] = PD_2
   };
   ```

This creates the bidirectional mapping: `PC12` macro → `17` → `PC_12` PinName.

## ST GPIO HAL Integration

### Direct Conversion from Pin Macro to ST GPIO HAL

To convert from a Pin Macro variable directly to ST GPIO HAL calls:

```cpp
#include "stm32f4xx_hal_gpio.h"
#include "PinNames.h"
#include "PortNames.h"

// Your pin macro variable from board configuration
uint32_t pin_macro = PC12;  // Resolves to Arduino pin number 17

// Step 1: Convert Arduino pin number to PinName
PinName pin_name = digitalPinToPinName(pin_macro);  // Gets PC_12

// Step 2: Extract hardware port and pin numbers
uint32_t port_num = STM_PORT(pin_name);     // Extract port: 2 (PortC)
uint32_t pin_num = STM_PIN(pin_name);       // Extract pin: 12

// Step 3: Get GPIO port base address
GPIO_TypeDef* gpio_port = get_GPIO_Port(port_num);  // Gets GPIOC

// Step 4: Create GPIO pin mask for ST HAL
uint16_t gpio_pin = (1U << pin_num);        // Create mask: GPIO_PIN_12

// Step 5: Use with ST GPIO HAL directly
HAL_GPIO_WritePin(gpio_port, gpio_pin, GPIO_PIN_SET);
HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);  // Equivalent
```

### Simplified Conversion Functions

**One-liner conversion**:
```cpp
// Pin macro → ST HAL in one step
uint32_t pin = PC12;  // Arduino pin macro
HAL_GPIO_WritePin(
    get_GPIO_Port(STM_PORT(digitalPinToPinName(pin))),  // GPIOC
    (1U << STM_PIN(digitalPinToPinName(pin))),          // GPIO_PIN_12
    GPIO_PIN_SET
);
```

**Most efficient approach** (bypassing Arduino pin lookup):
```cpp
// If you know the PinName directly from the core:
PinName pin = PC_12;  // Direct core constant, no lookup needed
HAL_GPIO_WritePin(
    get_GPIO_Port(STM_PORT(pin)),    // GPIOC
    (1U << STM_PIN(pin)),           // GPIO_PIN_12
    GPIO_PIN_SET
);
```

**Helper macros for convenience**:
```cpp
// Create helper macros for easier ST HAL integration
#define PIN_TO_HAL_PORT(pin_macro) \
    get_GPIO_Port(STM_PORT(digitalPinToPinName(pin_macro)))

#define PIN_TO_HAL_PIN(pin_macro) \
    (1U << STM_PIN(digitalPinToPinName(pin_macro)))

// Usage:
uint32_t pin = PC12;
HAL_GPIO_WritePin(PIN_TO_HAL_PORT(pin), PIN_TO_HAL_PIN(pin), GPIO_PIN_SET);
```

### Key Functions for Conversion

| Function | Purpose | Returns |
|----------|---------|---------|
| `digitalPinToPinName(pin)` | Arduino pin → PinName | `PC_12` from `17` |
| `STM_PORT(pinname)` | Extract port number | `2` from `PC_12` |
| `STM_PIN(pinname)` | Extract pin number | `12` from `PC_12` |
| `get_GPIO_Port(port_num)` | Get GPIO base address | `GPIOC` from `2` |
| `(1U << pin_num)` | Create HAL pin mask | `GPIO_PIN_12` from `12` |

This integration approach allows you to maintain the hardware clarity and Arduino compatibility of Pin Macros while accessing the full power of ST's GPIO HAL when needed.

## Validated Hardware Pin Assignments

### NUCLEO_F411RE + ICM42688P Configuration (Validated 2025-09-30)

**Target Platform**: STM32 NUCLEO-F411RE development board with ICM-42688-P 6-axis IMU sensor

**Hardware Validated Configuration** (working on HIL test rig):
```cpp
// BoardConfig configuration using Arduino pin macros
static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 1000000, CS_Mode::SOFTWARE};
static constexpr IMUConfig imu{imu_spi, PC4, 0};  // PC4 = interrupt pin

// Resolves to these physical pin assignments:
//   MOSI: PA7 (Arduino pin macro → auto-resolved to variant pin number)
//   MISO: PA6 (Arduino pin macro → auto-resolved to variant pin number)
//   SCLK: PA5 (Arduino pin macro → auto-resolved to variant pin number)
//   CS:   PA4 (Software controlled, Arduino pin macro → auto-resolved)
//   INT:  PC4 (Interrupt pin for sensor events, currently unused)
//   Freq: 1MHz (jumper wire connections on HIL test rig)
```

**STM32 SPI Hardware Assignment**:
- **SPI Peripheral**: Uses STM32F411RE SPI1 peripheral pins (PA5/PA6/PA7)
- **CS Control**: Software-controlled CS via PA4 (manual digitalWrite control)
- **Clock Speed**: 1MHz for reliable jumper wire connections on test rig
- **Interrupt**: PC4 configured for sensor interrupt (EXTI4), currently unused in minimal example

**Physical Connections** (HIL Test Rig):
```
NUCLEO_F411RE    ICM42688P Module
=============    ================
PA7 (MOSI)   →   SDI/SDA pin
PA6 (MISO)   ←   SDO/SAO pin
PA5 (SCLK)   →   SCLK/SCL pin
PA4 (CS)     →   CS pin
PC4 (INT)    ←   INT pin (optional)
3.3V         →   VCC
GND          →   GND
```

**BoardConfig Integration Pattern**:
```cpp
// Current working integration pattern (validated on hardware)
#include "targets/NUCLEO_F411RE_LITTLEFS.h"

// SPIClass constructor with BoardConfig (working pattern)
SPIClass spi(BoardConfig::imu.spi.mosi_pin,
             BoardConfig::imu.spi.miso_pin,
             BoardConfig::imu.spi.sclk_pin,
             BoardConfig::imu.spi.get_ssel_pin());  // Returns PNUM_NOT_DEFINED for SOFTWARE CS

// ICM42688P initialization (validated working)
ICM42688P_Simple imu;
if (!imu.begin(spi, BoardConfig::imu.spi.cs_pin, BoardConfig::imu.spi.freq_hz)) {
    // Initialization failed
}
```

**CS Mode Implementation Details**:
- **SOFTWARE Mode**: CS pin managed manually with pinMode/digitalWrite calls
- **get_ssel_pin()**: Returns PNUM_NOT_DEFINED to disable STM32 SPI peripheral CS control
- **Manual CS**: ICM42688P library handles CS assertion/deassertion in software
- **Always Available**: BoardConfig::imu.spi.cs_pin always provides the CS pin regardless of mode

## Summary

The STM32 Arduino Core's pin management system elegantly bridges three different naming conventions:

1. **Core PinNames** (`PC_12`): Hardware-direct, universal across families
2. **Arduino Pin Macros** (`PC12`): Variant-specific, Arduino-compatible, hardware-clear
3. **Arduino Pin Numbers** (`17`): Sequential, Arduino-standard, board-dependent

The conversion system provides multiple access patterns optimized for different use cases, from Arduino compatibility to maximum performance, while maintaining clear hardware abstraction through the sophisticated pin encoding architecture.

The current BoardConfig system leverages Arduino pin macros to provide hardware-clear configurations that automatically resolve to the correct variant-specific pin numbers while maintaining full Arduino compatibility and self-documenting code.