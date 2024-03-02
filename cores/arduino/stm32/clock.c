#include "clock.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief  Function called to read the current micro second
  * @param  None
  * @retval None
  */
uint32_t getCurrentMicros(void)
{
  uint32_t m0 = HAL_GetTick();
  __IO uint32_t u0 = SysTick->VAL;
  uint32_t m1 = HAL_GetTick();
  __IO uint32_t u1 = SysTick->VAL;
  const uint32_t tms = SysTick->LOAD + 1;

  if (m1 != m0) {
    return (m1 * 1000 + ((tms - u1) * 1000) / tms);
  } else {
    return (m0 * 1000 + ((tms - u0) * 1000) / tms);
  }
}

/**
  * @brief  Function called wto read the current millisecond
  * @param  None
  * @retval None
  */
uint32_t getCurrentMillis(void)
{
  return HAL_GetTick();
}

void noOsSystickHandler() { }

void osSystickHandler() __attribute__((weak, alias("noOsSystickHandler")));
/**
  * @brief  Function called when the tick interruption falls
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
  osSystickHandler();
}

/**
  * @brief  This function enables clocks for some system IP
  * @param  None
  * @retval None
  */
void configIPClock(void)
{
#ifdef HSEM_BASE
  __HAL_RCC_HSEM_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_PWR_CLK_ENABLE)
  /* Enable PWR clock, needed for example: voltage scaling, low power ... */
  __HAL_RCC_PWR_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_SYSCFG_CLK_ENABLE)
  /* Enable SYSCFG clock, needed for example: Pin remap or Analog switch ... */
  __HAL_RCC_SYSCFG_CLK_ENABLE();
#endif

#if defined(HAL_CRC_MODULE_ENABLED)
  /* Enable CRC clock, needed for example: MotionFX Library ... */
#if defined(__HAL_RCC_CRC2_CLK_ENABLE)
  __HAL_RCC_CRC2_CLK_ENABLE();
#elif defined(__HAL_RCC_CRC_CLK_ENABLE)
  __HAL_RCC_CRC_CLK_ENABLE();
#endif
#endif /* defined(HAL_CRC_MODULE_ENABLED) */
}

#ifdef __cplusplus
}
#endif
