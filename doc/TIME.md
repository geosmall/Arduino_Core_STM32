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

### How ISR Detection Works

The timing functions check two conditions to detect ISR or critical-section context:

1. **`SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk`** - Non-zero when executing inside an interrupt handler (Handler mode vs Thread mode)
2. **`__get_BASEPRI()`** - Non-zero when interrupts are being masked via BASEPRI (e.g., inside an `ATOMIC_BLOCK`)

If either condition is true, SysTick_Handler may be blocked from running, so the ISR-safe path is used to avoid incorrect timing values.

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

## Implementation Details

This section describes the ISR-safe timing implementation for developers maintaining or extending the code.

### Source Files

| File | Purpose |
|------|---------|
| `cores/arduino/stm32/clock.c` | Core timing functions: `getCurrentMicros64()`, `getCurrentMicros64ISR()`, `SysTick_Handler()` |
| `cores/arduino/stm32/clock.h` | Public declarations for timing functions |
| `cores/arduino/stm32/atomic.h` | `ATOMIC_BLOCK()` macro using ARM BASEPRI register |
| `cores/arduino/stm32/hw_config.c` | Initializes `usTicks` during startup |
| `cores/arduino/wiring_time.c` | Arduino API wrappers: `micros()`, `micros64()`, `millis()` |

### Function Call Chain

```
micros64()                      [wiring_time.c]
  └─► getCurrentMicros64()      [clock.c]
        ├─► getCurrentMicros64ISR()   (if in ISR or BASEPRI elevated)
        └─► lock-free path            (if in main thread)

micros()                        [wiring_time.c]
  └─► getCurrentMicros()        [clock.c]
        └─► getCurrentMicros64()      (32-bit truncation of 64-bit result)
```

### Global Variables (clock.c)

Three variables coordinate timing across `SysTick_Handler` and the read functions:

| Variable | Type | Set By | Purpose |
|----------|------|--------|---------|
| `usTicks` | `uint32_t` | `hw_config_init()` | CPU cycles per microsecond (e.g., 100 for 100 MHz) |
| `sysTickValStamp` | `volatile uint32_t` | `SysTick_Handler()` | `SysTick->VAL` captured immediately after each reload |
| `sysTickPending` | `volatile int` | `getCurrentMicros64ISR()` | Flag indicating rollover detected but handler blocked |

**Why `sysTickValStamp`?** SysTick counts DOWN from LOAD to 0. After reload, VAL jumps back to ~LOAD. By capturing VAL right after reload in `SysTick_Handler`, we get a reference value (~99999 for 100 MHz). If a main-thread read sees `cycle_cnt > sysTickValStamp`, it knows VAL reloaded but `HAL_GetTick()` hasn't incremented yet—a race condition requiring retry.

**Why `sysTickPending`?** In ISR context, `SysTick_Handler` may be blocked (by BASEPRI). If COUNTFLAG indicates rollover, we set this flag to add 1ms to the result. Cleared by `SysTick_Handler` when it finally runs.

### Two Read Paths

**Why two paths?** Reading microsecond time requires combining two values: the millisecond count (`HAL_GetTick()`) and the sub-millisecond portion (`SysTick->VAL`). A race condition occurs if SysTick rolls over between reading these values—you get a stale ms with a fresh VAL, resulting in ~1ms error.

The solution depends on execution context:

| Context | Problem | Solution |
|---------|---------|----------|
| **Main thread** | `SysTick_Handler` can interrupt us mid-read | Retry loop—if inconsistency detected, try again |
| **ISR context** | `SysTick_Handler` is blocked (by BASEPRI or lower priority) | Check COUNTFLAG to detect rollover manually |

The main-thread path is **lock-free** (no interrupt disabling), which is preferred when possible because it doesn't increase interrupt latency. But this approach relies on `SysTick_Handler` running to update `sysTickValStamp`—if we're blocking the handler, we must use the ISR path instead.

### getCurrentMicros64() — Main Entry Point

This function (in `clock.c`) detects execution context and dispatches to the appropriate path:

```c
uint64_t getCurrentMicros64(void)
{
    // ──────────────────────────────────────────────────────────────────────
    // [A] Context detection: Are we in an ISR or critical section?
    // ──────────────────────────────────────────────────────────────────────
    if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) || (__get_BASEPRI())) {
        return getCurrentMicros64ISR();  // Use ISR-safe path
    }

    // ──────────────────────────────────────────────────────────────────────
    // [B] Main-thread path: Lock-free read with validation loop
    // ──────────────────────────────────────────────────────────────────────
    uint32_t ms, cycle_cnt;
    do {
        ms = HAL_GetTick();           // [B1] Read milliseconds
        cycle_cnt = SysTick->VAL;     // [B2] Read sub-ms cycles
    } while (ms != HAL_GetTick()      // [B3] Check: did ms change?
             || cycle_cnt > sysTickValStamp);  // [B4] Check: did VAL reload?

    // ──────────────────────────────────────────────────────────────────────
    // [C] Calculate microseconds from consistent ms + cycle_cnt pair
    // ──────────────────────────────────────────────────────────────────────
    const uint32_t partial = (usTicks * 1000U - cycle_cnt) / usTicks;
    return ((uint64_t)ms * 1000ULL) + partial;
}
```

**Line-by-line explanation:**

- **[A]** `SCB->ICSR & VECTACTIVE_Msk` is non-zero if we're in Handler mode (inside an ISR). `__get_BASEPRI()` is non-zero if interrupts are being masked. Either condition means `SysTick_Handler` may be blocked, so we must use the ISR-safe path.

- **[B1-B2]** Read the two timing values. Between these reads, `SysTick_Handler` could fire and update both `HAL_GetTick()` and `sysTickValStamp`.

- **[B3]** If `ms != HAL_GetTick()`, the handler ran between our reads—retry.

- **[B4]** If `cycle_cnt > sysTickValStamp`, SysTick reloaded (VAL jumped from ~0 back to ~LOAD) but `HAL_GetTick()` hasn't incremented yet. Since `sysTickValStamp` is captured right after reload (~99999 for 100MHz), any larger value means we caught a stale ms—retry.

- **[C]** Once we have a consistent pair, calculate: `(cycles elapsed this tick) / (cycles per µs)` gives sub-ms portion in microseconds.

### getCurrentMicros64ISR() — ISR-Safe Path

This function (in `clock.c`) is used when `SysTick_Handler` may be blocked:

```c
uint64_t getCurrentMicros64ISR(void)
{
    uint32_t ms, pending, cycle_cnt;

    // ──────────────────────────────────────────────────────────────────────
    // [A] ATOMIC_BLOCK: Elevate BASEPRI to prevent preemption during read
    // ──────────────────────────────────────────────────────────────────────
    ATOMIC_BLOCK(NVIC_PRIO_MAX) {
        cycle_cnt = SysTick->VAL;     // [A1] Read current cycle count

        // ──────────────────────────────────────────────────────────────────
        // [B] Check COUNTFLAG: Did SysTick roll over while handler blocked?
        // ──────────────────────────────────────────────────────────────────
        if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
            sysTickPending = 1;       // [B1] Mark rollover detected
            cycle_cnt = SysTick->VAL; // [B2] Re-read VAL (now post-reload)
        }

        ms = HAL_GetTick();           // [A2] Read milliseconds
        pending = sysTickPending;     // [A3] Capture pending flag
    }
    // BASEPRI restored here automatically

    // ──────────────────────────────────────────────────────────────────────
    // [C] Calculate microseconds, adding 1ms if rollover pending
    // ──────────────────────────────────────────────────────────────────────
    const uint32_t partial = (usTicks * 1000U - cycle_cnt) / usTicks;
    return ((uint64_t)(ms + pending) * 1000ULL) + partial;
}
```

**Line-by-line explanation:**

- **[A]** `ATOMIC_BLOCK(NVIC_PRIO_MAX)` elevates BASEPRI to block all maskable interrupts (priority ≥1). This ensures the entire read sequence is atomic—no ISR can preempt us mid-read. BASEPRI is automatically restored when the block exits.

- **[A1]** Read `SysTick->VAL` first (before checking COUNTFLAG, since reading CTRL clears the flag).

- **[B]** **COUNTFLAG** (bit 16 of `SysTick->CTRL`) is set by hardware when SysTick counts from 1→0, and cleared by reading CTRL. If set, SysTick rolled over but `SysTick_Handler` hasn't run (we're blocking it).

- **[B1]** Set `sysTickPending = 1` so we add 1ms to the result (compensating for the tick increment that hasn't happened yet).

- **[B2]** Re-read VAL because the first read [A1] was pre-rollover (near 0), but now it's post-rollover (near LOAD).

- **[A2-A3]** Read ms and the pending flag while still in the atomic block.

- **[C]** Calculate microseconds. If `pending == 1`, we add 1ms to compensate for the blocked `SysTick_Handler`.

### SysTick_Handler Atomicity

In `SysTick_Handler()` (clock.c), the timing variables are updated atomically:

```c
void SysTick_Handler(void) {
    ATOMIC_BLOCK(NVIC_PRIO_MAX) {
        sysTickPending = 0;
        sysTickValStamp = SysTick->VAL;
    }
    HAL_IncTick();
}
```

**Why ATOMIC_BLOCK here?** A priority 0 ISR can preempt `SysTick_Handler` (which typically runs at priority 0-1). If that ISR calls `micros64()`, it must see consistent values. Without the atomic block, it could see `sysTickPending = 0` (cleared) but a stale `sysTickValStamp` (not yet updated), causing a ~1ms timing error.

The block also provides a memory barrier, preventing the compiler from reordering the two writes.

## Performance

Typical overhead per call:
- `millis()`: ~5-10 cycles
- `micros()`: ~15-25 cycles
- `micros64()`: ~15-25 cycles

## See Also

- `cores/arduino/stm32/clock.c` - Implementation
- `cores/arduino/stm32/atomic.h` - ATOMIC_BLOCK macro
