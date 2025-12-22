/*
 *******************************************************************************
 * Copyright (c) 2017-2021, STMicroelectronics
 * All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */
#if defined(ARDUINO_OPEN_REVO)

#include "pins_arduino.h"

// Pin number
// This array allows to wrap Arduino pin number(Dx or x)
// to STM32 PinName (PX_n)
const PinName digitalPin[] = {
  // Motor outputs
  PB_0,   // D0  - Motor 1 (TIM3_CH3)
  PB_1,   // D1  - Motor 2 (TIM3_CH4)
  PA_3,   // D2  - Motor 3 (TIM2_CH4)
  PA_2,   // D3  - Motor 4 (TIM2_CH3)
  PA_1,   // D4  - Motor 5 (TIM5_CH2)
  PA_0,   // D5  - Motor 6 (TIM5_CH1)

  // SPI1 - IMU
  PA_4,   // D6  - SPI1 CS (Gyro)
  PA_5,   // D7  - SPI1 SCK
  PA_6,   // D8  - SPI1 MISO
  PA_7,   // D9  - SPI1 MOSI
  PC_4,   // D10 - Gyro EXTI

  // SPI3 - Flash
  PB_3,   // D11 - Flash CS
  PC_10,  // D12 - SPI3 SCK
  PC_11,  // D13 - SPI3 MISO
  PC_12,  // D14 - SPI3 MOSI

  // I2C1
  PB_8,   // D15 - I2C1 SCL
  PB_9,   // D16 - I2C1 SDA

  // USART1
  PA_9,   // D17 - USART1 TX
  PA_10,  // D18 - USART1 RX

  // USART3
  PB_10,  // D19 - USART3 TX
  PB_11,  // D20 - USART3 RX

  // USART6
  PC_6,   // D21 - USART6 TX
  PC_7,   // D22 - USART6 RX

  // LEDs
  PB_5,   // D23 - LED1 (Blue)
  PB_4,   // D24 - LED2 (Amber)

  // ADC
  PC_2,   // D25 - A0 VBAT
  PC_1,   // D26 - A1 Current
  PC_0,   // D27 - A2 RSSI/Inverter

  // PWM inputs
  PB_14,  // D28 - PPM/PWM1 (TIM12_CH1)
  PB_15,  // D29 - PWM2 (TIM12_CH2)
  PC_8,   // D30 - PWM5/PINIO (TIM8_CH3)
  PC_9,   // D31 - PWM6 (TIM8_CH4)

  // USB
  PA_11,  // D32 - USB_DM
  PA_12,  // D33 - USB_DP
  PC_5,   // D34 - USB_DETECT

  // Debug
  PA_13,  // D35 - SWDIO
  PA_14,  // D36 - SWCLK
};

// Analog (Ax) pin number array
const uint32_t analogInputPin[] = {
  25, // A0 - PC2 (VBAT)
  26, // A1 - PC1 (Current)
  27  // A2 - PC0 (RSSI)
};

// ----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief  System Clock Configuration
  *         OpenPilot Revolution uses 8MHz external crystal
  *         Target: 168MHz SYSCLK
  * @param  None
  * @retval None
  */
WEAK void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {};

  /** Configure the main internal regulator output voltage */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the CPU, AHB and APB busses clocks
   *  HSE = 8MHz, SYSCLK = 168MHz
   *  PLL: M=8, N=336, P=2, Q=7
   *  SYSCLK = 8 * 336 / (8 * 2) = 168 MHz
   *  USB = 8 * 336 / (8 * 7) = 48 MHz
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB busses clocks
   *  HCLK = 168MHz, APB1 = 42MHz, APB2 = 84MHz
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
    Error_Handler();
  }

  /* Ensure CCM RAM clock is enabled */
  __HAL_RCC_CCMDATARAMEN_CLK_ENABLE();
}

#ifdef __cplusplus
}
#endif

#endif /* ARDUINO_OPEN_REVO */
