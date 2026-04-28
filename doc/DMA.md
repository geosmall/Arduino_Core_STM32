# STM32 DMA Reference

Technical reference for Direct Memory Access (DMA) on STM32
microcontrollers in the Arduino STM32 Core. Covers the per-family
DMA model, the core's claim/dispatch registry, the F411 stream
map, and the current consumer profile.

DShot-specific implementation detail (burst mechanics, per-family
burst trigger rules, bidirectional-DShot future work) lives in
[`DSHOT.md`](DSHOT.md). User-facing DShot API is in
[`../libraries/DShot/README.md`](../libraries/DShot/README.md).

## Overview

DMA enables data transfers between memory and peripherals without
CPU intervention. This reduces interrupt overhead for timing-
critical outputs like DShot motor control.

## DMA architecture by family

### STM32F4xx / STM32F7xx — fixed channel assignment

DMA1 and DMA2, each with 8 streams. Each stream has a fixed table
of channel→peripheral mappings; the channel select bits in
`DMA_SxCR` pick which peripheral request triggers the stream.

```
DMA Controller → Stream (0-7) → Channel (0-7) → Peripheral
```

A given peripheral request can typically be serviced by exactly
one (stream, channel) combination, with a few alternate slots for
some peripherals. The mapping is hard-wired in silicon and
documented in the reference manual (e.g. RM0383 Tables 27 / 28
for F411).

### STM32G4xx — DMAMUX, channel-based

DMA1 and DMA2, each with up to 8 channels. DMAMUX routes any
peripheral request to any channel.

```
Peripheral → DMAMUX → DMA Channel → Memory
```

### STM32H7xx — DMAMUX, stream-based

DMA1 and DMA2, each with 8 streams. DMAMUX1 routes any peripheral
request to any stream.

```
Peripheral → DMAMUX1 → DMA Stream → Memory
```

H7 also has a D-Cache that requires explicit cache management
for DMA buffers — see "Cortex-M7 cache coherency" below (the
same applies to F7, which uses the same M7 core).

## DMA allocation strategy

Two patterns, depending on the family:

**F4 / F7 — table lookup.** The (peripheral, request) → (DMA,
stream, channel-select) mapping is fixed. Each DMA-using
peripheral consults a static table to find its stream. Conflict
detection is on the consumer side — at init, it checks
`dma_is_claimed(dma, stream)` against the central registry; if
the stream is already taken, the consumer either fails or falls
back to an alternate strategy (e.g. DShot upgrades to burst
mode).

**G4 / H7 — DMAMUX scan.** The first unclaimed stream/channel
across DMA1 then DMA2 is taken, and DMAMUX is configured to
route that stream's request line to the desired peripheral via
`LL_DMAMUX_SetPeriphRequest()`. Conflicts cannot happen unless
all streams are exhausted.

This library defers DMA init until the consumer's first transfer
(rather than at construction time) so that all consumers have a
chance to register before any allocation is committed. DShot
implements this via lazy `initAllDMA()` from the first `Send()`.

## The `dma.c` claim registry

`cores/arduino/stm32/dma.{c,h}` provides the single source of
truth for DMA stream/channel ownership across the core and all
libraries. Three operations:

| API | Use |
|---|---|
| `dma_claim(dma, stream)` | Reserve a stream/channel without an IRQ callback. Used by DShot (output is fire-and-forget; no TC interrupt needed). Returns -1 if already claimed. |
| `dma_set_handler(dma, stream, callback, context)` | Reserve a stream and register an IRQ callback for TC events. Used by future DMA consumers (e.g. UART RX, SPI gyro). |
| `dma_is_claimed(dma, stream)` | Query without modifying state. Used during conflict detection on F4/F7 to decide between per-channel and burst mode. |
| `dma_release(dma, stream)` | Inverse of `dma_claim` / `dma_set_handler`. Caller is responsible for halting hardware first (`LL_DMA_DisableStream` etc.). |

Backing storage is a 16-slot table — one per DMA1/DMA2 stream
(F4/F7/H7) or channel (G4). Macro-generated IRQ handlers in
`dma.c` dispatch interrupts to the registered callback. New DMA
consumers integrate by calling `dma_set_handler()` at init —
they participate in the same ownership table that DShot uses, so
conflicts surface at init rather than as silent register-write
contention at runtime.

## F411 DMA stream map

Hardware reference for STM32F411xC/E, replicated from RM0383
Tables 27 / 28. The "DShot occupant" column shows the entries
that this core's `dma_map[]` / `tim_up_map[]` claim today; a
stream not listed here is free for future DMA consumers.

**DMA1 — channels 0–7, streams 0–7**

| Stream | DShot occupant | Channel select |
|---|---|---|
| 0 | TIM4_CH1 | 2 |
| 2 | TIM3_UP, TIM3_CH4 | 5 |
| 3 | TIM4_CH2 | 2 |
| 4 | TIM3_CH1 | 5 |
| 5 | TIM3_CH2 | 5 |
| 6 | TIM4_UP | 2 |
| 7 | TIM3_CH3 *or* TIM4_CH3 | 5 / 2 |

**DMA2 — channels 0–7, streams 0–7**

| Stream | DShot occupant | Channel select |
|---|---|---|
| 1 | TIM1_CH1 | 6 |
| 2 | TIM1_CH2 | 6 |
| 4 | TIM1_CH4 | 6 |
| 5 | TIM1_UP | 6 |
| 6 | TIM1_CH3 | 6 |

Note that TIM1 CH1/CH2/CH3 all resolve to DMA2 Stream 6 in the
per-channel `dma_map[]` — this is a silicon constraint, not a
choice. A 3-motor TIM1 group on F411 needs DMAR burst to avoid
stream contention. F722 has the same TIM1 layout. See
[`DSHOT.md`](DSHOT.md) "Burst trigger rules" for how this is
handled.

For F722, G4, H7 stream maps consult RM0431 (F722) Tables 26 /
27, RM0440 (G4) and RM0433 (H7) DMAMUX request tables. With
DMAMUX-based families the table is just an enumeration of
`LL_DMAMUX_REQ_*` values — not a stream allocation.

## Current DMA consumer profile

DShot motor output is currently the **only DMA consumer** in this
codebase.

| Peripheral | This core | Betaflight | INav |
|---|---|---|---|
| DShot motors | DMA (per-channel or DMAR burst) | DMA | DMA |
| LED strip (WS2812) | not implemented | DMA | DMA |
| SPI (gyro/IMU) | polled | DMA TX+RX (8 kHz loop) | polled/interrupt |
| UART (RC, GPS, telemetry) | interrupt-driven | DMA TX+RX | interrupt-driven |
| ADC (vbat, current) | not implemented | DMA (circular) | DMA |
| SDIO (SD card) | not implemented | DMA | DMA (~2 targets) |
| I2C (baro, compass) | HAL interrupts | HAL interrupts | HAL interrupts |

The interrupt-mode UART path is sufficient for all current serial
protocols at ≤115200 baud (IBus / SBUS / CRSF / GPS). At
F4/G4/H7's typical clock rates this measures well under 1% CPU.
SPI for IMU is polled — adequate for the loop rates this core
targets (1–2 kHz typical), versus Betaflight's 8 kHz PID loop
where SPI DMA frees CPU during the gyro read. Re-adding UART or
SPI DMA is straightforward — the consumer would call
`dma_set_handler()` to participate in the registry — but doesn't
currently clear a "we measured this is a problem" bar.

The DShot library makes use of about half of the F411 DMA stream
budget at most (a 5-motor JHEF411 board uses 3 streams via burst);
adding any single other DMA consumer would not exhaust the pool.

## Cortex-M7 cache coherency

Both Cortex-M7 chip families — F7 and H7 — have a small
CPU-side data cache (D-Cache) that buffers SRAM reads and writes
for performance. **This Arduino core enables D-Cache by default
on every Cortex-M7 chip** in `cores/arduino/main.cpp` (via
`SCB_EnableDCache()`); set `D_CACHE_DISABLED` at build time to
opt out. F4 and G4 are Cortex-M4 and have no L1 data cache.

When D-Cache is enabled, DMA buffers in cached SRAM regions can
get out of sync with what the peripheral hardware actually sees
(DMA accesses SRAM directly, bypassing the cache). The fix is
either:

1. **Cache management on every transfer.** The producer flushes
   the cache before triggering DMA (`SCB_CleanDCache_by_Addr`);
   the consumer invalidates before reading (after a
   peripheral-to-memory transfer, `SCB_InvalidateDCache_by_Addr`).
   DShot uses the flush-before-trigger pattern; the call sites
   are guarded by `__DCACHE_PRESENT` so the same code is a no-op
   on F4 / G4 builds.
2. **Place the buffer in non-cached memory.** F7 has DTCM RAM
   (Data Tightly-Coupled Memory) that's directly attached to the
   M7 core and isn't cached. H7 ships with D2 SRAM3
   (0x30040000–0x30047FFF, 32 KB) pre-configured as non-cached
   via MPU; placing DMA-touched buffers there skips the manual
   flush entirely.

DShot does its own cache management internally (option 1) so
sketch authors don't need to think about this. If you add another
DMA consumer that uses statically-placed buffers, choose either
option above for that consumer's buffers.

## References

- **STM32 AN3109:** "Communication peripheral FIFO emulation
  with DMA"
- **STM32F4 Reference Manual** (RM0383 for F411, RM0431 for F722):
  DMA controller chapter — peripheral request mapping tables
- **STM32G4 Reference Manual** (RM0440): DMA + DMAMUX chapters
- **STM32H7 Reference Manual** (RM0433): DMA + DMAMUX1 chapter,
  cache architecture
- [`DSHOT.md`](DSHOT.md) — DShot's use of this DMA infrastructure
  (burst mechanics, trigger rules, bidirectional future work)
- [`../libraries/DShot/README.md`](../libraries/DShot/README.md)
  — DShot user-facing API
