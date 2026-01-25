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
#include "dwt.h"
#include "hw_config.h"
#include "clock.h"
#include "usbd_if.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * STM32H7xx DMA buffer region MPU configuration
 * Configure D2 SRAM3 (32KB @ 0x30040000) as non-cached for DMA coherency
 *
 * D2 SRAM Layout:
 *   SRAM1: 0x30000000 - 0x3001FFFF (128KB) - general purpose, cached
 *   SRAM2: 0x30020000 - 0x3003FFFF (128KB) - general purpose, cached
 *   SRAM3: 0x30040000 - 0x30047FFF (32KB)  - DMA buffers, non-cached
 *
 * Using SRAM3 provides clean separation without fragmenting SRAM1/SRAM2.
 ******************************************************************************/
#if defined(STM32H7xx) && defined(HAL_DMA_MODULE_ENABLED)
static void MPU_Config_DMA_Region(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  HAL_MPU_Disable();

  /* D2 SRAM3 (32KB): 0x30040000 - 0x30047FFF as non-cached, shareable */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x30040000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}
#endif /* STM32H7xx && HAL_DMA_MODULE_ENABLED */

#if defined(HAL_CRC_MODULE_ENABLED)
CRC_HandleTypeDef hcrc = {.Instance =
#if defined(CRC2_BASE)
                            CRC2,
#elif defined(CRC_BASE)
                            CRC,
#else
#error "No CRC instance available!"
#endif
#if defined(CRC_INPUTDATA_FORMAT_BYTES)
                          .InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES
#endif
                         };
#endif

/**
  * @brief  This function performs the global init of the system (HAL, IOs...)
  * @param  None
  * @retval None
  */
void hw_config_init(void)
{
  configIPClock();

#if defined(PWR_CR3_UCPD_DBDIS) || defined(PWR_UCPDR_UCPD_DBDIS)
  /* Disable the internal Pull-Up in Dead Battery pins of UCPD peripheral */
  HAL_PWREx_DisableUCPDDeadBattery();
#endif
#if defined(SYSCFG_CFGR1_UCPD1_STROBE) || defined(SYSCFG_CFGR1_UCPD2_STROBE)
  /* Disable the internal Pull-Up in Dead Battery pins of UCPD peripheral */
  HAL_SYSCFG_StrobeDBattpinsConfig(SYSCFG_CFGR1_UCPD1_STROBE | SYSCFG_CFGR1_UCPD2_STROBE);
#endif /* SYSCFG_CFGR1_UCPD1_STROBE || SYSCFG_CFGR1_UCPD2_STROBE */

#if defined(PWR_SVMCR_ASV)
  HAL_PWREx_EnableVddA();
#endif
  /* Init DWT if present */
#ifdef DWT_BASE
  dwt_init();
#endif

  /* Initialize the HAL */
  HAL_Init();

#if defined(STM32H7xx) && defined(HAL_DMA_MODULE_ENABLED)
  /* Configure D2 SRAM3 as non-cached for DMA buffers */
  MPU_Config_DMA_Region();
#endif

  configHSECapacitorTuning();

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure cycles per microsecond for micros64() */
  usTicks = SystemCoreClock / 1000000;

  /* Initialize the CRC */
#if defined(HAL_CRC_MODULE_ENABLED)
  HAL_CRC_Init(&hcrc);
#endif

#if defined (USBCON) && defined(USBD_USE_CDC)
  USBD_CDC_init();
#endif

#if (__CORTEX_M == 33U) &&\
  defined(HAL_ICACHE_MODULE_ENABLED) && !defined(HAL_ICACHE_MODULE_DISABLED)
  if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_ICACHE_Enable() != HAL_OK) {
    Error_Handler();
  }
#endif
}
#ifdef __cplusplus
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
