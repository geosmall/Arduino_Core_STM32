/*
 * wiring_time.cpp — Time functions
 *
 * Renamed from .c to .cpp for consistency with Pin refactor.
 * No pin dependency — content unchanged from original.
 */

#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t millis(void)
{
  return getCurrentMillis();
}

uint32_t micros(void)
{
  return getCurrentMicros();
}

uint64_t micros64(void)
{
  return getCurrentMicros64();
}

void delay(uint32_t ms)
{
  if (ms != 0) {
    uint32_t start = getCurrentMillis();
    do {
      yield();
    } while (getCurrentMillis() - start < ms);
  }
}

#ifdef __cplusplus
}
#endif
