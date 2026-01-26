/*
 *******************************************************************************
 * Copyright (c) 2016-2021, STMicroelectronics
 * All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */
#include "atomic.h"
#include "backup.h"
#include "clock.h"
#include "lock_resource.h"
#include "otp.h"
#include "stm32yyxx_ll_cortex.h"
#include "stm32yyxx_ll_rcc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ISR-safe timing support variables
 *
 * sysTickValStamp: SysTick->VAL captured in SysTick_Handler immediately after reload.
 *   Since SysTick counts DOWN from LOAD to 0, this value is always near LOAD (e.g., ~99999
 *   for 100MHz). Used by main-thread reads to detect if SysTick rolled over mid-read:
 *   if cycle_cnt > sysTickValStamp, we read a post-reload VAL but ms hasn't updated yet.
 *
 * sysTickPending: Set by ISR-context reads when COUNTFLAG indicates rollover occurred
 *   but SysTick_Handler hasn't run yet. Cleared by SysTick_Handler. When set, add 1ms
 *   to the result to account for the pending tick increment.
 *
 * usTicks: CPU cycles per microsecond (e.g., 100 for 100MHz). Set once during
 *   hw_config_init() as SystemCoreClock / 1000000.
 */
static volatile uint32_t sysTickValStamp = 0;
static volatile int sysTickPending = 0;
uint32_t usTicks = 0;

/**
  * @brief  ISR-safe version of getCurrentMicros64
  * @note   Uses ATOMIC_BLOCK to safely read timing values from interrupt context
  * @retval Current microsecond count as 64-bit value
  */
uint64_t getCurrentMicros64ISR(void)
{
  uint32_t ms, pending, cycle_cnt;
  ATOMIC_BLOCK(NVIC_PRIO_MAX) {
    cycle_cnt = SysTick->VAL;
    /*
     * COUNTFLAG (bit 16 of SysTick->CTRL) is set by hardware when SysTick counts
     * from 1 to 0, and cleared by reading SysTick->CTRL. If set, SysTick rolled
     * over but SysTick_Handler hasn't run yet (we're blocking it via BASEPRI).
     * Set sysTickPending so we add 1ms to compensate, and re-read VAL to get
     * the post-reload value.
     */
    if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
      sysTickPending = 1;
      cycle_cnt = SysTick->VAL;
    }
    ms = HAL_GetTick();
    pending = sysTickPending;
  }
  const uint32_t partial = (usTicks * 1000U - cycle_cnt) / usTicks;
  return ((uint64_t)(ms + pending) * 1000ULL) + partial;
}

/**
  * @brief  Read current microsecond count as 64-bit value
  * @note   ISR-safe: auto-detects interrupt context and uses appropriate method
  * @retval Current microsecond count (overflows at ~49 days due to millis() limit)
  */
uint64_t getCurrentMicros64(void)
{
  /* Use ISR version if called from interrupt context or with elevated BASEPRI */
  if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) || (__get_BASEPRI())) {
    return getCurrentMicros64ISR();
  }

  /*
   * Lock-free read for main thread. Loop until we get a consistent pair:
   *   1. ms != HAL_GetTick(): SysTick_Handler ran between our two reads
   *   2. cycle_cnt > sysTickValStamp: We read VAL after it reloaded but before
   *      SysTick_Handler updated ms. Since sysTickValStamp is captured right
   *      after reload (~LOAD value), a larger cycle_cnt means we caught a stale ms.
   */
  uint32_t ms, cycle_cnt;
  do {
    ms = HAL_GetTick();
    cycle_cnt = SysTick->VAL;
  } while (ms != HAL_GetTick() || cycle_cnt > sysTickValStamp);

  const uint32_t partial = (usTicks * 1000U - cycle_cnt) / usTicks;
  return ((uint64_t)ms * 1000ULL) + partial;
}

/**
  * @brief  Read current microsecond count (32-bit, legacy API)
  * @note   ISR-safe wrapper around getCurrentMicros64()
  * @retval Current microsecond count (overflows at ~71 minutes)
  */
uint32_t getCurrentMicros(void)
{
  return (uint32_t)getCurrentMicros64();
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

void noOsSystickHandler()
{

}

void osSystickHandler() __attribute__((weak, alias("noOsSystickHandler")));
/**
  * @brief  Function called when the tick interruption falls
  * @note   Updates sysTickValStamp and clears sysTickPending for ISR-safe timing
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  /*
   * ATOMIC_BLOCK here protects against a higher-priority ISR (priority 0) calling
   * getCurrentMicros64ISR() and seeing inconsistent values (e.g., sysTickPending=0
   * but old sysTickValStamp). Also provides memory barrier to prevent reordering.
   * Cost is minimal (~4 cycles) and ensures correctness for nested interrupt scenarios.
   */
  ATOMIC_BLOCK(NVIC_PRIO_MAX) {
    sysTickPending = 0;
    sysTickValStamp = SysTick->VAL;
  }
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
  osSystickHandler();
}

/**
  * @brief  Enable the specified clock if not already set
  * @param  source: clock source: LSE_CLOCK, LSI_CLOCK, HSI_CLOCK or HSE_CLOCK
  * @retval None
  */
void enableClock(sourceClock_t source)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
#if defined(RCC_PLL_NONE)
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
#endif

#if defined(STM32MP1xx)
  /** Clock source selection is done by First Stage Boot Loader on Cortex A
   *  See variant.cpp for corresponding boards.
   */
  if (!IS_ENGINEERING_BOOT_MODE()) {
    return;
  }
#endif /* STM32MP1xx */

  enableBackupDomain();

  switch (source) {
    case LSI_CLOCK:
#ifdef RCC_FLAG_LSI1RDY
      __HAL_RCC_LSI1_ENABLE();
      if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSI1RDY) == RESET) {
        RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI1;
#else
      __HAL_RCC_LSI_ENABLE();
      if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY) == RESET) {
        RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI;
#endif
        RCC_OscInitStruct.LSIState = RCC_LSI_ON;
      }
      break;
    case HSI_CLOCK:
      __HAL_RCC_HSI_ENABLE();
      if (__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == RESET) {
        RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_HSI;
        RCC_OscInitStruct.HSIState = RCC_HSI_ON;
#if defined(STM32MP1xx)
        RCC_OscInitStruct.HSICalibrationValue = 0x00;
#else
        RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
#endif
      }
      break;
    case LSE_CLOCK:
      __HAL_RCC_LSE_CONFIG(RCC_LSE_ON);
      if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET) {
#ifdef __HAL_RCC_LSEDRIVE_CONFIG
        __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
#endif
        RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSE;
        RCC_OscInitStruct.LSEState = RCC_LSE_ON;
      }
      break;
    case HSE_CLOCK: {
#if defined(RCC_HSE_BYPASS_PWR) && defined(LORAWAN_BOARD_HAS_TCXO) && (LORAWAN_BOARD_HAS_TCXO == 1)
        uint32_t HSEState = RCC_HSE_BYPASS_PWR;
#else
        uint32_t HSEState = RCC_HSE_ON;
#endif
        __HAL_RCC_HSE_CONFIG(HSEState);
        if (__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == RESET) {
          RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_HSE;
          RCC_OscInitStruct.HSEState = HSEState;
        }
      }
      break;
    default:
      /* No valid clock to enable */
      break;
  }
  if (RCC_OscInitStruct.OscillatorType != RCC_OSCILLATORTYPE_NONE) {
    hsem_lock(CFG_HW_RCC_SEMID, HSEM_LOCK_DEFAULT_RETRY);
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
      Error_Handler();
    }
    hsem_unlock(CFG_HW_RCC_SEMID);
  }
}

void configHSECapacitorTuning(void)
{
#if defined(OTP_AREA_BASE) && defined(STM32WBxx)
  OTP_BT_t *p_otp;
  /* Read HSE_Tuning from OTP with index 0 */
  p_otp = (OTP_BT_t *) OTP_Read(0);
  if ((p_otp) && (!LL_RCC_HSE_IsReady())) {
    LL_RCC_HSE_SetCapacitorTuning(p_otp->hse_tuning);
  }
#endif
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
#endif
}

#ifdef __cplusplus
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
