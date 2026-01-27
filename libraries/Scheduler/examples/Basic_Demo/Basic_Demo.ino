/**
 * @file Basic_Demo.ino
 * @brief INav cooperative scheduler demo for Arduino
 *
 * Demonstrates the INav scheduler library with:
 * - Forced REALTIME task execution (1000 Hz gyro task)
 * - Dynamic priority aging to prevent task starvation
 * - System load monitoring
 * - Multiple priority levels
 *
 * Hardware: Any STM32 board (tested on NUCLEO_F411RE)
 * Output: Serial Monitor at 115200 baud
 *
 * Based on INav flight controller scheduler.
 */

// Step 1: Include task_list.h BEFORE Scheduler.h
#include "task_list.h"
#include <Scheduler.h>

//=============================================================================
// Task Implementations
//=============================================================================

static volatile uint32_t gyroCount = 0;
static volatile uint32_t blinkCount = 0;
static volatile uint32_t statusCount = 0;

// GYRO task - runs at 1000 Hz with REALTIME priority (forced execution)
extern "C" void taskGyro(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);
    gyroCount++;
    // In real application: read gyro, run PID, update motors
}

// BLINK task - 2 Hz LED toggle
extern "C" void taskBlink(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);
    blinkCount++;
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

// STATUS task - 1 Hz reporting
extern "C" void taskStatus(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);
    statusCount++;

    // Get task info for rate calculation
    cfTaskInfo_t gyroInfo, blinkInfo, statusInfo;
    getTaskInfo(TASK_GYRO, &gyroInfo);
    getTaskInfo(TASK_BLINK, &blinkInfo);
    getTaskInfo(TASK_STATUS, &statusInfo);

    // Calculate actual rates from delta times
    int gyroRate = gyroInfo.latestDeltaTime > 0 ? 1000000 / gyroInfo.latestDeltaTime : 0;
    int blinkRate = blinkInfo.latestDeltaTime > 0 ? 1000000 / blinkInfo.latestDeltaTime : 0;
    int statusRate = statusInfo.latestDeltaTime > 0 ? 1000000 / statusInfo.latestDeltaTime : 0;

    Serial.print("CPU:");
    Serial.print(averageSystemLoadPercent);
    Serial.print("% | GYRO:");
    Serial.print(gyroRate);
    Serial.print("Hz (cnt:");
    Serial.print(gyroCount);
    Serial.print(") | BLINK:");
    Serial.print(blinkRate);
    Serial.print("Hz (cnt:");
    Serial.print(blinkCount);
    Serial.print(") | STATUS:");
    Serial.print(statusRate);
    Serial.println("Hz");
}

//=============================================================================
// Task Configuration Table
//=============================================================================

cfTask_t cfTasks[TASK_COUNT] = {
    [TASK_SYSTEM] = SCHEDULER_TASK_SYSTEM_INIT,
    [TASK_GYRO] = {
        .taskName = "GYRO",
        .taskFunc = taskGyro,
        .desiredPeriod = TASK_PERIOD_HZ(1000),  // 1000 Hz (1ms period)
        .staticPriority = TASK_PRIORITY_REALTIME,  // Forced execution when overdue
    },
    [TASK_BLINK] = {
        .taskName = "BLINK",
        .taskFunc = taskBlink,
        .desiredPeriod = TASK_PERIOD_HZ(2),     // 2 Hz (500ms period)
        .staticPriority = TASK_PRIORITY_LOW,
    },
    [TASK_STATUS] = {
        .taskName = "STATUS",
        .taskFunc = taskStatus,
        .desiredPeriod = TASK_PERIOD_HZ(1),     // 1 Hz
        .staticPriority = TASK_PRIORITY_MEDIUM,
    },
};

//=============================================================================
// Setup and Loop
//=============================================================================

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);  // Wait for Serial (with timeout)

    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println();
    Serial.println("=== INav Scheduler Demo ===");
    Serial.println("Features:");
    Serial.println("  - REALTIME priority tasks (forced execution)");
    Serial.println("  - Dynamic priority aging");
    Serial.println("  - System load monitoring");
    Serial.println();
    Serial.println("Tasks:");
    Serial.println("  GYRO:   1000 Hz (REALTIME)");
    Serial.println("  BLINK:     2 Hz (LOW)");
    Serial.println("  STATUS:    1 Hz (MEDIUM)");
    Serial.println();

    // Initialize scheduler (enables TASK_SYSTEM automatically)
    schedulerInit();

    // Enable user tasks
    setTaskEnabled(TASK_GYRO, true);
    setTaskEnabled(TASK_BLINK, true);
    setTaskEnabled(TASK_STATUS, true);

    Serial.println("Scheduler running...");
    Serial.println();
}

void loop() {
    // Run the cooperative scheduler
    scheduler();
}
