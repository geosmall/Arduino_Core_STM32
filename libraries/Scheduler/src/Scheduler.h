/**
 * @file Scheduler.h
 * @brief Arduino wrapper for INav cooperative task scheduler
 *
 * This library provides a priority-based cooperative task scheduler ported from
 * the INav flight controller. It features forced REALTIME execution for
 * time-critical tasks, dynamic priority aging, and system load monitoring.
 *
 * @section usage Usage
 *
 * 1. Create task_list.h in your sketch folder:
 * @code
 * #ifndef TASK_LIST_H
 * #define TASK_LIST_H
 *
 * #define SCHEDULER_TASK_LIST_DEFINED
 *
 * typedef enum {
 *     TASK_SYSTEM = 0,
 *     TASK_FLIGHT,      // Your tasks here
 *     TASK_TELEMETRY,
 *     TASK_COUNT,
 *     TASK_NONE = TASK_COUNT,
 *     TASK_SELF
 * } cfTaskId_e;
 *
 * #endif
 * @endcode
 *
 * 2. Include task_list.h BEFORE Scheduler.h in your sketch:
 * @code
 * #include "task_list.h"
 * #include <Scheduler.h>
 * @endcode
 *
 * 3. Implement task functions:
 * @code
 * extern "C" void taskFlight(timeUs_t currentTimeUs) {
 *     UNUSED(currentTimeUs);
 *     // Flight control code
 * }
 * @endcode
 *
 * 4. Define task configuration array:
 * @code
 * cfTask_t cfTasks[TASK_COUNT] = {
 *     [TASK_SYSTEM] = SCHEDULER_TASK_SYSTEM_INIT,
 *     [TASK_FLIGHT] = {
 *         .taskName = "FLIGHT",
 *         .taskFunc = taskFlight,
 *         .desiredPeriod = TASK_PERIOD_HZ(1000),
 *         .staticPriority = TASK_PRIORITY_REALTIME,
 *     },
 * };
 * @endcode
 *
 * 5. Initialize and run:
 * @code
 * void setup() {
 *     schedulerInit();
 *     setTaskEnabled(TASK_FLIGHT, true);
 * }
 * void loop() {
 *     scheduler();
 * }
 * @endcode
 *
 * @section attribution Attribution
 *
 * - Original source: INav Flight Controller v9.0.0
 * - Heritage: Cleanflight/Betaflight scheduler
 * - License: GPL-3.0
 * - Arduino adaptation: GLS (January 2026)
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "scheduler.h"

/**
 * @brief Convenience macro for TASK_SYSTEM initialization
 *
 * Use this in your cfTasks array for the TASK_SYSTEM entry:
 * @code
 * cfTask_t cfTasks[TASK_COUNT] = {
 *     [TASK_SYSTEM] = SCHEDULER_TASK_SYSTEM_INIT,
 *     // ... your tasks ...
 * };
 * @endcode
 */
#define SCHEDULER_TASK_SYSTEM_INIT { \
    .taskName = "SYSTEM", \
    .taskFunc = taskSystem, \
    .desiredPeriod = TASK_PERIOD_HZ(10), \
    .staticPriority = TASK_PRIORITY_MEDIUM_HIGH \
}

#endif // SCHEDULER_H
