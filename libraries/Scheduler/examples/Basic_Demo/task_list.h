/**
 * @file task_list.h
 * @brief Task definitions for Basic_Demo sketch
 *
 * This file defines the cfTaskId_e enum with your application's tasks.
 * Include this file BEFORE <Scheduler.h> in your sketch.
 *
 * Requirements:
 * - Must define SCHEDULER_TASK_LIST_DEFINED before the enum
 * - Enum must start with TASK_SYSTEM = 0
 * - Enum must end with TASK_COUNT
 * - Add your tasks between TASK_SYSTEM and TASK_COUNT
 *
 * Note: TASK_NONE and TASK_SELF are provided by the library as macros.
 *
 * Copy this file to your sketch folder and modify the task list as needed.
 */

#ifndef TASK_LIST_H
#define TASK_LIST_H

// Tell scheduler.h that we're providing the task enum
#define SCHEDULER_TASK_LIST_DEFINED

typedef enum {
    // System task (always required, must be first)
    TASK_SYSTEM = 0,

    //=========================================================================
    // User tasks - add your tasks here
    //=========================================================================
    TASK_GYRO,      // High-frequency control loop (REALTIME)
    TASK_BLINK,     // LED blink (LOW priority)
    TASK_STATUS,    // Status reporting (MEDIUM priority)
    //=========================================================================

    // Task count (always required, must be last)
    TASK_COUNT
} cfTaskId_e;

#endif // TASK_LIST_H
