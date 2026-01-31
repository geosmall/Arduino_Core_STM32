/*
 * Scheduler Example - Task List Definition
 *
 * This file defines the task IDs for the scheduler.
 * Must be included BEFORE <scheduler.h>
 */

#ifndef TASK_LIST_H
#define TASK_LIST_H

#define SCHEDULER_TASK_LIST_DEFINED

typedef enum {
    TASK_SYSTEM = 0,    // Required: System housekeeping

    // Application tasks (add your tasks here)
    TASK_GYRO,          // REALTIME: IMU sampling at 1 kHz
    TASK_RX,            // HIGH: RC receiver input
    TASK_FLIGHT,        // HIGH: Flight control (uses gyro + RC data)
    TASK_SERIAL,        // LOW: Serial output for debugging

    TASK_COUNT          // Required: Must be last
} cfTaskId_e;

#endif // TASK_LIST_H
