/**
 * @file Sched_Test.ino
 * @brief Scheduler library validation test
 *
 * Tests the INav cooperative scheduler with validation:
 * - REALTIME task runs at ~1000 Hz (validated)
 * - LOW priority task runs at ~2 Hz
 * - MEDIUM priority task runs at ~1 Hz
 * - System load monitoring
 *
 * Hardware: Any STM32 board (tested on NUCLEO_F411RE)
 * Output: Serial at 115200 baud
 *
 * Based on INav flight controller scheduler.
 */

#include "task_list.h"
#include <scheduler.h>

//=============================================================================
// Test Configuration
//=============================================================================
const uint32_t TEST_DURATION_MS = 5000;  // 5 second test
const int GYRO_MIN_HZ = 950;             // Minimum acceptable GYRO rate
const int GYRO_MAX_HZ = 1050;            // Maximum acceptable GYRO rate

//=============================================================================
// Task Counters
//=============================================================================
static volatile uint32_t gyroCount = 0;
static volatile uint32_t blinkCount = 0;
static volatile uint32_t statusCount = 0;

//=============================================================================
// Task Implementations
//=============================================================================

// GYRO task - 1000 Hz REALTIME priority
extern "C" void taskGyro(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);
    gyroCount++;
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

    cfTaskInfo_t gyroInfo;
    getTaskInfo(TASK_GYRO, &gyroInfo);
    int gyroRate = gyroInfo.latestDeltaTime > 0 ? 1000000 / gyroInfo.latestDeltaTime : 0;

    Serial.printf("CPU:%u%% | GYRO:%dHz (cnt:%lu) | BLINK:%lu | STATUS:%lu\n",
                  averageSystemLoadPercent, gyroRate, gyroCount, blinkCount, statusCount);
}

//=============================================================================
// Task Configuration Table
//=============================================================================

cfTask_t cfTasks[TASK_COUNT] = {
    [TASK_SYSTEM] = SCHEDULER_TASK_SYSTEM_INIT,
    [TASK_GYRO] = {
        .taskName = "GYRO",
        .taskFunc = taskGyro,
        .desiredPeriod = TASK_PERIOD_HZ(1000),
        .staticPriority = TASK_PRIORITY_REALTIME,
    },
    [TASK_BLINK] = {
        .taskName = "BLINK",
        .taskFunc = taskBlink,
        .desiredPeriod = TASK_PERIOD_HZ(2),
        .staticPriority = TASK_PRIORITY_LOW,
    },
    [TASK_STATUS] = {
        .taskName = "STATUS",
        .taskFunc = taskStatus,
        .desiredPeriod = TASK_PERIOD_HZ(1),
        .staticPriority = TASK_PRIORITY_MEDIUM,
    },
};

//=============================================================================
// Setup and Loop
//=============================================================================

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println("\n=== Scheduler Library Test ===");
    Serial.printf("Test duration: %lu ms\n", TEST_DURATION_MS);
    Serial.printf("GYRO target: 1000 Hz (valid: %d-%d Hz)\n\n", GYRO_MIN_HZ, GYRO_MAX_HZ);

    if (!schedulerInit(cfTasks, TASK_COUNT)) {
        Serial.println("ERROR: TASK_COUNT exceeds SCHEDULER_MAX_TASKS");
        Serial.println("*STOP*");
        while (1);
    }

    setTaskEnabled(TASK_GYRO, true);
    setTaskEnabled(TASK_BLINK, true);
    setTaskEnabled(TASK_STATUS, true);

    Serial.println("Scheduler running...\n");
}

void loop() {
    scheduler();

    // Test completion after TEST_DURATION_MS
    if (millis() > TEST_DURATION_MS) {
        // Calculate final GYRO rate
        cfTaskInfo_t gyroInfo;
        getTaskInfo(TASK_GYRO, &gyroInfo);
        int gyroRate = gyroInfo.latestDeltaTime > 0 ? 1000000 / gyroInfo.latestDeltaTime : 0;

        // Validate results
        Serial.println("\n=== Test Results ===");
        Serial.printf("GYRO: %d Hz (expected ~1000)\n", gyroRate);
        Serial.printf("GYRO executions: %lu (expected ~%lu)\n", gyroCount, TEST_DURATION_MS);
        Serial.printf("BLINK executions: %lu (expected ~%lu)\n", blinkCount, TEST_DURATION_MS * 2 / 1000);
        Serial.printf("STATUS executions: %lu (expected ~%lu)\n", statusCount, TEST_DURATION_MS / 1000);

        bool pass = (gyroRate >= GYRO_MIN_HZ && gyroRate <= GYRO_MAX_HZ);
        Serial.printf("\nGYRO rate validation: %s\n", pass ? "PASS" : "FAIL");

        Serial.println("*STOP*");
        while (1);
    }
}
