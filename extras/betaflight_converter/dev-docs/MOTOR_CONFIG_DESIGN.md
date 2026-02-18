# Motor Configuration Design: Betaflight → BoardConfig → TimerPWM

## How Motors Tie to TimerPWM Library

Motors use the same `PWMOutputBank` class as servos, but with different timing parameters optimized for ESC protocols. The converter generates a flat `motors[]` array where each entry includes its timer assignment, allowing sketch code to group by timer at initialization time.

## Implemented Design: Flat Array with Timer Field

### Generated Output Structure

The converter generates a `Motor` namespace with a flat array of `MotorConfig` entries:

```cpp
namespace BoardConfig {
  namespace Motor {
    static constexpr uint32_t frequency_hz = 2000;  // OneShot125

    struct MotorConfig {
      TIM_TypeDef* timer;
      uint32_t pin;
      uint32_t channel;
      uint32_t min_us;
      uint32_t max_us;
    };

    static constexpr MotorConfig motors[] = {
      {TIM1, PA8, 1, 125, 250},       // Motor 1: TIM1_CH1
      {TIM1, PA9, 2, 125, 250},       // Motor 2: TIM1_CH2
      {TIM1, PA10, 3, 125, 250},      // Motor 3: TIM1_CH3
      {TIM3, PB0_ALT1, 3, 125, 250},  // Motor 4: TIM3_CH3
      {TIM3, PB4, 1, 125, 250},       // Motor 5: TIM3_CH1
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}
```

The same pattern is used for servos:

```cpp
namespace BoardConfig {
  namespace Servo {
    static constexpr uint32_t frequency_hz = 50;

    struct ServoConfig {
      TIM_TypeDef* timer;
      uint32_t pin;
      uint32_t channel;
      uint32_t min_us;
      uint32_t max_us;
    };

    static constexpr ServoConfig servos[] = {
      {TIM15, PE5, 1, 1000, 2000},  // Servo 1: TIM15_CH1
      {TIM15, PE6, 2, 1000, 2000},  // Servo 2: TIM15_CH2
    };

    static constexpr int num_servos = sizeof(servos) / sizeof(servos[0]);
  };
}
```

### Why Flat Array

The flat array design was chosen over per-timer-bank namespaces (`TIM1_Bank`, `TIM3_Bank`) because:

1. **Simpler code generation** — no grouping logic needed in the generator
2. **Uniform access** — `motors[0]` through `motors[num_motors-1]` works for any board
3. **Timer field included** — each entry carries its timer, so sketch code can group at init time
4. **Matches manual targets** — `NUCLEO_F411RE_HIL001.h` already uses this pattern

### Usage with TimerPWM Library

```cpp
#include <PWMOutputBank.h>
#include "output/JHEF-JHEF411.h"

PWMOutputBank motor_tim1;
PWMOutputBank motor_tim3;

void setup() {
  uint32_t freq = BoardConfig::Motor::frequency_hz;

  // Initialize one PWMOutputBank per timer
  motor_tim1.Init(TIM1, freq);
  motor_tim3.Init(TIM3, freq);

  // Attach motors by iterating the array
  for (int i = 0; i < BoardConfig::Motor::num_motors; i++) {
    auto& m = BoardConfig::Motor::motors[i];
    if (m.timer == TIM1) {
      motor_tim1.AttachChannel(m.channel, m.pin, m.min_us, m.max_us);
    } else if (m.timer == TIM3) {
      motor_tim3.AttachChannel(m.channel, m.pin, m.min_us, m.max_us);
    }
  }

  motor_tim1.Start();
  motor_tim3.Start();
}

void loop() {
  uint32_t throttle = 187;  // Midpoint for OneShot125
  motor_tim1.SetPulseWidth(1, throttle);
  motor_tim3.SetPulseWidth(3, throttle);
}
```

## Betaflight Config → Motor Generation Pipeline

### Input: Native config.h

```c
#define MOTOR1_PIN           PA8
#define MOTOR2_PIN           PA9
#define MOTOR3_PIN           PA10
#define MOTOR4_PIN           PB0
#define MOTOR5_PIN           PB4

#define TIMER_PIN_MAPPING \
    TIMER_PIN_MAP( 1, PA8 , 1,  1) \
    TIMER_PIN_MAP( 2, PA9 , 1,  1) \
    TIMER_PIN_MAP( 3, PA10, 1,  1) \
    TIMER_PIN_MAP( 4, PB0 , 2,  0) \
    TIMER_PIN_MAP( 5, PB4 , 1,  0)
```

### Resolution Steps

1. **Parse motor pins**: `#define MOTOR(\d+)_PIN\s+(P\w+)` extracts pin per motor index
2. **Parse TIMER_PIN_MAP**: `TIMER_PIN_MAP(index, pin, occurrence, dma)` extracts occurrence per pin
3. **Resolve against PeripheralPins.c**: For each motor pin, look up all PinMap_TIM entries, filter to non-complementary channels, select the Nth entry (occurrence is 1-based)
4. **Determine ALT variant**: If occurrence > 1, the selected entry uses an ALT PinName (e.g., `PB_0_ALT1`), which converts to Arduino macro `PB0_ALT1`
5. **Generate MotorConfig**: `{timer, pin, channel, min_us, max_us}`

### ALT Variant Example: Motor 4 (PB0)

**TIMER_PIN_MAP**: `TIMER_PIN_MAP(4, PB0, 2, 0)` — occurrence=2

**PeripheralPins.c** for STM32F411:
```c
{PB_0,      TIM1, GPIO_AF1_TIM1, 2, 1},  // Entry 1: TIM1_CH2N (complementary, skipped)
{PB_0,      TIM3, GPIO_AF2_TIM3, 3, 0},  // Entry 1 (non-complementary): TIM3_CH3
// Wait — actually for F411, after filtering complementary:
// Non-complementary entry 1: TIM1_CH2N is complementary → skip
// Non-complementary entry 1: TIM3_CH3 (PB_0_ALT1)
```

The occurrence=2 selects the 2nd non-complementary entry for PB_0, which uses `PB_0_ALT1` → TIM3_CH3.

**Generated**: `{TIM3, PB0_ALT1, 3, 125, 250}`

Without `_ALT1`, PB0 would default to TIM1_CH2N — wrong timer, wrong channel type.

## Protocol and Timing

### OneShot125 (Default for Generated Configs)

All generated BoardConfig headers use OneShot125 protocol:
- **Frequency**: 2000 Hz (500 µs period)
- **Pulse range**: 125-250 µs (throttle 0-100%)
- **Midpoint**: 187 µs (~50% throttle)

### Protocol Reference Table

| Protocol | Frequency | Min µs | Max µs | Notes |
|----------|-----------|--------|--------|-------|
| Standard PWM | 50-490 Hz | 1000 | 2000 | Standard servo/ESC PWM |
| OneShot125 | 1-4 kHz | 125 | 250 | 1/8 of standard PWM |
| OneShot42 | 1-8 kHz | 42 | 84 | 1/24 of standard PWM |
| Multishot | 8-32 kHz | 5 | 25 | Ultra-fast analog |
| DShot150/300/600 | N/A | N/A | N/A | Digital protocol (not supported) |

The converter defaults to OneShot125 regardless of the Betaflight protocol setting. DShot requires DMA-based implementation beyond simple PWM and is not currently supported.

## Multi-Board Examples

### JHEF411 (5 motors, 2 timers)

```cpp
static constexpr MotorConfig motors[] = {
  {TIM1, PA8, 1, 125, 250},       // Motor 1: TIM1_CH1
  {TIM1, PA9, 2, 125, 250},       // Motor 2: TIM1_CH2
  {TIM1, PA10, 3, 125, 250},      // Motor 3: TIM1_CH3
  {TIM3, PB0_ALT1, 3, 125, 250},  // Motor 4: TIM3_CH3
  {TIM3, PB4, 1, 125, 250},       // Motor 5: TIM3_CH1
};
```

### MATEKH743 (8 motors + 2 servos, 3 timer banks)

```cpp
// Motors
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

// Servos (separate namespace, 50 Hz)
static constexpr ServoConfig servos[] = {
  {TIM15, PE5, 1, 1000, 2000},  // Servo 1: TIM15_CH1
  {TIM15, PE6, 2, 1000, 2000},  // Servo 2: TIM15_CH2
};
```

Motors 1-6 use ALT1 variants (occurrence=2 in TIMER_PIN_MAP). Motors 7-8 use default pins (occurrence=1). Servos are separated into their own namespace with 50 Hz frequency and 1000-2000 µs range.

### REVO (6 motors, 3 timer banks)

```cpp
static constexpr MotorConfig motors[] = {
  {TIM3, PB0_ALT1, 3, 125, 250},  // Motor 1: TIM3_CH3
  {TIM3, PB1_ALT1, 4, 125, 250},  // Motor 2: TIM3_CH4
  {TIM2, PA3, 4, 125, 250},       // Motor 3: TIM2_CH4
  {TIM2, PA2, 3, 125, 250},       // Motor 4: TIM2_CH3
  {TIM5, PA1_ALT1, 2, 125, 250},  // Motor 5: TIM5_CH2
  {TIM5, PA0_ALT1, 1, 125, 250},  // Motor 6: TIM5_CH1
};
```

## Summary

1. **Generated configs use flat `motors[]` array** with `MotorConfig` struct containing timer, pin, channel, min/max pulse
2. **Servos use identical pattern** in separate `Servo` namespace with `ServoConfig` struct
3. **Timer resolution** uses `TIMER_PIN_MAP` occurrence as 1-based index into PeripheralPins.c entries
4. **ALT variants** are automatically applied when occurrence > 1 selects a non-default timer mapping
5. **OneShot125** is the default protocol (125-250 µs at 2000 Hz)
6. **Sketch code groups by timer** at initialization time using the timer field in each entry
