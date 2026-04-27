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

The `Init()` template reads the optional DMA fields from each
`MotorConfig` (set by the Betaflight-config converter) and uses them
as overrides; if absent, the library auto-resolves the DMA stream.

## Supported Platforms

| Family | DMA model | Burst trigger rule | Validated boards |
|--------|-----------|--------------------|------------------|
| F4     | Fixed stream/channel maps (RM tables) | conflict-driven (auto-upgrade) | NUCLEO-F411RE, NUCLEO-F405RG (custom-rework breadboard rig) |
| F7     | Fixed stream/channel maps (RM tables) | conflict-driven (auto-upgrade) | NUCLEO-F722ZE |
| G4     | DMAMUX, channel-based | ≥2 motors per timer | NUCLEO-G474RE |
| H7     | DMAMUX, stream-based + DCache | ≥2 motors per timer | NUCLEO-H753ZI |

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
  `dma_override` pre-fills the DMA stream for that motor; otherwise
  the library auto-resolves at first `Send()`. See
  [Architecture: DMA allocation](#dma-allocation-strategy).
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
  DMA. Non-blocking: the function returns once DMA is started; bit
  clocking happens in hardware. Safe to call from any loop rate
  ≤ ~10 kHz (one frame is ~28 µs at DSHOT600).
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
`CMD_MOTOR_STOP`, `CMD_BEEP1..5`, `CMD_SPIN_DIRECTION_1/2/NORMAL/REVERSED`,
`CMD_3D_MODE_OFF/ON`, `CMD_SAVE_SETTINGS`, `CMD_ESC_INFO`.

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
  `DShot_Basic`, throttle pattern tuned for external decode with a
  Saleae Logic 16 + `logic-dshot` analyzer. Top comment block has
  the probe map, capture config, PASS criteria.
- **DShot_Validation** — multi-board HIL loopback test (NUCLEO-64
  and Nucleo-144). Two-phase: concurrent burst + per-channel.
  Per-rig fixture, expected output, and troubleshooting are in
  the example's own [README.md](examples/DShot_Validation/README.md).

## Troubleshooting

- **`IsInitFailed()` returns true** — DMA stream conflict. Build
  with `core_debug()` enabled (e.g. RTT) to see which stream
  couldn't be claimed. Check that no other peripheral has reserved
  the stream.
- **No output on a motor pin** — verify the (timer, pin, channel)
  triple is valid for your chip (check `PeripheralPins.c` for the
  variant). On F4/F7 some pins map to multiple timers; the library
  picks the timer you passed.
- **Burst was expected but isn't active** — F4/F7 burst is
  conflict-driven: it activates only when two motors on the same
  timer resolve to the same DMA stream. G4/H7 use the simpler
  rule (≥ 2 motors per timer). Inspect `TIM->DCR` register or the
  diagnostic output of `DShot_Validation` for the per-timer mode.
- **Calling `Init()`/`AddMotor()` a second time fails** — call
  `Release()` first to free the previously claimed DMA streams.
- **Garbled output on F4/F7 with TIM4** — TIM4_UP and TIM2_CH2
  capture share DMA1 Stream 6 in silicon (RM0383 Tbl 27, RM0431
  Tbl 26). They cannot run concurrently. This bites loopback test
  fixtures more than real applications, but the constraint exists.

## Architecture

### Three layers

`src/DShot_packet.h` — pure protocol encoding. No hardware deps.
`encodePacket()`, `fillDmaBuffer()`, `Speed` and `Command` enums,
`BIT_*_DUTY` and `BIT_PERIOD` constants. Header-only.

`src/DShot_ll.{h,cpp}` — hardware abstraction. GPIO AF lookup,
timer setup (clock-source aware: APB1/APB2 doubling), per-channel
DMA init, DMAR burst init, family branches via
`#if defined(STM32xxxx)`. The family-branched parts are the
DMA stream/channel resolution, DMAMUX request enums (G4/H7), and
DCache flush (H7).

`src/DShotOutput.{h,cpp}` — user-facing class. Maintains up to 4
`TimerGroup`s (motors sharing a timer) and decides per-group
whether to use per-channel DMA or DMAR burst. Defers DMA init to
the first `Send()` call so that `dma_claim()` happens after every
other consumer has had a chance to register.

### DMA allocation strategy

On **F4 / F7**, the (timer, channel) → (DMA, stream, channel-select)
mapping is fixed in silicon and is replicated in `dma_map[]` /
`tim_up_map[]` in `DShot_ll.cpp` straight from the reference
manuals. `resolveDMA()` is a table lookup; conflict detection is
done by checking `dma_is_claimed()` against the `dma.c` registry.
Conflicts on F4/F7 *auto-upgrade the timer group to DMAR burst* —
multiple motors on the same timer share one DMA stream targeting
`TIMx->DMAR`.

On **G4 / H7**, DMAMUX makes any DMA stream/channel routable to
any peripheral request. `resolveDMA()` scans DMA1 then DMA2 for
the first unclaimed slot and uses `getDMAMUXRequest()` to wire
the peripheral request. Conflicts can't happen the way they do
on F4/F7. Burst is used proactively (any 2-motors-per-timer
group) for stream conservation.

`tim_up_map[]` (in `DShot_ll.cpp:200`) is annotated with the
exact RM table sources: Betaflight `timer_def.h:236-243` (F4) and
`:339-346` (F7) cross-checked against RM0383 Tables 27 / 28
(F411 DMA1 / DMA2) and RM0431 Tables 26 / 27 (F722 DMA1 / DMA2).

### DMAR burst mode

DMAR burst writes multiple CCRs (CCR1..CCRn) in one DMA transfer
triggered by a single timer event, instead of one DMA stream per
channel. The library always uses **burst length = highest channel
index + 1** and a **stride-N interleaved buffer** (motor with
channel index c writes to slots `c, c+N, c+2N, ...`). Unused
channel slots are zeroed; writing 0 to an unused CCR is harmless
because the channel's output stays low.

The trigger is the timer's UPDATE event (period-paced, one DMA
request per PWM period) on every family. This was changed from a
CC-event trigger as part of the F4/F7/H7 burst-DMA fix landed in
submodule commit `ec3584c06`; the original G4-only fix was
`6d996794a`. Long-form rationale lives in those commit messages.

H7 has the additional wrinkle that some `TIMx_CHy` peripherals
have no per-channel DMA request line at all
(e.g. `DMA_REQUEST_TIM4_CH4`); only `TIM_UP` is available. Burst
mode is the only way to drive those channels via DMA.

The trigger policy summary:

| Family | When does burst kick in? |
|--------|--------------------------|
| F4, F7 | Two motors on same timer resolve to same DMA stream (silicon conflict in `dma_map[]`) |
| G4, H7 | Any 2+ motors share a timer (proactive stream conservation; DMAMUX flexibility means the conflict-driven path never fires anyway) |

The decision lives in `DShotOutput::shouldUseBurst()`.

### F411 DMA stream map (DShot-relevant rows)

Hardware reference, replicated from RM0383 Table 28. Other
peripheral request lines (UART, SPI, I2C) are intentionally not
shown here — they're in the RM table or `dma_map[]`.

**DMA1**

| Stream | DShot occupant | Channel select |
|--------|----------------|----------------|
| 0 | TIM4_CH1 | 2 |
| 2 | TIM3_UP, TIM3_CH4 | 5 |
| 3 | TIM4_CH2 | 2 |
| 4 | TIM3_CH1 | 5 |
| 5 | TIM3_CH2 | 5 |
| 6 | TIM4_UP | 2 |
| 7 | TIM3_CH3 *or* TIM4_CH3 | 5 / 2 |

**DMA2**

| Stream | DShot occupant | Channel select |
|--------|----------------|----------------|
| 1 | TIM1_CH1 | 6 |
| 2 | TIM1_CH2 | 6 |
| 4 | TIM1_CH4 | 6 |
| 5 | TIM1_UP | 6 |
| 6 | TIM1_CH3 | 6 |

TIM1 channels CH1/CH2/CH3 all resolve to DMA2 Stream 6 in the
per-channel `dma_map[]`, which is precisely why the F4/F7 burst
auto-upgrade rule is conflict-driven — a 3-motor TIM1 group on
F411 needs burst to avoid stream contention. F722 has the same
TIM1 layout.

## Design Notes

### Why no UART/SPI DMA

DShot is currently the **only DMA consumer in the core**. UART RX
runs interrupt-driven (sufficient for IBus / SBUS / CRSF /
115200-baud GPS — well under 1% CPU at the loop rates this core
targets). SPI for IMU is polled. ADC isn't wired up. Re-adding
UART or SPI DMA is straightforward — claim via `dma_set_handler()`
in `dma.c`, the same registry DShot uses — but doesn't currently
clear a "we measured this is a problem" bar.

For comparison: Betaflight runs SPI gyro DMA at 8 kHz PID-loop
rates and UART DMA for blackbox; INav stays interrupt-driven for
both. We currently match INav's DMA-consumer profile.

### Future: bidirectional DShot

Bidirectional DShot (ESC → FC eRPM telemetry) is out of scope
today but the infrastructure is mostly in place:

- `dma.c` callback dispatch (added with the unified DMA registry)
  supports DMA TC interrupt chaining within the ~25 µs response
  window between output completion and input capture start.
- `dma_claim()` / `dma_release()` already exist for stream
  reclaim during the direction flip.
- `DShotOutput::Release()` and the lazy `initAllDMA()` pattern
  are compatible with reconfiguring a stream from output to
  input mid-loop.

Remaining work: input-capture DMA setup, GPIO-direction flip,
response-frame decoding, eRPM calculation. None of this requires
public API changes.

## References

- DShot protocol spec: Betaflight wiki, `betaflight/src/main/drivers/dshot.{c,h}`
- INav per-channel and burst DMA patterns: `inav/src/main/drivers/pwm_output.c`,
  `timer_impl_hal.c`, `timer_def_stm32h7xx.h` (TIM_UP substitution)
- Betaflight DMA request maps: `dma_reqmap_mcu.c`, `timer_def.h`
- ST reference manuals (DMA peripheral request maps):
  RM0383 Table 28 (F411), RM0431 Table 26 (F722),
  RM0440 (G4), RM0433 (H7)
- HIL fixture, validation matrix, expected output, and
  per-rig troubleshooting:
  [`examples/DShot_Validation/README.md`](examples/DShot_Validation/README.md)
- Burst-DMA fix rationale: submodule commits `6d996794a`
  (G4 UPDATE-trigger fix) and `ec3584c06` (F4/F7/H7 port).
