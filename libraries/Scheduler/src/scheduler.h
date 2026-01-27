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
 * ARDUINO: Changes from INav original (5 locations):
 *   1. Line ~33: Include scheduler_config.h instead of common/time.h
 *   2. Line ~68: Add #ifndef SCHEDULER_TASK_LIST_DEFINED guard for user task enum
 *   3. Line ~75: Define TASK_NONE/TASK_SELF as macros (not enum values)
 *   4. Line ~97: Use unsized cfTasks[] declaration for variable TASK_COUNT
 *   5. Line ~102: Add extern "C" guards for C++ compatibility
 */

#pragma once

// ARDUINO: Platform adaptations in scheduler_config.h
#include "scheduler_config.h"
// #include "common/time.h"  // ARDUINO: Replaced by scheduler_config.h

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

// ARDUINO: Service task IDs as macros (not enum values)
// This avoids library/sketch enum value mismatch issues
#define TASK_NONE  SCHEDULER_TASK_NONE
#define TASK_SELF  SCHEDULER_TASK_SELF

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

// ARDUINO: Use unsized array declaration so library works with any user-defined TASK_COUNT
extern cfTask_t cfTasks[];
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

void schedulerInit(void);
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
