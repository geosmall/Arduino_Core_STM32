# Scheduler Example: IMU + RC Receiver Integration

This example demonstrates the INav-based Scheduler with multiple tasks:
- **TASK_GYRO** (REALTIME): MPU-6000 IMU sampling at 1 kHz
- **TASK_RX** (HIGH): RC receiver with event-driven checkFunc
- **TASK_FLIGHT** (HIGH): Flight control processing at 500 Hz
- **TASK_SERIAL** (LOW): Debug output at 10 Hz

## Architecture Overview

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│  TASK_GYRO      │     │  TASK_RX        │     │  TASK_FLIGHT    │
│  (REALTIME)     │     │  (HIGH)         │     │  (HIGH)         │
│                 │     │                 │     │                 │
│  IMU sampling   │────►│  RC receiver    │────►│  Flight control │
│  1 kHz          │     │  event-driven   │     │  500 Hz         │
└─────────────────┘     └─────────────────┘     └─────────────────┘
        │                       │                       │
        ▼                       ▼                       ▼
   gyroData[]              rcChannels[]           motorOutput[]
```

**Key insights:**
- REALTIME priority guarantees the gyro task runs on schedule
- RC uses `checkFunc` for event-driven scheduling (only runs when frame ready)
- Flight control runs after gyro/RC have updated their data
- Lower priority tasks (SERIAL) run in remaining time

## Hardware Requirements

- STM32F411RE Nucleo board
- MPU-6000 IMU on SPI1 (auto-detected by IMU library)
- IBus RC receiver on USART1

**Wiring (NUCLEO_F411RE_HIL005):**
| Signal | Pin | Description |
|--------|-----|-------------|
| IMU MOSI | PA7 | SPI1 data out |
| IMU MISO | PA6 | SPI1 data in |
| IMU SCK | PA5 | SPI1 clock |
| IMU CS | PA4 | SPI1 chip select |
| IMU INT | PB3 | Data ready interrupt (optional) |
| RC RX | PB7 | USART1 receive (IBus 115200 baud) |
| RC TX | PB6 | USART1 transmit (unused) |

## Task Configuration

| Task | Priority | Rate | Description |
|------|----------|------|-------------|
| `TASK_GYRO` | REALTIME | 1 kHz | MPU-6000 IMU sampling - guaranteed timing |
| `TASK_RX` | HIGH | event-driven | RC receiver with checkFunc |
| `TASK_FLIGHT` | HIGH | 500 Hz | Flight control processing |
| `TASK_SERIAL` | LOW | 10 Hz | Debug output |

## Files

### task_list.h

```c
#ifndef TASK_LIST_H
#define TASK_LIST_H

#define SCHEDULER_TASK_LIST_DEFINED

typedef enum {
    TASK_SYSTEM = 0,    // Required: System housekeeping

    TASK_GYRO,          // REALTIME: IMU sampling at 1 kHz
    TASK_RX,            // HIGH: RC receiver input
    TASK_FLIGHT,        // HIGH: Flight control
    TASK_SERIAL,        // LOW: Serial output for debugging

    TASK_COUNT          // Required: Must be last
} cfTaskId_e;

#endif
```

### Key Code Sections

#### IMU Task (REALTIME)

```cpp
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
```

#### RC Task (HIGH, Event-Driven)

```cpp
// checkFunc: Only run task when frame is ready
extern "C" bool taskRxCheck(timeUs_t currentTimeUs, timeDelta_t currentDeltaTime) {
    UNUSED(currentTimeUs);
    UNUSED(currentDeltaTime);

    rc.update();           // Poll serial, parse bytes
    return rc.available(); // True if complete frame ready
}

// taskFunc: Process the received frame
extern "C" void taskRxMain(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);

    while (rc.getMessage(&rcMsg)) {
        for (int i = 0; i < RC_NUM_CHANNELS; i++) {
            rcChannels[i] = rcMsg.channels[i];
        }
        rcDataNew = true;
        rcFailsafe = false;
        rcFrameCount++;
    }

    // Failsafe check
    if (rc.timeout(RC_TIMEOUT_MS)) {
        rcFailsafe = true;
        rcChannels[2] = 1000;  // Throttle low
    }
}
```

#### Task Array Configuration

```cpp
cfTask_t cfTasks[TASK_COUNT] = {
    [TASK_SYSTEM] = SCHEDULER_TASK_SYSTEM_INIT,

    [TASK_GYRO] = {
        .taskName = "GYRO",
        .checkFunc = NULL,
        .taskFunc = taskGyro,
        .desiredPeriod = TASK_PERIOD_HZ(1000),    // 1 kHz
        .staticPriority = TASK_PRIORITY_REALTIME,
    },

    [TASK_RX] = {
        .taskName = "RX",
        .checkFunc = taskRxCheck,                 // Event-driven
        .taskFunc = taskRxMain,
        .desiredPeriod = TASK_PERIOD_HZ(50),      // 50 Hz fallback
        .staticPriority = TASK_PRIORITY_HIGH,
    },

    [TASK_FLIGHT] = {
        .taskName = "FLIGHT",
        .checkFunc = NULL,
        .taskFunc = taskFlight,
        .desiredPeriod = TASK_PERIOD_HZ(500),     // 500 Hz
        .staticPriority = TASK_PRIORITY_HIGH,
    },

    [TASK_SERIAL] = {
        .taskName = "SERIAL",
        .checkFunc = NULL,
        .taskFunc = taskSerial,
        .desiredPeriod = TASK_PERIOD_HZ(10),      // 10 Hz
        .staticPriority = TASK_PRIORITY_LOW,
    },
};
```

## Data Flow

```
  TASK_GYRO              TASK_RX                 TASK_FLIGHT           TASK_SERIAL
  (REALTIME 1kHz)        (HIGH event)            (HIGH 500Hz)          (LOW 10Hz)
  ──────────────         ────────────            ────────────          ───────────

  imu.ReadIMU6()         rc.update()             Read shared data      Print stats
       │                 rc.available()                │                    │
       ▼                      │                        ▼                    ▼
  gyroData[3] ◄───────────────┼──────────────► rcChannels[14]         "Gyro: 1000 Hz"
  accelData[3]                │                        │               "RC: 140 Hz"
       │                      ▼                        │               "Load: 12%"
       │                 rc.getMessage()               │
       │                      │                        │
       └──────────────────────┴────────────────────────┘
                              │
                              ▼
                        motorOutput[4]
```

## Expected Output

```
=== Scheduler Example: IMU + RC ===

Initializing IMU... OK (ChipID: 0x68)
Initializing RC... OK
Initializing Scheduler... OK

Scheduler running. Monitoring rates...

Gyro: 1000 Hz | RC: 140 Hz | Load: 12%
Gyro: 1000 Hz | RC: 140 Hz | Load: 12%
Gyro: 1000 Hz | RC: 0 Hz [FAILSAFE] | Load: 11%
```

## SBUS Configuration

For SBUS receivers (inverted signal, 100000 baud):

```cpp
SerialRx rc(SerialRx::SBUS);

SerialRx::Config rxConfig;
rxConfig.serial = &Serial2;
rxConfig.rx_protocol = SerialRx::SBUS;
rxConfig.baudrate = 100000;
rxConfig.timeout_ms = 500;
rxConfig.invert_rx = true;  // Hardware inversion (F7/H7/G4 only)
rc.begin(rxConfig);
```

**Note:** STM32F4 does not support hardware RX inversion. Use IBus or external inverter.

## Priority Guidelines

| Priority | Value | Use For | Examples |
|----------|-------|---------|----------|
| REALTIME | 18 | Hard real-time loops | Gyro sampling, PID |
| HIGH | 5 | Time-sensitive I/O | RC input, servo output |
| MEDIUM | 3-4 | Sensor polling | GPS, baro, compass |
| LOW | 1 | Non-critical tasks | Serial debug, OSD |
| IDLE | 0 | Background tasks | Telemetry, LED effects |

## Common Mistakes

1. **Polling IMU in flight task** - Use separate REALTIME gyro task for consistent timing

2. **Using `taskRunRealtimeCallbacks()` for RC** - Use HIGH priority task with checkFunc instead

3. **Not using `checkFunc` for RC** - Without it, you waste CPU polling when no data available

4. **Wrong priority for gyro** - IMU sampling needs REALTIME to maintain consistent loop timing

5. **Reading shared data without flags** - Use `gyroDataNew`/`rcDataNew` flags to detect fresh data

## INav Reference

This scheduler is ported from INav flight controller. For reference on how INav structures its tasks:

- [INav Scheduler](https://github.com/iNavFlight/inav/tree/master/src/main/scheduler)
- [INav FC Tasks](https://github.com/iNavFlight/inav/blob/master/src/main/fc/fc_tasks.c)
- [INav RX System](https://github.com/iNavFlight/inav/tree/master/src/main/rx)

### INav Task Priorities (Reference)

| Task | Priority | Rate | Purpose |
|------|----------|------|---------|
| PID | REALTIME | 1 kHz | Flight control loop |
| GYRO | REALTIME | 1-8 kHz | IMU sampling |
| RX | HIGH | 10 Hz* | RC receiver (*event-driven) |
| GPS | MEDIUM | 50 Hz | Position updates |
| BARO | MEDIUM | 20 Hz | Altitude |
| SERIAL | LOW | 100 Hz | CLI, MSP protocol |
| TELEMETRY | IDLE | 500 Hz | FrSky, CRSF |
