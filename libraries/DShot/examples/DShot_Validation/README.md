# DShot_Validation — multi-board HIL loopback test

Per-board fixture and procedure for validating the DShot library
across all four supported STM32 families (F4 / F7 / G4 / H7) on
ST Nucleo-64 and Nucleo-144 boards.

The sketch is a single two-phase test:

- **Phase A** — concurrent burst (DMAR) on as many timers as the
  board fixture supports, with a second motor per timer (decoy) at
  a different throttle to prove DMA stride/interleaving.
- **Phase B** — per-channel (non-burst) DShot600 on TIM1 (CH1 on
  Nucleo-64, CH2 on Nucleo-144 — see fixture tables) and TIM3_CH3.

Each phase runs 5 throttle values (0, 48, 500, 1000, 2047) and
verifies captured packets via TIM2 input capture + DMA. CRC, bit
period (±5 % tolerance), and decoded throttle must match. A full
run is **10 PASS rows** for any board (the row count is the same;
F722ZE Phase A is 2-timer instead of 3-timer).

---

## Supported rigs

| Rig | MCU | Connector style | Jumpers | Notes |
|-----|-----|-----------------|---------|-------|
| NUCLEO-G474RE | STM32G474RET6 | Nucleo-64 (CN5/CN8/CN9) | 4 | Primary rig; G4 burst fix verified here. |
| NUCLEO-F411RE | STM32F411RET6 | Nucleo-64 (CN5/CN8/CN9) | 3 | No TIM4 — chip doesn't route TIM2_CH4 to any free pin (PA3=VCOM RX). |
| NUCLEO-F722ZE | STM32F722ZET6 | Nucleo-144 (Zio) | 3 | No TIM4 burst — F4/F7 DMA1 Stream 6 hard-shared between TIM4_UP and TIM2_CH2 capture (RM0431 Table 26). |
| NUCLEO-H753ZI | STM32H753ZIT6 | Nucleo-144 (Zio) | 4 | H7 captures on DMA2 (DMAMUX), no Stream 6 conflict — TIM4 burst kept. |

All fixtures are **permanent** — no rewiring between phases or runs.

### Pin-finding tip (Nucleo-144 specific)

The morpho headers (CN11/CN12) on F722ZE / H753ZI are **unpopulated by
default** — the through-holes have no pin strip soldered. The Nucleo-144
fixture below avoids morpho-only pins (notably **PA8** and **PA1**, which
are not on Zio at all) by using the Zio extension pins on
CN7/CN8/CN9/CN10, which are populated.

Every Zio pin has both a `Dxx` number and the raw MCU `Pxx` name on
the silkscreen, but the labels are very small (~1 mm font) and
oriented along the connector edge. **Find pins by the raw `Pxx`
label.** Dx numbers vary per board (e.g. `PB6` is `D26` on F722,
`D1` on H753); the `Pxx` name is always the same.

---

## Fixture tables

All fixture tables list pins by their raw MCU `Pxx` silkscreen label.

### NUCLEO-F411RE — 3 jumpers (Nucleo-64 Arduino headers)

| # | Output | Role     | ⟶ | Input | TIM2 capture |
|---|--------|----------|---|-------|--------------|
| 1 | **PB4**  | TIM3_CH1 | → | **PA0**  | TIM2_CH1 |
| 2 | **PA8**  | TIM1_CH1 | → | **PA1**  | TIM2_CH2 |
| 3 | **PB0**  | TIM3_CH3 | → | **PB10** | TIM2_CH3 |

**TIM4 not covered on F411RE** — chip routes TIM2_CH4 only to PA3,
which is VCOM RX. Hardware constraint. TIM4 burst code path executes
on F411RE but isn't verified via loopback.

### NUCLEO-G474RE — 4 jumpers (Nucleo-64 Arduino headers)

| # | Output | Role      | ⟶ | Input  | TIM2 capture     |
|---|--------|-----------|---|--------|------------------|
| 1 | **PB4**  | TIM3_CH1  | → | **PA0**  | TIM2_CH1         |
| 2 | **PA8**  | TIM1_CH1  | → | **PA1**  | TIM2_CH2         |
| 3 | **PB0**  | TIM3_CH3  | → | **PB10** | TIM2_CH3         |
| 4 | **PB6**  | TIM4_CH1  | → | **PA10** | TIM2_CH4 (AF10)  |

### NUCLEO-F722ZE — 3 jumpers (Nucleo-144 Zio)

| # | Output | Role     | ⟶ | Input  | TIM2 capture            |
|---|--------|----------|---|--------|-------------------------|
| 1 | **PB4**  | TIM3_CH1 | → | **PA0**  | TIM2_CH1                |
| 2 | **PE11** | TIM1_CH2 | → | **PB3**  | TIM2_CH2 (alt — AF1)    |
| 3 | **PB0**  | TIM3_CH3 | → | **PB10** | TIM2_CH3                |

**TIM4 burst not covered on F722ZE** — F4/F7 DMA1 Stream 6 carries
both `TIM4_UP` (sole at Ch2/S6) and `TIM2_CH2` capture (sole at
Ch3/S6) in the same physical stream slot. They cannot run
concurrently. Silicon constraint per RM0431 Table 26; not a library
limitation.

The TIM_UP code path on F722 is still exercised end-to-end by TIM1
burst and TIM3 burst — only the TIM4_UP table entry is unverified at
runtime (silicon-validated against RM, see `DSHOT_BURST_FIX_PLAN.md`
in the workspace).

The Nucleo-144 fixture differs from Nucleo-64 in two places:
**PE11/PE13 in place of PA8/PA9** for TIM1 (PA8 lives only on the
unpopulated morpho header), and **PB3 in place of PA1** for
TIM2_CH2 capture (PA1 likewise morpho-only). PE11 is TIM1_CH2 not
CH1 — the captured channel index changes accordingly, but the
library code path is identical.

### NUCLEO-H753ZI — 4 jumpers (Nucleo-144 Zio)

| # | Output | Role     | ⟶ | Input  | TIM2 capture           |
|---|--------|----------|---|--------|------------------------|
| 1 | **PB4**  | TIM3_CH1 | → | **PA0**  | TIM2_CH1               |
| 2 | **PE11** | TIM1_CH2 | → | **PB3**  | TIM2_CH2 (alt — AF1)   |
| 3 | **PB0**  | TIM3_CH3 | → | **PB10** | TIM2_CH3               |
| 4 | **PB6**  | TIM4_CH1 | → | **PB11** | TIM2_CH4               |

H753 keeps the TIM4 jumper because H7 input capture on DMA2 (via
DMAMUX) doesn't share a controller with DShot burst on DMA1 — no
Stream 6 conflict.

### Decoy outputs (no jumper, output-only)

| Pin   | Role                          | G474RE | F411RE | F722ZE | H753ZI |
|-------|-------------------------------|:------:|:------:|:------:|:------:|
| PA9   | TIM1_CH2 (Nucleo-64 burst)    | ✓ | ✓ | — | — |
| PE13  | TIM1_CH3 (Nucleo-144 burst)   | — | — | ✓ | ✓ |
| PB5   | TIM3_CH2 (forces TIM3 burst)  | ✓ | ✓ | ✓ | ✓ |
| PB7   | TIM4_CH2 (forces TIM4 burst)  | ✓ (Morpho) | — | — | ✓ (LED_BLUE, flickers) |

Decoys get a throttle offset from the captured motor; a DMA
stride/interleaving bug would cause the captured channel to decode
the decoy's throttle value — the test fails immediately.

---

## Coverage matrix

| Scenario | G474RE | F411RE | F722ZE | H753ZI |
|----------|:------:|:------:|:------:|:------:|
| Phase A — TIM1 burst (advanced timer, UPDATE-triggered post-fix) | ✓ | ✓ | ✓ | ✓ |
| Phase A — TIM3 burst (general-purpose timer) | ✓ | ✓ | ✓ | ✓ |
| Phase A — TIM4 burst | ✓ | — (PA3=VCOM) | — (DMA1 S6 conflict) | ✓ |
| Phase A — concurrent burst groups | ✓ (3 timers) | ✓ (2 timers) | ✓ (2 timers) | ✓ (3 timers) |
| Phase A — DMA stride / interleaving correct | ✓ | ✓ | ✓ | ✓ |
| Phase B — TIM1 per-channel | ✓ (CH1) | ✓ (CH1) | ✓ (CH2) | ✓ (CH2) |
| Phase B — TIM3_CH3 per-channel | ✓ | ✓ | ✓ | ✓ |
| Bit rate — DShot600 timing (±5 %) | ✓ | ✓ | ✓ | ✓ |
| CRC valid on 100 % of captured frames | ✓ | ✓ | ✓ | ✓ |
| 5 throttle values (0, 48, 500, 1000, 2047) | ✓ | ✓ | ✓ | ✓ |

Not covered (deferred, not regressions):

- DShot150 / DShot300 / DShot1200 bit rates — add a third phase later
  if desired.
- Bidirectional DShot (telemetry) — not in library today.
- Flight-controller boards (`BKMN_NERO`, `BEFH_BETAFPVG473`,
  `MATEK_H743VI`, etc.) — motor pins go to ESC headers; loopback
  jumper not practical. Use `DShot_Basic_FC` for those boards.

---

## Procedure

### 1. Identify the rig

```
./ci/detect_device.sh
```

Match the ST-Link / J-Link serial against the workspace `README.md`
HIL table to confirm which board is connected.

### 2. Confirm fixture

Visually check the jumpers against the fixture table for the
identified board. No rewiring between phases.

### 3. Build + flash + capture

Use `saflash_stlink.sh` for boards with onboard ST-Link (G474RE,
H753ZI), `saflash.sh` for boards with onboard or reflashed J-Link
(F411RE on HIL-001, F722ZE).

```
# ST-Link
./ci/saflash_stlink.sh \
    Arduino_Core_STM32/libraries/DShot/examples/DShot_Validation \
    <FQBN> \
    --timeout 20

# J-Link
./ci/saflash.sh \
    Arduino_Core_STM32/libraries/DShot/examples/DShot_Validation \
    <FQBN> \
    --timeout 20
```

FQBN per board:

- G474RE: `STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_G474RE`
- F411RE: `STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE`
- F722ZE: `STM32_Robotics:stm32:Nucleo_144:pnum=NUCLEO_F722ZE`
- H753ZI: `STM32_Robotics:stm32:Nucleo_144:pnum=NUCLEO_H753ZI`

Both `saflash` scripts default to F411RE FQBN — **pass the FQBN
explicitly** for any other board. Exit wildcard is `*STOP*`,
emitted by the sketch after all phases complete.

### 4. Read the Serial log

The captured log lives at
`test_logs/serial/DShot_Validation_<timestamp>.txt` (symlinked as
`latest`).

---

## Expected PASS output

### G474RE (10 pass rows)

```
DShot_Validation (NUCLEO-G474RE)
--- Phase A: concurrent burst on TIM1 + TIM3 + TIM4 ---
  TIM1 DCR=0x10D burst=ACTIVE
  TIM3 DCR=0x10D burst=ACTIVE
  TIM4 DCR=0x10D burst=ACTIVE
  PASS throttle=0     TIM1|TIM3|TIM4
  PASS throttle=48    TIM1|TIM3|TIM4
  PASS throttle=500   TIM1|TIM3|TIM4
  PASS throttle=1000  TIM1|TIM3|TIM4
  PASS throttle=2047  TIM1|TIM3|TIM4
--- Phase B: per-channel on TIM1_CH1 + TIM3_CH3 ---
  TIM1 DCR=0x0 mode=per-channel
  TIM3 DCR=0x0 mode=per-channel
  PASS throttle=0     TIM1_CH1|TIM3_CH3
  ... (4 more)
=== Results: 10 passed, 0 failed ===
*STOP*
```

### F411RE (10 pass rows, no TIM4 column)

Phase A row reads `TIM1|TIM3`, Phase B row reads
`TIM1_CH1|TIM3_CH3`.

### F722ZE (10 pass rows, no TIM4 column, TIM1_CH2 in Phase B)

Phase A header: `Phase A: concurrent burst on TIM1 + TIM3` (no
TIM4), row reads `TIM1|TIM3`. Phase B row reads
`TIM1_CH2|TIM3_CH3`.

### H753ZI (10 pass rows, TIM1_CH2 in Phase B)

Phase A identical to G474RE (3-timer burst). Phase B row reads
`TIM1_CH2|TIM3_CH3` (Nucleo-144 fixture uses PE11=TIM1_CH2 instead
of PA8=TIM1_CH1).

---

## Troubleshooting

**Wrong throttle decoded on captured channel (stride/interleaving bug)**

- Symptom: one captured channel decodes a throttle matching the
  decoy offset, not the captured motor's value.
- Check: register dump of DMA channel (CPAR, CMAR, CNDTR) and
  `TIM->DCR` (DBA, DBL). Stride should match motor count.

**Phase A reports `burst=INACTIVE` unexpectedly**

- The library silently falls back to per-channel mode if stream
  conservation fails on F4/F7 or DMA allocation fails on G4/H7.
- Check: `TIM->DCR` register dump; library `initAllDMA()` return
  code.

**Phase B reports `burst(unexpected)` on TIM1 or TIM3**

- `DShotOutput::Release()` doesn't clear stale `TIM->DCR`. Harmless
  (library ignores DCR on per-channel path) but the heuristic check
  fails. Sketch pre-clears DCR at Phase B start — if you see this,
  the pre-clear code is missing/broken.

**Only captured N edges for throttle=X (N < 32)**

- Likely a flash-cache issue — re-flash with power cycle.
- Or capture DMA didn't arm before DShot fired. Widen
  `delayMicroseconds` between `armAllCaptures()` and
  `dshot.Send()`.

**CRC invalid but bit pattern looks right**

- Sketch computes CRC via `DShot::encodePacket()`. If the library's
  CRC engine is wrong, every test fails identically. Cross-check
  against `DShot_Basic_Logic_Analyzer` + logic analyzer capture to
  isolate library vs test.

**FAIL only on F411RE Phase B pass 2 (TIM3_CH3)**

- F4 DMA TIM2_CH3 = DMA1_Stream1 CH3. If the stream init omitted
  CH3 DMAMUX-equivalent (not applicable on F4, direct mapping),
  check `initCaptureDMA()` family branch.

---

## Rig swap checklist

When moving between rigs:

1. Run `./ci/detect_device.sh` before doing anything else.
2. Confirm the expected jumper count and layout for the identified
   board against this document.
3. Verify probe / jumper-wire continuity at the MCU pins
   (multimeter), not just at the headers — crimp failures are the
   common mode.
4. If the rig has a different ST-Link / J-Link serial than last
   session, reconfirm the board entry in the workspace `README.md`
   matches.

---

## Related documents

- `DSHOT_BURST_FIX_PLAN.md` (workspace root) — library fix this
  test validates (port of the G4 UPDATE-trigger fix to F4/F7/H7),
  including silicon-level RM cross-check of the TIM_UP DMA mapping
  table.
- `DShot_Basic_Logic_Analyzer/DShot_Basic_Logic_Analyzer.ino` —
  sister example sketch for external logic-analyzer (Saleae) DShot
  waveform decode. Setup procedure, probe maps, capture config and
  PASS criteria are in its top comment block.
- `libraries/DShot/src/DShot_ll.cpp` — library source exercised by
  this test.
- Workspace `README.md` (HIL table) — rig identifier serials +
  per-rig fixture notes.
