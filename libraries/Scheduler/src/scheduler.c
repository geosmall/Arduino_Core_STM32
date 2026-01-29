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
 *   File: src/main/scheduler/scheduler.c
 *   Version: 9.0.0 (January 2025)
 *   INav forked scheduler from Cleanflight/Betaflight
 *
 * ARDUINO: Changes from INav original (11 locations):
 *   1. Line ~38: Replace platform.h with Arduino.h
 *   2. Line ~44: Comment out INav-specific includes (build_config, debug, maths, time, utils, drivers/time)
 *   3. Line ~52: Define INav memory section macros (STATIC_FASTRAM, FASTRAM, FAST_CODE, NOINLINE)
 *   4. Line ~58: Define unit test macros (STATIC_UNIT_TESTED, STATIC_INLINE_UNIT_TESTED)
 *   5. Line ~62: Add weak taskRunRealtimeCallbacks() default implementation
 *   6. Line ~70: Add cfTasksPtr to store task array passed to schedulerInit (INav uses global cfTasks[])
 *   7. Line ~74: Add schedulerTaskCount for runtime bounds checking (INav uses compile-time TASK_COUNT)
 *   8. Line ~87: Use SCHEDULER_MAX_TASKS for array sizing (TASK_COUNT unavailable at library compile)
 *   9. Line ~187-243: Access tasks via cfTasksPtr[] (INav uses global cfTasks[])
 *  10. Line ~248: schedulerInit() accepts tasks pointer and count (INav uses void parameter)
 *  11. Line ~258: Use micros64() for 64-bit timestamps (INav uses micros() with USE_64BIT_TIME)
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// ARDUINO: Use Arduino platform
// #include "platform.h"
#include "Arduino.h"

#include "scheduler.h"

// ARDUINO: INav-specific includes not needed - definitions provided in scheduler.h
// #include "build/build_config.h"
// #include "build/debug.h"
// #include "common/maths.h"
// #include "common/time.h"
// #include "common/utils.h"
// #include "drivers/time.h"

// ARDUINO: Define INav memory section macros as empty for Arduino
#define STATIC_FASTRAM static
#define FASTRAM
#define FAST_CODE
#define NOINLINE

// ARDUINO: Define unit test macros as standard static
#define STATIC_UNIT_TESTED static
#define STATIC_INLINE_UNIT_TESTED static inline

// ARDUINO: Provide weak default implementation for optional user override
__attribute__((weak)) void taskRunRealtimeCallbacks(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);
}

STATIC_FASTRAM cfTask_t *currentTask = NULL;

// ARDUINO: Task array pointer - INav uses global cfTasks[], we store pointer from schedulerInit()
STATIC_FASTRAM cfTask_t *cfTasksPtr = NULL;

// ARDUINO: Runtime task count - INav uses compile-time TASK_COUNT, we store from schedulerInit()
STATIC_FASTRAM uint8_t schedulerTaskCount = 0;

STATIC_FASTRAM uint32_t totalWaitingTasks;
STATIC_FASTRAM uint32_t totalWaitingTasksSamples;

FASTRAM uint16_t averageSystemLoadPercent = 0;


STATIC_FASTRAM int taskQueuePos = 0;
STATIC_FASTRAM int taskQueueSize = 0;
// No need for a linked list for the queue, since items are only inserted at startup
// ARDUINO: Use SCHEDULER_MAX_TASKS for array sizing (TASK_COUNT not available at library compile time)
#ifdef UNIT_TEST
STATIC_FASTRAM_UNIT_TESTED cfTask_t* taskQueueArray[SCHEDULER_MAX_TASKS + 2]; // 1 extra space so test code can check for buffer overruns
#else
STATIC_FASTRAM cfTask_t* taskQueueArray[SCHEDULER_MAX_TASKS + 1]; // extra item for NULL pointer at end of queue
#endif
STATIC_UNIT_TESTED void queueClear(void)
{
    memset(taskQueueArray, 0, sizeof(taskQueueArray));
    taskQueuePos = 0;
    taskQueueSize = 0;
}

#ifdef UNIT_TEST
STATIC_UNIT_TESTED int queueSize(void)
{
    return taskQueueSize;
}
#endif

STATIC_UNIT_TESTED bool queueContains(cfTask_t *task)
{
    for (int ii = 0; ii < taskQueueSize; ++ii) {
        if (taskQueueArray[ii] == task) {
            return true;
        }
    }
    return false;
}

STATIC_UNIT_TESTED bool queueAdd(cfTask_t *task)
{
    // ARDUINO: Use SCHEDULER_MAX_TASKS for bounds check (TASK_COUNT not available at library compile time)
    if ((taskQueueSize >= SCHEDULER_MAX_TASKS) || queueContains(task)) {
        return false;
    }
    for (int ii = 0; ii <= taskQueueSize; ++ii) {
        if (taskQueueArray[ii] == NULL || taskQueueArray[ii]->staticPriority < task->staticPriority) {
            memmove(&taskQueueArray[ii+1], &taskQueueArray[ii], sizeof(task) * (taskQueueSize - ii));
            taskQueueArray[ii] = task;
            ++taskQueueSize;
            return true;
        }
    }
    return false;
}

STATIC_UNIT_TESTED bool queueRemove(cfTask_t *task)
{
    for (int ii = 0; ii < taskQueueSize; ++ii) {
        if (taskQueueArray[ii] == task) {
            memmove(&taskQueueArray[ii], &taskQueueArray[ii+1], sizeof(task) * (taskQueueSize - ii));
            --taskQueueSize;
            return true;
        }
    }
    return false;
}

/*
 * Returns first item queue or NULL if queue empty
 */
STATIC_INLINE_UNIT_TESTED cfTask_t *queueFirst(void)
{
    taskQueuePos = 0;
    return taskQueueArray[0]; // guaranteed to be NULL if queue is empty
}

/*
 * Returns next item in queue or NULL if at end of queue
 */
STATIC_INLINE_UNIT_TESTED cfTask_t *queueNext(void)
{
    return taskQueueArray[++taskQueuePos]; // guaranteed to be NULL at end of queue
}

void taskSystem(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    // Calculate system load
    if (totalWaitingTasksSamples > 0) {
        averageSystemLoadPercent = 100 * totalWaitingTasks / totalWaitingTasksSamples;
        totalWaitingTasksSamples = 0;
        totalWaitingTasks = 0;
    }
}

#define TASK_MOVING_SUM_COUNT           32
FASTRAM timeUs_t checkFuncMaxExecutionTime;
FASTRAM timeUs_t checkFuncTotalExecutionTime;
FASTRAM timeUs_t checkFuncMovingSumExecutionTime;

void getCheckFuncInfo(cfCheckFuncInfo_t *checkFuncInfo)
{
    checkFuncInfo->maxExecutionTime = checkFuncMaxExecutionTime;
    checkFuncInfo->totalExecutionTime = checkFuncTotalExecutionTime;
    checkFuncInfo->averageExecutionTime = checkFuncMovingSumExecutionTime / TASK_MOVING_SUM_COUNT;
}

void getTaskInfo(cfTaskId_e taskId, cfTaskInfo_t * taskInfo)
{
    taskInfo->taskName = cfTasksPtr[taskId].taskName;
    taskInfo->isEnabled = queueContains(&cfTasksPtr[taskId]);
    taskInfo->desiredPeriod = cfTasksPtr[taskId].desiredPeriod;
    taskInfo->staticPriority = cfTasksPtr[taskId].staticPriority;
    taskInfo->maxExecutionTime = cfTasksPtr[taskId].maxExecutionTime;
    taskInfo->totalExecutionTime = cfTasksPtr[taskId].totalExecutionTime;
    taskInfo->averageExecutionTime = cfTasksPtr[taskId].movingSumExecutionTime / TASK_MOVING_SUM_COUNT;
    taskInfo->latestDeltaTime = cfTasksPtr[taskId].taskLatestDeltaTime;
}

void rescheduleTask(cfTaskId_e taskId, timeDelta_t newPeriodUs)
{
    if (taskId == TASK_SELF) {
        cfTask_t *task = currentTask;
        task->desiredPeriod = MAX(SCHEDULER_DELAY_LIMIT, newPeriodUs);  // Limit delay to 10us (100 kHz) to prevent scheduler clogging
    } else if (taskId < schedulerTaskCount) {
        cfTask_t *task = &cfTasksPtr[taskId];
        task->desiredPeriod = MAX(SCHEDULER_DELAY_LIMIT, newPeriodUs);  // Limit delay to 10us (100 kHz) to prevent scheduler clogging
    }
}

void setTaskEnabled(cfTaskId_e taskId, bool enabled)
{
    if (taskId == TASK_SELF || taskId < schedulerTaskCount) {
        cfTask_t *task = taskId == TASK_SELF ? currentTask : &cfTasksPtr[taskId];
        if (enabled && task->taskFunc) {
            queueAdd(task);
        } else {
            queueRemove(task);
        }
    }
}

timeDelta_t getTaskDeltaTime(cfTaskId_e taskId)
{
    if (taskId == TASK_SELF) {
        return currentTask->taskLatestDeltaTime;
    } else if (taskId < schedulerTaskCount) {
        return cfTasksPtr[taskId].taskLatestDeltaTime;
    } else {
        return 0;
    }
}

void schedulerResetTaskStatistics(cfTaskId_e taskId)
{
    if (taskId == TASK_SELF) {
        currentTask->movingSumExecutionTime = 0;
        currentTask->totalExecutionTime = 0;
        currentTask->maxExecutionTime = 0;
    } else if (taskId < schedulerTaskCount) {
        cfTasksPtr[taskId].movingSumExecutionTime = 0;
        cfTasksPtr[taskId].totalExecutionTime = 0;
    }
}

// ARDUINO: INav uses schedulerInit(void) with global cfTasks[] and compile-time TASK_COUNT
// Arduino passes task array and count at runtime (library compiles before sketch defines TASK_COUNT)
void schedulerInit(cfTask_t* tasks, uint8_t taskCount)
{
    cfTasksPtr = tasks;
    schedulerTaskCount = taskCount;
    queueClear();
    queueAdd(&cfTasksPtr[TASK_SYSTEM]);
}

void FAST_CODE NOINLINE scheduler(void)
{
    // Cache currentTime (64-bit for overflow-safe timestamps)
    const timeUs_t currentTimeUs = micros64();

    // The task to be invoked
    cfTask_t *selectedTask = NULL;
    uint16_t selectedTaskDynamicPriority = 0;
    bool forcedRealTimeTask = false;

    // Update task dynamic priorities
    uint16_t waitingTasks = 0;
    for (cfTask_t *task = queueFirst(); task != NULL; task = queueNext()) {
        // Task has checkFunc - event driven
        if (task->checkFunc) {
            const timeUs_t currentTimeBeforeCheckFuncCallUs = micros();

            // Increase priority for event driven tasks
            if (task->dynamicPriority > 0) {
                task->taskAgeCycles = 1 + ((timeDelta_t)(currentTimeUs - task->lastSignaledAt)) / task->desiredPeriod;
                task->dynamicPriority = 1 + task->staticPriority * task->taskAgeCycles;
                waitingTasks++;
            } else if (task->checkFunc(currentTimeBeforeCheckFuncCallUs, currentTimeBeforeCheckFuncCallUs - task->lastExecutedAt)) {
                const timeUs_t checkFuncExecutionTime = micros() - currentTimeBeforeCheckFuncCallUs;
                checkFuncMovingSumExecutionTime -= checkFuncMovingSumExecutionTime / TASK_MOVING_SUM_COUNT;
                checkFuncMovingSumExecutionTime += checkFuncExecutionTime;
                checkFuncTotalExecutionTime += checkFuncExecutionTime;   // time consumed by scheduler + task
                checkFuncMaxExecutionTime = MAX(checkFuncMaxExecutionTime, checkFuncExecutionTime);
                task->lastSignaledAt = currentTimeBeforeCheckFuncCallUs;
                task->taskAgeCycles = 1;
                task->dynamicPriority = 1 + task->staticPriority;
                waitingTasks++;
            } else {
                task->taskAgeCycles = 0;
            }
        } else if (task->staticPriority == TASK_PRIORITY_REALTIME) {
            //realtime tasks take absolute priority. Any RT tasks that is overdue, should be execute immediately
            if (((timeDelta_t)(currentTimeUs - task->lastExecutedAt)) > task->desiredPeriod) {
                selectedTaskDynamicPriority = task->dynamicPriority;
                selectedTask = task;
                waitingTasks++;
                forcedRealTimeTask = true;
            }
        } else {
            // Task is time-driven, dynamicPriority is last execution age (measured in desiredPeriods)
            // Task age is calculated from last execution
            task->taskAgeCycles = ((timeDelta_t)(currentTimeUs - task->lastExecutedAt)) / task->desiredPeriod;
            if (task->taskAgeCycles > 0) {
                task->dynamicPriority = 1 + task->staticPriority * task->taskAgeCycles;
                waitingTasks++;
            }
        }

        if (!forcedRealTimeTask && task->dynamicPriority > selectedTaskDynamicPriority) {
            selectedTaskDynamicPriority = task->dynamicPriority;
            selectedTask = task;
        }
    }

    totalWaitingTasksSamples++;
    totalWaitingTasks += waitingTasks;

    currentTask = selectedTask;

    if (selectedTask) {
        // Found a task that should be run
        selectedTask->taskLatestDeltaTime = (timeDelta_t)(currentTimeUs - selectedTask->lastExecutedAt);
        selectedTask->lastExecutedAt = currentTimeUs;
        selectedTask->dynamicPriority = 0;

        // Execute task
        const timeUs_t currentTimeBeforeTaskCall = micros();
        selectedTask->taskFunc(currentTimeBeforeTaskCall);
        const timeUs_t taskExecutionTime = micros() - currentTimeBeforeTaskCall;
        selectedTask->movingSumExecutionTime += taskExecutionTime - selectedTask->movingSumExecutionTime / TASK_MOVING_SUM_COUNT;
        selectedTask->totalExecutionTime += taskExecutionTime;   // time consumed by scheduler + task
        selectedTask->maxExecutionTime = MAX(selectedTask->maxExecutionTime, taskExecutionTime);
    }

    if (!selectedTask || forcedRealTimeTask) {
        // Execute system real-time callbacks and account for them to SYSTEM account
        const timeUs_t currentTimeBeforeTaskCall = micros();
        taskRunRealtimeCallbacks(currentTimeBeforeTaskCall);
        selectedTask = &cfTasksPtr[TASK_SYSTEM];
        const timeUs_t taskExecutionTime = micros() - currentTimeBeforeTaskCall;
        selectedTask->movingSumExecutionTime += taskExecutionTime - selectedTask->movingSumExecutionTime / TASK_MOVING_SUM_COUNT;
        selectedTask->totalExecutionTime += taskExecutionTime;   // time consumed by scheduler + task
        selectedTask->maxExecutionTime = MAX(selectedTask->maxExecutionTime, taskExecutionTime);
    }
}
