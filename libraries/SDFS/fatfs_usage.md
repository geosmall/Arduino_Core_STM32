# FatFs Usage Checklist (STM32 · Config + Logging)

> Paste this into your project docs. It’s a terse, actionable checklist—no code, just decisions.

## Mount / Unmount
- [ ] **Mount once** after SD power-up/init (`"0:"`), set a **sd_available** flag.
- [ ] Keep the volume **mounted for the entire session**; avoid periodic un/mount churn.
- [ ] **Unmount** only on orderly shutdown or before **power-gating** the card.
- [ ] Implement **card-detect** and retry mount on insertion if not present at boot.

## File Access Patterns
- [ ] **Configs:** open → read/write → **close immediately** (no long-lived handles).
- [ ] **Logs:** **open once per session**, stream writes, **periodic `sync`**, **close at session end**.
- [ ] Prefer a **single FS owner** task; if multiple tasks must touch FS, enable FatFs reentrancy + mutex.

## Config I/O (Atomic & Safe)
- [ ] Write configs **atomically**: write to `cfg.tmp` → `sync` → `close` → optional rotate `cfg.bin → cfg.bak` → **rename** `cfg.tmp → cfg.bin`.
- [ ] Include **version + CRC** in the config blob; **validate** on read or fall back to defaults.
- [ ] Keep a **last-known-good** backup (`cfg.bak`) to recover from corruption.

## Logging (Throughput & Integrity)
- [ ] Start of session: create a **new filename** (timestamp or monotonic index).
- [ ] **Append in chunks ≥512 B** (prefer **2–16 KB**) to align with card sectors/multiblock.
- [ ] **Periodic `sync`** by **bytes** (e.g., 4–16 KB) or **time** (e.g., 250–1000 ms), whichever first.
- [ ] **Rotate** the log at a defined **max size/time** to bound close time and directory updates.
- [ ] End of session: **final `sync` + close** (then optional unmount/power-gate).

## Concurrency & Context
- [ ] **Never** do FS calls in **ISRs**; push data into a **lock-free ring buffer** from ISR → drain in a lower-priority task.
- [ ] If RTOS and multi-task I/O: **FF_FS_REENTRANT=1** with proper **mutex hooks**; otherwise **serialize** all I/O via one task/queue.

## Error Handling & Media Removal
- [ ] Check **every** `FRESULT`; on error: **stop logging**, close if possible, keep flight-critical loops running.
- [ ] Detect **card removal** (or bus faults) → **gracefully degrade**: disable logging, preserve system stability.
- [ ] On **mount failure**: run with defaults, surface a status flag/telemetry, allow remount attempts later.
- [ ] Accept that power loss risks only the data since the **last `sync`**; tune your `sync` cadence accordingly.

## Time & Metadata
- [ ] Provide a working **`get_fattime()`** (RTC-based) for correct file timestamps.
- [ ] Use **short/fixed-width names** (e.g., `LOG_000123.BIN`) if LFN costs matter.

## Performance Hints (STM32F411RE + microSD over SPI)
- [ ] Aggregate writes in RAM (2–8 KB typical) before calling file write.
- [ ] Avoid directory scans in tight loops; **precompute** next log name.

## FatFs Build Options (ffconf.h) — Minimal & Fast
- [ ] **`FF_MAX_SS = FF_MIN_SS = 512`** (typical microSD).
- [ ] **`FF_FS_TINY=0`** (per-file sector buffer for better throughput).
- [ ] **`FF_USE_LFN=0 or 1`** (disable or keep minimal if RAM/time constrained).
- [ ] **`FF_FS_REENTRANT=1`** only if multi-task FS is required (supply mutex).
- [ ] **`FF_USE_FASTSEEK=1`** (optional; helps random seeks; less critical for append).
- [ ] **`FF_USE_EXPAND=1`** if you will **preallocate** large logs to reduce fragmentation.

## Removal/Power-Loss Strategy (Decide & Document)
- [ ] **Sync policy**: ☐ size-based (e.g., 8 KB) ☐ time-based (e.g., 500 ms) ☐ hybrid (recommend).
- [ ] **Data-loss budget**: at most the last unsynced chunk/window.
- [ ] **Shutdown path** guarantees: final **`sync` → `close` → `unmount` → power-gate** (in that order).

## Telemetry & QA
- [ ] Expose **status counters**: bytes written, last error code, sync count/age, mount state.
- [ ] Provide a **self-test**: mount → small write/read → verify → clean up.
- [ ] Capture **latency spikes** (max write time) to size buffers and tune `sync`.

--- 

**Rule of thumb:** for UAV logging, start with **2–8 KB write chunks** and **`sync` every 4–16 KB or 250–1000 ms**; tighten `sync` if you need higher safety, loosen if you need more throughput (but always close cleanly at session end).