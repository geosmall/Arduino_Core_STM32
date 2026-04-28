# DShot Implementation Reference

Internal design notes for the DShot library at
`libraries/DShot/`. Companion to that library's user-facing
README — this doc covers *why* and *how* the library is built the
way it is, for maintainers and contributors.

User-facing API documentation is in
[`libraries/DShot/README.md`](../libraries/DShot/README.md).
The general STM32 DMA infrastructure DShot relies on is in
[`DMA.md`](DMA.md).

## Three-layer architecture

The library is organized as three layers, top-down:

**`src/DShot_packet.h`** — pure protocol encoding. No hardware
dependency. `encodePacket()`, `fillDmaBuffer()`, `Speed` and
`Command` enums, `BIT_*_DUTY` and `BIT_PERIOD` timing constants.
Header-only. Directly portable to any platform.

**`src/DShot_ll.{h,cpp}`** — hardware abstraction. GPIO AF lookup
(`pin_function`-based, iterates `PinMap_TIM` matching both pin and
timer), timer setup (clock-source aware, applies APB1/APB2
prescaler doubling), per-channel DMA init, DMAR burst init.
Family branches via `#if defined(STM32xxxx)`. The branched parts:

| Concern | F4 / F7 | G4 | H7 |
|---|---|---|---|
| DMA controller | streams (DMA1/2 × 8 streams) | channels (DMA1/2 × 8 channels) | streams + DMAMUX |
| Request routing | fixed stream↔peripheral table | DMAMUX (any-to-any) | DMAMUX1 (any-to-any) |
| LL API | `LL_DMA_SetStream()` | `LL_DMA_SetChannel()` | `LL_DMA_SetStream()` + DMAMUX |
| DCache | none (F4) / optional (F7) | none | must flush (`SCB_CleanDCache_by_Addr`) |

**`src/DShotOutput.{h,cpp}`** — user-facing class. Maintains up to
4 `TimerGroup`s (motors that share a timer) and decides per-group
whether to use per-channel DMA or DMAR burst. Defers DMA init to
the first `Send()` call so that `dma_claim()` happens after every
other consumer has had a chance to register their streams.

## DMA completion strategy: cleanup-at-start-of-Send()

The library does **not** install DMA TC interrupt handlers for
output. Normal-mode DMA auto-disables the stream/channel on
transfer completion. The timer DMA request stays enabled but is
harmless (a disabled stream ignores requests). At the top of each
`Send()` call, the library disables all timer DMA requests
(cleanup from the previous transfer), clears flags, and re-arms.

At any practical loop rate (1–8 kHz), the ~28 µs DMA transfer at
DSHOT600 is always complete before the next `Send()`. This
deliberately avoids DMA IRQ handler conflicts — historically a
problem with `uart.c`'s strong handlers for DMA1_Stream5,
DMA2_Stream1, DMA2_Stream2 on F4 (now removed; UART is
interrupt-driven).

**Limitation:** this approach is incompatible with bidirectional
DShot, which requires chaining input-capture setup to the output
TC event. See "Bidirectional DShot" below.

## DMAR burst mode mechanics

DMAR (DMA Burst) writes multiple CCRs (CCR1..CCRn) in one DMA
transfer triggered by a single timer event, instead of using one
DMA stream per channel. The library always uses:

- **Burst length = highest active channel index + 1** — e.g. a
  3-motor TIM1 group on CH1/CH2/CH3 has burst length 3. Unused
  channels in the run get zero (writing 0 to an unused CCR is
  harmless because the channel's output stays low).
- **Stride-N interleaved buffer layout** — motor with channel
  index *c* in a group of length *N* writes its bit timings to
  buffer slots `c, c+N, c+2N, ...`. The DMA engine walks the
  buffer linearly; the timer's burst engine distributes each run
  of *N* words to CCR1..CCRn.
- **UPDATE-event trigger** on every family. The trigger is the
  timer's UPDATE event (period-paced, one DMA request per PWM
  period). This was changed from a CC-event trigger as part of
  the F4/F7/H7 burst-DMA fix landed in submodule commit
  `ec3584c06`; the original G4-only fix was `6d996794a`. Long-form
  rationale lives in those commit messages.

### H7 TIM_UP substitution

Some `TIMx_CHy` peripherals on H7 have no per-channel DMA request
line at all (e.g. there's no `DMA_REQUEST_TIM4_CH4`); only
`TIM_UP` is exposed. Burst mode is the only way to drive those
channels via DMA. INav handles this in `timer_def_stm32h7xx.h` by
substituting `DMA_REQUEST_TIM4_UP` for the missing CH request
when `USE_DSHOT_DMAR` is defined. This library always uses
TIM_UP for burst, so the substitution is implicit.

## Burst trigger rules

The decision to use burst vs per-channel happens in
`DShotOutput::shouldUseBurst()`. The rules differ by family:

| Family | When does burst kick in? |
|---|---|
| F4, F7 | Two motors on same timer resolve to same DMA stream — silicon conflict in `dma_map[]` triggers auto-upgrade |
| G4, H7 | Any 2+ motors share a timer (proactive stream conservation; DMAMUX flexibility means the conflict-driven path never fires anyway) |

Why the asymmetry? On F4/F7, `dma_map[]` is a fixed table from
the reference manual, so motor-to-stream resolution is
deterministic. Conflicts (two motors on the same timer mapping to
the same stream — e.g. TIM1 CH1/CH2/CH3 all to DMA2 Stream 6 on
F411) genuinely require burst to function. On G4/H7, DMAMUX makes
streams fungible, so conflicts cannot happen — but burst is still
preferred when the choice is between *N* streams per timer and
1 stream per timer. INav targets opt into this via `USE_DSHOT_DMAR`
per board; this library makes it the default for ≥2 motors per
timer on DMAMUX families, equivalent to what every multi-motor
INav board would have set anyway.

## Bidirectional DShot (future work)

Bidirectional DShot (ESC → FC eRPM telemetry) is out of scope
today but the infrastructure is mostly in place:

- `dma.c` callback dispatch (added in the unified DMA registry,
  see [`DMA.md`](DMA.md)) supports DMA TC interrupt chaining
  within the ~25 µs response window between output completion
  and input-capture start.
- `dma_claim()` / `dma_release()` already exist for stream
  reclaim during the output→input direction flip.
- `DShotOutput::Release()` and the lazy `initAllDMA()` pattern
  are compatible with reconfiguring a stream from output to
  input mid-loop.

Remaining work: input-capture DMA setup, GPIO direction switching
on the response edge, response-frame decoding, eRPM calculation.
None of this requires public API changes — `DShotOutput` would
gain `BeginBidirectional()` / `ReadERPM(int motor_idx)` or similar,
the rest stays the same.

## References

- DShot protocol spec: Betaflight wiki,
  `betaflight/src/main/drivers/dshot.{c,h}`
- INav per-channel and burst DMA patterns:
  `inav/src/main/drivers/pwm_output.c`,
  `timer_impl_hal.c`,
  `timer_def_stm32h7xx.h` (TIM_UP substitution)
- Betaflight DMA request maps:
  `dma_reqmap_mcu.c`, `timer_def.h`
- ST reference manuals (DMA peripheral request maps):
  RM0383 Tables 27 / 28 (F411 DMA1 / DMA2),
  RM0431 Tables 26 / 27 (F722 DMA1 / DMA2),
  RM0440 (G4), RM0433 (H7)
- Submodule commits with full burst-DMA fix rationale:
  `6d996794a` (G4 UPDATE-trigger fix),
  `ec3584c06` (F4/F7/H7 port)
- HIL fixture and validation matrix:
  [`libraries/DShot/examples/DShot_Validation/README.md`](../libraries/DShot/examples/DShot_Validation/README.md)
