#ifndef _STM32_DEF_
#define _STM32_DEF_

#define USE_HAL_DRIVER

#if defined(STM32C0xx)
  #include "stm32c0xx.h"
#elif defined(STM32F0xx)
  #include "stm32f0xx.h"
#elif defined(STM32F1xx)
  #include "stm32f1xx.h"
#elif defined(STM32F2xx)
  #include "stm32f2xx.h"
#elif defined(STM32F3xx)
  #include "stm32f3xx.h"
#elif defined(STM32F4xx)
  #include "stm32f4xx.h"
#elif defined(STM32F7xx)
  #include "stm32f7xx.h"
#elif defined(STM32G0xx)
  #include "stm32g0xx.h"
#elif defined(STM32G4xx)
  #include "stm32g4xx.h"
#elif defined(STM32H7xx)
  #include "stm32h7xx.h"
#elif defined(STM32L0xx)
  #include "stm32l0xx.h"
#elif defined(STM32L1xx)
  #include "stm32l1xx.h"
#elif defined(STM32L4xx)
  #include "stm32l4xx.h"
#elif defined(STM32L5xx)
  #include "stm32l5xx.h"
#elif defined(STM32MP1xx)
  #include "stm32mp1xx.h"
#elif defined(STM32U5xx)
  #include "stm32u5xx.h"
#elif defined(STM32WBxx)
  #include "stm32wbxx.h"
#elif defined(STM32WLxx)
  #include "stm32wlxx.h"
#else
  #error "STM32YYxx chip series is not defined in boards.txt."
#endif

/**
 * Libc porting layers
 */
#if defined (  __GNUC__  ) /* GCC CS3 */
#ifndef WEAK
  #define WEAK __attribute__ ((weak))
#endif /* #ifndef WEAK */
#endif /* GCC CS3 */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// weaked functions declaration
void SystemClock_Config(void);

#if !defined(Error_Handler)
#define Error_Handler() \
  while (1) {\
  }
#endif /* !defined(Error_Handler) */

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif //_STM32_DEF_