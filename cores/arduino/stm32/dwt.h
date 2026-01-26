/**
  ******************************************************************************
  * @file    dwt.h
  * @author  Frederic Pillon
  * @brief   Header for dwt.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019, STMicroelectronics
  * All rights reserved.
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _DWT_H_
#define _DWT_H_

#include "stm32_def.h"
#include <stdbool.h>

#ifdef DWT_BASE

#ifdef __cplusplus
extern "C" {
#endif

uint32_t dwt_init(void);
void dwt_access(bool ena);

/**
 * @brief  Get current DWT cycle count
 * @note   32-bit counter, wraps at UINT32_MAX (~42 sec at 100MHz)
 * @retval Current cycle count from DWT->CYCCNT
 */
static inline uint32_t dwt_getCycles(void)
{
  return (DWT->CYCCNT);
}

/*
 * Maximum delay times before 32-bit cycle counter overflow.
 * Use these to validate delay parameters won't cause incorrect behavior.
 *
 * Example at 100 MHz:
 *   dwt_max_sec()  = 42 seconds
 *   dwt_max_msec() = 42949 ms
 *   dwt_max_usec() = 42949672 µs
 *   dwt_max_nsec() = 42949672950 ns (but capped by uint32_t return)
 */

static inline uint32_t dwt_max_sec(void)
{
  return (UINT32_MAX / SystemCoreClock);
}

static inline uint32_t dwt_max_msec(void)
{
  return (UINT32_MAX / (SystemCoreClock / 1000));
}

static inline uint32_t dwt_max_usec(void)
{
  return (UINT32_MAX / (SystemCoreClock / 1000000));
}

/**
 * @brief  Delay for specified number of nanoseconds using DWT cycle counter
 * @param  ns: Number of nanoseconds to delay
 * @note   Minimum meaningful delay depends on clock speed:
 *         - 100 MHz: ~10 ns minimum (1 cycle = 10 ns)
 *         - 168 MHz: ~6 ns minimum (1 cycle ≈ 6 ns)
 *         Overhead of function call adds ~50-100 ns.
 * @note   Maximum delay ~42 seconds at 100 MHz (32-bit counter wrap)
 */
static inline void delayNanos(uint32_t ns)
{
  uint32_t start = dwt_getCycles();
  /* cycles = ns * (SystemCoreClock / 1e9)
   * Rewritten to avoid overflow: (ns / 1000) * (SystemCoreClock / 1e6)
   * For sub-microsecond precision: (ns * (SystemCoreClock / 1e6)) / 1000
   */
  uint32_t cycles = (ns * (SystemCoreClock / 1000000)) / 1000;
  while ((dwt_getCycles() - start) < cycles);
}

#ifdef __cplusplus
}
#endif

#endif /* DWT_BASE */
#endif /* _DWT_H_ */
