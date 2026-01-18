# TimerPWM Library - Design Approach

This document captures the research, design rationale, and technical decisions behind the TimerPWM library.

---

## Project Goal

Create a hardware timer-based PWM library for STM32 Arduino that provides **1 µs resolution** for servo and ESC control on UAV flight controllers.

### Target Use Cases
- **Servo Control**: 50-200 Hz with 1000-2000 µs pulses
- **ESC Control**: 400 Hz with 1000-2000 µs pulses
- **OneShot125**: 1-2 kHz with 125-250 µs pulses (future)

---

## Research Summary

### Documentation Created
1. **`doc/TIMERS.md`** - Complete STM32 timer architecture reference
   - HardwareTimer → HAL → LL → Register stack
   - RCC clock tree and 2× multiplier rule
   - Pin mapping resolution

2. **`doc/TIMERS_PWM_OUT.md`** - Practical PWM output guide
   - Servo and OneShot125 configuration examples
   - Complete calculation chain (RCC → PSC → ARR → CCRx)
   - Working code examples with HardwareTimer

3. **UVOS TimerPWM Analysis** - Reference implementation study
   - Confirmed 1 MHz tick rate approach
   - Validated prescaler calculation
   - Array-based channel configuration pattern

4. **Arduino Library Survey** - Comparative analysis
   - Arduino Servo library (time-multiplexing pattern)
   - ServoHT library (hardware PWM per channel)
   - Betaflight/iNav flight controller libraries (resource ownership)
   - HardwareTimer API capabilities

### Key Technical Findings

#### 1. RCC Clock Rule (Critical)
**STM32 Timer Clock Multiplier**:
```
If APB prescaler = 1 → Timer Clock = APB Clock (1×)
If APB prescaler > 1 → Timer Clock = 2 × APB Clock (2×)
```

**STM32F411RE @ 100 MHz**:
```
SYSCLK = 100 MHz
→ AHB /1 → HCLK = 100 MHz
→ APB1 /4 → PCLK1 = 25 MHz
→ Timer ×2 (because /4 > 1) → TIM3 Clock = 50 MHz
```

**Always query dynamically**:
```cpp
uint32_t timer_clk = mytimer->getTimerClkFreq();  // Returns 50000000
```

#### 2. HardwareTimer Capabilities
Arduino STM32 Core provides:
- **MICROSEC_COMPARE_FORMAT**: Direct microsecond pulse width setting
- **setOverflow()** with **MICROSEC_FORMAT**: Easy frequency control
- **Auto pin configuration**: No manual alternate function setup
- **Dynamic prescaler calculation**: Built-in helpers

#### 3. Flight Controller Validation
Betaflight and iNav confirm:
- ✅ **Same timer = same frequency** (all channels share PSC/ARR)
- ✅ **Explicit timer banks** (resource ownership visibility)
- ✅ **Protocol isolation** (servos @ 50Hz, ESCs @ 400Hz on different timers)
- ✅ **1 MHz tick rate** (industry standard for servo/ESC)

---

## Design Decisions

### Architecture: Explicit Timer Bank Configuration

**Rationale**: Flight controllers require explicit timer resource management to prevent frequency conflicts.

**Pattern**:
```cpp
// In targets/*.h files
namespace BoardConfig {
  namespace Servo {
    static inline TIM_TypeDef* const timer = TIM3;
    static constexpr uint32_t frequency_hz = 50;

    struct Channel {
      uint32_t pin, ch, min_us, max_us;
    };

    static constexpr Channel pwm_output = {PB4, 1, 1000, 2000};
  };
}
```

**Why Explicit Configuration**:
1. ✅ **Prevents timer conflicts** - Visible at compile-time
2. ✅ **Shows frequency constraints** - All channels share same frequency
3. ✅ **Hardware resource visibility** - Designer sees allocation
4. ✅ **Config IS documentation** - Self-documenting hardware setup

### API Design: Hybrid Servo + HardwareTimer

Combines **Arduino Servo simplicity** with **HardwareTimer power**:

```cpp
class PWMOutputBank {
  bool Init(TIM_TypeDef *timer, uint32_t frequency_hz = 50);
  bool AttachChannel(uint32_t channel, uint32_t pin, uint32_t min_us, max_us);
  void SetPulseWidth(uint32_t channel, uint32_t pulse_us);
  void Write(uint32_t channel, int value);  // Servo-compatible (0-180° or µs)
  void Start();
  void Stop();
  uint32_t GetPulseWidth(uint32_t channel);
  bool IsInitialized() const;
};
```

**Key Decisions**:

| Aspect | Decision | Rationale |
|--------|----------|-----------|
| **Resolution** | 1 MHz (1 µs) | Industry standard, validated by UVOS/Betaflight |
| **API Style** | Servo-like + explicit timer | Familiar + clear resource ownership |
| **Pin Format** | Arduino uint32_t (PA0, PB1) | Matches BoardConfig pattern |
| **Prescaler** | Fixed 1 MHz | Simplicity, auto-calc from timer clock |
| **Frequency** | Init-time parameter | Servo (50Hz) vs ESC (400Hz) |
| **Multi-Timer** | Separate PWMOutputBank instances | Clear ownership, no conflicts |
| **Channel Limit** | 4 per timer | Hardware constraint |
| **Pulse Format** | Microseconds (uint32_t) | Direct, no conversion needed |

---

## Implementation Notes

### 1 MHz Prescaler Calculation
```cpp
uint32_t PWMOutputBank::Calculate1MHzPrescaler() {
  uint32_t timer_clk = timer_->getTimerClkFreq();  // Dynamic query
  uint32_t prescaler = (timer_clk / 1'000'000) - 1;  // 1 MHz tick rate
  return prescaler;
}
```

**Example** (STM32F411RE with TIM3):
- Timer clock: 50 MHz (from RCC)
- Target: 1 MHz (1 µs tick)
- Prescaler: (50,000,000 / 1,000,000) - 1 = **49**

### Frequency to Period Conversion
```cpp
void PWMOutputBank::Init(TIM_TypeDef *timer, uint32_t frequency_hz) {
  timer_ = new HardwareTimer(timer);
  uint32_t prescaler = Calculate1MHzPrescaler();
  timer_->setPrescaleFactor(prescaler);

  // Set period from frequency (at 1 MHz tick rate)
  uint32_t period_us = (1'000'000 / frequency_hz);
  timer_->setOverflow(period_us, MICROSEC_FORMAT);
}
```

### BoardConfig Integration Pattern
```cpp
#include <PWMOutputBank.h>
#include "targets/NUCLEO_F411RE_HIL001.h"

PWMOutputBank pwm;
auto& ch = BoardConfig::Servo::pwm_output;
pwm.Init(BoardConfig::Servo::timer, BoardConfig::Servo::frequency_hz);
pwm.AttachChannel(ch.ch, ch.pin, ch.min_us, ch.max_us);
pwm.Start();
```

---

## Hardware Constraints

### Timer Hardware Limits
- **Max 4 channels per timer** (STM32 hardware)
- **All channels share same frequency** (same PSC/ARR registers)
- **16-bit counters** (except TIM2/TIM5 are 32-bit)
- **PSC range**: 0-65535
- **ARR range**: 0-65535 (16-bit), 0-4294967295 (32-bit)

### Resolution vs Frequency Tradeoff
```
Resolution (ticks) = f_counter / f_pwm

Example @ 1 MHz counter:
- 50 Hz   → 20,000 ticks (~14.3 bit resolution)
- 200 Hz  → 5,000 ticks  (~12.3 bit resolution)
- 400 Hz  → 2,500 ticks  (~11.3 bit resolution)
- 1 kHz   → 1,000 ticks  (~10.0 bit resolution)
```

**Practical Ranges**:
- **Servo**: 50-200 Hz (standard to high-speed)
- **ESC Standard**: 50-490 Hz
- **ESC OneShot125**: 1-2 kHz
- **Resolution needed**: 1 µs sufficient for all use cases

### Recommended Timer Selection
**APB1 Timers** (50 MHz on F411RE):
- **TIM3**: 4 channels, well-documented, servo-friendly
- **TIM5**: 4 channels, 32-bit (extended range)

**APB2 Timers** (100 MHz on F411RE):
- **TIM1**: Advanced timer, 4 channels + complementary

---

## What We Learned from Research

### From UVOS TimerPWM
- ✅ 1 MHz tick rate is correct approach
- ✅ Prescaler calculation formula validated
- ✅ Array-based channel config scales well

### From Arduino Servo Library
- ✅ write() method with angle/µs detection is intuitive
- ✅ attach() with min/max calibration is useful
- ❌ Time-multiplexing not needed (abundant hardware channels)

### From Betaflight/iNav
- ✅ Explicit timer banks prevent conflicts
- ✅ Same timer = same protocol constraint is real
- ✅ Resource ownership flags improve clarity

### From HardwareTimer API
- ✅ MICROSEC_COMPARE_FORMAT simplifies pulse width setting
- ✅ setOverflow() with MICROSEC_FORMAT is perfect for our needs
- ✅ Auto pin configuration eliminates manual AF setup
- ✅ Dynamic clock query handles RCC complexity

---

## PWMOutputBank Implementation Notes

### Channel Enable Fix (October 2025)

**Problem**: Initial PWMOutputBank implementation configured PWM mode and pulse width correctly but generated no output signals (0% duty cycle measured via hardware testing).

**Root Cause**: Missing channel enable step in `AttachChannel()` method. Per STM32 timer documentation (AN4013 Section 2.5), PWM generation requires:
1. Configure PWM mode (OCxM bits) ✓ - via `setMode()`
2. Set pulse width (CCRx value) ✓ - via `setCaptureCompare()`
3. **Enable capture/compare output (CCxE bit)** ✗ - **was missing**

**Fix**: Added `_timer->resumeChannel(channel)` call in `AttachChannel()` after PWM configuration:

```cpp
// Configure timer channel for PWM output
_timer->setMode(channel, TIMER_OUTPUT_COMPARE_PWM1, pin);

// Set initial pulse width using MICROSEC_COMPARE_FORMAT
_timer->setCaptureCompare(channel, min_us, MICROSEC_COMPARE_FORMAT);

// Enable PWM output on this channel (CCxE bit per AN4013 Section 2.5)
_timer->resumeChannel(channel);
```

**Verification**: Hardware-in-loop testing with jumper wires (PA8→PA0, PB0→PA1):
- Before fix: 0 HIGH samples (no PWM signal)
- After fix: 4868-4888 HIGH samples out of 10000 (48.7%-48.9% duty cycle) ✓ PASS

**API Note**: `HardwareTimer::resume()` starts the counter (no parameters), while `HardwareTimer::resumeChannel(uint32_t channel)` enables specific channel output. PWMOutputBank requires both - `Start()` calls `resume()`, `AttachChannel()` calls `resumeChannel(channel)`.

---

## Reference Material

### Local Documentation
- `doc/TIMERS.md` - Complete STM32 timer architecture
- `doc/TIMERS_PWM_OUT.md` - Practical PWM output guide
- `Arduino_Core_STM32/cores/arduino/HardwareTimer.h` - Core API

### External References
- [HardwareTimer Wiki](https://github.com/stm32duino/Arduino_Core_STM32/wiki/HardwareTimer-library)
- [STM32Examples PWM_FullConfiguration](https://github.com/stm32duino/STM32Examples/blob/main/examples/Peripherals/HardwareTimer/PWM_FullConfiguration/)
- [Arduino Servo Library](https://github.com/arduino-libraries/Servo)
- [AN4013: Introduction to Timers for STM32 MCUs](https://www.st.com/resource/en/application_note/an4013-introduction-to-timers-for-stm32-mcus-stmicroelectronics.pdf)
- STM32F4 Reference Manual RM0383 (Timers: Chapter 13-18)
