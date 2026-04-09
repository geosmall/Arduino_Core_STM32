# STM32 Arduino Core EXTI Interrupt Reference

Technical reference for External Interrupt (EXTI) implementation in the STM32 Arduino Core for embedded systems programmers.

## Overview

The STM32 Arduino Core provides two complementary interrupt interfaces:
1. **Arduino-standard `attachInterrupt()`** - Portable, high-level interface
2. **STM32-specific `stm32_interrupt_enable()`** - Direct HAL access with advanced configuration

Both interfaces utilize the STM32 EXTI peripheral with proper GPIO clock management and NVIC configuration.

## EXTI Line Mapping

### STM32F4xx Series (Primary Target: STM32F411RE)

| Pin Number | EXTI Line | IRQ Handler | Sharing |
|------------|-----------|-------------|---------|
| Px0        | EXTI0     | EXTI0_IRQn | Individual |
| Px1        | EXTI1     | EXTI1_IRQn | Individual |
| Px2        | EXTI2     | EXTI2_IRQn | Individual |
| Px3        | EXTI3     | EXTI3_IRQn | Individual |
| Px4        | EXTI4     | EXTI4_IRQn | Individual |
| Px5–Px9   | EXTI5-9   | EXTI9_5_IRQn | Shared |
| Px10–Px15 | EXTI10-15 | EXTI15_10_IRQn | Shared |

**Note:** Only the pin number (0–15) matters for EXTI line assignment, not the GPIO port. For example, PA1, PB1, PC1 all use EXTI1. Only one pin per EXTI line can have an active interrupt at a time.

### Interrupt Priority Configuration

```c
// Default priorities (configurable via defines)
#define EXTI_IRQ_PRIO       6    // Main priority
#define EXTI_IRQ_SUBPRIO    0    // Sub-priority
```

Lower numbers = higher priority. ARM Cortex-M4 supports 4-bit priority (0-15).

## Arduino-Standard Interface

### Implementation (`WInterrupts.h/cpp`)

```cpp
void attachInterrupt(Pin pin, callback_function_t callback, uint32_t mode);
void detachInterrupt(Pin pin);
```

### Trigger Modes

| Mode | STM32 HAL Equivalent | Description |
|------|---------------------|-------------|
| `RISING` | `GPIO_MODE_IT_RISING` | Rising edge trigger |
| `FALLING` | `GPIO_MODE_IT_FALLING` | Falling edge trigger |
| `CHANGE` | `GPIO_MODE_IT_RISING_FALLING` | Both edges |
| `LOW` | `GPIO_MODE_IT_FALLING` | Falling edge (Arduino compatibility) |
| `HIGH` | `GPIO_MODE_IT_RISING` | Rising edge (Arduino compatibility) |

### Usage Example

```cpp
#include <Arduino.h>

volatile bool data_ready = false;

void sensor_interrupt() {
    data_ready = true;
}

void setup() {
    pinMode(PB1, INPUT_PULLUP);
    attachInterrupt(PB1, sensor_interrupt, FALLING);
}

void loop() {
    if (data_ready) {
        // Process interrupt
        data_ready = false;
    }
}
```

### Pin Conversion

The `attachInterrupt(Pin, ...)` overload converts internally via `pin.toPinName()` to reach the HAL layer. The EXTI line is determined by the pin number (0–15) extracted from the PinName.

## STM32-Specific Interface

### Implementation (`stm32/interrupt.h/cpp`)

```cpp
void stm32_interrupt_enable(GPIO_TypeDef *port, uint16_t pin,
                           void (*callback)(void), uint32_t mode);
void stm32_interrupt_enable(GPIO_TypeDef *port, uint16_t pin,
                           std::function<void(void)> callback, uint32_t mode);
void stm32_interrupt_disable(GPIO_TypeDef *port, uint16_t pin);
```

### Direct HAL Mode Configuration

```cpp
// Available GPIO interrupt modes
GPIO_MODE_IT_RISING          // Rising edge trigger
GPIO_MODE_IT_FALLING         // Falling edge trigger
GPIO_MODE_IT_RISING_FALLING  // Both edges
```

### Usage Example

```cpp
#include "stm32/interrupt.h"

volatile uint32_t interrupt_count = 0;

void precision_timer_interrupt() {
    interrupt_count++;
}

void setup() {
    // Direct HAL configuration with specific port/pin
    stm32_interrupt_enable(GPIOB, GPIO_PIN_1, precision_timer_interrupt,
                          GPIO_MODE_IT_FALLING);
}
```

## Internal Implementation Details

### Callback Management

```cpp
// Internal callback storage (per EXTI line)
typedef struct {
    IRQn_Type irqnb;
    std::function<void(void)> callback;
} gpio_irq_conf_str;

static gpio_irq_conf_str gpio_irq_conf[16];  // 16 EXTI lines
```

### IRQ Handler Chain

1. **Hardware Interrupt** → STM32 EXTI IRQ Handler
2. **EXTI Handler** → `HAL_GPIO_EXTI_IRQHandler(GPIO_Pin)`
3. **HAL Handler** → `HAL_GPIO_EXTI_Callback(GPIO_Pin)`
4. **Core Callback** → User function via `gpio_irq_conf[pin_id].callback()`

### Shared EXTI Line Handling

For shared EXTI lines (5-9, 10-15), the IRQ handler iterates through all possible pins:

```cpp
void EXTI9_5_IRQHandler(void) {
    uint32_t pin;
    for (pin = GPIO_PIN_5; pin <= GPIO_PIN_9; pin = pin << 1) {
        HAL_GPIO_EXTI_IRQHandler(pin);
    }
}
```

Only pins with active interrupts and registered callbacks are processed.

## Pin Selection Guidelines

### For High-Performance Applications

**Recommended: Individual EXTI Lines (0-4)**
- Any pin ending in 0–4 (e.g., PA0, PB1, PC2, PA3, PA4)
- Dedicated IRQ handlers (lowest latency)
- No interrupt sharing overhead

**Example Pins on NUCLEO_F411RE:**
- `PB1` — EXTI1, individual line
- `PA4` — EXTI4, individual line

### For General Applications

**Acceptable: Shared EXTI Lines (5-15)**
- Any pin ending in 5–15 (e.g., PB6, PC7, PA8)
- Shared IRQ handlers with iteration overhead
- Still suitable for most sensor applications

**Example Pins on NUCLEO_F411RE:**
- `PB6` — EXTI6, shared EXTI9_5_IRQn
- `PC7` — EXTI7, shared EXTI9_5_IRQn

## Board-Specific Pin Assignments

### NUCLEO_F411RE Recommended INT Pins

| Pin | EXTI Line | IRQ Handler | Notes |
|-----|-----------|-------------|-------|
| PB1 | EXTI1 | Individual | **Recommended** for IMU INT |
| PA4 | EXTI4 | Individual | Check for SPI CS conflicts |
| PB6 | EXTI6 | Shared | Good alternative |
| PC7 | EXTI7 | Shared | Good alternative |
| PB7 | EXTI7 | Shared | Morpho connector |

### BlackPill F411CE Considerations

Similar EXTI mapping applies. Verify pin availability and conflicts with board-specific peripherals.

## Performance Characteristics

### Interrupt Latency

| Configuration | Latency (approximate) | Use Case |
|---------------|---------------------|----------|
| Individual EXTI (0-4) | ~1-2μs | High-frequency sensors |
| Shared EXTI (5-9) | ~2-4μs | General sensor applications |
| Shared EXTI (10-15) | ~2-4μs | General sensor applications |

*Latency measured from hardware signal to user callback execution at 84MHz system clock.*

### Throughput Considerations

- **Maximum sustained rate:** ~50kHz (individual EXTI lines)
- **Burst capability:** Limited by ISR processing time
- **Shared lines:** Reduced performance with multiple active interrupts

## Integration with ICM42688P Library

### Recommended Configuration

```cpp
// IMU INT1 pin configuration for data ready interrupts
#define IMU_INT_PIN     PB1          // Individual EXTI line
#define IMU_CS_PIN      PA4          // SPI chip select
#define IMU_TRIGGER     FALLING      // ICM42688P INT active low

volatile bool imu_data_ready = false;

void imu_interrupt() {
    imu_data_ready = true;
}

void setup() {
    // Configure SPI and IMU
    // ...

    // Enable data ready interrupt
    attachInterrupt(IMU_INT_PIN, imu_interrupt, IMU_TRIGGER);
}
```

### Data Ready Interrupt Flow

1. **IMU Sample Ready** → INT1 pin asserted (active low)
2. **EXTI1 Triggered** → `imu_interrupt()` called
3. **Flag Set** → `imu_data_ready = true`
4. **Main Loop** → Read sensor data, clear flag

### Thread Safety

```cpp
// Atomic flag operations for ISR safety
volatile bool data_ready = false;

void imu_interrupt() {
    data_ready = true;  // Atomic on Cortex-M4
}

void loop() {
    bool local_flag = data_ready;  // Atomic read
    if (local_flag) {
        data_ready = false;        // Atomic write
        process_imu_data();
    }
}
```

## Debugging and Troubleshooting

### Common Issues

1. **Pin Conflicts:** Verify pin isn't used by other peripherals
2. **Pull-up/Pull-down:** Configure GPIO pull resistors appropriately
3. **Interrupt Sharing:** Multiple pins on same EXTI line can interfere
4. **Priority Conflicts:** High-priority interrupts can block EXTI

### Debug Techniques

```cpp
// Interrupt activity monitoring
volatile uint32_t interrupt_count = 0;
volatile uint32_t last_interrupt_time = 0;

void debug_interrupt() {
    interrupt_count++;
    last_interrupt_time = millis();
}

// Add to main loop
void debug_print() {
    static uint32_t last_print = 0;
    if (millis() - last_print > 1000) {
        printf("Interrupts: %lu, Last: %lu ms ago\n",
               interrupt_count, millis() - last_interrupt_time);
        last_print = millis();
    }
}
```

### Oscilloscope Verification

- Monitor INT pin signal quality and timing
- Verify interrupt latency from signal to GPIO toggle in ISR
- Check for signal bounce or noise issues

## Advanced Configuration

### Custom Priority Settings

```cpp
// Override default priorities before including headers
#define EXTI_IRQ_PRIO     4    // Higher priority
#define EXTI_IRQ_SUBPRIO  1    // Custom sub-priority

#include "stm32/interrupt.h"
```

### Multiple Interrupt Sources

```cpp
// Handle multiple sensors with different priorities
void high_priority_sensor() {
    // Critical timing sensor
}

void general_sensor() {
    // General purpose sensor
}

void setup() {
    // High priority on individual line
    attachInterrupt(PB1, high_priority_sensor, FALLING);

    // Lower priority on shared line
    attachInterrupt(PB6, general_sensor, RISING);
}
```

## Reference Implementation

See `libraries/ICM42688P/examples/example-raw-ag/` and `example-raw-data-registers/` for complete interrupt-driven IMU data acquisition examples.

## Related Documentation

- STM32F4xx Reference Manual: EXTI peripheral (Section 12)
- ARM Cortex-M4 Programming Manual: NVIC configuration
- Arduino Core Documentation: Interrupt handling best practices