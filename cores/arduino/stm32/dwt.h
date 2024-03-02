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

static inline uint32_t dwt_max_sec(void)
{
  return (UINT32_MAX / SystemCoreClock);
};

static inline uint32_t dwt_max_msec(void)
{
  return (UINT32_MAX / (SystemCoreClock / 1000));
};

static inline uint32_t dwt_max_usec(void)
{
  return (UINT32_MAX / (SystemCoreClock / 1000000));
};

static inline uint32_t dwt_getCycles(void)
{
  return (DWT->CYCCNT);
};

#ifdef __cplusplus
}
#endif

#endif /* DWT_BASE */

#endif /* _DWT_H_ */
