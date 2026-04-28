# DShot — STM32 digital motor protocol

Unidirectional DShot motor output for STM32 F4 / F7 / G4 / H7. The
library encodes 16-bit DShot frames (11-bit throttle + telemetry +
CRC) and clocks them out via DMA-driven timer pulses, so the CPU
isn't involved in bit timing. DSHOT150 / 300 / 600 / 1200 supported.

## Quick Start

**Manual motor assignment** — explicit timer/pin/channel triples,
matches the `DShot_Basic` example:

```cpp
#include <DShot.h>

DShotOutput motors;

void setup() {
  motors.AddMotor(TIM3, PB4, 1, DShot::DSHOT600);
  motors.AddMotor(TIM3, PB5, 2, DShot::DSHOT600);
  motors.AddMotor(TIM4, PB6, 1, DShot::DSHOT600);
  motors.AddMotor(TIM4, PB7, 2, DShot::DSHOT600);
}

void loop() {
  motors.SetAllThrottle(1000);   // 0 = disarm, 48..2047 = throttle
  motors.Send();                  // non-blocking; DMA fires once
  delayMicroseconds(200);         // ~5 kHz pacing, well under ESC limits
}
```

**BoardConfig-driven** — pulls motor list from a flight-controller
target header (matches the `DShot_Basic_FC` example):

```cpp
#include <DShot.h>
#include "targets/BKMN-NERO.h"

DShotOutput motors;

void setup() {
  motors.Init(BoardConfig::Motor::motors,
              BoardConfig::Motor::num_motors,
              DShot::DSHOT600);
}
```

The `Init()` template reads optional DMA fields from each
`MotorConfig` (set by the Betaflight-config converter) and uses
them as overrides; if absent, the library auto-resolves the DMA
stream.

## Supported Platforms

| Family | Validated boards |
|--------|------------------|
| F4     | NUCLEO-F411RE; NUCLEO-F405RG (custom-rework breadboard rig) |
| F7     | NUCLEO-F722ZE |
| G4     | NUCLEO-G474RE |
| H7     | NUCLEO-H753ZI |

Speeds, library version, and architecture list are declared in
`library.properties`. Public symbols are in `keywords.txt`.

## API Reference

The full method list is in `keywords.txt`; signatures are in
`src/DShotOutput.h` and `src/DShot_packet.h`. This section explains
the lifecycle and what each group of calls is for.

### Construction & motor registration

- `DShotOutput()` — zero-arg constructor; no allocation, all state
  is static.
- `int AddMotor(TIM_TypeDef *timer, Pin pin, uint32_t channel,
  DShot::Speed speed = DShot::DSHOT600,
  const DShot::DMAResource *dma_override = nullptr)` — register
  one motor. Returns the motor index, or -1 on failure (out of
  motor slots, pin/timer mismatch). Up to `MAX_MOTORS` (8) motors
  across up to `MAX_TIMER_GROUPS` (4) timers. The optional
  `dma_override` pre-fills the DMA stream for that motor;
  otherwise the library auto-resolves at first `Send()`.
- `template<typename T> bool Init(const T &motors, int count,
  DShot::Speed speed = DShot::DSHOT600)` — bulk register a
  `BoardConfig::Motor::motors[]`-style array. Calls `AddMotor()`
  for each entry, passing through the DMA fields as overrides.

### Throttle and Send

- `SetThrottle(int motor_idx, uint16_t throttle, bool telemetry =
  false)` — stage one motor's value (0–2047). Does not transmit.
- `SetAllThrottle(uint16_t throttle, bool telemetry = false)` —
  stage every motor with the same value.
- `Send()` — encode all staged values, fill DMA buffers, trigger
  DMA. Non-blocking: the function returns once DMA is started;
  bit clocking happens in hardware. Safe to call from any loop
  rate ≤ ~10 kHz (one frame is ~28 µs at DSHOT600).
- `Disarm()` — convenience for `SetAllThrottle(0); Send();`.
- `IsTransferComplete() const` — poll if you need to know the last
  `Send()` finished before staging new values. Most loops don't
  need this.

### ESC commands

- `SendCommand(DShot::Command cmd)` — blocking. Sends DShot special
  commands (values 0–47). Configuration commands (spin direction,
  3D mode, save settings) repeat 10× internally per the DShot
  spec; beep commands send once with a 100 ms post-delay. Motors
  must be disarmed first.
- `Beep(uint8_t pattern = 1)` — convenience wrapper for
  `DShot::CMD_BEEP1..BEEP5`.

Defined commands are in `DShot::Command` (see `DShot_packet.h`):
`CMD_MOTOR_STOP`, `CMD_BEEP1..5`,
`CMD_SPIN_DIRECTION_1/2/NORMAL/REVERSED`, `CMD_3D_MODE_OFF/ON`,
`CMD_SAVE_SETTINGS`, `CMD_ESC_INFO`.

### Lifecycle

- `Release()` — disable timer outputs, stop DMA, release every
  claimed DMA stream. Object can then be reused (re-`AddMotor()`)
  or destroyed cleanly. Useful for verification harnesses that
  rotate through configurations.
- `IsInitFailed() const` — returns `true` if `initAllDMA()` (called
  internally on first `Send()`) couldn't claim a stream. Surfaces
  DMA conflicts that escaped compile-time review.
- `GetNumMotors() const` — count of motors successfully registered.

### Speeds and protocol constants

`DShot::Speed`: `DSHOT150` (3 MHz), `DSHOT300` (6 MHz), `DSHOT600`
(12 MHz, default), `DSHOT1200` (24 MHz). Bit timing constants
(`BIT_0_DUTY`, `BIT_1_DUTY`, `BIT_PERIOD`, `DMA_BUF_SIZE`) live in
`DShot_packet.h` and rarely need to be touched from sketch code.

## Examples

- **DShot_Basic** — manual 4-motor DShot600 on TIM3 + TIM4
  (PB4/PB5/PB6/PB7). Smallest possible end-to-end demo.
- **DShot_Basic_FC** — BoardConfig-driven, runs on every supported
  flight-controller target (OPEN_REVO, JHEF_JHEF411, BKMN_NERO,
  BEFH_BETAFPVG473, MATEK_H743VI). Cycles throttle 0..2047 in a
  loop.
- **DShot_Basic_Logic_Analyzer** — same 4-motor layout as
  `DShot_Basic`, throttle pattern tuned for external decode with
  a Saleae Logic 16 + `logic-dshot` analyzer. Top comment block
  has the probe map, capture config, PASS criteria.
- **DShot_Validation** — multi-board HIL loopback test (Nucleo-64
  and Nucleo-144). Two-phase: concurrent burst + per-channel.
  Per-rig fixture, expected output, and troubleshooting are in
  the example's own [README.md](examples/DShot_Validation/README.md).

## H7-specific notes

The H7 D-Cache and DMA can interact in subtle ways — DMA buffers
in cached SRAM need explicit cache management to stay coherent
with peripheral hardware. **The DShot library handles this
internally** for its own DMA buffers: `SCB_CleanDCache_by_Addr`
is called before every per-motor and burst-mode DMA trigger
(`DShot_ll.cpp`). Sketch authors do not need to manage cache or
arrange special buffer placement for DShot.

If you're integrating other DMA consumers alongside DShot in the
same sketch, see [`doc/DMA.md`](../../doc/DMA.md) "STM32H7 cache
coherency" for the broader H7 DMA-buffer story — including the
core's pre-configured non-cached D2 SRAM3 region at `0x30040000`,
which lets you skip the manual flush for buffers placed there.

## Troubleshooting

- **`IsInitFailed()` returns true** — DMA stream conflict. Build
  with `core_debug()` enabled (e.g. RTT) to see which stream
  couldn't be claimed. Check that no other peripheral has
  reserved the stream.
- **No output on a motor pin** — verify the (timer, pin, channel)
  triple is valid for your chip (check `PeripheralPins.c` for the
  variant). On F4/F7 some pins map to multiple timers; the library
  picks the timer you passed.
- **Burst was expected but isn't active** — F4/F7 burst is
  conflict-driven; G4/H7 use a simpler ≥2-motors-per-timer rule.
  See `doc/DSHOT.md` "Burst trigger rules" for the full per-family
  policy. Inspect `TIM->DCR` register or the diagnostic output of
  `DShot_Validation` for the per-timer mode.
- **Calling `Init()` / `AddMotor()` a second time fails** — call
  `Release()` first to free the previously claimed DMA streams.
- **Garbled output on F4/F7 with TIM4** — TIM4_UP and TIM2_CH2
  capture share DMA1 Stream 6 in silicon (RM0383 Tbl 27, RM0431
  Tbl 26). They cannot run concurrently. This bites loopback test
  fixtures more than real applications, but the constraint exists.

## See also

- [`doc/DSHOT.md`](../../doc/DSHOT.md) — DShot implementation
  reference: three-layer architecture, DMAR burst mechanics, per-
  family burst trigger rules, bidirectional DShot future work.
- [`doc/DMA.md`](../../doc/DMA.md) — core-level DMA reference: per-
  family DMA model, the `dma.c` claim registry, F411 DMA stream
  map, current DMA-consumer profile.
- [`examples/DShot_Validation/README.md`](examples/DShot_Validation/README.md)
  — HIL fixture, validation matrix, expected output, per-rig
  troubleshooting.
