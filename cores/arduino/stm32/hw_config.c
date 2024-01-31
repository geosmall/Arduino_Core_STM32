#include "hw_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief  This function performs the global init of the system (HAL, IOs...)
  * @param  None
  * @retval None
  */
void hw_config_init(void)
{

  /* Init DWT if present */
#ifdef DWT_BASE
  dwt_init();
#endif

  /* Initialize the HAL */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

}

#ifdef __cplusplus
}
#endif
