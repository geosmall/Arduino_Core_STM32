# Motor Configuration Design: Betaflight → BoardConfig → TimerPWM

## Question: How do Motors tie to TimerPWM library?

**Answer**: Motors use the same `PWMOutputBank` class as servos, but with different timing parameters optimized for ESC protocols (DShot, OneShot125, etc.).

## Current TimerPWM Integration Pattern

### Existing Pattern: NUCLEO_F411RE_LITTLEFS.h

```cpp
namespace BoardConfig {
  // Servo: TIM3 @ 50 Hz (standard servo control)
  namespace Servo {
    static inline TIM_TypeDef* const timer = TIM3;
    static constexpr uint32_t frequency_hz = 50;

    struct Channel {
      uint32_t pin;
      uint32_t ch;
      uint32_t min_us;
      uint32_t max_us;
    };

    static constexpr Channel pwm_output = {PB4, 1, 1000, 2000};
  };

  // ESC: TIM4 @ 1000 Hz (OneShot125 protocol)
  namespace ESC {
    static inline TIM_TypeDef* const timer = TIM4;
    static constexpr uint32_t frequency_hz = 1000;

    struct Channel {
      uint32_t pin;
      uint32_t ch;
      uint32_t min_us;
      uint32_t max_us;
    };

    static constexpr Channel esc1 = {PB6, 1, 125, 250};
    static constexpr Channel esc2 = {PB7, 2, 125, 250};
  };
}
```

### Usage in Sketch

```cpp
#include <PWMOutputBank.h>
#include "targets/NUCLEO_F411RE_LITTLEFS.h"

PWMOutputBank esc_pwm;

void setup() {
  // Initialize ESC bank
  esc_pwm.Init(BoardConfig::ESC::timer, BoardConfig::ESC::frequency_hz);

  // Attach individual ESC channels
  auto& esc1 = BoardConfig::ESC::esc1;
  esc_pwm.AttachChannel(esc1.ch, esc1.pin, esc1.min_us, esc1.max_us);

  auto& esc2 = BoardConfig::ESC::esc2;
  esc_pwm.AttachChannel(esc2.ch, esc2.pin, esc2.min_us, esc2.max_us);

  esc_pwm.Start();
}

void loop() {
  // Set throttle (125-250 µs for OneShot125)
  esc_pwm.SetPulseWidth(1, 187);  // ESC1 midpoint
  esc_pwm.SetPulseWidth(2, 187);  // ESC2 midpoint
}
```

## Betaflight Motor Configuration

### JHEF411 Example (5 Motors)

**From Betaflight config**:
```
# Motors on two different timers
resource MOTOR 1 A08
resource MOTOR 2 A09
resource MOTOR 3 A10
resource MOTOR 4 B00
resource MOTOR 5 B04

# Timer assignments with channels
timer A08 AF1  # TIM1 CH1
timer A09 AF1  # TIM1 CH2
timer A10 AF1  # TIM1 CH3
timer B00 AF2  # TIM3 CH3
timer B04 AF2  # TIM3 CH1

# DMA assignments (critical for DShot)
dma pin A08 1  # DMA2 Stream 1 Channel 6
dma pin A09 1  # DMA2 Stream 2 Channel 6
dma pin A10 1  # DMA2 Stream 6 Channel 6
dma pin B00 0  # DMA1 Stream 7 Channel 5
dma pin B04 0  # DMA1 Stream 4 Channel 5

# Protocol and frequency
set motor_pwm_protocol = DSHOT300
set dshot_burst = ON
```

### Key Observations

**Two timer banks required**:
- TIM1: Motors 1-3 (PA08, PA09, PA10)
- TIM3: Motors 4-5 (PB00, PB04)

**ALT variants required** (CRITICAL):
- Motor 4 (PB00) uses `timer B00 AF2` for TIM3_CH3
- PeripheralPins.c shows PB_0 default is TIM1_CH2N (AF1), not TIM3
- Must use `PB0_ALT1` to access TIM3_CH3 (AF2)
- Converter validates timer/AF against PeripheralPins.c and adds ALT suffix when needed

**Protocol determines frequency**:
- Standard PWM: 50-400 Hz (1000-2000 µs pulses)
- OneShot125: 1-4 kHz (125-250 µs pulses)
- OneShot42: 1-8 kHz (42-84 µs pulses)
- Multishot: 8-32 kHz (5-25 µs pulses)
- DShot300: 300 kbps digital protocol (special DMA handling)
- DShot600: 600 kbps digital protocol (special DMA handling)

**DShot requires DMA**: Cannot use simple PWM, needs dedicated implementation.

## Proposed Motor Namespace Design

### Option 1: Single Motor Namespace (All motors same protocol)

```cpp
namespace BoardConfig {
  namespace Motor {
    // Protocol and frequency
    enum class Protocol {
      PWM_50HZ,      // Standard PWM @ 50 Hz
      PWM_400HZ,     // Fast PWM @ 400 Hz
      ONESHOT125,    // OneShot125 @ 1-4 kHz
      ONESHOT42,     // OneShot42 @ 1-8 kHz
      MULTISHOT,     // Multishot @ 8-32 kHz
      DSHOT150,      // DShot @ 150 kbps (future)
      DSHOT300,      // DShot @ 300 kbps (future)
      DSHOT600       // DShot @ 600 kbps (future)
    };

    static constexpr Protocol protocol = Protocol::ONESHOT125;
    static constexpr uint32_t frequency_hz = 1000;  // For OneShot125

    // Timer bank 1 (TIM1) - Motors 1-3
    namespace Bank1 {
      static inline TIM_TypeDef* const timer = TIM1;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor1 = {PA8, 1, 125, 250};  // TIM1_CH1
      static constexpr Channel motor2 = {PA9, 2, 125, 250};  // TIM1_CH2
      static constexpr Channel motor3 = {PA10, 3, 125, 250}; // TIM1_CH3
    };

    // Timer bank 2 (TIM3) - Motors 4-5
    namespace Bank2 {
      static inline TIM_TypeDef* const timer = TIM3;

      static constexpr Bank1::Channel motor4 = {PB0_ALT1, 3, 125, 250};  // TIM3_CH3 (ALT1 required)
      static constexpr Bank1::Channel motor5 = {PB4, 1, 125, 250};  // TIM3_CH1
    };
  };
}
```

**Usage**:
```cpp
PWMOutputBank motor_bank1;
PWMOutputBank motor_bank2;

void setup() {
  // Initialize both motor banks at same frequency
  motor_bank1.Init(BoardConfig::Motor::Bank1::timer, BoardConfig::Motor::frequency_hz);
  motor_bank2.Init(BoardConfig::Motor::Bank2::timer, BoardConfig::Motor::frequency_hz);

  // Attach motors from bank 1
  auto& m1 = BoardConfig::Motor::Bank1::motor1;
  motor_bank1.AttachChannel(m1.ch, m1.pin, m1.min_us, m1.max_us);

  auto& m2 = BoardConfig::Motor::Bank1::motor2;
  motor_bank1.AttachChannel(m2.ch, m2.pin, m2.min_us, m2.max_us);

  // ... similar for bank2

  motor_bank1.Start();
  motor_bank2.Start();
}

void loop() {
  // Set all motors to same throttle
  for (int ch = 1; ch <= 3; ch++) {
    motor_bank1.SetPulseWidth(ch, 187);  // Midpoint
  }
  for (int ch = 1; ch <= 2; ch++) {
    motor_bank2.SetPulseWidth(ch, 187);  // Midpoint
  }
}
```

### Option 2: Simplified Array-Based Design

```cpp
namespace BoardConfig {
  namespace Motor {
    static constexpr uint32_t frequency_hz = 1000;  // OneShot125

    struct MotorChannel {
      TIM_TypeDef* timer;
      uint32_t pin;
      uint32_t ch;
      uint32_t min_us;
      uint32_t max_us;
    };

    static constexpr MotorChannel motors[] = {
      {TIM1, PA8, 1, 125, 250},   // Motor 1
      {TIM1, PA9, 2, 125, 250},   // Motor 2
      {TIM1, PA10, 3, 125, 250},  // Motor 3
      {TIM3, PB0_ALT1, 3, 125, 250},   // Motor 4 (ALT1 required for TIM3)
      {TIM3, PB4, 1, 125, 250}    // Motor 5
    };

    static constexpr size_t count = sizeof(motors) / sizeof(motors[0]);
  };
}
```

**Usage** (requires array iteration to group by timer):
```cpp
// Helper to group motors by timer
PWMOutputBank motor_tim1;
PWMOutputBank motor_tim3;

void setup() {
  motor_tim1.Init(TIM1, BoardConfig::Motor::frequency_hz);
  motor_tim3.Init(TIM3, BoardConfig::Motor::frequency_hz);

  for (auto& motor : BoardConfig::Motor::motors) {
    if (motor.timer == TIM1) {
      motor_tim1.AttachChannel(motor.ch, motor.pin, motor.min_us, motor.max_us);
    } else if (motor.timer == TIM3) {
      motor_tim3.AttachChannel(motor.ch, motor.pin, motor.min_us, motor.max_us);
    }
  }

  motor_tim1.Start();
  motor_tim3.Start();
}
```

### Option 3: Explicit Bank Declaration (Recommended)

Most explicit, matches existing Servo/ESC pattern:

```cpp
namespace BoardConfig {
  namespace Motor {
    enum class Protocol { PWM_50HZ, ONESHOT125, DSHOT300 };
    static constexpr Protocol protocol = Protocol::ONESHOT125;
    static constexpr uint32_t frequency_hz = 1000;

    // Timer Bank 1: TIM1 @ 1 kHz
    namespace TIM1_Bank {
      static inline TIM_TypeDef* const timer = TIM1;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor1 = {PA8, 1, 125, 250};
      static constexpr Channel motor2 = {PA9, 2, 125, 250};
      static constexpr Channel motor3 = {PA10, 3, 125, 250};
    };

    // Timer Bank 2: TIM3 @ 1 kHz
    namespace TIM3_Bank {
      static inline TIM_TypeDef* const timer = TIM3;

      static constexpr TIM1_Bank::Channel motor4 = {PB0_ALT1, 3, 125, 250};  // ALT1 required
      static constexpr TIM1_Bank::Channel motor5 = {PB4, 1, 125, 250};
    };
  };
}
```

## Betaflight Converter Output

### Parsing Strategy

```python
def extract_motor_config(bf_config):
    motors = {}  # motor_num → (pin, timer, channel, dma)

    # 1. Parse resource MOTOR lines
    for line in resources:
        if line.startswith("resource MOTOR"):
            motor_num, pin = parse_motor_resource(line)
            motors[motor_num] = {"pin": pin}

    # 2. Parse timer assignments
    for line in timers:
        pin, af, timer_info = parse_timer(line)
        # timer_info = "TIM1 CH1", "TIM3 CH3", etc.
        if pin in motors.values():
            motor_num = find_motor_by_pin(pin)
            motors[motor_num]["timer"] = extract_timer(timer_info)
            motors[motor_num]["channel"] = extract_channel(timer_info)

    # 3. Parse protocol
    protocol = bf_config.get_setting("motor_pwm_protocol")  # DSHOT300, etc.

    # 4. Group motors by timer
    timer_banks = group_by_timer(motors)

    return timer_banks, protocol
```

### Generated Output Example

From JHEF411:

```cpp
// Generated from JHEF-JHEF411.config
namespace BoardConfig {
  namespace Motor {
    // Protocol: DSHOT300 (from set motor_pwm_protocol = DSHOT300)
    // Note: DShot requires DMA and special implementation (future)
    // For now, use OneShot125 as fallback
    static constexpr uint32_t frequency_hz = 1000;

    // TIM1 Bank: Motors 1-3
    namespace TIM1_Bank {
      static inline TIM_TypeDef* const timer = TIM1;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor1 = {PA8, 1, 125, 250};   // TIM1_CH1
      static constexpr Channel motor2 = {PA9, 2, 125, 250};   // TIM1_CH2
      static constexpr Channel motor3 = {PA10, 3, 125, 250};  // TIM1_CH3
    };

    // TIM3 Bank: Motors 4-5
    namespace TIM3_Bank {
      static inline TIM_TypeDef* const timer = TIM3;

      static constexpr TIM1_Bank::Channel motor4 = {PB0_ALT1, 3, 125, 250};  // TIM3_CH3 (ALT1 required)
      static constexpr TIM1_Bank::Channel motor5 = {PB4, 1, 125, 250};  // TIM3_CH1
    };
  };
}
```

## Protocol Mapping Table

| Betaflight Protocol | Frequency | Min µs | Max µs | Notes |
|---------------------|-----------|--------|--------|-------|
| `PWM` | 50-490 Hz | 1000 | 2000 | Standard servo PWM |
| `ONESHOT125` | 1-4 kHz | 125 | 250 | 1/8 of standard PWM |
| `ONESHOT42` | 1-8 kHz | 42 | 84 | 1/24 of standard PWM |
| `MULTISHOT` | 8-32 kHz | 5 | 25 | Ultra-fast analog |
| `DSHOT150` | 150 kbps | N/A | N/A | Digital protocol (future) |
| `DSHOT300` | 300 kbps | N/A | N/A | Digital protocol (future) |
| `DSHOT600` | 600 kbps | N/A | N/A | Digital protocol (future) |

**Converter default behavior**:
- For analog protocols (PWM, OneShot, Multishot): Generate PWMOutputBank config
- For DShot: Generate comment warning + fallback to OneShot125

## Implementation Phases

### Phase 1: OneShot125 Support (Current)
- Parse MOTOR resources + timers
- Group by timer bank
- Generate namespace Motor::TIM*_Bank
- Default to OneShot125 (125-250 µs @ 1 kHz)

### Phase 2: Protocol Detection
- Parse `motor_pwm_protocol` setting
- Map to frequency + pulse range
- Generate protocol-specific configs

### Phase 3: DShot Support (Future)
- Requires new DMA-based PWMOutputBank implementation
- Parse DMA assignments
- Validate DMA conflicts
- Generate DShot-specific initialization

## Summary

**How Motors tie to TimerPWM**:
1. Motors use same `PWMOutputBank` class as servos
2. Different namespace (`Motor` vs `Servo`) with protocol-specific timing
3. Multiple banks needed if motors span multiple timers
4. Converter groups motors by timer automatically
5. Protocol determines frequency and pulse width ranges

**Recommended Design**: Option 3 (Explicit Bank Declaration)
- Matches existing Servo/ESC pattern
- Clear, explicit timer bank separation
- Easy to use in sketches
- Straightforward for converter to generate
