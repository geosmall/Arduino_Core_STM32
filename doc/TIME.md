# Timekeeping in Arduino_Core_STM32

## Overview

This core provides microsecond and millisecond timing functions built on the ARM Cortex-M SysTick timer. All timing functions are ISR-safe.

## Functions

| Function | Return Type | Range | Overflow |
|----------|-------------|-------|----------|
| `millis()` | `uint32_t` | 0 - 4,294,967,295 ms | ~49.7 days |
| `micros()` | `uint32_t` | 0 - 4,294,967,295 µs | ~71.6 minutes |
| `micros64()` | `uint64_t` | 0 - 2^64 µs | ~584,942 years* |

*Practical limit is ~49.7 days due to underlying `millis()` overflow.

## ISR Safety

All timing functions automatically detect if called from interrupt context:
- **Main thread**: Uses lock-free algorithm with SysTick validation
- **ISR context**: Uses ATOMIC_BLOCK with BASEPRI for safe reads

You can safely call `micros()`, `micros64()`, or `millis()` from any context.

## Safety-Critical Applications (Motor Control, E-Stop)

The timing functions use BASEPRI-based critical sections, which **cannot block priority 0 interrupts**. This is intentional for safety-critical applications.

### Recommended Priority Assignment

| Priority | Use Case | Blocked by ATOMIC_BLOCK? |
|----------|----------|--------------------------|
| 0 | E-stop, motor commutation, fault handlers | **No** (always runs) |
| 1-2 | SysTick, high-priority sensors | Yes |
| 3-15 | UART, SPI, I2C, application timers | Yes |

### Setting Up Priority 0 ISRs

```c
// Example: Configure TIM1 for motor commutation at priority 0
void setupMotorTimer(void) {
    // ... timer configuration ...

    // Set to priority 0 (highest, never blocked)
    HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
}

// This ISR will NEVER be delayed by micros64() or ATOMIC_BLOCK
void TIM1_UP_TIM10_IRQHandler(void) {
    // Motor commutation - runs with zero added latency
    commutate_motor();
    __HAL_TIM_CLEAR_IT(&htim1, TIM_IT_UPDATE);
}
```

### Guidelines for Priority 0 ISRs

1. **Keep them extremely short** - No loops, no blocking calls
2. **Avoid calling `micros64()`** - Use the peripheral's own timer/counter for timing
3. **No ATOMIC_BLOCK inside** - They can preempt other ATOMIC_BLOCK sections, so internal state may be inconsistent
4. **Reserve for true safety-critical code** - E-stop, motor commutation, fault handlers

### Why Not `__disable_irq()`?

| Method | Priority 0 | Priorities 1-15 |
|--------|-----------|-----------------|
| `__disable_irq()` | Blocked | Blocked |
| `ATOMIC_BLOCK(NVIC_PRIO_MAX)` | **Runs** | Blocked |

Using BASEPRI instead of `__disable_irq()` ensures that safety-critical ISRs are never delayed, even by core timing functions.

## Architecture

### Hardware: SysTick Timer

- 24-bit down counter, reloads every 1 ms
- Generates SysTick_Handler interrupt at 1 kHz
- `SysTick->VAL`: Current count (decrements toward 0)
- `SysTick->LOAD`: Reload value (cycles per ms - 1)

### SysTick Configuration

SysTick is configured during Arduino startup via the HAL initialization chain:

```
hw_config_init()
    ├─► HAL_Init()
    │       └─► HAL_InitTick(TICK_INT_PRIORITY)
    │               └─► HAL_SYSTICK_Config(SystemCoreClock / 1000)
    │
    └─► SystemClock_Config()      ← Sets up clocks, PLL
            └─► SystemCoreClockUpdate()  ← Updates SystemCoreClock variable
```

### SystemCoreClock Derivation

`SystemCoreClock` is a global variable holding the CPU frequency in Hz. It's derived from the clock configuration in each board's variant file.

**Clock sources:**

| Source | Typical Value | Notes |
|--------|---------------|-------|
| HSI | 16 MHz | Internal RC oscillator (default at reset) |
| HSE | 8-25 MHz | External crystal (board-specific) |
| PLL | Up to 180 MHz (F4) | Multiplied from HSI or HSE |

**PLL calculation formula:**
```
PLL_VCO = (HSE_VALUE or HSI_VALUE) / PLLM × PLLN
SystemCoreClock = PLL_VCO / PLLP / AHB_Prescaler
```

**Example: NUCLEO_F411RE (100 MHz from 8 MHz HSE)**

```c
// From variant_NUCLEO_F411RE.cpp
RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;  // 8 MHz
RCC_OscInitStruct.PLL.PLLM = 4;
RCC_OscInitStruct.PLL.PLLN = 100;
RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;  // = 2

// Calculation:
//   PLL_VCO = 8 MHz / 4 × 100 = 200 MHz
//   SYSCLK  = 200 MHz / 2 = 100 MHz
//   SystemCoreClock = 100,000,000
```

**SystemCoreClockUpdate()** reads the actual RCC registers and updates `SystemCoreClock`:
```c
// From system_stm32f4xx.c
switch (RCC->CFGR & RCC_CFGR_SWS) {
    case 0x00: SystemCoreClock = HSI_VALUE; break;  // HSI
    case 0x04: SystemCoreClock = HSE_VALUE; break;  // HSE
    case 0x08: SystemCoreClock = pllvco/pllp; break; // PLL
}
SystemCoreClock >>= AHBPrescTable[...];  // Apply AHB divider
```

### SysTick Timer Setup

**HAL_SYSTICK_Config()** configures SysTick for 1 kHz interrupts:
```c
// Reload value for 1ms tick period
SysTick->LOAD = (SystemCoreClock / 1000) - 1;
SysTick->VAL  = 0;  // Clear current value
SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |  // Use CPU clock
                SysTick_CTRL_TICKINT_Msk   |  // Enable interrupt
                SysTick_CTRL_ENABLE_Msk;      // Enable counter

// Example at 100 MHz:
//   LOAD = 100,000,000 / 1000 - 1 = 99,999
//   SysTick counts down: 99999 → 99998 → ... → 1 → 0 → [interrupt] → reload
```

**Default priority** (`TICK_INT_PRIORITY`): **0** (highest)

This means SysTick runs at priority 0 by default, which:
- Cannot be blocked by `ATOMIC_BLOCK(NVIC_PRIO_MAX)`
- Shares priority level with any safety-critical ISRs you define

**Important:** With SysTick at priority 0, calling `micros64()` from another priority 0 ISR may have rare timing glitches (~1ms) if SysTick fires mid-read. For most applications this is not an issue. If you need guaranteed accuracy from priority 0 ISRs, move SysTick to priority 1.

### Changing SysTick Priority

If you need priority 0 exclusively for safety-critical ISRs (motor control, e-stop), move SysTick to priority 1:

```c
// In setup(), after Serial.begin() or other init
void setup() {
    // Move SysTick to priority 1 so ATOMIC_BLOCK can mask it
    HAL_NVIC_SetPriority(SysTick_IRQn, 1, 0);

    // Now priority 0 is free for motor/e-stop ISRs
}
```

**Recommended priority layout for motor control applications:**

| Priority | ISR | Notes |
|----------|-----|-------|
| 0 | Motor commutation, E-stop | Never blocked |
| 1 | SysTick | Blocked by ATOMIC_BLOCK |
| 2-15 | UART, SPI, sensors, etc. | Blocked by ATOMIC_BLOCK |

### Key Variables

| Variable | Type | Purpose |
|----------|------|---------|
| `usTicks` | `uint32_t` | CPU cycles per microsecond (e.g., 96 for 96 MHz) |
| `sysTickValStamp` | `volatile uint32_t` | SysTick->VAL captured at each ms tick |
| `sysTickPending` | `volatile int` | Flag: rollover detected but handler not yet run |

### Time Calculation

Microseconds = (milliseconds × 1000) + sub-millisecond portion

```c
// Sub-ms calculation (cycles elapsed this tick, converted to µs)
partial = (usTicks * 1000 - SysTick->VAL) / usTicks;
return (ms * 1000) + partial;
```

### Race Condition Handling

**Problem**: SysTick may roll over between reading `millis()` and `SysTick->VAL`.

**Solution**:
- Main thread: Loop until reads are consistent (`cycle_cnt <= sysTickValStamp`)
- ISR context: Check COUNTFLAG, set `sysTickPending` if rollover detected

## Performance

Typical overhead per call:
- `millis()`: ~5-10 cycles
- `micros()`: ~15-25 cycles
- `micros64()`: ~15-25 cycles

## See Also

- `cores/arduino/stm32/clock.c` - Implementation
- `cores/arduino/stm32/atomic.h` - ATOMIC_BLOCK macro
