/*
 * variant_NUCLEO_F722ZE.cpp — Board-specific system clock configuration
 *
 * No digitalPin[] or analogInputPin[] arrays — those are eliminated
 * by the Pin refactor. Only SystemClock_Config remains.
 */
#if defined(ARDUINO_NUCLEO_F722ZE)

#include "pins_arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief  System Clock Configuration
  *         SYSCLK = 216 MHz for Nucleo F722ZE (8 MHz HSE bypass)
  *         8 MHz / 4 = 2 MHz → * 216 = 432 MHz VCO → / 2 = 216 MHz
  *         USB: 432 MHz / 9 = 48 MHz
  * @param  None
  * @retval None
  */
WEAK void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {};

  /* Configure LSE Drive Capability */
  HAL_PWR_EnableBkUpAccess();

  /* Configure the main internal regulator output voltage */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Initializes the RCC Oscillators
   * HSE = 8 MHz from ST-Link MCO (bypass mode)
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;    // 8 MHz / 4 = 2 MHz
  RCC_OscInitStruct.PLL.PLLN = 216;  // 2 MHz * 216 = 432 MHz VCO
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;  // 432 MHz / 2 = 216 MHz (SYSCLK)
  RCC_OscInitStruct.PLL.PLLQ = 9;    // 432 MHz / 9 = 48 MHz (USB/SDMMC)
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /* Activate the Over-Drive mode */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
    Error_Handler();
  }

  /* Initializes the CPU, AHB and APB buses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;   // 216 MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;    // 54 MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;    // 108 MHz

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK) {
    Error_Handler();
  }
}

#ifdef __cplusplus
}
#endif

#endif /* ARDUINO_NUCLEO_F722ZE */
