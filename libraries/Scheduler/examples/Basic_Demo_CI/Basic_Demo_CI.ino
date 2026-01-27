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
 *
 * Usage:
 * - Arduino IDE: Open Serial Monitor at 115200 baud
 * - CI/RTT: ./ci/aflash.sh Arduino_Core_STM32/libraries/Scheduler/examples/Basic_Demo --use-rtt --build-id
 *
 * Based on INav flight controller scheduler.
 */

// Step 1: Include task_list.h BEFORE Scheduler.h
#include "task_list.h"
#include <Scheduler.h>

// CI logging support (works with both Serial and RTT)
#ifdef USE_RTT
  #include <SEGGER_RTT.h>
  #define LOG_INIT()        SEGGER_RTT_Init()
  #define LOG(s)            SEGGER_RTT_WriteString(0, s)
  #define LOGF(fmt, ...)    SEGGER_RTT_printf(0, fmt, ##__VA_ARGS__)
#else
  #define LOG_INIT()        do { Serial.begin(115200); while (!Serial && millis() < 3000); } while(0)
  #define LOG(s)            Serial.print(s)
  #define LOGF(fmt, ...)    Serial.printf(fmt, ##__VA_ARGS__)
#endif

//=============================================================================
// Step 2: Implement task functions
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

    LOGF("CPU:%u%% | GYRO:%uHz (cnt:%lu) | BLINK:%uHz (cnt:%lu) | STATUS:%uHz\n",
         averageSystemLoadPercent, gyroRate, gyroCount, blinkRate, blinkCount, statusRate);
}

//=============================================================================
// Step 3: Define task configuration table
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
// Optional: Override realtime callbacks
//=============================================================================

// Called during scheduler idle time or after REALTIME task execution
// Use for time-critical polling that doesn't fit the task model
void taskRunRealtimeCallbacks(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);
    // Example uses:
    // - Poll serial RX for RC receiver data
    // - Check DMA completion flags
    // - Service hardware FIFOs
}

//=============================================================================
// Step 4: Setup and loop
//=============================================================================

void setup() {
    // Initialize logging (Serial or RTT)
    LOG_INIT();

    // Initialize LED
    pinMode(LED_BUILTIN, OUTPUT);

    LOG("\n=== INav Scheduler Demo ===\n");
    LOG("Features:\n");
    LOG("  - REALTIME priority tasks (forced execution)\n");
    LOG("  - Dynamic priority aging\n");
    LOG("  - System load monitoring\n\n");

    LOG("Tasks:\n");
    LOGF("  GYRO:   %4d Hz (REALTIME)\n", 1000000 / TASK_PERIOD_HZ(1000));
    LOGF("  BLINK:  %4d Hz (LOW)\n", 1000000 / TASK_PERIOD_HZ(2));
    LOGF("  STATUS: %4d Hz (MEDIUM)\n", 1000000 / TASK_PERIOD_HZ(1));
    LOG("\n");

    // Initialize scheduler (enables TASK_SYSTEM automatically)
    schedulerInit();

    // Enable user tasks
    setTaskEnabled(TASK_GYRO, true);
    setTaskEnabled(TASK_BLINK, true);
    setTaskEnabled(TASK_STATUS, true);

    LOG("Scheduler running...\n\n");
}

void loop() {
    // Run the cooperative scheduler
    scheduler();

#ifdef USE_RTT
    // For CI testing: exit after 5 seconds
    static bool stopped = false;
    if (!stopped && millis() > 5000) {
        LOG("\n=== Demo Complete ===\n");
        LOGF("GYRO executions: %lu (expected ~5000)\n", gyroCount);
        LOGF("BLINK executions: %lu (expected ~10)\n", blinkCount);
        LOGF("STATUS executions: %lu (expected ~5)\n", statusCount);
        LOG("*STOP*\n");
        stopped = true;
        while (1);
    }
#endif
}
