/*
 * variant_MATEK_H743VI.cpp — Board-specific system clock configuration
 *
 * No digitalPin[] or analogInputPin[] arrays — those are eliminated
 * by the Pin refactor. Only SystemClock_Config remains.
 */
#if defined(ARDUINO_MATEK_H743VI)

#include "pins_arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief  System Clock Configuration
  *         SYSCLK = 400 MHz for Matek H743 (8 MHz HSE crystal)
  *         Aligned to Betaflight default profile (400 MHz, VOS1) — works on
  *         both Rev.Y (pre-2018) and Rev.V silicon. Rev.V can do 480 MHz
  *         with VOS0; we trade 80 MHz performance for broad silicon
  *         compatibility and lower power.
  *         PLL1: 8 MHz / 1 = 8 MHz → * 100 = 800 MHz → / 2 = 400 MHz
  *         HCLK = 200 MHz (AHB /2), PCLK1/2/3/4 = 100 MHz (APB /2)
  *         USB: PLL3 Q = 48 MHz (PLL3 unchanged)
  * @param  None
  * @retval None
  */
WEAK void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {};
  RCC_CRSInitTypeDef RCC_CRSInitStruct = {};

  /* Supply configuration update enable */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /* Configure the main internal regulator output voltage — VOS1 (Rev.Y compatible) */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);

  /* PLL1: 8 MHz HSE → 400 MHz SYSCLK; HSI48 enabled for USB */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;    // 8 MHz / 1 = 8 MHz
  RCC_OscInitStruct.PLL.PLLN = 100;  // 8 MHz * 100 = 800 MHz
  RCC_OscInitStruct.PLL.PLLP = 2;    // 800 MHz / 2 = 400 MHz SYSCLK
  RCC_OscInitStruct.PLL.PLLQ = 8;    // 800 MHz / 8 = 100 MHz
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /* CPU, AHB and APB buses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                                | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;     // 200 MHz
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;     // 100 MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;     // 100 MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;     // 100 MHz
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;     // 100 MHz
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }

  /* Peripheral clocks — PLL2 for ADC; USB on HSI48 (CRS-trimmed); USART/I2C on PCLK */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB | RCC_PERIPHCLK_QSPI
                                             | RCC_PERIPHCLK_SDMMC | RCC_PERIPHCLK_ADC
                                             | RCC_PERIPHCLK_LPUART1 | RCC_PERIPHCLK_USART16
                                             | RCC_PERIPHCLK_USART234578 | RCC_PERIPHCLK_I2C123
                                             | RCC_PERIPHCLK_I2C4 | RCC_PERIPHCLK_SPI123
                                             | RCC_PERIPHCLK_SPI45 | RCC_PERIPHCLK_SPI6;
  /* PLL2: 8 MHz / 1 → * 20 = 160 MHz → / 2 = 80 MHz (used for ADC) */
  PeriphClkInitStruct.PLL2.PLL2M = 1;
  PeriphClkInitStruct.PLL2.PLL2N = 20;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0.0;
  /* PLL3 no longer needed — USB moved to HSI48+CRS, LPUART1 to D3PCLK1 */
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;       // CRS-trimmed to USB SOF
  PeriphClkInitStruct.QspiClockSelection = RCC_QSPICLKSOURCE_D1HCLK;
  PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
  PeriphClkInitStruct.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_D3PCLK1;  // 100 MHz (was PLL3_R 96)
  PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_D3PCLK1;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
  PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.Spi6ClockSelection = RCC_SPI6CLKSOURCE_D3PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
    Error_Handler();
  }

  // Configure Clock Recovery System for HSI48 → USB SOF auto-trim.
  // Without CRS, HSI48 (±1% factory, ±2-3% over temp) is out of USB-FS spec
  // (±2500 ppm). Pattern matches Betaflight system_stm32h7xx.c:539-549.
  __HAL_RCC_CRS_CLK_ENABLE();
  RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
  RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB1;
  RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
  RCC_CRSInitStruct.ReloadValue = RCC_CRS_RELOADVALUE_DEFAULT;
  RCC_CRSInitStruct.ErrorLimitValue = RCC_CRS_ERRORLIMIT_DEFAULT;
  RCC_CRSInitStruct.HSI48CalibrationValue = RCC_CRS_HSI48CALIBRATION_DEFAULT;
  HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
}

#ifdef __cplusplus
}
#endif

#endif /* ARDUINO_MATEK_H743VI */
