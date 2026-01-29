/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Attribution:
 *   Original source: INav Flight Controller (https://github.com/iNavFlight/inav)
 *   File: src/main/scheduler/scheduler.h
 *   Version: 9.0.0 (January 2025)
 *
 * ARDUINO: Changes from INav original (10 locations):
 *   1. Line ~35: Replace common/time.h with inline type definitions (timeDelta_t, timeUs_t, timeMs_t)
 *   2. Line ~51: Add SCHEDULER_DELAY_LIMIT constant (from INav config)
 *   3. Line ~55: Add SCHEDULER_MAX_TASKS for library array sizing (TASK_COUNT unavailable at compile)
 *   4. Line ~62: Add utility macros UNUSED, MAX, MIN (from INav common headers)
 *   5. Line ~108: Add #ifndef SCHEDULER_TASK_LIST_DEFINED guard for user task enum
 *   6. Line ~115: Define TASK_NONE/TASK_SELF as macros (fixed values 254/255 for library compatibility)
 *   7. Line ~140: Replace extern cfTasks[TASK_COUNT] with pointer passed to schedulerInit()
 *   8. Line ~144: Add extern "C" guards for C++ compatibility
 *   9. Line ~158: schedulerInit() accepts tasks array pointer and taskCount (INav uses void parameter)
 *  10. Line ~173: Add SCHEDULER_TASK_SYSTEM_INIT convenience macro
 */

#pragma once

// ARDUINO: Replace INav common/time.h with inline definitions
// #include "common/time.h"
#include <stdbool.h>
#include <stdint.h>

//=============================================================================
// ARDUINO: Time types (from INav common/time.h)
//=============================================================================
typedef int32_t timeDelta_t;      // time difference, signed (overflows ~35 min)
typedef uint64_t timeUs_t;        // microsecond time, 64-bit
typedef uint32_t timeMs_t;        // millisecond time
#define TIMEUS_MAX UINT64_MAX

//=============================================================================
// ARDUINO: Configuration constants
//=============================================================================
#define SCHEDULER_DELAY_LIMIT 10   // 10us minimum period (100 kHz max)

// Maximum tasks supported (for queue array sizing in library)
// Library is compiled before sketch, so TASK_COUNT is not available
#ifndef SCHEDULER_MAX_TASKS
#define SCHEDULER_MAX_TASKS 16
#endif

//=============================================================================
// ARDUINO: Utility macros (from INav common headers)
//=============================================================================
#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

//=============================================================================
// INav Scheduler Types and API
//=============================================================================

typedef enum {
    TASK_PRIORITY_IDLE = 0,     // Disables dynamic scheduling, task is executed only if no other task is active this cycle
    TASK_PRIORITY_LOW = 1,
    TASK_PRIORITY_MEDIUM = 3,
    TASK_PRIORITY_MEDIUM_HIGH = 4,
    TASK_PRIORITY_HIGH = 5,
    TASK_PRIORITY_REALTIME = 18,
    TASK_PRIORITY_MAX = 255
} cfTaskPriority_e;

typedef struct {
    timeUs_t     maxExecutionTime;
    timeUs_t     totalExecutionTime;
    timeUs_t     averageExecutionTime;
} cfCheckFuncInfo_t;

typedef struct {
    const char * taskName;
    bool         isEnabled;
    uint8_t      staticPriority;
    timeDelta_t     desiredPeriod;
    timeUs_t     maxExecutionTime;
    timeUs_t     totalExecutionTime;
    timeUs_t     averageExecutionTime;
    timeDelta_t     latestDeltaTime;
} cfTaskInfo_t;

// ARDUINO: User defines cfTaskId_e in their own task_list.h header
// User enum only needs: TASK_SYSTEM=0, user tasks, TASK_COUNT
// TASK_NONE and TASK_SELF are defined as macros below (not in user enum)
#ifndef SCHEDULER_TASK_LIST_DEFINED
typedef enum {
    TASK_SYSTEM = 0,
    TASK_COUNT
} cfTaskId_e;
#endif

// ARDUINO: Service task IDs as macros (fixed values for library/sketch compatibility)
#define TASK_NONE  254
#define TASK_SELF  255

typedef struct {
    /* Configuration */
    const char * taskName;
    bool (*checkFunc)(timeUs_t currentTimeUs, timeDelta_t currentDeltaTimeUs);
    void (*taskFunc)(timeUs_t currentTimeUs);
    timeDelta_t desiredPeriod;         // target period of execution
    const uint8_t staticPriority;   // dynamicPriority grows in steps of this size, shouldn't be zero

    /* Scheduling */
    uint16_t dynamicPriority;       // measurement of how old task was last executed, used to avoid task starvation
    uint16_t taskAgeCycles;
    timeUs_t lastExecutedAt;        // last time of invocation
    timeUs_t lastSignaledAt;        // time of invocation event for event-driven tasks
    timeDelta_t taskLatestDeltaTime;

    /* Statistics */
    timeUs_t movingSumExecutionTime;  // moving sum over 32 samples
    timeUs_t maxExecutionTime;
    timeUs_t totalExecutionTime;    // total time consumed by task since boot
} cfTask_t;

// ARDUINO: Task array passed to schedulerInit() and stored internally
// INav uses: extern cfTask_t cfTasks[TASK_COUNT] resolved at link time
// Arduino:   User defines cfTasks[] in sketch, passes to schedulerInit(cfTasks, TASK_COUNT)
extern uint16_t averageSystemLoadPercent;

// ARDUINO: C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

void getCheckFuncInfo(cfCheckFuncInfo_t *checkFuncInfo);
void getTaskInfo(cfTaskId_e taskId, cfTaskInfo_t *taskInfo);
void rescheduleTask(cfTaskId_e taskId, timeDelta_t newPeriodUs);
void setTaskEnabled(cfTaskId_e taskId, bool newEnabledState);
timeDelta_t getTaskDeltaTime(cfTaskId_e taskId);
void schedulerResetTaskStatistics(cfTaskId_e taskId);

// ARDUINO: INav uses schedulerInit(void) with global cfTasks[]; we accept array and count
// Returns false if taskCount > SCHEDULER_MAX_TASKS (queue cannot hold all tasks)
bool schedulerInit(cfTask_t* tasks, uint8_t taskCount);
void scheduler(void);
void taskSystem(timeUs_t currentTimeUs);
void taskRunRealtimeCallbacks(timeUs_t currentTimeUs);

#ifdef __cplusplus
}
#endif

#define TASK_PERIOD_HZ(hz) (1000000 / (hz))
#define TASK_PERIOD_MS(ms) ((ms) * 1000)
#define TASK_PERIOD_US(us) (us)

#define isSystemOverloaded() (averageSystemLoadPercent >= 100)

// ARDUINO: Convenience macro for TASK_SYSTEM initialization
#define SCHEDULER_TASK_SYSTEM_INIT { \
    .taskName = "SYSTEM", \
    .taskFunc = taskSystem, \
    .desiredPeriod = TASK_PERIOD_HZ(10), \
    .staticPriority = TASK_PRIORITY_MEDIUM_HIGH \
}
