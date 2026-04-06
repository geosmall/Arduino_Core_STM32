/*
 * variant_BLACKPILL_F411CE_8MHZ.cpp — Board-specific system clock configuration
 *
 * No digitalPin[] or analogInputPin[] arrays — those are eliminated
 * by the Pin refactor. Only SystemClock_Config remains.
 */
#if defined(ARDUINO_BLACKPILL_F411CE_8MHZ)

#include "pins_arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief  System Clock Configuration
  *         SYSCLK = 96 MHz for BlackPill F411CE (8 MHz HSE)
  *         8 MHz / 4 = 2 MHz → * 96 = 192 MHz VCO → / 2 = 96 MHz
  *         USB: 192 MHz / 4 = 48 MHz
  * @param  None
  * @retval None
  */
WEAK void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {};

  /* Configure the main internal regulator output voltage */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   * BlackPill 8MHz uses 8 MHz HSE crystal
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;   // 8 MHz / 4 = 2 MHz
  RCC_OscInitStruct.PLL.PLLN = 96;  // 2 MHz * 96 = 192 MHz VCO
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;  // 192 MHz / 2 = 96 MHz (SYSCLK)
  RCC_OscInitStruct.PLL.PLLQ = 4;   // 192 MHz / 4 = 48 MHz (USB)
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /* Initializes the CPU, AHB and APB buses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;   // 96 MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;    // 48 MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;    // 96 MHz

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
    Error_Handler();
  }
}

#ifdef __cplusplus
}
#endif

#endif /* ARDUINO_BLACKPILL_F411CE_8MHZ */
