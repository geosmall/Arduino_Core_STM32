#include "board.h"
#include "stm32_def.h"

#ifdef __cplusplus
extern "C" {
#endif

void __libc_init_array(void);

WEAK void init(void)
{
  /* Initialize the HAL */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();}

#ifdef __cplusplus
}
#endif
