# STM32 Hardware Timer PWM Output Guide

Practical guide for configuring STM32 hardware timers for servo and ESC control using the Arduino Core.

## Overview

This guide focuses on using STM32 hardware timers to drive **1-4 PWM output channels** for:
- **Servo control**: 200 Hz with 1000-2000 µs pulses (1 µs resolution)
- **ESC control**: OneShot125 protocol (125-250 µs pulses at up to 1-2 kHz)

## RCC Clock to PWM Output Signal Chain

```
┌─────────────────────────────────────────────────────────┐
│ 1. System Clock (RCC Configuration)                     │
│    STM32F411 @ 100 MHz                                  │
│    • APB1 Prescaler /4 → PCLK1 = 25 MHz                 │
│    • Timer Clock Multiplier ×2 (when APB prescaler > 1) │
│    → TIM3 Clock = 50 MHz                                │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────┐
│ 2. Timer Prescaler Register (PSC)                       │
│    Divides timer clock to set counter tick frequency    │
│                                                         │
│    f_counter = f_timer_clock / (PSC + 1)                │
│                                                         │
│    Example: 50 MHz / 50 = 1 MHz (1 µs ticks)            │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────┐
│ 3. Auto-Reload Register (ARR)                           │
│    Sets PWM period (frequency)                          │
│                                                         │
│    f_pwm = f_counter / (ARR + 1)                        │
│                                                         │
│    Example: 1 MHz / 5000 = 200 Hz (5 ms period)         │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────┐
│ 4. Capture/Compare Register (CCRx)                      │
│    Sets pulse width (per channel)                       │
│                                                         │
│    pulse_width_us = CCRx / f_counter * 1,000,000        │
│                                                         │
│    Example: CCR1 = 1500 → 1500 µs pulse                 │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────┐
│ 5. GPIO Pin (via PinMap_PWM)                            │
│    Hardware PWM output (no CPU load)                    │
│                                                         │
│    200 Hz square wave, 1500 µs high, 3500 µs low        │
└─────────────────────────────────────────────────────────┘
```

## Critical RCC Clock Rule

**STM32 Timer Clock Multiplier**:
- If APB prescaler = 1 → Timer Clock = APB Clock (1× multiplier)
- If APB prescaler > 1 → Timer Clock = 2 × APB Clock (2× multiplier)

**Example for STM32F411RE @ 100 MHz** (typical Arduino configuration):
```
SYSCLK = 100 MHz
↓
AHB Prescaler /1 → HCLK = 100 MHz
↓
APB1 Prescaler /4 → PCLK1 = 25 MHz
↓
Timer Multiplier ×2 (because /4 > 1) → TIM3 Clock = 50 MHz ✓
```

**Always query dynamically**:
```cpp
uint32_t timer_clock = mytimer->getTimerClkFreq();  // Returns 50000000
```

## Timer Hardware Selection

### APB1 Timers (50 MHz on F411 @ 100 MHz)
- **TIM2, TIM3, TIM4, TIM5** - General purpose, 4 channels each
- **TIM5** - 32-bit timer (extended range, good for slow PWM)

### APB2 Timers (100 MHz on F411 @ 100 MHz)
- **TIM1** - Advanced timer, 4 channels + complementary outputs
- **TIM9, TIM10, TIM11** - 16-bit timers, 1-2 channels

**Recommendation**: Use **TIM3** or **TIM5** for servo/ESC (APB1, 4 channels, well-documented pin mappings)

## Servo Control Configuration (200 Hz, 1000-2000 µs)

### Specifications
- **Frequency**: 200 Hz (5 ms period)
- **Pulse Range**: 1000-2000 µs (1 ms - 2 ms)
- **Resolution**: 1 µs
- **Channels**: 1-4 servos per timer

### Timer Configuration

**Step 1: Calculate Prescaler for 1 µs Resolution**
```cpp
HardwareTimer *servo_timer = new HardwareTimer(TIM3);

uint32_t f_timer_clock = servo_timer->getTimerClkFreq();  // 50 MHz
uint32_t f_counter_target = 1'000'000;                     // 1 MHz = 1 µs ticks
uint32_t PSC = (f_timer_clock / f_counter_target) - 1;    // (50M / 1M) - 1 = 49

servo_timer->setPrescaleFactor(PSC);
```

**Step 2: Set Period for 200 Hz (5000 µs)**
```cpp
uint32_t period_us = 5000;  // 5 ms = 200 Hz
servo_timer->setOverflow(period_us, MICROSEC_FORMAT);  // ARR = 4999
```

**Step 3: Configure Channels and Set Pulse Widths**
```cpp
// Channel 1: PA_6 (TIM3_CH1) - 1500 µs (center position)
servo_timer->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_6);
servo_timer->setCaptureCompare(1, 1500, MICROSEC_COMPARE_FORMAT);
servo_timer->resumeChannel(1);  // Enable channel output (CCxE bit)

// Channel 2: PA_7 (TIM3_CH2) - 1000 µs (min position)
servo_timer->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, PA_7);
servo_timer->setCaptureCompare(2, 1000, MICROSEC_COMPARE_FORMAT);
servo_timer->resumeChannel(2);

// Channel 3: PB_0 (TIM3_CH3) - 2000 µs (max position)
servo_timer->setMode(3, TIMER_OUTPUT_COMPARE_PWM1, PB_0);
servo_timer->setCaptureCompare(3, 2000, MICROSEC_COMPARE_FORMAT);
servo_timer->resumeChannel(3);

// Channel 4: PB_1 (TIM3_CH4) - 1750 µs
servo_timer->setMode(4, TIMER_OUTPUT_COMPARE_PWM1, PB_1);
servo_timer->setCaptureCompare(4, 1750, MICROSEC_COMPARE_FORMAT);
servo_timer->resumeChannel(4);
```

**Step 4: Start PWM Output**
```cpp
servo_timer->resume();  // Start timer counter
```

### Runtime Pulse Width Updates

```cpp
void loop() {
  // Sweep servo 1 from 1000 to 2000 µs
  for (uint32_t pulse_us = 1000; pulse_us <= 2000; pulse_us += 10) {
    servo_timer->setCaptureCompare(1, pulse_us, MICROSEC_COMPARE_FORMAT);
    delay(20);  // 50 Hz update rate (5 ms servo period = 200 Hz)
  }
}
```

### Complete Servo Example

```cpp
HardwareTimer *servo_timer;

void setup() {
  servo_timer = new HardwareTimer(TIM3);

  // Configure for 1 µs resolution
  uint32_t timer_clk = servo_timer->getTimerClkFreq();
  servo_timer->setPrescaleFactor((timer_clk / 1'000'000) - 1);

  // Set 200 Hz (5 ms period)
  servo_timer->setOverflow(5000, MICROSEC_FORMAT);

  // Configure 4 servo channels (TIM3_CH1-4)
  servo_timer->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_6);
  servo_timer->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, PA_7);
  servo_timer->setMode(3, TIMER_OUTPUT_COMPARE_PWM1, PB_0);
  servo_timer->setMode(4, TIMER_OUTPUT_COMPARE_PWM1, PB_1);

  // Set initial positions (all center) and enable outputs
  for (int ch = 1; ch <= 4; ch++) {
    servo_timer->setCaptureCompare(ch, 1500, MICROSEC_COMPARE_FORMAT);
    servo_timer->resumeChannel(ch);  // Enable channel output
  }

  servo_timer->resume();  // Start timer counter
}

void loop() {
  // Independent servo control
  servo_timer->setCaptureCompare(1, 1000, MICROSEC_COMPARE_FORMAT);  // Min
  servo_timer->setCaptureCompare(2, 1500, MICROSEC_COMPARE_FORMAT);  // Center
  servo_timer->setCaptureCompare(3, 2000, MICROSEC_COMPARE_FORMAT);  // Max
  servo_timer->setCaptureCompare(4, 1250, MICROSEC_COMPARE_FORMAT);  // Custom

  delay(1000);
}
```

## ESC OneShot125 Configuration (125-250 µs @ 1-2 kHz)

### OneShot125 Specifications
- **Pulse Range**: 125-250 µs (8× faster than standard PWM)
- **Throttle Mapping**: 125 µs = 0%, 250 µs = 100%
- **Max Update Rate**: 1-2 kHz (practical), up to 490 Hz (conservative)
- **Resolution**: 1 µs (125 discrete throttle steps)

### Timer Configuration for 1 kHz Update Rate

**Step 1: Calculate Prescaler for 1 µs Resolution**
```cpp
HardwareTimer *esc_timer = new HardwareTimer(TIM5);  // Use TIM5 (APB1)

uint32_t f_timer_clock = esc_timer->getTimerClkFreq();  // 50 MHz
uint32_t PSC = (f_timer_clock / 1'000'000) - 1;         // 49
esc_timer->setPrescaleFactor(PSC);
```

**Step 2: Set Period for 1 kHz (1000 µs)**
```cpp
uint32_t period_us = 1000;  // 1 ms = 1 kHz
esc_timer->setOverflow(period_us, MICROSEC_FORMAT);  // ARR = 999
```

**Step 3: Configure ESC Channels (OneShot125 Range)**
```cpp
// Channel 1: PA_0 (TIM5_CH1) - Motor 1
esc_timer->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_0);
esc_timer->setCaptureCompare(1, 125, MICROSEC_COMPARE_FORMAT);  // 0% throttle
esc_timer->resumeChannel(1);  // Enable channel output

// Channel 2: PA_1 (TIM5_CH2) - Motor 2
esc_timer->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, PA_1);
esc_timer->setCaptureCompare(2, 187, MICROSEC_COMPARE_FORMAT);  // 50% throttle
esc_timer->resumeChannel(2);

// Channel 3: PA_2 (TIM5_CH3) - Motor 3
esc_timer->setMode(3, TIMER_OUTPUT_COMPARE_PWM1, PA_2);
esc_timer->setCaptureCompare(3, 250, MICROSEC_COMPARE_FORMAT);  // 100% throttle
esc_timer->resumeChannel(3);

// Channel 4: PA_3 (TIM5_CH4) - Motor 4
esc_timer->setMode(4, TIMER_OUTPUT_COMPARE_PWM1, PA_3);
esc_timer->setCaptureCompare(4, 156, MICROSEC_COMPARE_FORMAT);  // 25% throttle
esc_timer->resumeChannel(4);
```

**Step 4: Start PWM**
```cpp
esc_timer->resume();  // Start timer counter
```

### Throttle Conversion Helper

```cpp
// Convert throttle percentage (0-100) to OneShot125 pulse width
uint32_t throttleToOneShot125(uint8_t throttle_percent) {
  if (throttle_percent > 100) throttle_percent = 100;
  return 125 + (throttle_percent * 125) / 100;  // 125-250 µs range
}

// Usage:
esc_timer->setCaptureCompare(1, throttleToOneShot125(0),   MICROSEC_COMPARE_FORMAT);  // 125 µs
esc_timer->setCaptureCompare(2, throttleToOneShot125(50),  MICROSEC_COMPARE_FORMAT);  // 187 µs
esc_timer->setCaptureCompare(3, throttleToOneShot125(100), MICROSEC_COMPARE_FORMAT);  // 250 µs
```

### Complete OneShot125 Example

```cpp
HardwareTimer *esc_timer;

uint32_t throttleToOneShot125(uint8_t throttle_percent) {
  if (throttle_percent > 100) throttle_percent = 100;
  return 125 + (throttle_percent * 125) / 100;
}

void setup() {
  esc_timer = new HardwareTimer(TIM5);

  // Configure for 1 µs resolution
  uint32_t timer_clk = esc_timer->getTimerClkFreq();
  esc_timer->setPrescaleFactor((timer_clk / 1'000'000) - 1);

  // Set 1 kHz update rate (1 ms period)
  esc_timer->setOverflow(1000, MICROSEC_FORMAT);

  // Configure 4 ESC channels (TIM5_CH1-4)
  esc_timer->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_0);
  esc_timer->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, PA_1);
  esc_timer->setMode(3, TIMER_OUTPUT_COMPARE_PWM1, PA_2);
  esc_timer->setMode(4, TIMER_OUTPUT_COMPARE_PWM1, PA_3);

  // Initialize all ESCs to 0% throttle (125 µs) and enable outputs
  for (int ch = 1; ch <= 4; ch++) {
    esc_timer->setCaptureCompare(ch, throttleToOneShot125(0), MICROSEC_COMPARE_FORMAT);
    esc_timer->resumeChannel(ch);  // Enable channel output
  }

  esc_timer->resume();  // Start timer counter

  // ESC calibration sequence (if needed)
  delay(1000);  // Wait for ESCs to initialize
}

void loop() {
  // Quadcopter motor mixing example
  uint8_t throttle = 50;  // Base throttle

  // Front-left motor
  esc_timer->setCaptureCompare(1, throttleToOneShot125(throttle + 10), MICROSEC_COMPARE_FORMAT);

  // Front-right motor
  esc_timer->setCaptureCompare(2, throttleToOneShot125(throttle + 5), MICROSEC_COMPARE_FORMAT);

  // Rear-left motor
  esc_timer->setCaptureCompare(3, throttleToOneShot125(throttle - 5), MICROSEC_COMPARE_FORMAT);

  // Rear-right motor
  esc_timer->setCaptureCompare(4, throttleToOneShot125(throttle - 10), MICROSEC_COMPARE_FORMAT);

  delay(1);  // 1 kHz update rate
}
```

### Alternative: 2 kHz Update Rate (Maximum Performance)

For faster ESC response, increase to 2 kHz:

```cpp
// Period: 500 µs (2 kHz)
esc_timer->setOverflow(500, MICROSEC_FORMAT);

// Throttle range still 125-250 µs (within 500 µs period)
esc_timer->setCaptureCompare(1, throttleToOneShot125(75), MICROSEC_COMPARE_FORMAT);

// Update loop can run at 2 kHz
void loop() {
  // Update throttle
  esc_timer->setCaptureCompare(1, throttleToOneShot125(new_throttle), MICROSEC_COMPARE_FORMAT);
  delayMicroseconds(500);  // 2 kHz
}
```

**Note**: Verify ESC supports 2 kHz update rate (most OneShot125 ESCs support 1-2 kHz).

## Pin Mapping Reference (STM32F411RE Nucleo)

### TIM3 Channels (APB1, 50 MHz timer clock)
| Pin  | Timer | Channel | Alternate Function | Common Use |
|------|-------|---------|-------------------|------------|
| PA_6 | TIM3  | CH1     | GPIO_AF2_TIM3     | Servo 1    |
| PA_7 | TIM3  | CH2     | GPIO_AF2_TIM3     | Servo 2    |
| PB_0 | TIM3  | CH3     | GPIO_AF2_TIM3     | Servo 3    |
| PB_1 | TIM3  | CH4     | GPIO_AF2_TIM3     | Servo 4    |

### TIM5 Channels (APB1, 50 MHz timer clock)
| Pin  | Timer | Channel | Alternate Function | Common Use |
|------|-------|---------|-------------------|------------|
| PA_0 | TIM5  | CH1     | GPIO_AF2_TIM5     | ESC 1      |
| PA_1 | TIM5  | CH2     | GPIO_AF2_TIM5     | ESC 2      |
| PA_2 | TIM5  | CH3     | GPIO_AF2_TIM5     | ESC 3      |
| PA_3 | TIM5  | CH4     | GPIO_AF2_TIM5     | ESC 4      |

**Automatic Pin Configuration**: HardwareTimer configures GPIO alternate function automatically - no `pinMode()` needed.

## Key Register Calculations

### Prescaler (PSC) - Sets Counter Tick Frequency
```cpp
f_counter = f_timer_clock / (PSC + 1)

// For 1 µs resolution:
PSC = (f_timer_clock / 1'000'000) - 1
// Example: (50 MHz / 1 MHz) - 1 = 49
```

### Auto-Reload (ARR) - Sets PWM Frequency
```cpp
f_pwm = f_counter / (ARR + 1)

// For 200 Hz @ 1 MHz counter:
ARR = (f_counter / f_pwm) - 1
// Example: (1 MHz / 200 Hz) - 1 = 4999
```

### Capture/Compare (CCRx) - Sets Pulse Width
```cpp
pulse_width_us = (CCRx / f_counter) × 1,000,000

// For 1500 µs @ 1 MHz counter:
CCRx = 1500
// Pulse width = (1500 / 1,000,000) × 1,000,000 = 1500 µs
```

## Configuration Summary Tables

### Servo Control (200 Hz)
| Parameter | Value | Calculation |
|-----------|-------|-------------|
| Timer Clock | 50 MHz | From RCC (APB1 ×2 multiplier) |
| PSC | 49 | (50M / 1M) - 1 |
| Counter Freq | 1 MHz | 50M / 50 = 1 µs ticks |
| ARR | 4999 | (1M / 200) - 1 |
| PWM Freq | 200 Hz | 1M / 5000 = 5 ms period |
| CCR Range | 1000-2000 | 1000-2000 µs pulses |
| Duty Cycle | 20-40% | At 200 Hz |

### OneShot125 ESC (1 kHz)
| Parameter | Value | Calculation |
|-----------|-------|-------------|
| Timer Clock | 50 MHz | From RCC (APB1 ×2 multiplier) |
| PSC | 49 | (50M / 1M) - 1 |
| Counter Freq | 1 MHz | 50M / 50 = 1 µs ticks |
| ARR | 999 | (1M / 1k) - 1 |
| PWM Freq | 1 kHz | 1M / 1000 = 1 ms period |
| CCR Range | 125-250 | OneShot125 spec |
| Duty Cycle | 12.5-25% | At 1 kHz |

### OneShot125 ESC (2 kHz - Maximum)
| Parameter | Value | Calculation |
|-----------|-------|-------------|
| Timer Clock | 50 MHz | From RCC (APB1 ×2 multiplier) |
| PSC | 49 | (50M / 1M) - 1 |
| Counter Freq | 1 MHz | 50M / 50 = 1 µs ticks |
| ARR | 499 | (1M / 2k) - 1 |
| PWM Freq | 2 kHz | 1M / 500 = 500 µs period |
| CCR Range | 125-250 | OneShot125 spec |
| Duty Cycle | 25-50% | At 2 kHz |

## HardwareTimer API Quick Reference

### Initialization
```cpp
HardwareTimer *timer = new HardwareTimer(TIM3);  // Must use 'new'
```

### Clock Configuration
```cpp
uint32_t timer_clk = timer->getTimerClkFreq();  // Query RCC
timer->setPrescaleFactor(PSC);                   // 0-65535
```

### Period (Frequency) Configuration
```cpp
timer->setOverflow(value, TICK_FORMAT);          // Raw ticks
timer->setOverflow(value, MICROSEC_FORMAT);      // Microseconds
timer->setOverflow(value, HERTZ_FORMAT);         // Frequency in Hz
```

### Channel Configuration
```cpp
timer->setMode(channel, TIMER_OUTPUT_COMPARE_PWM1, pin);  // PWM mode, high when CNT < CCR
timer->setMode(channel, TIMER_OUTPUT_COMPARE_PWM2, pin);  // Inverted PWM
```

### Pulse Width (Duty Cycle) Configuration
```cpp
timer->setCaptureCompare(ch, value, TICK_COMPARE_FORMAT);       // Raw ticks
timer->setCaptureCompare(ch, value, MICROSEC_COMPARE_FORMAT);   // Microseconds
timer->setCaptureCompare(ch, value, PERCENT_COMPARE_FORMAT);    // 0-100%
```

### Timer Control
```cpp
timer->resume();       // Start all channels
timer->pause();        // Stop all channels
timer->refresh();      // Force register update
```

## Best Practices

### 1. Enable Channels for Manual PWM Configuration
```cpp
// ✅ GOOD: Manual configuration with channel enable
timer->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_6);
timer->setCaptureCompare(1, 1500, MICROSEC_COMPARE_FORMAT);
timer->resumeChannel(1);  // Enable channel output (CCxE bit)
timer->resume();          // Start timer counter

// ❌ BAD: Missing channel enable - no PWM output!
timer->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_6);
timer->setCaptureCompare(1, 1500, MICROSEC_COMPARE_FORMAT);
timer->resume();  // Counter runs but channel output disabled

// ✅ GOOD: setPWM() handles both automatically
timer->setPWM(1, PA_6, 200, 50);  // Calls resumeChannel() internally
```

**Note**: Per STM32 timer documentation (AN4013 Section 2.5), PWM generation requires enabling the capture/compare output (CCxE bit) via `resumeChannel()`. The all-in-one `setPWM()` method handles this automatically, but manual configuration requires explicit `resumeChannel()` call.

### 2. Always Use Dynamic Allocation
```cpp
// ✅ GOOD: Persists after setup()
HardwareTimer *timer = new HardwareTimer(TIM3);

// ❌ BAD: Destroyed when setup() exits
HardwareTimer timer(TIM3);
```

### 3. Query Timer Clock Dynamically
```cpp
// ✅ GOOD: Portable across boards/configs
uint32_t timer_clk = timer->getTimerClkFreq();
uint32_t PSC = (timer_clk / 1'000'000) - 1;

// ❌ BAD: Hardcoded assumption
uint32_t PSC = 49;  // Only works for 50 MHz timer clock
```

### 4. Use Microsecond Format for Clarity
```cpp
// ✅ GOOD: Clear intent
timer->setOverflow(5000, MICROSEC_FORMAT);           // 5 ms period
timer->setCaptureCompare(1, 1500, MICROSEC_COMPARE_FORMAT);  // 1.5 ms pulse

// ❌ Less Clear: Requires mental calculation
timer->setOverflow(4999, TICK_FORMAT);
timer->setCaptureCompare(1, 1499, TICK_COMPARE_FORMAT);
```

### 5. No pinMode() Needed
```cpp
// ✅ GOOD: HardwareTimer handles GPIO config
timer->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_6);

// ❌ UNNECESSARY
pinMode(PA_6, OUTPUT);
timer->setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA_6);
```

### 6. Update During Pause for Major Changes
```cpp
// ✅ GOOD: Safe reconfiguration
timer->pause();
timer->setPrescaleFactor(new_PSC);
timer->setOverflow(new_period, MICROSEC_FORMAT);
timer->refresh();
timer->resume();

// ⚠️ OKAY: Runtime CCR updates (pulse width only)
timer->setCaptureCompare(1, new_pulse, MICROSEC_COMPARE_FORMAT);
```

### 7. Constrain User Inputs
```cpp
// ✅ GOOD: Safety limits
uint32_t servo_pulse_us = constrain(user_input, 1000, 2000);
timer->setCaptureCompare(ch, servo_pulse_us, MICROSEC_COMPARE_FORMAT);

uint8_t throttle = constrain(user_throttle, 0, 100);
timer->setCaptureCompare(ch, throttleToOneShot125(throttle), MICROSEC_COMPARE_FORMAT);
```

## Common Pitfalls

### Pitfall 1: Forgetting PSC is Zero-Indexed
```cpp
// ❌ WRONG: Off-by-one error
PSC = timer_clk / 1'000'000;  // Results in 1.02 MHz, not 1 MHz

// ✅ CORRECT
PSC = (timer_clk / 1'000'000) - 1;
```

### Pitfall 2: ARR Off-by-One
```cpp
// ❌ WRONG: 199.96 Hz instead of 200 Hz
timer->setOverflow(5000, TICK_FORMAT);  // Period = 5001 ticks

// ✅ CORRECT: Let API handle it
timer->setOverflow(5000, MICROSEC_FORMAT);  // ARR set to 4999 automatically
```

### Pitfall 3: Exceeding 16-bit Timer Limits
```cpp
// ❌ WRONG: Overflow on 16-bit timer
uint32_t period_us = 100000;  // 100 ms needs ARR = 99,999 (> 65535)

// ✅ CORRECT: Use TIM5 (32-bit) or increase prescaler
HardwareTimer *timer = new HardwareTimer(TIM5);  // 32-bit timer
```

### Pitfall 4: Ignoring RCC Multiplier
```cpp
// ❌ WRONG: Assumes APB clock = timer clock
uint32_t timer_clk = 25000000;  // Hardcoded APB1 freq

// ✅ CORRECT: Account for 2× multiplier
uint32_t timer_clk = timer->getTimerClkFreq();  // Returns 50 MHz, not 25 MHz
```

## Verification Techniques

### Method 1: Serial Debug Output
```cpp
Serial.print("Timer Clock: "); Serial.println(timer->getTimerClkFreq());
Serial.print("PSC: "); Serial.println(timer->getPrescaleFactor());
Serial.print("ARR: "); Serial.println(timer->getOverflow(TICK_FORMAT));
Serial.print("CCR1: "); Serial.println(timer->getCaptureCompare(1, TICK_COMPARE_FORMAT));

// Calculate expected values
uint32_t f_counter = timer->getTimerClkFreq() / (timer->getPrescaleFactor() + 1);
uint32_t f_pwm = f_counter / (timer->getOverflow(TICK_FORMAT) + 1);
Serial.print("Counter Freq: "); Serial.println(f_counter);
Serial.print("PWM Freq: "); Serial.println(f_pwm);
```

### Method 2: Oscilloscope Measurement
- Measure PWM frequency (should match configured value)
- Measure pulse width (should match CCR value in µs)
- Verify rise/fall times (should be clean square wave)

### Method 3: Logic Analyzer
- Capture all 4 channels simultaneously
- Verify phase relationships (should all be synchronized)
- Measure jitter (should be < 1 µs for hardware PWM)

## Related Documentation

- **TIMERS.md**: Comprehensive technical reference covering all timer features
- **Reference Manual RM0383**: STM32F411 hardware documentation (Chapter 13-18: Timers)
- **Arduino Core Source**: `cores/arduino/HardwareTimer.{h,cpp}`
- **HAL Driver**: `system/Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h`
