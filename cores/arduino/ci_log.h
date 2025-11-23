// ci_log.h — Unified CI/HIL logging abstraction
//
// Provides consistent logging across RTT (HIL testing) and Serial (Arduino IDE) modes.
//
// MACRO REFERENCE:
//   CI_LOG(s)           - String literal output (no formatting)
//   CI_LOGF(...)        - Printf-style formatting (integers only in RTT mode, NO %f support)
//   CI_PRINTF(...)      - Full printf with float support (requires libPrintf, see below)
//   CI_LOG_FLOAT(p,v,d) - Float output helper (prefix, value, decimals)
//   CI_BUILD_INFO()     - Show build SHA + timestamp (RTT only)
//   CI_READY_TOKEN()    - Show ready token (RTT only)
//
// CI_PRINTF USAGE:
//   CI_PRINTF requires libPrintf and a putchar_() implementation in your sketch:
//
//   #include <libPrintf.h>
//   extern "C" void putchar_(char c) {
//   #ifdef USE_RTT
//       SEGGER_RTT_PutChar(0, c);
//   #else
//       Serial.write(c);
//   #endif
//   }
//
//   Then use CI_PRINTF() for any formatted output including floats:
//   CI_PRINTF("Accel: %.3f, %.3f, %.3f\n", ax, ay, az);
//
#pragma once

#include <libPrintf.h>  // eyalroz/printf - full printf with float support

#if defined(USE_RTT)
  #include "SEGGER_RTT.h"

  // Build traceability handling
  #ifdef __has_include
    #if __has_include("build_id.h")
      #include "build_id.h"
      #define HAS_BUILD_ID
    #endif
  #endif
  #ifndef HAS_BUILD_ID
    #define BUILD_GIT_SHA "unknown"
    #define BUILD_UTC_TIME "unknown"
  #endif

  // Basic logging (RTT native - no float support in SEGGER_RTT_printf)
  #define CI_LOGF(...)   SEGGER_RTT_printf(0, __VA_ARGS__)
  #define CI_LOG(s)      SEGGER_RTT_WriteString(0, s)
  #define CI_BUILD_INFO() CI_LOGF("Build: %s (%s)\n", BUILD_GIT_SHA, BUILD_UTC_TIME)
  #define CI_READY_TOKEN() CI_LOGF("READY " BOARD_NAME " %s %s\n", BUILD_GIT_SHA, BUILD_UTC_TIME)

  // Full printf with float support via libPrintf
  // Requires putchar_() to be defined in sketch (routes to SEGGER_RTT_PutChar)
  #define CI_PRINTF(...) printf(__VA_ARGS__)

  // RTT doesn't support float printf natively, so provide helper using dtostrf
  inline void CI_LOG_FLOAT(const char* prefix, float value, int decimals = 2) {
    char buffer[32];
    dtostrf(value, 0, decimals, buffer);
    CI_LOG(prefix);
    CI_LOG(buffer);
  }
#else
  // Serial mode - all formatting supported natively
  #define CI_LOGF(...)   Serial.printf(__VA_ARGS__)
  #define CI_LOG(s)      Serial.print(s)
  #define CI_BUILD_INFO() do { } while(0) // No-op in Serial mode
  #define CI_READY_TOKEN() do { } while(0) // No-op in Serial mode

  // Full printf with float support via libPrintf
  // Requires putchar_() to be defined in sketch (routes to Serial.write)
  #define CI_PRINTF(...) printf(__VA_ARGS__)

  // Serial mode supports float natively
  inline void CI_LOG_FLOAT(const char* prefix, float value, int decimals = 2) {
    Serial.print(prefix);
    Serial.print(value, decimals);
  }
#endif