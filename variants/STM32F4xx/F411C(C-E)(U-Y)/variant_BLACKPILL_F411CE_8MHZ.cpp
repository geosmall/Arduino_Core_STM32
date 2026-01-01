/*
 *******************************************************************************
 * Copyright (c) 2011-2021, STMicroelectronics
 * All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 * BlackPill F411CE 8MHz variant
 * WeAct Studio v3.1 with 8 MHz crystal and TinyUF2 bootloader
 */
#if defined(ARDUINO_BLACKPILL_F411CE_8MHZ)
#include "pins_arduino.h"

// Digital PinName array - same as 25MHz variant
const PinName digitalPin[] = {
  PA_0,  // D0/A0
  PA_1,  // D1/A1
  PA_2,  // D2/A2
  PA_3,  // D3/A3
  PA_4,  // D4/A4
  PA_5,  // D5/A5
  PA_6,  // D6/A6
  PA_7,  // D7/A7
  PA_8,  // D8
  PA_9,  // D9
  PA_10, // D10
  PA_11, // D11
  PA_12, // D12
  PA_13, // D13
  PA_14, // D14
  PA_15, // D15
  PB_0,  // D16/A8
  PB_1,  // D17/A9
  PB_2,  // D18
  PB_3,  // D19
  PB_4,  // D20
  PB_5,  // D21
  PB_6,  // D22
  PB_7,  // D23
  PB_8,  // D24
  PB_9,  // D25
  PB_10, // D26
  PB_12, // D27
  PB_13, // D28
  PB_14, // D29
  PB_15, // D30
  PC_13, // D31
  PC_14, // D32
  PC_15, // D33
  PH_0,  // D34
  PH_1   // D35
};

// Analog (Ax) pin number array
const uint32_t analogInputPin[] = {
  0,  // A0,  PA0
  1,  // A1,  PA1
  2,  // A2,  PA2
  3,  // A3,  PA3
  4,  // A4,  PA4
  5,  // A5,  PA5
  6,  // A6,  PA6
  7,  // A7,  PA7
  16, // A8,  PB0
  17  // A9,  PB1
};

// ----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  System Clock Configuration for 8 MHz HSE
 *         SYSCLK = 96 MHz, USB = 48 MHz
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {};

  /** Configure the main internal regulator output voltage */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   *  in the RCC_OscInitTypeDef structure.
   *  BlackPill 8MHz uses 8 MHz HSE crystal
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

  /** Initializes the CPU, AHB and APB buses clocks */
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

#endif /* ARDUINO_BLACKPILL_F411CE && HSE_VALUE == 8000000U */
