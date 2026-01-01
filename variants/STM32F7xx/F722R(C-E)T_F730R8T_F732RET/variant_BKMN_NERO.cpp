/*
 *******************************************************************************
 * Copyright (c) 2020, STMicroelectronics
 * All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 *
 * Board: NERO F7 Flight Controller (BKMN)
 * MCU: STM32F722RET6
 *
 * Auto-generated variant for NERO F7 flight controller board
 * Based on Betaflight unified target configuration
 */
#if defined(ARDUINO_BKMN_NERO)
#include "pins_arduino.h"

// Digital PinName array
const PinName digitalPin[] = {
  PA_0,   // D0/A0  - Motor 1 (TIM5_CH1)
  PA_1,   // D1/A1  - Motor 2 (TIM5_CH2)
  PA_2,   // D2/A2  - Motor 3 (TIM5_CH3)
  PA_3,   // D3/A3  - Motor 4 (TIM5_CH4)
  PA_4,   // D4/A4
  PA_5,   // D5/A5  - SPI1 SCK (IMU)
  PA_6,   // D6/A6  - SPI1 MISO (IMU)
  PA_7,   // D7/A7  - SPI1 MOSI (IMU)
  PA_8,   // D8
  PA_9,   // D9     - USART1 TX
  PA_10,  // D10    - USART1 RX
  PA_11,  // D11
  PA_12,  // D12
  PA_13,  // D13    - SWDIO
  PA_14,  // D14    - SWCLK
  PA_15,  // D15    - SPI3 CS (SD Card)
  PB_0,   // D16/A8
  PB_1,   // D17/A9 - Motor 6 (TIM3_CH4)
  PB_2,   // D18    - IMU INT
  PB_3,   // D19
  PB_4,   // D20
  PB_5,   // D21    - LED2
  PB_6,   // D22    - LED1
  PB_7,   // D23
  PB_8,   // D24    - I2C1 SCL
  PB_9,   // D25    - I2C1 SDA
  PB_10,  // D26    - USART3 TX
  PB_11,  // D27    - USART3 RX
  PB_12,  // D28
  PB_13,  // D29
  PB_14,  // D30
  PB_15,  // D31
  PC_0,   // D32/A10
  PC_1,   // D33/A11
  PC_2,   // D34/A12 - ADC Current
  PC_3,   // D35/A13 - ADC Voltage
  PC_4,   // D36/A14 - SPI1 CS (IMU)
  PC_6,   // D37     - USART6 TX
  PC_7,   // D38     - USART6 RX
  PC_8,   // D39     - Motor 7 (TIM8_CH3)
  PC_9,   // D40     - Motor 8 (TIM8_CH4)
  PC_10,  // D41     - SPI3 SCK (SD Card)
  PC_11,  // D42     - SPI3 MISO (SD Card)
  PC_12,  // D43     - SPI3 MOSI (SD Card)
  PC_13,  // D44
  PC_14,  // D45
  PC_15,  // D46
  PD_2,   // D47
  PH_0,   // D48     - OSC_IN
  PH_1    // D49     - OSC_OUT
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
  17, // A9,  PB1
  32, // A10, PC0
  33, // A11, PC1
  34, // A12, PC2 - ADC Current
  35, // A13, PC3 - ADC Voltage
  36  // A14, PC4
};

/**
  * @brief  System Clock Configuration
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;   // 336 MHz / 7 = 48 MHz (USB, SDMMC, RNG)
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

#endif /* ARDUINO_BKMN_NERO */
