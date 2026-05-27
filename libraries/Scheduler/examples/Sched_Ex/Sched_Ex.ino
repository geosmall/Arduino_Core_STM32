/*
 * Scheduler Example: IMU + RC Receiver Integration
 *
 * Demonstrates the INav-based scheduler with:
 * - TASK_GYRO (REALTIME): IMU sampling at 1 kHz
 * - TASK_RX (HIGH): RC receiver input with event-driven checkFunc
 * - TASK_FLIGHT (HIGH): Flight control processing
 * - TASK_SERIAL (LOW): Debug output at 10 Hz
 *
 * Target: NUCLEO_F411RE_HIL005
 * Hardware:
 * - STM32F411RE Nucleo
 * - MPU-6000 IMU on SPI1 (PA7/PA6/PA5/PA4), INT=PB3
 * - IBus receiver on USART1 (RX=PB7)
 *
 * Board selection: STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE
 */

// Task list MUST be included before scheduler.h
#include "task_list.h"
#include <scheduler.h>

// Libraries
#include <SPI.h>
#include <IMU.h>
#include <SerialRx.h>

// BoardConfig for HIL005 rig
#include "targets/NUCLEO_F411RE_HIL005.h"

// =============================================================================
// Board Configuration (from HIL005 target)
// =============================================================================

// IMU: MPU-6000 on SPI1 (auto-detected by IMU library)
#define IMU_CS_PIN      BoardConfig::imu.spi.cs_pin
#define IMU_SPI_FREQ    BoardConfig::imu.spi.freq_hz

// RC Receiver: IBus on USART1 (Serial1) — overrides variant's default Serial1
// pins with the IBus-receiver pinout on HIL-005 (PB7=RX, PB6=TX).
HardwareSerial Serial1(USART1, PB7, PB6);
#define RC_SERIAL       Serial1
#define RC_BAUDRATE     BoardConfig::rc_receiver.baud_rate
#define RC_TIMEOUT_MS   BoardConfig::rc_receiver.timeout_ms

// =============================================================================
// Global Objects
// =============================================================================

IMU imu;
SerialRx rc(SerialRx::IBUS);

// =============================================================================
// Shared Data (written by tasks, read by flight control)
// =============================================================================

// IMU data (updated by TASK_GYRO at 1 kHz)
volatile int16_t gyroData[3] = {0, 0, 0};   // Raw gyro X, Y, Z
volatile int16_t accelData[3] = {0, 0, 0};  // Raw accel X, Y, Z
volatile bool gyroDataNew = false;

// RC data (updated by TASK_RX when frames arrive)
volatile int16_t rcChannels[RC_NUM_CHANNELS] = {1500, 1500, 1000, 1500};  // AETR defaults
volatile bool rcDataNew = false;
volatile bool rcFailsafe = false;

// Flight control output (for demonstration)
volatile int16_t motorOutput[4] = {1000, 1000, 1000, 1000};

// Statistics
volatile uint32_t gyroReadCount = 0;
volatile uint32_t rcFrameCount = 0;

// Test duration (0 = run forever)
#define TEST_DURATION_MS  5000

// Number of RC channels to display
#define RC_DISPLAY_CHANNELS  4

// =============================================================================
// TASK_GYRO: IMU Sampling (REALTIME Priority)
// =============================================================================
// Runs at 1 kHz to sample the MPU-6000 gyro and accelerometer.
// REALTIME priority ensures this task runs on schedule regardless of other load.

extern "C" void taskGyro(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);

    std::array<int16_t, 6> imuBuf;

    if (imu.ReadIMU6(imuBuf) == 0) {
        // Accel is first 3 values, Gyro is next 3
        accelData[0] = imuBuf[0];
        accelData[1] = imuBuf[1];
        accelData[2] = imuBuf[2];
        gyroData[0] = imuBuf[3];
        gyroData[1] = imuBuf[4];
        gyroData[2] = imuBuf[5];
        gyroDataNew = true;
        gyroReadCount++;
    }
}

// =============================================================================
// TASK_RX: RC Receiver (HIGH Priority, Event-Driven)
// =============================================================================
// Uses checkFunc to poll SerialRx and only run taskFunc when a frame is ready.
// HIGH priority ensures RC data is processed promptly after REALTIME tasks.

RCMessage rcMsg;

extern "C" bool taskRxCheck(timeUs_t currentTimeUs, timeDelta_t currentDeltaTime) {
    UNUSED(currentTimeUs);
    UNUSED(currentDeltaTime);

    rc.update();           // Poll serial, parse bytes
    return rc.available(); // True if complete frame ready
}

extern "C" void taskRxMain(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);

    // Drain message queue
    while (rc.getMessage(&rcMsg)) {
        // Copy channels (atomic from scheduler perspective)
        for (int i = 0; i < RC_NUM_CHANNELS; i++) {
            rcChannels[i] = rcMsg.channels[i];
        }
        rcDataNew = true;
        rcFailsafe = false;
        rcFrameCount++;
    }

    // Check for failsafe (no valid frames for RC_TIMEOUT_MS)
    if (rc.timeout(RC_TIMEOUT_MS)) {
        rcFailsafe = true;
        // Set failsafe values
        rcChannels[0] = 1500;  // Roll center
        rcChannels[1] = 1500;  // Pitch center
        rcChannels[2] = 1000;  // Throttle low
        rcChannels[3] = 1500;  // Yaw center
    }
}

// =============================================================================
// TASK_FLIGHT: Flight Control (HIGH Priority)
// =============================================================================
// Processes gyro and RC data to compute motor outputs.
// Runs at 500 Hz, after TASK_GYRO and TASK_RX have updated their data.

extern "C" void taskFlight(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);

    // Skip if in failsafe
    if (rcFailsafe) {
        motorOutput[0] = motorOutput[1] = motorOutput[2] = motorOutput[3] = 1000;
        return;
    }

    // Read latest data (safe - higher priority tasks have completed)
    int16_t roll  = rcChannels[0];
    int16_t pitch = rcChannels[1];
    int16_t thr   = rcChannels[2];
    int16_t yaw   = rcChannels[3];

    int16_t gx = gyroData[0];
    int16_t gy = gyroData[1];
    int16_t gz = gyroData[2];

    // Simple demonstration: map throttle to motor output
    // In a real flight controller, this would be PID + mixer
    int16_t baseThrottle = constrain(thr, 1000, 2000);

    // Placeholder for motor mixing (no actual PID here)
    motorOutput[0] = baseThrottle;
    motorOutput[1] = baseThrottle;
    motorOutput[2] = baseThrottle;
    motorOutput[3] = baseThrottle;

    // Mark gyro data as consumed
    gyroDataNew = false;
}

// =============================================================================
// TASK_SERIAL: Debug Output (LOW Priority)
// =============================================================================
// Prints statistics at 10 Hz. LOW priority ensures it doesn't interfere
// with time-critical tasks.

extern "C" void taskSerial(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);

    static uint32_t startTimeMs = 0;
    static bool stopped = false;

    uint32_t now = millis();

    // Initialize start time on first call
    if (startTimeMs == 0) {
        startTimeMs = now;
        return;  // Skip first print to establish baseline
    }

    // Check for test completion
    if (TEST_DURATION_MS > 0 && !stopped && (now - startTimeMs) >= TEST_DURATION_MS) {
        stopped = true;

        // Print summary
        uint32_t durationMs = now - startTimeMs;
        float gyroHz = (float)gyroReadCount * 1000.0f / durationMs;
        float rcHz = (float)rcFrameCount * 1000.0f / durationMs;

        Serial.println("\n=== Test Complete ===");
        Serial.print("Duration: "); Serial.print(durationMs); Serial.println(" ms");
        Serial.print("Gyro: "); Serial.print(gyroReadCount); Serial.print(" samples (");
        Serial.print(gyroHz, 1); Serial.println(" Hz avg)");
        Serial.print("RC: "); Serial.print(rcFrameCount); Serial.print(" frames (");
        Serial.print(rcHz, 1); Serial.println(" Hz avg)");

        // Print task statistics
        Serial.println("Task Stats:");
        cfTaskInfo_t info;
        const char* taskNames[] = {"GYRO", "RX", "FLIGHT", "SERIAL"};
        cfTaskId_e taskIds[] = {TASK_GYRO, TASK_RX, TASK_FLIGHT, TASK_SERIAL};

        for (int i = 0; i < 4; i++) {
            getTaskInfo(taskIds[i], &info);
            Serial.print("  "); Serial.print(taskNames[i]); Serial.print(": ");
            Serial.print("avg="); Serial.print(info.averageExecutionTime); Serial.print("us ");
            Serial.print("max="); Serial.print(info.maxExecutionTime); Serial.println("us");
        }

        Serial.println("*STOP*");
        return;
    }

    if (stopped) return;

    // Calculate running average rates from test start (eliminates jitter)
    // Use integer math: rate_x10 gives one decimal place (e.g., 9962 = 996.2 Hz)
    uint32_t elapsedMs = now - startTimeMs;
    if (elapsedMs == 0) elapsedMs = 1;

    uint32_t gyroHz_x10 = (gyroReadCount * 10000UL) / elapsedMs;
    uint32_t rcHz_x10 = (rcFrameCount * 10000UL) / elapsedMs;

    // Alternate between two shorter lines to reduce blocking time
    static uint8_t cycle = 0;
    char buf[80];

    if (cycle++ % 2 == 0) {
        // Even: Rates and load
        snprintf(buf, sizeof(buf), "[%lu.%03lus] Gyro:%lu.%luHz RC:%lu.%luHz Load:%d%%%s",
                 elapsedMs / 1000, elapsedMs % 1000,
                 gyroHz_x10 / 10, gyroHz_x10 % 10,
                 rcHz_x10 / 10, rcHz_x10 % 10,
                 averageSystemLoadPercent,
                 rcFailsafe ? " FAILSAFE" : "");
    } else {
        // Odd: Sensor data
        snprintf(buf, sizeof(buf), "[%lu.%03lus] Accel[%d %d %d] Ch[%d %d %d %d]",
                 elapsedMs / 1000, elapsedMs % 1000,
                 accelData[0], accelData[1], accelData[2],
                 rcChannels[0], rcChannels[1], rcChannels[2], rcChannels[3]);
    }
    Serial.println(buf);
}

// =============================================================================
// Task Configuration Array
// =============================================================================

cfTask_t cfTasks[TASK_COUNT] = {
    // TASK_SYSTEM: Required system task (always first)
    [TASK_SYSTEM] = SCHEDULER_TASK_SYSTEM_INIT,

    // TASK_GYRO: IMU sampling at 1 kHz (REALTIME - guaranteed timing)
    [TASK_GYRO] = {
        .taskName = "GYRO",
        .checkFunc = NULL,
        .taskFunc = taskGyro,
        .desiredPeriod = TASK_PERIOD_HZ(1000),    // 1 kHz
        .staticPriority = TASK_PRIORITY_REALTIME,
    },

    // TASK_RX: RC receiver (HIGH - event-driven via checkFunc)
    [TASK_RX] = {
        .taskName = "RX",
        .checkFunc = taskRxCheck,
        .taskFunc = taskRxMain,
        .desiredPeriod = TASK_PERIOD_HZ(50),      // 50 Hz fallback
        .staticPriority = TASK_PRIORITY_HIGH,
    },

    // TASK_FLIGHT: Flight control at 500 Hz (HIGH priority)
    [TASK_FLIGHT] = {
        .taskName = "FLIGHT",
        .checkFunc = NULL,
        .taskFunc = taskFlight,
        .desiredPeriod = TASK_PERIOD_HZ(500),     // 500 Hz
        .staticPriority = TASK_PRIORITY_HIGH,
    },

    // TASK_SERIAL: Debug output at 10 Hz (LOW priority)
    [TASK_SERIAL] = {
        .taskName = "SERIAL",
        .checkFunc = NULL,
        .taskFunc = taskSerial,
        .desiredPeriod = TASK_PERIOD_HZ(10),      // 10 Hz
        .staticPriority = TASK_PRIORITY_LOW,
    },
};

// =============================================================================
// Setup
// =============================================================================

void setup() {
    // Initialize debug serial
    Serial.begin(115200);
    while (!Serial && millis() < 3000);  // Wait for Serial with timeout
    Serial.println("\n=== Scheduler Example: IMU + RC ===\n");

    // Initialize SPI for IMU
    SPI.begin();

    // Initialize IMU
    Serial.print("Initializing IMU... ");
    if (imu.Init(SPI, IMU_CS_PIN, IMU_SPI_FREQ) != IMU::Result::OK) {
        Serial.println("FAILED!");
        while (1);
    }

    // Apply balanced preset (4 kHz ODR, balanced filtering)
    if (imu.ApplyPreset(IMU::Preset::BALANCED) != IMU::Result::OK) {
        Serial.println("Preset FAILED!");
        while (1);
    }

    IMU::ChipType chip = imu.GetChipType();
    Serial.print("OK (ChipID: 0x");
    Serial.print((uint8_t)chip, HEX);
    Serial.println(")");

    // Initialize RC receiver
    Serial.print("Initializing RC... ");
    SerialRx::Config rxConfig;
    rxConfig.serial = &RC_SERIAL;  // Serial1 on USART1 (PB7 RX)
    rxConfig.rx_protocol = SerialRx::IBUS;
    rxConfig.baudrate = RC_BAUDRATE;
    rxConfig.timeout_ms = RC_TIMEOUT_MS;

    if (!rc.begin(rxConfig)) {
        Serial.println("FAILED!");
        while (1);
    }
    Serial.println("OK");

    // Initialize scheduler
    Serial.print("Initializing Scheduler... ");
    if (!schedulerInit(cfTasks, TASK_COUNT)) {
        Serial.println("FAILED! (too many tasks?)");
        while (1);
    }
    Serial.println("OK");

    // Enable tasks
    setTaskEnabled(TASK_GYRO, true);
    setTaskEnabled(TASK_RX, true);
    setTaskEnabled(TASK_FLIGHT, true);
    setTaskEnabled(TASK_SERIAL, true);

    Serial.println("\nScheduler running. Monitoring rates...\n");
}

// =============================================================================
// Loop
// =============================================================================

void loop() {
    // Run the scheduler - this is the only thing in loop()
    scheduler();
}
