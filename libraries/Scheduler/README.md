# Scheduler Library

Priority-based cooperative task scheduler ported from the INav flight controller.

## Why This Scheduler?

This library is ported from the **INav flight controller** (which evolved from
Cleanflight/Betaflight). It's designed for applications requiring guaranteed
timing for critical tasks.

### Comparison with Other Schedulers

| Feature | Arduino Scheduler | TaskScheduler | **INav Scheduler** |
|---------|-------------------|---------------|---------------------|
| Architecture | Pseudo-threading | Cooperative | Priority-based |
| REALTIME guarantee | No | No | **Yes** |
| Priority levels | None | Basic | 6 levels |
| Starvation prevention | No | No | **Dynamic aging** |
| Target use case | General | General | **Real-time control** |
| Max loop rate | ~100 Hz | ~1 kHz | **100 kHz** |
| System load monitoring | No | No | **Yes** |

### Key Feature: Forced REALTIME Execution

Other schedulers are cooperative - if a task takes too long, other tasks wait.

This scheduler **guarantees** REALTIME priority tasks execute on schedule.
When a REALTIME task becomes overdue, it immediately runs regardless of other
task priorities. This is critical for:

- Flight control loops (1-2 kHz)
- Motor control
- Hard real-time sensor fusion
- Any application where timing jitter is unacceptable

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
| `#include "common/utils.h"` | Inline macros (UNUSED, MAX, MIN) |
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
| `schedulerInit(cfTasks, TASK_COUNT)` | Initialize scheduler with task array and count, enable TASK_SYSTEM |
| `scheduler()` | Run one scheduler cycle (call from loop) |
| `setTaskEnabled(taskId, bool)` | Enable/disable a task |
| `rescheduleTask(taskId, periodUs)` | Change task period at runtime |
| `getTaskInfo(taskId, &info)` | Get task statistics |
| `getTaskDeltaTime(taskId)` | Get time since last execution |
| `schedulerResetTaskStatistics(taskId)` | Reset task statistics |
| `getCheckFuncInfo(&info)` | Get checkFunc statistics (for event-driven tasks) |

### Priority Levels

| Priority | Value | Description |
|----------|-------|-------------|
| `TASK_PRIORITY_IDLE` | 0 | Only runs if nothing else ready |
| `TASK_PRIORITY_LOW` | 1 | Background tasks |
| `TASK_PRIORITY_MEDIUM` | 3 | Normal tasks |
| `TASK_PRIORITY_MEDIUM_HIGH` | 4 | Above normal |
| `TASK_PRIORITY_HIGH` | 5 | Time-sensitive |
| `TASK_PRIORITY_REALTIME` | 18 | **Forced execution when overdue** |

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

Override this weak function for operations during idle time:

```cpp
void taskRunRealtimeCallbacks(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);
    // Poll serial RX, manage DMA buffers, etc.
}
```

Called when no task is selected or after forced REALTIME execution.

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
