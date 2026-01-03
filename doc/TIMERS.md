# STM32 Arduino Core Timer Architecture

Technical reference for hardware timer implementation in Arduino_Core_STM32.

## Architecture Stack

```
┌─────────────────────────────────────────────────────────┐
│ HardwareTimer Class (C++)                               │
│ Location: cores/arduino/HardwareTimer.{h,cpp}           │
│ • Object-oriented API with format conversion            │
│ • Automatic pin configuration via PinMap_PWM            │
│ • Callback management (std::function)                   │
└────────────────┬────────────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────────────┐
│ Core Timer Abstraction (C)                              │
│ Location: cores/arduino/stm32/timer.{c,h}               │
│ • Clock enable/disable (RCC)                            │
│ • Pin-to-timer-channel mapping                          │
│ • NVIC interrupt management                             │
│ • HAL MSP callbacks                                     │
└────────────────┬────────────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────────────┐
│ Pin Mapping Tables (Variant-specific)                   │
│ Location: variants/STM32F4xx/*/PeripheralPins.c         │
│ • PinMap_TIM[] array                                    │
│ • Pin → Timer + Channel + AF mapping                   │
└────────────────┬────────────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────────────┐
│ STM32 HAL Timer Driver                                  │
│ Location: system/Drivers/STM32F4xx_HAL_Driver/          │
│ • TIM_HandleTypeDef management                          │
│ • HAL_TIM_PWM_ConfigChannel()                           │
│ • Register initialization                               │
└────────────────┬────────────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────────────┐
│ STM32 LL (Low Layer) - Optional                         │
│ Location: stm32yyxx_ll_tim.h                            │
│ • Direct register macros                                │
│ • Performance-critical operations                       │
└────────────────┬────────────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────────────┐
│ Hardware Registers                                      │
│ TIMx->{CR1, PSC, ARR, CCRx, CCMR1, CCER, ...}          │
└─────────────────────────────────────────────────────────┘
```

## HardwareTimer Class API

### Constructor and Setup

```cpp
HardwareTimer(TIM_TypeDef *instance);  // Pass TIM1, TIM2, TIM3, etc.
void setup(TIM_TypeDef *instance);     // Alternative to constructor
```

**Important**: Use `new HardwareTimer(Instance)` to prevent premature destruction after `setup()` returns.

### Core Configuration Methods

```cpp
// Prescaler: divides timer clock (0-65535)
void setPrescaleFactor(uint32_t prescaler);
uint32_t getPrescaleFactor();

// Period (Auto-Reload Register): timer overflow value
void setOverflow(uint32_t val, TimerFormat_t format = TICK_FORMAT);
uint32_t getOverflow(TimerFormat_t format = TICK_FORMAT);

// Channel mode configuration (PWM, input capture, etc.)
void setMode(uint32_t channel, TimerModes_t mode, PinName pin = NC);
TimerModes_t getMode(uint32_t channel);

// Compare value (duty cycle for PWM)
void setCaptureCompare(uint32_t channel, uint32_t compare,
                       TimerCompareFormat_t format = TICK_COMPARE_FORMAT);
uint32_t getCaptureCompare(uint32_t channel,
                           TimerCompareFormat_t format = TICK_COMPARE_FORMAT);
```

### All-in-One PWM Configuration

```cpp
void setPWM(uint32_t channel, PinName pin,
            uint32_t frequency, uint32_t dutycycle,
            callback_function_t PeriodCallback = nullptr,
            callback_function_t CompareCallback = nullptr);
```

Internally calls: `setMode()` → `setOverflow()` → `setCaptureCompare()` → `resumeChannel()` → `resume()`

**Note**: `setPWM()` handles both channel enable (`resumeChannel()`) and counter start (`resume()`). When manually configuring PWM, you must call both methods explicitly.

### Timer Control

```cpp
void resume();              // Start timer counter (global)
void pause();               // Stop timer counter (global)
void resumeChannel(uint32_t channel);  // Enable channel output (CCxE bit)
void pauseChannel(uint32_t channel);   // Disable channel output (CCxE bit)
void refresh();             // Force register update (trigger UG event)
bool isRunning();           // Check if timer counter is running
bool isRunningChannel(uint32_t channel);
```

**Important**: For PWM generation, you need **both**:
1. `resumeChannel(channel)` - Enables the channel output (sets CCxE bit per AN4013 Section 2.5)
2. `resume()` - Starts the timer counter

The `setPWM()` method handles both automatically. When manually configuring PWM, you must call both methods.

### Format Enums

**TimerFormat_t** (for overflow/count):
- `TICK_FORMAT` - Raw timer ticks (default)
- `MICROSEC_FORMAT` - Microseconds
- `HERTZ_FORMAT` - Frequency in Hz

**TimerCompareFormat_t** (for capture/compare):
- `TICK_COMPARE_FORMAT` - Raw timer ticks (default)
- `MICROSEC_COMPARE_FORMAT` - Microseconds
- `HERTZ_COMPARE_FORMAT` - Frequency
- `PERCENT_COMPARE_FORMAT` - Duty cycle 0-100%
- `RESOLUTION_xB_COMPARE_FORMAT` - x-bit resolution (1-16 bit)

**TimerModes_t**:
- `TIMER_OUTPUT_COMPARE_PWM1` - Standard PWM (high when counter < compare)
- `TIMER_OUTPUT_COMPARE_PWM2` - Inverted PWM (low when counter < compare)
- `TIMER_DISABLED` - No output, timer running (interrupt-only)
- `TIMER_OUTPUT_COMPARE_TOGGLE` - Toggle output on match
- `TIMER_OUTPUT_COMPARE_ACTIVE` - Set high on match
- `TIMER_OUTPUT_COMPARE_INACTIVE` - Set low on match
- `TIMER_INPUT_CAPTURE_RISING` - Capture on rising edge
- `TIMER_INPUT_CAPTURE_FALLING` - Capture on falling edge
- `TIMER_INPUT_CAPTURE_BOTHEDGE` - Capture on both edges
- `TIMER_INPUT_FREQ_DUTY_MEASUREMENT` - Dual-channel frequency/duty measurement

## Pin Mapping Resolution

### Automatic Pin Lookup

```cpp
// Get timer instance for a pin
TIM_TypeDef *Instance = (TIM_TypeDef *)pinmap_peripheral(
    digitalPinToPinName(pin), PinMap_PWM);

// Get timer channel for a pin
uint32_t channel = STM_PIN_CHANNEL(pinmap_function(
    digitalPinToPinName(pin), PinMap_PWM));
```

### PinMap_TIM Structure

**Location**: `variants/STM32F4xx/*/PeripheralPins.c`

```c
WEAK const PinMap PinMap_TIM[] = {
  // Pin        Timer   STM_PIN_DATA_EXT(mode, pull, AF, channel, complementary)
  {PA_0,        TIM2,   STM_PIN_DATA_EXT(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF1_TIM2, 1, 0)},
  {PA_0_ALT1,   TIM5,   STM_PIN_DATA_EXT(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF2_TIM5, 1, 0)},
  {PA_6,        TIM3,   STM_PIN_DATA_EXT(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF2_TIM3, 1, 0)},
  {PA_7,        TIM3,   STM_PIN_DATA_EXT(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF2_TIM3, 2, 0)},
  {PB_0,        TIM3,   STM_PIN_DATA_EXT(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF2_TIM3, 3, 0)},
  {PB_1,        TIM3,   STM_PIN_DATA_EXT(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF2_TIM3, 4, 0)},
  // ... more entries
  {NC,          NP,     0}
};
```

**Note**: Multiple pins can map to the same timer/channel. Use `_ALTx` suffix for alternatives.

### STM32F411RE TIM3 Example

| Pin  | Timer | Channel | Alternate Function |
|------|-------|---------|-------------------|
| PA_6 | TIM3  | CH1     | GPIO_AF2_TIM3     |
| PA_7 | TIM3  | CH2     | GPIO_AF2_TIM3     |
| PB_0 | TIM3  | CH3     | GPIO_AF2_TIM3     |
| PB_1 | TIM3  | CH4     | GPIO_AF2_TIM3     |
| PB_4 | TIM3  | CH1     | GPIO_AF2_TIM3     |
| PB_5 | TIM3  | CH2     | GPIO_AF2_TIM3     |
| PC_6 | TIM3  | CH1     | GPIO_AF2_TIM3     |
| PC_7 | TIM3  | CH2     | GPIO_AF2_TIM3     |
| PC_8 | TIM3  | CH3     | GPIO_AF2_TIM3     |
| PC_9 | TIM3  | CH4     | GPIO_AF2_TIM3     |

## RCC Clock Tree and Timer Clock Derivation

### STM32 Clock Architecture Overview

```
                        ┌──────────────┐
                        │ System Clock │
                        │   (SYSCLK)   │
                        └──────┬───────┘
                               │
                        ┌──────▼───────┐
                        │ AHB Prescaler│
                        │   (/1-512)   │
                        └──────┬───────┘
                               │
                          ┌────▼────┐
                          │  HCLK   │
                          └────┬────┘
                               │
          ┌────────────────────┼────────────────────┐
          │                    │                    │
    ┌─────▼──────┐      ┌─────▼──────┐      ┌─────▼──────┐
    │   APB1     │      │   APB2     │      │ Cortex     │
    │ Prescaler  │      │ Prescaler  │      │ Peripherals│
    │ (/1-16)    │      │ (/1-16)    │      └────────────┘
    └─────┬──────┘      └─────┬──────┘
          │                   │
    ┌─────▼──────┐      ┌─────▼──────┐
    │  PCLK1     │      │  PCLK2     │
    │ (APB1 Clk) │      │ (APB2 Clk) │
    └─────┬──────┘      └─────┬──────┘
          │                   │
          │                   │
    ┌─────▼──────┐      ┌─────▼──────┐
    │ Timer Clk  │      │ Timer Clk  │
    │ Multiplier │      │ Multiplier │
    │  (×1 or ×2)│      │  (×1 or ×2)│
    └─────┬──────┘      └─────┬──────┘
          │                   │
    ┌─────▼─────────────┐     │
    │ TIM2, TIM3, TIM4, │     │
    │ TIM5, TIM6, TIM7, │     │
    │ TIM12, TIM13,     │     │
    │ TIM14             │     │
    └───────────────────┘     │
                        ┌─────▼─────────────┐
                        │ TIM1, TIM8, TIM9, │
                        │ TIM10, TIM11      │
                        └───────────────────┘
```

### Timer Clock Multiplier Rule

**Critical Hardware Behavior**: STM32 timers include an automatic clock multiplier that adjusts based on the APB prescaler setting.

**Rule** (Reference Manual RM0383 Section 7.2):
- **If APB prescaler = 1** (no division): Timer Clock = APB Clock (1× multiplier)
- **If APB prescaler > 1** (divided): Timer Clock = 2 × APB Clock (2× multiplier)

This rule applies to **both APB1 and APB2 buses**.

**Rationale**: Ensures timers can maintain higher frequencies even when APB buses are prescaled down to meet peripheral speed requirements.

### Timer Clock Source Assignment

**APB1 Timers** (`getTimerClkSrc()` returns `1`):
- TIM2, TIM3, TIM4, TIM5, TIM6, TIM7
- TIM12, TIM13, TIM14 (if present)

**APB2 Timers** (`getTimerClkSrc()` returns `2`):
- TIM1, TIM8, TIM9, TIM10, TIM11
- TIM15, TIM16, TIM17, TIM20 (if present)

**Source**: `cores/arduino/stm32/timer.c:627-707`

### getTimerClkFreq() Implementation

**Location**: `HardwareTimer.cpp:1368-1493`

**Algorithm** (for STM32F4, non-TIMPRE variants):
```cpp
uint32_t HardwareTimer::getTimerClkFreq() {
  RCC_ClkInitTypeDef clkconfig;
  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

  // Step 1: Determine APB clock source (APB1 or APB2)
  if (timer on APB1) {
    uwAPBxPrescaler = clkconfig.APB1CLKDivider;
    uwTimclock = HAL_RCC_GetPCLK1Freq();
  } else {
    uwAPBxPrescaler = clkconfig.APB2CLKDivider;
    uwTimclock = HAL_RCC_GetPCLK2Freq();
  }

  // Step 2: Apply automatic 2× multiplier if prescaler > 1
  switch (uwAPBxPrescaler) {
    case RCC_HCLK_DIV1:
      // Timer clock = APB clock (no multiplier)
      break;
    case RCC_HCLK_DIV2:
    case RCC_HCLK_DIV4:
    case RCC_HCLK_DIV8:
    case RCC_HCLK_DIV16:
      uwTimclock *= 2;  // Timer clock = 2 × APB clock
      break;
  }

  return uwTimclock;
}
```

### Clock Configuration Examples

#### Example 1: STM32F411RE @ 100 MHz (Typical Arduino Configuration)

**System Configuration**:
```
SYSCLK = 100 MHz (from PLL)
AHB Prescaler = /1  →  HCLK = 100 MHz
APB1 Prescaler = /4  →  PCLK1 = 25 MHz
APB2 Prescaler = /2  →  PCLK2 = 50 MHz
```

**Timer Clocks**:
```
APB1 Timers (TIM2-7):
  PCLK1 = 25 MHz
  APB1 Prescaler = /4 (> 1)  →  Apply 2× multiplier
  Timer Clock = 25 MHz × 2 = 50 MHz

APB2 Timers (TIM1, TIM9-11):
  PCLK2 = 50 MHz
  APB2 Prescaler = /2 (> 1)  →  Apply 2× multiplier
  Timer Clock = 50 MHz × 2 = 100 MHz
```

**Verify in code**:
```cpp
HardwareTimer *tim3 = new HardwareTimer(TIM3);  // APB1 timer
Serial.print("TIM3 clock: ");
Serial.println(tim3->getTimerClkFreq());  // Prints: 50000000 (50 MHz)

HardwareTimer *tim1 = new HardwareTimer(TIM1);  // APB2 timer
Serial.print("TIM1 clock: ");
Serial.println(tim1->getTimerClkFreq());  // Prints: 100000000 (100 MHz)
```

#### Example 2: Maximum Performance Configuration

**System Configuration**:
```
SYSCLK = 100 MHz
AHB Prescaler = /1  →  HCLK = 100 MHz
APB1 Prescaler = /2  →  PCLK1 = 50 MHz  (max for APB1 on F411)
APB2 Prescaler = /1  →  PCLK2 = 100 MHz
```

**Timer Clocks**:
```
APB1 Timers (TIM2-7):
  PCLK1 = 50 MHz
  APB1 Prescaler = /2 (> 1)  →  Apply 2× multiplier
  Timer Clock = 50 MHz × 2 = 100 MHz  ← Maximum for APB1 timers!

APB2 Timers (TIM1, TIM9-11):
  PCLK2 = 100 MHz
  APB2 Prescaler = /1 (= 1)  →  No multiplier
  Timer Clock = 100 MHz × 1 = 100 MHz
```

**Note**: APB1 peripherals (I2C, UART, SPI1) limited to 50 MHz max on STM32F411.

#### Example 3: 1 MHz Timer Tick Rate

**Goal**: Configure TIM3 for 1 µs resolution (1 MHz counter frequency)

**Given**: TIM3 clock = 50 MHz (from Example 1)

**Prescaler Calculation**:
```cpp
HardwareTimer *tim3 = new HardwareTimer(TIM3);

uint32_t timer_clock_hz = tim3->getTimerClkFreq();  // 50 MHz
uint32_t target_tick_freq_hz = 1'000'000;           // 1 MHz
uint32_t prescaler = (timer_clock_hz / target_tick_freq_hz) - 1;
// prescaler = (50000000 / 1000000) - 1 = 49

tim3->setPrescaleFactor(prescaler);

// Verify:
// Timer Input Clock = 50 MHz
// Prescaler = 49
// Counter Clock = 50 MHz / (49 + 1) = 1 MHz ✓
```

**Counter Clock Formula**:
```
f_counter = f_timer_clock / (PSC + 1)
```

Where:
- `f_counter` = timer counter frequency (tick rate)
- `f_timer_clock` = result of `getTimerClkFreq()`
- `PSC` = prescaler register value (0-65535)

### HAL RCC Functions Used

**Get APB Clock Frequencies**:
```cpp
uint32_t HAL_RCC_GetPCLK1Freq(void);  // Returns APB1 peripheral clock
uint32_t HAL_RCC_GetPCLK2Freq(void);  // Returns APB2 peripheral clock
uint32_t HAL_RCC_GetHCLKFreq(void);   // Returns AHB clock (HCLK)
```

**Get Clock Configuration**:
```cpp
void HAL_RCC_GetClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct,
                            uint32_t *pFLatency);

// Returns:
typedef struct {
  uint32_t ClockType;       // SYSCLK, HCLK, PCLK1, PCLK2
  uint32_t SYSCLKSource;    // HSI, HSE, PLL
  uint32_t AHBCLKDivider;   // HCLK prescaler
  uint32_t APB1CLKDivider;  // APB1 prescaler (RCC_HCLK_DIVx)
  uint32_t APB2CLKDivider;  // APB2 prescaler (RCC_HCLK_DIVx)
} RCC_ClkInitTypeDef;
```

### Advanced: TIMPRE Bit (Extended Timer Prescaler)

Some STM32F4 variants (not F405/F407/F415/F417) support an additional TIMPRE bit in RCC_DCKCFGR register.

**When TIMPRE = 0** (default):
- APB prescaler = 1  →  Timer Clock = APB Clock
- APB prescaler > 1  →  Timer Clock = 2 × APB Clock

**When TIMPRE = 1**:
- APB prescaler = 1, 2, or 4  →  Timer Clock = HCLK
- APB prescaler = 8 or 16  →  Timer Clock = 4 × APB Clock

**Check if TIMPRE is active**:
```cpp
RCC_PeriphCLKInitTypeDef PeriphClkConfig;
HAL_RCCEx_GetPeriphCLKConfig(&PeriphClkConfig);

if (PeriphClkConfig.TIMPresSelection == RCC_TIMPRES_ACTIVATED) {
  // TIMPRE = 1: Use extended prescaler rules
}
```

**Source**: `HardwareTimer.cpp:1456-1477` (STM32F4/F7 specific)

**Note**: Arduino Core handles this automatically in `getTimerClkFreq()`. Most applications use default TIMPRE = 0.

### Get Timer Clock Frequency

**Best Practice**: Always query the timer clock dynamically:

```cpp
HardwareTimer *timer = new HardwareTimer(TIM3);
uint32_t timer_clock_hz = timer->getTimerClkFreq();
```

**Why?**
- Clock configuration may vary between boards
- User code may change system clocks dynamically
- Ensures portability across STM32 families

## Complete PWM Frequency Calculation Chain

This section demonstrates the complete calculation from system clock (RCC) through timer registers to final PWM output frequency.

### Formula Overview

```
                    RCC Clock Tree
                          ↓
              ┌───────────────────────┐
              │ Timer Input Frequency │ ← getTimerClkFreq()
              │   (f_timer_clock)     │
              └───────────┬───────────┘
                          ↓
                    ÷ (PSC + 1)
                          ↓
              ┌───────────────────────┐
              │  Counter Frequency    │
              │    (f_counter)        │
              └───────────┬───────────┘
                          ↓
                    ÷ (ARR + 1)
                          ↓
              ┌───────────────────────┐
              │   PWM Frequency       │
              │     (f_pwm)           │
              └───────────────────────┘
                          ↓
              Duty Cycle = CCRx / (ARR + 1)
                          ↓
              ┌───────────────────────┐
              │   PWM Output Signal   │
              │  Frequency + Duty     │
              └───────────────────────┘
```

### Complete Formula

```
f_pwm = f_timer_clock / [(PSC + 1) × (ARR + 1)]

Duty Cycle (%) = (CCRx / ARR) × 100
Pulse Width (µs) = (CCRx / f_counter) × 1,000,000
```

Where:
- `f_timer_clock` = Timer input clock from RCC (Hz)
- `PSC` = Prescaler register value (0-65535)
- `ARR` = Auto-Reload Register value (0-65535 for 16-bit timers)
- `CCRx` = Capture/Compare Register value for channel x
- `f_counter` = Counter tick frequency = `f_timer_clock / (PSC + 1)`
- `f_pwm` = PWM output frequency (Hz)

### Example Calculation: 1 kHz PWM with 50% Duty Cycle

**Step 1: Determine Timer Clock (RCC)**

```cpp
HardwareTimer *tim3 = new HardwareTimer(TIM3);
uint32_t f_timer_clock = tim3->getTimerClkFreq();
// Result: 50 MHz (for STM32F411 @ 100 MHz, APB1 /4 with 2× multiplier)
```

**Step 2: Calculate Prescaler for Desired Counter Frequency**

Target: 1 MHz counter frequency (1 µs resolution)

```cpp
uint32_t f_counter_target = 1'000'000;  // 1 MHz
uint32_t PSC = (f_timer_clock / f_counter_target) - 1;
// PSC = (50,000,000 / 1,000,000) - 1 = 49

tim3->setPrescaleFactor(PSC);

// Actual counter frequency:
uint32_t f_counter = f_timer_clock / (PSC + 1);
// f_counter = 50,000,000 / 50 = 1,000,000 Hz = 1 MHz ✓
```

**Step 3: Calculate ARR for Desired PWM Frequency**

Target: 1 kHz PWM frequency (1 ms period)

```cpp
uint32_t f_pwm_target = 1000;  // 1 kHz
uint32_t ARR = (f_counter / f_pwm_target) - 1;
// ARR = (1,000,000 / 1,000) - 1 = 999

tim3->setOverflow(ARR, TICK_FORMAT);
// Or equivalently:
// tim3->setOverflow(1000, MICROSEC_FORMAT);

// Actual PWM frequency:
uint32_t f_pwm = f_counter / (ARR + 1);
// f_pwm = 1,000,000 / 1000 = 1,000 Hz = 1 kHz ✓
```

**Step 4: Calculate CCR for Desired Duty Cycle**

Target: 50% duty cycle (500 µs high, 500 µs low)

```cpp
uint32_t duty_percent = 50;
uint32_t CCR1 = (ARR * duty_percent) / 100;
// CCR1 = (999 * 50) / 100 = 499

tim3->setCaptureCompare(1, CCR1, TICK_COMPARE_FORMAT);
// Or equivalently:
// tim3->setCaptureCompare(1, 50, PERCENT_COMPARE_FORMAT);
// tim3->setCaptureCompare(1, 500, MICROSEC_COMPARE_FORMAT);

// Actual duty cycle:
float duty_actual = (CCR1 / (float)ARR) * 100.0;
// duty_actual = (499 / 999) * 100 = 49.95% ≈ 50% ✓

// Actual pulse width:
float pulse_width_us = (CCR1 * 1000000.0) / f_counter;
// pulse_width_us = (499 * 1000000) / 1000000 = 499 µs
```

**Step 5: Start PWM**

```cpp
tim3->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_6);  // TIM3_CH1
tim3->resume();
```

**Final Configuration Summary**:
```
Timer: TIM3 (APB1)
Input Clock: 50 MHz (from RCC)
PSC = 49  →  Counter: 1 MHz (1 µs ticks)
ARR = 999  →  PWM Frequency: 1 kHz (1 ms period)
CCR1 = 499  →  Duty Cycle: 49.95% (499 µs high, 500 µs low)
Output: PA_6 (TIM3_CH1)
```

### Example Calculation: Servo Control (50 Hz, 1500 µs)

**Requirement**: Standard RC servo at 50 Hz (20 ms period), 1500 µs neutral position.

```cpp
HardwareTimer *servo_tim = new HardwareTimer(TIM3);

// Step 1: Timer clock (APB1 timer on F411)
uint32_t f_timer_clock = servo_tim->getTimerClkFreq();  // 50 MHz

// Step 2: Prescaler for 1 MHz counter (1 µs resolution)
uint32_t f_counter_target = 1'000'000;
uint32_t PSC = (f_timer_clock / f_counter_target) - 1;  // 49
servo_tim->setPrescaleFactor(PSC);

// Step 3: ARR for 50 Hz (20 ms period)
uint32_t period_us = 20000;  // 20 ms = 20,000 µs
uint32_t ARR = period_us - 1;  // 19,999
servo_tim->setOverflow(period_us, MICROSEC_FORMAT);

// Step 4: CCR for 1500 µs pulse (neutral)
uint32_t pulse_us = 1500;
servo_tim->setCaptureCompare(1, pulse_us, MICROSEC_COMPARE_FORMAT);

// Step 5: Start PWM
servo_tim->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_6);
servo_tim->resume();

// Verification:
// f_counter = 50 MHz / 50 = 1 MHz
// f_pwm = 1 MHz / 20,000 = 50 Hz ✓
// Pulse width = 1500 ticks @ 1 MHz = 1500 µs ✓
// Duty cycle = (1500 / 20000) * 100 = 7.5%
```

### Example Calculation: High-Frequency ESC (400 Hz, 1250 µs)

**Requirement**: Modern quadcopter ESC at 400 Hz (2.5 ms period), 1250 µs throttle.

```cpp
HardwareTimer *esc_tim = new HardwareTimer(TIM5);  // Use TIM5 for higher resolution

// Step 1: Timer clock (APB1 timer on F411)
uint32_t f_timer_clock = esc_tim->getTimerClkFreq();  // 50 MHz

// Step 2: Prescaler for 1 MHz counter
uint32_t PSC = (f_timer_clock / 1'000'000) - 1;  // 49
esc_tim->setPrescaleFactor(PSC);

// Step 3: ARR for 400 Hz (2500 µs period)
uint32_t period_us = 2500;
esc_tim->setOverflow(period_us, MICROSEC_FORMAT);  // ARR = 2499

// Step 4: CCR for 1250 µs pulse (mid throttle)
esc_tim->setCaptureCompare(1, 1250, MICROSEC_COMPARE_FORMAT);

// Step 5: Start PWM
esc_tim->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_0);  // TIM5_CH1
esc_tim->resume();

// Verification:
// f_pwm = 1 MHz / 2500 = 400 Hz ✓
// Pulse width = 1250 µs ✓
// Duty cycle = (1250 / 2500) * 100 = 50%
```

### Practical Constraints

**16-bit Timer Limits**:
- PSC max = 65,535
- ARR max = 65,535 (TIM2 and TIM5 are 32-bit on some STM32)
- Counter frequency: `f_timer_clock / 65,536` to `f_timer_clock / 1`
- PWM period: 1 tick to 65,536 ticks

**Minimum PWM Frequency** (TIM3 @ 50 MHz):
```
PSC = 65,535  →  f_counter = 50 MHz / 65,536 ≈ 763 Hz
ARR = 65,535  →  f_pwm = 763 Hz / 65,536 ≈ 0.0116 Hz (86 second period!)
```

**Maximum PWM Frequency** (TIM3 @ 50 MHz):
```
PSC = 0  →  f_counter = 50 MHz
ARR = 1  →  f_pwm = 50 MHz / 2 = 25 MHz
```

**Practical Range**:
- **Servo/ESC control**: 50-500 Hz (requires 1 µs resolution)
- **Motor drive PWM**: 1-20 kHz (good efficiency/noise tradeoff)
- **LED dimming**: 100 Hz - 10 kHz (above flicker perception)
- **Switching power supplies**: 20-100 kHz (depends on inductor/capacitor sizing)

### Resolution vs Frequency Tradeoff

For a fixed counter frequency `f_counter`:

```
PWM Resolution (ticks) = f_counter / f_pwm
```

**Example**: `f_counter = 1 MHz`, `f_pwm = 1 kHz`
```
Resolution = 1,000,000 / 1,000 = 1,000 ticks (10-bit equivalent)
```

Higher PWM frequency reduces resolution:

| PWM Freq | Period | Resolution @ 1 MHz | Equivalent Bits |
|----------|--------|-------------------|-----------------|
| 50 Hz    | 20 ms  | 20,000 ticks      | ~14.3 bits      |
| 100 Hz   | 10 ms  | 10,000 ticks      | ~13.3 bits      |
| 1 kHz    | 1 ms   | 1,000 ticks       | ~10.0 bits      |
| 10 kHz   | 100 µs | 100 ticks         | ~6.6 bits       |
| 100 kHz  | 10 µs  | 10 ticks          | ~3.3 bits       |

**Equivalent bits** = `log₂(resolution)`

### Multi-Channel PWM on Same Timer

All channels on a timer share the same PSC and ARR (same frequency), but have independent CCRx values (different duty cycles).

```cpp
HardwareTimer *quad_esc = new HardwareTimer(TIM3);

// Common configuration for all 4 ESC channels
uint32_t f_timer_clock = quad_esc->getTimerClkFreq();
uint32_t PSC = (f_timer_clock / 1'000'000) - 1;  // 1 MHz counter
quad_esc->setPrescaleFactor(PSC);
quad_esc->setOverflow(2500, MICROSEC_FORMAT);     // 400 Hz

// Configure all 4 channels
quad_esc->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_6);  // Motor 1
quad_esc->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, PA_7);  // Motor 2
quad_esc->setMode(3, TIMER_OUTPUT_COMPARE_PWM1, PB_0);  // Motor 3
quad_esc->setMode(4, TIMER_OUTPUT_COMPARE_PWM1, PB_1);  // Motor 4

// Independent throttle control
quad_esc->setCaptureCompare(1, 1100, MICROSEC_COMPARE_FORMAT);  // Front-left
quad_esc->setCaptureCompare(2, 1200, MICROSEC_COMPARE_FORMAT);  // Front-right
quad_esc->setCaptureCompare(3, 1300, MICROSEC_COMPARE_FORMAT);  // Rear-left
quad_esc->setCaptureCompare(4, 1400, MICROSEC_COMPARE_FORMAT);  // Rear-right

quad_esc->resume();

// All channels run at 400 Hz, but with different pulse widths
```

### Prescaler Calculation for 1 MHz Tick Rate

```cpp
// Target: 1 µs per timer tick
uint32_t timer_clock_hz = MyTim->getTimerClkFreq();
uint32_t target_tick_freq_hz = 1'000'000;  // 1 MHz
uint32_t prescaler = (timer_clock_hz / target_tick_freq_hz) - 1;

MyTim->setPrescaleFactor(prescaler);
```

**Example**: If `timer_clock_hz = 100 MHz`, then `prescaler = 99`.
Timer will tick at 100 MHz / (99 + 1) = 1 MHz.

### Period Calculation

```cpp
// For 1000 µs (1 ms) period at 1 MHz tick rate:
uint32_t period_us = 1000;
uint32_t period_ticks = period_us - 1;  // ARR is zero-indexed

MyTim->setOverflow(period_ticks, TICK_FORMAT);
// Or directly in microseconds:
MyTim->setOverflow(period_us, MICROSEC_FORMAT);
```

## PWM Generation Patterns

### Simple PWM (All-in-One)

```cpp
#define pin PA_6  // TIM3_CH1

void setup() {
  TIM_TypeDef *Instance = (TIM_TypeDef *)pinmap_peripheral(
      digitalPinToPinName(pin), PinMap_PWM);
  uint32_t channel = STM_PIN_CHANNEL(pinmap_function(
      digitalPinToPinName(pin), PinMap_PWM));

  HardwareTimer *MyTim = new HardwareTimer(Instance);
  MyTim->setPWM(channel, pin, 1000, 50);  // 1 kHz, 50% duty
}

void loop() {
  // PWM runs in hardware, no CPU load
}
```

### Granular PWM Configuration

```cpp
void setup() {
  HardwareTimer *timer = new HardwareTimer(TIM3);

  // 1. Configure timer base (1 MHz tick, 1 kHz PWM)
  uint32_t timer_clock_hz = timer->getTimerClkFreq();
  uint32_t prescaler = (timer_clock_hz / 1'000'000) - 1;  // 1 MHz
  timer->setPrescaleFactor(prescaler);
  timer->setOverflow(1000, MICROSEC_FORMAT);  // 1000 µs = 1 kHz

  // 2. Configure channels
  timer->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_6);  // CH1
  timer->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, PA_7);  // CH2
  timer->setMode(3, TIMER_OUTPUT_COMPARE_PWM1, PB_0);  // CH3
  timer->setMode(4, TIMER_OUTPUT_COMPARE_PWM1, PB_1);  // CH4

  // 3. Set initial pulse widths
  timer->setCaptureCompare(1, 1500, MICROSEC_COMPARE_FORMAT);  // 1.5 ms
  timer->setCaptureCompare(2, 1000, MICROSEC_COMPARE_FORMAT);  // 1.0 ms
  timer->setCaptureCompare(3, 2000, MICROSEC_COMPARE_FORMAT);  // 2.0 ms
  timer->setCaptureCompare(4, 1250, MICROSEC_COMPARE_FORMAT);  // 1.25 ms

  // 4. Enable channel outputs (CCxE bit - required for PWM generation)
  timer->resumeChannel(1);
  timer->resumeChannel(2);
  timer->resumeChannel(3);
  timer->resumeChannel(4);

  // 5. Start timer counter
  timer->resume();
}
```

### Runtime Pulse Width Updates

```cpp
HardwareTimer *servo_timer;

void setup() {
  servo_timer = new HardwareTimer(TIM3);
  // ... configure as above ...
  servo_timer->resume();
}

void loop() {
  // Update CH1 pulse width: 1000-2000 µs range
  for (uint32_t pulse_us = 1000; pulse_us <= 2000; pulse_us += 10) {
    servo_timer->setCaptureCompare(1, pulse_us, MICROSEC_COMPARE_FORMAT);
    delay(20);
  }
}
```

**Note**: `setCaptureCompare()` updates immediately if preload is disabled, or on next overflow if enabled.

### Servo Control Pattern

```cpp
class ServoChannel {
public:
  ServoChannel(HardwareTimer *timer, uint32_t channel, PinName pin)
    : _timer(timer), _channel(channel) {
    _timer->setMode(channel, TIMER_OUTPUT_COMPARE_PWM1, pin);
  }

  void writeMicroseconds(uint32_t pulse_us) {
    pulse_us = constrain(pulse_us, 1000, 2000);  // Safety limits
    _timer->setCaptureCompare(_channel, pulse_us, MICROSEC_COMPARE_FORMAT);
  }

  void write(uint8_t angle) {
    uint32_t pulse_us = map(angle, 0, 180, 1000, 2000);
    writeMicroseconds(pulse_us);
  }

private:
  HardwareTimer *_timer;
  uint32_t _channel;
};

// Usage
HardwareTimer *timer;
ServoChannel servo1, servo2;

void setup() {
  timer = new HardwareTimer(TIM3);
  timer->setPrescaleFactor((timer->getTimerClkFreq() / 1'000'000) - 1);
  timer->setOverflow(20000, MICROSEC_FORMAT);  // 20 ms = 50 Hz

  servo1 = ServoChannel(timer, 1, PA_6);  // TIM3_CH1
  servo2 = ServoChannel(timer, 2, PA_7);  // TIM3_CH2

  timer->resume();
}

void loop() {
  servo1.write(90);   // Center position
  servo2.write(180);  // Max position
}
```

## Interrupt Callbacks

### Period (Overflow) Callback

Triggered when timer counter overflows (reaches ARR and wraps to 0).

```cpp
void overflow_callback() {
  // Called every timer period
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void setup() {
  HardwareTimer *timer = new HardwareTimer(TIM3);
  timer->setOverflow(1000, HERTZ_FORMAT);  // 1 kHz overflow rate
  timer->attachInterrupt(overflow_callback);
  timer->resume();
}
```

### Capture/Compare Callback

Triggered when counter matches compare register (CCRx).

```cpp
void compare_callback() {
  // Called when counter == CCRx
  // In PWM mode: rising or falling edge depending on PWM mode
}

void setup() {
  HardwareTimer *timer = new HardwareTimer(TIM3);
  timer->setOverflow(1000, MICROSEC_FORMAT);
  timer->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_6);
  timer->setCaptureCompare(1, 500, MICROSEC_COMPARE_FORMAT);

  timer->attachInterrupt(compare_callback);           // Overflow callback
  timer->attachInterrupt(1, compare_callback);        // CH1 compare callback

  timer->resume();
}
```

**Warning**: Callbacks execute in interrupt context. Keep them short and avoid:
- `delay()`
- `Serial.print()`
- Long computations
- Blocking operations

## HAL Layer Integration

### Data Structures

**timerObj_t** (cores/arduino/stm32/timer.h:270-276):
```c
typedef struct {
  void *__this;                  // Pointer to HardwareTimer C++ object
  TIM_HandleTypeDef handle;      // HAL timer handle
  uint32_t preemptPriority;      // NVIC preempt priority
  uint32_t subPriority;          // NVIC sub-priority
} timerObj_t;
```

**TIM_Base_InitTypeDef** (HAL):
```c
typedef struct {
  uint32_t Prescaler;            // PSC register (0-65535)
  uint32_t CounterMode;          // UP, DOWN, CENTER_ALIGNED
  uint32_t Period;               // ARR register (0-65535 or 0-4294967295)
  uint32_t ClockDivision;        // DTS divider (1, 2, or 4)
  uint32_t RepetitionCounter;    // RCR register (advanced timers only)
  uint32_t AutoReloadPreload;    // Buffer ARR updates
} TIM_Base_InitTypeDef;
```

**TIM_OC_InitTypeDef** (HAL):
```c
typedef struct {
  uint32_t OCMode;               // PWM1, PWM2, TOGGLE, ACTIVE, INACTIVE, TIMING
  uint32_t Pulse;                // CCRx register value
  uint32_t OCPolarity;           // Active high/low
  uint32_t OCFastMode;           // Fast compare mode enable
  uint32_t OCIdleState;          // Output during idle (advanced timers)
  uint32_t OCNPolarity;          // Complementary output polarity
  uint32_t OCNIdleState;         // Complementary output during idle
} TIM_OC_InitTypeDef;
```

### HAL Functions Called by HardwareTimer

```cpp
// Initialization
HAL_TIM_Base_Init(&_timerObj.handle);                    // Setup:129
HAL_TIM_PWM_ConfigChannel(&handle, &channelOC, channel); // setMode:694

// Start/Stop
HAL_TIM_Base_Stop(&handle);                              // pause:147
HAL_TIM_PWM_Start(&handle, channel);                     // resume (internal)

// Register access helpers
__HAL_TIM_SET_PRESCALER(&handle, prescaler);
__HAL_TIM_SET_AUTORELOAD(&handle, period);
__HAL_TIM_SET_COMPARE(&handle, channel, pulse);
__HAL_TIM_GET_COMPARE(&handle, channel);
```

### Low-Layer (LL) Macros

Used for performance-critical operations:

```cpp
LL_TIM_EnableCounter(TIMx);              // TIMx->CR1 |= TIM_CR1_CEN
LL_TIM_DisableCounter(TIMx);             // TIMx->CR1 &= ~TIM_CR1_CEN
LL_TIM_CC_EnableChannel(TIMx, channel);  // TIMx->CCER |= channel_mask
LL_TIM_CC_DisableChannel(TIMx, channel); // TIMx->CCER &= ~channel_mask
LL_TIM_GenerateEvent_UPDATE(TIMx);       // TIMx->EGR |= TIM_EGR_UG
```

## Register-Level Details

### Key Timer Registers

| Register | Name | Purpose |
|----------|------|---------|
| CR1 | Control Register 1 | Enable, direction, one-pulse, auto-reload preload |
| PSC | Prescaler | Divides timer clock: `f_cnt = f_clk / (PSC + 1)` |
| ARR | Auto-Reload Register | Period value (counter resets when CNT == ARR) |
| CNT | Counter | Current timer value |
| CCRx | Capture/Compare x | Compare value for channel x (duty cycle) |
| CCMRx | Capture/Compare Mode x | Channel mode (PWM1, PWM2, input capture, etc.) |
| CCER | Capture/Compare Enable | Enable channels and set polarity |
| DIER | DMA/Interrupt Enable | Enable update and CC interrupts |
| SR | Status Register | Overflow and CC flags |

### PWM1 Mode Behavior

**TIM_OCMODE_PWM1** (most common):
- Output **HIGH** when `CNT < CCRx`
- Output **LOW** when `CNT >= CCRx`
- Duty cycle = `(CCRx / ARR) × 100%`

**TIM_OCMODE_PWM2** (inverted):
- Output **LOW** when `CNT < CCRx`
- Output **HIGH** when `CNT >= CCRx`

### Timer Clock Calculation Example (STM32F411)

```
System Clock (HCLK) = 100 MHz
  ├─ APB1 Prescaler = /4  →  APB1 = 25 MHz
  │    └─ Timer Clock Multiplier × 2  →  TIM2-7 Clock = 50 MHz
  └─ APB2 Prescaler = /2  →  APB2 = 50 MHz
       └─ Timer Clock Multiplier × 2  →  TIM1,9-11 Clock = 100 MHz

TIM3 (APB1 timer):
  Timer Clock = 50 MHz
  PSC = 49  →  Counter Clock = 50 MHz / (49 + 1) = 1 MHz
  ARR = 999  →  Period = 1 MHz / (999 + 1) = 1 kHz
  CCR1 = 500  →  Duty Cycle = 500 / 1000 = 50%
```

## Multi-Instance Timer Management

### Multiple Independent Timers

```cpp
HardwareTimer *tim3 = new HardwareTimer(TIM3);  // Servo outputs
HardwareTimer *tim5 = new HardwareTimer(TIM5);  // ESC outputs

void setup() {
  // TIM3: 4 servos @ 50 Hz (20 ms period)
  tim3->setPrescaleFactor((tim3->getTimerClkFreq() / 1'000'000) - 1);
  tim3->setOverflow(20000, MICROSEC_FORMAT);
  tim3->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_6);  // Servo 1
  tim3->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, PA_7);  // Servo 2
  tim3->resume();

  // TIM5: 4 ESCs @ 400 Hz (2.5 ms period)
  tim5->setPrescaleFactor((tim5->getTimerClkFreq() / 1'000'000) - 1);
  tim5->setOverflow(2500, MICROSEC_FORMAT);
  tim5->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_0);  // ESC 1
  tim5->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, PA_1);  // ESC 2
  tim5->resume();
}
```

### Global Timer Registry

```cpp
// cores/arduino/HardwareTimer.cpp:36
timerObj_t *HardwareTimer_Handle[TIMER_NUM] = {NULL};

// Populated during HardwareTimer::setup():75-85
HardwareTimer_Handle[index] = &_timerObj;
```

Used by interrupt handlers to route callbacks to correct instance.

## Best Practices

### 1. Use Dynamic Allocation

```cpp
// ✅ GOOD: Persists after setup() returns
HardwareTimer *timer = new HardwareTimer(TIM3);

// ❌ BAD: Destroyed when setup() exits
HardwareTimer timer(TIM3);
```

### 2. Verify Pin Capability

```cpp
PinName pin_name = digitalPinToPinName(pin);
TIM_TypeDef *instance = (TIM_TypeDef *)pinmap_peripheral(pin_name, PinMap_PWM);
if (instance == NP) {
  Serial.println("Pin does not support PWM!");
  return;
}
```

### 3. No pinMode() Needed

```cpp
// ❌ UNNECESSARY: HardwareTimer configures GPIO automatically
pinMode(PA_6, OUTPUT);

// ✅ GOOD: Just use setMode() or setPWM()
timer->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_6);
```

### 4. Call refresh() After Multiple Updates

```cpp
// Update multiple settings
timer->setPrescaleFactor(99);
timer->setOverflow(999, TICK_FORMAT);
timer->setCaptureCompare(1, 500, TICK_COMPARE_FORMAT);

// Force immediate update (generate UG event)
timer->refresh();
```

Without `refresh()`, updates may not take effect until next overflow if preload is enabled.

### 5. Interrupt Priority

```cpp
// Lower number = higher priority
timer->setInterruptPriority(5, 0);  // Preempt=5, Sub=0
timer->attachInterrupt(callback);
timer->resume();
```

Default: `TIM_IRQ_PRIO = 14` (low priority)

### 6. Pause Before Reconfiguration

```cpp
timer->pause();
timer->setPrescaleFactor(new_prescaler);
timer->setOverflow(new_period, TICK_FORMAT);
timer->refresh();
timer->resume();
```

## Known Limitations

1. **16-bit timers**: Most timers are 16-bit (ARR max = 65535). TIM2 and TIM5 are 32-bit on some STM32 families.
2. **Channel count**: Standard timers have 4 channels. Some advanced timers have complementary outputs.
3. **Pin conflicts**: Some pins map to multiple timers. Check `PinMap_TIM[]` for alternatives.
4. **DMA not exposed**: HardwareTimer does not expose DMA functionality (HAL supports it).
5. **Advanced features**: Break inputs, dead-time, and complementary outputs require direct HAL access.

## Reference Examples

### Official STM32Examples Repository

- [PWM_FullConfiguration](https://github.com/stm32duino/STM32Examples/blob/main/examples/Peripherals/HardwareTimer/PWM_FullConfiguration/PWM_FullConfiguration.ino)
- [All-in-one_setPWM](https://github.com/stm32duino/STM32Examples/blob/main/examples/Peripherals/HardwareTimer/All-in-one_setPWM/All-in-one_setPWM.ino)

### Documentation

- [HardwareTimer Wiki](https://github.com/stm32duino/Arduino_Core_STM32/wiki/HardwareTimer-library)
- STM32F4 Reference Manual RM0383 (Chapter 13-18: Timers)
- HAL Driver Documentation (stm32f4xx_hal_tim.h)

## Comparison: UVOS vs Arduino Core

| Aspect | UVOS | Arduino Core |
|--------|------|--------------|
| **API Style** | Direct HAL + custom types | C++ class wrapper |
| **Initialization** | Manual struct setup | Constructor + methods |
| **Pin Config** | Explicit GPIO + AF | Automatic via PinMap |
| **Timer Selection** | Enum (TIM_3) | Pointer (TIM3) |
| **Multi-Channel** | Array-based config | Per-channel methods |
| **Memory** | Static allocation | Dynamic (`new`) |
| **Flexibility** | Full HAL access | Simplified common use cases |
| **Learning Curve** | Steeper | Arduino-friendly |

Both approaches use the same underlying HAL layer and achieve identical hardware configuration.
