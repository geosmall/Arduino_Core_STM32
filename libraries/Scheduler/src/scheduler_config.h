/**
 * @file scheduler_config.h
 * @brief Arduino platform adaptations for INav scheduler
 *
 * This file bridges INav scheduler requirements to the Arduino platform.
 * All Arduino-specific adaptations are isolated here to keep scheduler.h/.c
 * as close to upstream INav as possible.
 *
 * Attribution:
 *   Type definitions from INav common/time.h
 *   Macro definitions from INav common headers
 *   Arduino adaptation: GLS (January 2026)
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Time Types (from INav common/time.h)
//=============================================================================
typedef int32_t timeDelta_t;      // time difference, signed (overflows ~35 min)
typedef uint64_t timeUs_t;        // microsecond time, 64-bit
typedef uint32_t timeMs_t;        // millisecond time
#define TIMEUS_MAX UINT64_MAX

//=============================================================================
// Configuration (from INav config)
//=============================================================================
#define SCHEDULER_DELAY_LIMIT 100  // 100us minimum period (10 kHz max)

// Maximum tasks supported (for queue array sizing in library)
// User can override by defining before including Scheduler.h
#ifndef SCHEDULER_MAX_TASKS
#define SCHEDULER_MAX_TASKS 16
#endif

//=============================================================================
// Service Task IDs (fixed constants for library/sketch compatibility)
//=============================================================================
// These must be fixed values that don't depend on TASK_COUNT since the library
// is compiled before the sketch and sees a different TASK_COUNT.
// User's task_list.h MUST use these same values for TASK_NONE and TASK_SELF.
#define SCHEDULER_TASK_NONE  254
#define SCHEDULER_TASK_SELF  255

//=============================================================================
// Memory Section Macros (INav uses for CCM RAM placement)
//=============================================================================
#define STATIC_FASTRAM static
#define FASTRAM
#define FAST_CODE
#define NOINLINE

//=============================================================================
// Unit Test Macros
//=============================================================================
#define STATIC_UNIT_TESTED static
#define STATIC_INLINE_UNIT_TESTED static inline

//=============================================================================
// Utility Macros (from INav common headers)
//=============================================================================
#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

//=============================================================================
// Time Function Mapping
//=============================================================================
// INav uses micros() which returns timeUs_t (64-bit with USE_64BIT_TIME)
// Arduino_Core_STM32 provides micros64() for 64-bit timestamps
// Define getMicros() as the scheduler's time source
#define getMicros() micros64()

#ifdef __cplusplus
}
#endif
