# DShot - Digital Motor Protocol for STM32

DMA-driven DShot output for STM32 Arduino. Sends DShot packets to ESCs using hardware timers and DMA — no CPU time spent bit-banging.

Supports DSHOT150, DSHOT300, DSHOT600, and DSHOT1200 on STM32 F4, F7, G4, and H7 families.

## Quick Start

```cpp
#include <DShot.h>

DShotOutput motors;

void setup() {
  // Add motors: specify timer, pin, channel (1-4), speed
  motors.AddMotor(TIM3, PB4, 1, DShot::DSHOT600);
  motors.AddMotor(TIM3, PB5, 2, DShot::DSHOT600);
  motors.AddMotor(TIM4, PB6, 1, DShot::DSHOT600);
  motors.AddMotor(TIM4, PB7, 2, DShot::DSHOT600);
}

void loop() {
  motors.SetThrottle(0, 1000);   // Motor 0: throttle 1000
  motors.SetThrottle(1, 1000);   // Motor 1: throttle 1000
  motors.SetAllThrottle(0);      // Or set all at once
  motors.Send();                 // Encode + DMA transfer
}
```

### With BoardConfig

If your target has a `BoardConfig::Motor::motors[]` array, use `Init()` for a one-liner setup:

```cpp
#include <DShot.h>
#include "targets/NUCLEO_F411RE_JHEF411.h"

DShotOutput motors;

void setup() {
  motors.Init(BoardConfig::Motor::motors,
              BoardConfig::Motor::num_motors,
              DShot::DSHOT600);
}
```

`Init()` calls `AddMotor()` for each entry, then starts all timers. The `MotorConfig` struct must have `.timer`, `.pin`, and `.channel` fields.

## API Reference

### DShotOutput

| Method | Description |
|--------|-------------|
| `int AddMotor(TIM_TypeDef *timer, uint32_t pin, uint32_t channel, DShot::Speed speed)` | Register a motor. Returns motor index (0-based), or -1 on failure. `channel` is 1-4. |
| `bool Init(motors[], count, speed)` | Register all motors from a BoardConfig array and start timers. |
| `void SetThrottle(int idx, uint16_t throttle, bool telemetry = false)` | Set throttle for one motor (0-2047). Not sent until `Send()`. |
| `void SetAllThrottle(uint16_t throttle, bool telemetry = false)` | Set throttle for all motors. |
| `void Send()` | Encode packets and trigger DMA transfers for all motors. |
| `void Disarm()` | Send throttle 0 to all motors. |
| `bool IsTransferComplete() const` | Returns true when all DMA transfers from the last `Send()` have finished. |
| `int GetNumMotors() const` | Number of registered motors. |

### DShot Speeds

| Speed | Timer Clock | Bit Period |
|-------|-------------|------------|
| `DShot::DSHOT150` | 3 MHz | ~6.67 us |
| `DShot::DSHOT300` | 6 MHz | ~3.33 us |
| `DShot::DSHOT600` | 12 MHz | ~1.67 us |
| `DShot::DSHOT1200` | 24 MHz | ~0.83 us |

DSHOT600 is the most common choice — fast enough for high-rate PID loops, widely supported by ESCs.

## How It Works

### DShot Packet Format

Each packet is 16 bits, sent MSB-first as PWM pulses:

```
[11-bit throttle] [1-bit telemetry] [4-bit CRC]
     0-2047            0/1          XOR nibbles
```

Each bit is one timer period. A `1` bit has ~74% duty cycle; a `0` bit has ~37% duty cycle. The ESC distinguishes bits by measuring pulse width.

### DMA Output

The library fills a buffer of 18 timer compare values (16 data bits + 2 zero-padding for frame reset), then triggers a single DMA transfer from memory to the timer's CCR register. The timer generates the PWM waveform with zero CPU involvement after the trigger.

```
Timer ARR = 19 ticks (one bit period)
Bit 0: CCR = 7   (37% duty)
Bit 1: CCR = 14  (74% duty)
```

### Multi-Motor DMA

Motors sharing the same timer are grouped automatically. On G4 and H7 (which have DMAMUX), each channel gets its own DMA stream — no conflicts.

On F4 and F7, the fixed DMA stream map can assign multiple timer channels to the same stream (e.g., TIM1 CH1/CH2/CH3 all map to DMA2_Stream6). When the library detects this conflict, it automatically switches to **DMAR burst mode**: a single DMA stream writes interleaved compare values to the timer's DMAR register, which distributes them to CCR1..CCRn via the DCR (DMA Control Register). This is transparent to the user — just call `AddMotor()` and `Send()`.

## Timer and Pin Selection

Each motor needs a timer channel and a GPIO pin with the correct alternate function for that channel. The timer/pin/channel mapping is determined by the MCU's datasheet.

Common examples:

| Pin | Timer | Channel | Notes |
|-----|-------|---------|-------|
| PA8 | TIM1 | CH1 | Advanced timer, MOE auto-enabled |
| PA9 | TIM1 | CH2 | |
| PB4 | TIM3 | CH1 | General-purpose timer |
| PB5 | TIM3 | CH2 | |
| PB6 | TIM4 | CH1 | |
| PB7 | TIM4 | CH2 | |

The library configures GPIO alternate functions, timer prescaler/ARR, output compare, and DMA automatically — you only provide the timer peripheral, pin, and channel number.

**Advanced timers** (TIM1, TIM8): The library enables MOE (Main Output Enable) automatically. No extra configuration needed.

## Supported MCU Families

| Family | DMA Model | Multi-Channel | Validated |
|--------|-----------|---------------|-----------|
| STM32F4 | Fixed stream map | DMAR burst (automatic) | F411RE |
| STM32F7 | Fixed stream map | DMAR burst (automatic) | F722ZE |
| STM32G4 | DMAMUX | Per-channel (no conflicts) | G474RE |
| STM32H7 | DMAMUX | Per-channel (no conflicts) | H753ZI |

## Typical Usage Pattern

```cpp
DShotOutput motors;

void setup() {
  motors.AddMotor(TIM1, PA8,  1, DShot::DSHOT600);
  motors.AddMotor(TIM1, PA9,  2, DShot::DSHOT600);
  motors.AddMotor(TIM3, PB4,  1, DShot::DSHOT600);
  motors.AddMotor(TIM3, PB0,  3, DShot::DSHOT600);
}

void loop() {
  // Your control loop computes throttle values...
  motors.SetThrottle(0, throttle_FL);
  motors.SetThrottle(1, throttle_FR);
  motors.SetThrottle(2, throttle_RL);
  motors.SetThrottle(3, throttle_RR);
  motors.Send();

  // Optionally wait for completion before next iteration
  while (!motors.IsTransferComplete()) { }
}
```

`Send()` is non-blocking — it triggers DMA and returns immediately. Use `IsTransferComplete()` if you need to synchronize before the next send.

## Limits

- Maximum 8 motors (`DShot::MAX_MOTORS`)
- Maximum 4 timer groups (4 distinct timers)
- Output only — no bidirectional DShot (ESC-to-FC telemetry over the signal wire)
- Timer channels 1-4 supported

## Examples

| Example | Purpose |
|---------|---------|
| `DShot_Basic` | 4-motor output on TIM3/TIM4, throttle sweep |
| `DShot_Verification` | Loopback test: captures DShot output via input capture, verifies packet encoding and timing |
| `DShot_Burst_Verification` | Validates DMAR burst mode with multiple TIM1 channels (F4/F7 only) |

## Hardware Validation

Packet encoding and DMA timing have been verified by loopback capture (DShot output pin jumpered to TIM2 input capture) on all four supported families:

| Board | MCU | Test | Result |
|-------|-----|------|--------|
| Nucleo F411RE | STM32F411 | DShot_Verification (per-channel + burst) | 10/10 + 5/5 PASS |
| Nucleo F722ZE | STM32F722 | DShot_Verification (per-channel + burst) | 10/10 + 5/5 PASS |
| Nucleo G474RE | STM32G474 | DShot_Verification (per-channel) | 10/10 PASS |
| Nucleo H753ZI | STM32H753 | DShot_Verification (per-channel) | 10/10 PASS |
