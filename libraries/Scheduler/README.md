# Scheduler Library

Priority-based cooperative task scheduler ported from the INav flight controller.

## Why This Scheduler?

This library is ported from the **INav flight controller** (which evolved from
Cleanflight/Betaflight). It is built for applications with a hard periodic
control loop, where an overdue loop must be the next thing that runs.

It is **cooperative**: a task runs to completion and nothing preempts it. Read
"What REALTIME does and does not do" below before giving any task a long or
blocking step — that section is the one thing that determines whether your
control loop holds its period.

### Comparison with Other Schedulers

| Feature | Arduino Scheduler | TaskScheduler | **INav Scheduler** |
|---------|-------------------|---------------|---------------------|
| Architecture | Pseudo-threading | Cooperative | Priority-based |
| Overdue-task catch-up | No | No | **Yes** (see limits below) |
| Priority levels | None | Basic | 6 levels |
| Starvation prevention | No | No | **Dynamic aging** |
| Target use case | General | General | **Real-time control** |
| Max loop rate | ~100 Hz | ~1 kHz | **100 kHz** |
| System load monitoring | No | No | **Yes** |

### What REALTIME does and does not do

**What it does.** Once a REALTIME task is overdue — its period has elapsed
since it last ran — it is selected ahead of every other ready task, whatever
their dynamic priority (`src/scheduler.c:295-302`). A REALTIME task therefore
never loses a turn to a busy lower-priority task, and never starves.

**What it does not do.** This scheduler is cooperative and has no fit check:

- **No preemption.** The selected task is called and runs to completion
  (`src/scheduler.c:332`). Nothing interrupts it.
- **No admission test.** Per-task execution times *are* measured
  (`averageExecutionTime`, `maxExecutionTime`), but they are recorded after a
  task returns and are never consulted before dispatch. A task is never held
  back because it would not fit before the next REALTIME deadline.
- **So "overdue" is detected, not prevented.** A lower-priority task selected
  just before the REALTIME deadline delays that task by its own full duration.
  REALTIME means *catch up next*, not *run on time*.

**What that means for your tasks.** REALTIME jitter is bounded by the longest
single execution of any other task, not by priority. So:

- Never put a blocking wait, a long transfer, or an erase in a lower-priority
  task while a REALTIME loop is running — its whole duration lands in your
  loop period.
- Break long work into steps short enough to fit the slack in your loop
  period, and return. If a step must be bounded tightly, do it inside the
  REALTIME task after its outputs, one small non-blocking piece per call —
  this is how INav logs at 2 kHz on this scheduler.
- Size your loop from measured worst case: `schedulerGetTaskInfo()` reports
  `maxExecutionTime` per task. The sum of your REALTIME task and the worst
  other task must fit the period.

> Betaflight's scheduler *does* have the admission test this one lacks: it
> learns each task's execution time and runs a task only if it fits before the
> next gyro tick. Task designs copied from Betaflight that rely on that test
> are not safe here.

**A note on the source comment.** `src/scheduler.c:296` reads "realtime tasks
take absolute priority". That is upstream INav's own wording, kept verbatim so
this library stays syncable (see "Major Changes from INav Scheduler"); it
describes selection order, not a timing guarantee. Prefer this section.

Suited to:

- Flight control loops (1-2 kHz)
- Motor control
- Hard real-time sensor fusion
- Any application where an overdue control loop must run next

## Major Changes from INav Scheduler

This library adapts the INav scheduler for Arduino with minimal divergence to enable future upstream syncing. Key changes:

### Architecture Changes

| Change | Reason |
|--------|--------|
| `schedulerInit(cfTasks, TASK_COUNT)` | Task array passed at runtime (Arduino libraries compile before sketches) |
| `SCHEDULER_MAX_TASKS=16` | Fixed array sizing since TASK_COUNT unknown at library compile time |
| `TASK_NONE`/`TASK_SELF` as macros | Fixed values (254/255) ensure library/sketch compatibility |
| No `extern cfTasks[]` | Task array pointer stored internally via schedulerInit() |
| `micros64()` for timestamps | Arduino STM32 core provides 64-bit microsecond timing |

### Platform Adaptations

| INav Original | Arduino Replacement |
|---------------|---------------------|
| `#include "platform.h"` | `#include "Arduino.h"` |
| `#include "common/time.h"` | Inline type definitions (timeUs_t, timeDelta_t) |
| `#include "common/utils.h"` | Inline macros (MAX, MIN); UNUSED from core |
| `STATIC_FASTRAM`, `FAST_CODE` | Empty macros (no CCM RAM on most Arduino boards) |
| `micros()` (32-bit) | `micros64()` (64-bit, overflow-safe) |

### User Task Definition

INav defines all tasks in `scheduler.h` with `#ifdef USE_xxx` guards. Arduino users define tasks in a separate `task_list.h` file included before `<scheduler.h>`. This allows custom task sets without modifying library code.

See source file headers for complete change documentation (10 locations in scheduler.h, 11 in scheduler.c).

## Quick Start

### Step 1: Create task_list.h

Create a file named `task_list.h` in your sketch folder:

```c
#ifndef TASK_LIST_H
#define TASK_LIST_H

#define SCHEDULER_TASK_LIST_DEFINED

typedef enum {
    TASK_SYSTEM = 0,

    // Your tasks here
    TASK_FLIGHT,
    TASK_TELEMETRY,
    TASK_LED,

    TASK_COUNT
} cfTaskId_e;

#endif
```

### Step 2: Include Headers (Order Matters!)

```cpp
#include "task_list.h"    // Your tasks FIRST
#include <scheduler.h>    // Library SECOND
```

### Step 3: Implement Task Functions

```cpp
extern "C" void taskFlight(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);
    // Flight control code - runs at 1000 Hz
}

extern "C" void taskTelemetry(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);
    // Telemetry code - runs at 10 Hz
}

extern "C" void taskLed(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}
```

### Step 4: Define Task Configuration

```cpp
cfTask_t cfTasks[TASK_COUNT] = {
    [TASK_SYSTEM] = SCHEDULER_TASK_SYSTEM_INIT,
    [TASK_FLIGHT] = {
        .taskName = "FLIGHT",
        .taskFunc = taskFlight,
        .desiredPeriod = TASK_PERIOD_HZ(1000),
        .staticPriority = TASK_PRIORITY_REALTIME,
    },
    [TASK_TELEMETRY] = {
        .taskName = "TELEM",
        .taskFunc = taskTelemetry,
        .desiredPeriod = TASK_PERIOD_HZ(10),
        .staticPriority = TASK_PRIORITY_MEDIUM,
    },
    [TASK_LED] = {
        .taskName = "LED",
        .taskFunc = taskLed,
        .desiredPeriod = TASK_PERIOD_HZ(2),
        .staticPriority = TASK_PRIORITY_LOW,
    },
};
```

### Step 5: Initialize and Run

```cpp
void setup() {
    schedulerInit(cfTasks, TASK_COUNT);  // Pass task array and count
    setTaskEnabled(TASK_FLIGHT, true);
    setTaskEnabled(TASK_TELEMETRY, true);
    setTaskEnabled(TASK_LED, true);
}

void loop() {
    scheduler();
}
```

## API Reference

### Core Functions

| Function | Description |
|----------|-------------|
| `schedulerInit(cfTasks, TASK_COUNT)` | Initialize scheduler, returns `false` if TASK_COUNT > SCHEDULER_MAX_TASKS |
| `scheduler()` | Run one scheduler cycle (call from loop) |
| `setTaskEnabled(taskId, bool)` | Enable/disable a task |
| `rescheduleTask(taskId, periodUs)` | Change task period at runtime |
| `getTaskInfo(taskId, &info)` | Get task statistics |
| `getTaskDeltaTime(taskId)` | Get time since last execution |
| `schedulerResetTaskStatistics(taskId)` | Reset task statistics |
| `getCheckFuncInfo(&info)` | Get checkFunc statistics (for event-driven tasks) |

### Task Limit

The scheduler queue supports **SCHEDULER_MAX_TASKS** (default: 16) tasks. If you need more:

```c
#define SCHEDULER_MAX_TASKS 32  // Before including scheduler.h
#include "task_list.h"
#include <scheduler.h>
```

`schedulerInit()` returns `false` if TASK_COUNT exceeds the limit. Tasks beyond the limit will silently fail to enable.

### Priority Levels

| Priority | Value | Description |
|----------|-------|-------------|
| `TASK_PRIORITY_IDLE` | 0 | Only runs if nothing else ready |
| `TASK_PRIORITY_LOW` | 1 | Background tasks |
| `TASK_PRIORITY_MEDIUM` | 3 | Normal tasks |
| `TASK_PRIORITY_MEDIUM_HIGH` | 4 | Above normal |
| `TASK_PRIORITY_HIGH` | 5 | Time-sensitive |
| `TASK_PRIORITY_REALTIME` | 18 | **Forced execution when overdue** |

### Choosing Priority Levels

Guidelines based on INav flight controller patterns:

| Priority | Use For | Examples | Typical Rate |
|----------|---------|----------|--------------|
| REALTIME | Hard real-time control loops | PID, gyro sampling | 1-8 kHz |
| HIGH | Time-sensitive I/O, system housekeeping | RC input, servo output | 10-200 Hz |
| MEDIUM | Sensor polling, navigation | GPS, baro, compass, rangefinder | 10-100 Hz |
| LOW | Non-critical periodic tasks | Serial, OSD, temperature | 10-250 Hz |
| IDLE | Background/cosmetic tasks | Telemetry, LED effects, logging | 1-500 Hz |

**Design principles:**
- Use REALTIME sparingly (1-2 tasks max) - only for control loops that must run as soon as they are due. Note REALTIME does not bound jitter on its own; the longest execution of any other task does (see above)
- HIGH priority for anything that feeds the control loop (RC input, sensor fusion)
- MEDIUM for sensors that update navigation state
- LOW/IDLE for telemetry and display - these can be delayed without affecting flight

### INav Reference: Flight Controller Task Rates

For reference, INav uses these rates for common flight controller tasks:

| Task | Priority | Rate | Purpose |
|------|----------|------|---------|
| PID | REALTIME | 1 kHz | Flight control loop |
| GYRO | REALTIME | 1-8 kHz | IMU sampling |
| SYSTEM | HIGH | 10 Hz | Housekeeping, watchdog |
| RX | HIGH | 10 Hz* | RC receiver (*event-driven) |
| AUX | HIGH | 100 Hz | Mode switches, arming |
| GPS | MEDIUM | 50 Hz | Position updates |
| BARO | MEDIUM | 20 Hz | Altitude |
| COMPASS | MEDIUM | 10 Hz | Heading |
| BATTERY | MEDIUM | 50 Hz | Voltage/current |
| SERIAL | LOW | 100 Hz | CLI, MSP protocol |
| OSD | LOW | 250 Hz | On-screen display |
| TELEMETRY | IDLE | 500 Hz | FrSky, CRSF, etc. |
| LEDSTRIP | IDLE | 100 Hz | Status LEDs |

### Period Macros

| Macro | Description |
|-------|-------------|
| `TASK_PERIOD_HZ(hz)` | Period for given frequency (e.g., `TASK_PERIOD_HZ(1000)` = 1ms) |
| `TASK_PERIOD_MS(ms)` | Period from milliseconds |
| `TASK_PERIOD_US(us)` | Period in microseconds |

### System Monitoring

| Variable/Macro | Description |
|----------------|-------------|
| `averageSystemLoadPercent` | Current CPU load (0-100+) |
| `isSystemOverloaded()` | True if load >= 100% |

### Optional: Realtime Callbacks

The scheduler calls `taskRunRealtimeCallbacks()` in two situations:
1. **Idle time** - when no task is ready to run
2. **After forced REALTIME execution** - immediately after a REALTIME task runs

Use this for low-latency operations that need servicing in scheduler gaps:

```cpp
void taskRunRealtimeCallbacks(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);
    // INav uses this for:
    // - DShot motor signal completion (pwmCompleteMotorUpdate)
    // - SD card filesystem polling (afatfs_poll)
    // - ESC telemetry updates
    //
    // NOT for RC receiver input - use a HIGH priority task instead
    // (see RX_EXAMPLE.md for details)
}
```

**Important:** Keep this function lightweight - no heavy calculations or blocking calls. Execution time is charged to TASK_SYSTEM for load monitoring.

If you don't need realtime callbacks, simply omit the function - the library provides an empty default.

**Note:** For RC receiver integration, use a HIGH priority task with `checkFunc` for event-driven scheduling. See [examples/Sched_Ex/EXAMPLE.md](examples/Sched_Ex/EXAMPLE.md) for the complete pattern.

## Creating task_list.h

Your `task_list.h` file must:

1. **Define `SCHEDULER_TASK_LIST_DEFINED`** before the enum
2. **Start with `TASK_SYSTEM = 0`** (always required)
3. **End with `TASK_COUNT`** (always required)
4. **Add your tasks** between TASK_SYSTEM and TASK_COUNT

Note: `TASK_NONE` and `TASK_SELF` are provided automatically by the library as
macros - do not include them in your enum.

```c
#ifndef TASK_LIST_H
#define TASK_LIST_H

#define SCHEDULER_TASK_LIST_DEFINED     // Required!

typedef enum {
    TASK_SYSTEM = 0,        // Required, must be first

    // === Your tasks here ===
    TASK_MY_TASK_1,
    TASK_MY_TASK_2,
    // === End your tasks ===

    TASK_COUNT              // Required, must be last
} cfTaskId_e;

#endif
```

## Attribution

- **Original source**: INav Flight Controller v9.0.0 (https://github.com/iNavFlight/inav)
- **Heritage**: Cleanflight/Betaflight scheduler
- **License**: GPL-3.0
- **Arduino adaptation**: GLS (January 2026)
