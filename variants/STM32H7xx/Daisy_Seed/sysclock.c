#include <stm32h7xx_hal.h>
#include <stm32yyxx.h>

static void Error_Handler()
{
    // Insert code to handle errors here.
    while(1) {}
}

/**
  * @brief Peripherals Common Clock Configuration
  * @param FLatency: FLASH Latency
  * @retval None
  * NOTE: PLLxRGE[1:0]: PLL input frequency range
  * Set and reset by software to select the proper reference frequency range used for PLLx.
  * These bits must be written before enabling the PLLx.
  * 00: The PLLx input (refx_ck) clock range frequency is between 1 and 2 MHz (default after reset)
  * 01: The PLLx input (refx_ck) clock range frequency is between 2 and 4 MHz
  * 10: The PLLx input (refx_ck) clock range frequency is between 4 and 8 MHz
  * 11: The PLLx input (refx_ck) clock range frequency is between 8 and 16 MHz
  */
void SystemClock_Config(uint32_t plln_val, uint32_t FLatency)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {};

    /** Macro to configure the PLL clock source
  */
    __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
    /** Initializes the CPU, AHB and APB busses clocks
  */
    RCC_OscInitStruct.OscillatorType
        = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState      = RCC_HSE_ON;
    RCC_OscInitStruct.HSI48State    = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState  = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM      = 4;
    //
    RCC_OscInitStruct.PLL.PLLN      = plln_val;
    RCC_OscInitStruct.PLL.PLLP      = 2;
    RCC_OscInitStruct.PLL.PLLQ      = 5; // was 4 in cube
    RCC_OscInitStruct.PLL.PLLR      = 2;
    RCC_OscInitStruct.PLL.PLLRGE    = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN  = 0;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB busses clocks
  */
    RCC_ClkInitStruct.ClockType
        = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1
          | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLatency) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInitStruct.PeriphClockSelection
        = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_USART6
          | RCC_PERIPHCLK_USART234578 | RCC_PERIPHCLK_LPUART1
          | RCC_PERIPHCLK_RNG | RCC_PERIPHCLK_SPI1 | RCC_PERIPHCLK_SAI2
          | RCC_PERIPHCLK_SAI1 | RCC_PERIPHCLK_SDMMC | RCC_PERIPHCLK_I2C2
          | RCC_PERIPHCLK_ADC | RCC_PERIPHCLK_I2C1 | RCC_PERIPHCLK_USB
          | RCC_PERIPHCLK_QSPI | RCC_PERIPHCLK_FMC;
    // PLL 2
    //  PeriphClkInitStruct.PLL2.PLL2N = 115; // Max Freq @ 3v3
    //PeriphClkInitStruct.PLL2.PLL2N      = 84; // Max Freq @ 1V9
    /** Old Timing  */
    /*
    PeriphClkInitStruct.PLL2.PLL2N      = 100; // Max supported freq of FMC;
    PeriphClkInitStruct.PLL2.PLL2M      = 4;
    PeriphClkInitStruct.PLL2.PLL2P      = 8;  // 57.5
    PeriphClkInitStruct.PLL2.PLL2Q      = 10; // 46
    PeriphClkInitStruct.PLL2.PLL2R      = 2;  // 115Mhz
    PeriphClkInitStruct.PLL2.PLL2FRACN  = 0;
    */

    // New Timing
    PeriphClkInitStruct.PLL2.PLL2N     = 12; // Max supported freq of FMC;
    PeriphClkInitStruct.PLL2.PLL2M     = 1;
    PeriphClkInitStruct.PLL2.PLL2P     = 8; // 25MHz
    PeriphClkInitStruct.PLL2.PLL2Q     = 2; // 100MHz
    PeriphClkInitStruct.PLL2.PLL2R     = 1; // 200MHz
    PeriphClkInitStruct.PLL2.PLL2FRACN = 4096;


    PeriphClkInitStruct.PLL2.PLL2RGE    = RCC_PLL2VCIRANGE_2;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    // PLL 3
    PeriphClkInitStruct.PLL3.PLL3M         = 6;
    PeriphClkInitStruct.PLL3.PLL3N         = 295;
    PeriphClkInitStruct.PLL3.PLL3P         = 16; // 49.xMhz
    PeriphClkInitStruct.PLL3.PLL3Q         = 4;
    PeriphClkInitStruct.PLL3.PLL3R         = 32; // 24.xMhz
    PeriphClkInitStruct.PLL3.PLL3RGE       = RCC_PLL3VCIRANGE_1;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL    = RCC_PLL3VCOWIDE;
    PeriphClkInitStruct.PLL3.PLL3FRACN     = 0;
    PeriphClkInitStruct.FmcClockSelection  = RCC_FMCCLKSOURCE_PLL2;
    PeriphClkInitStruct.QspiClockSelection = RCC_QSPICLKSOURCE_D1HCLK;
    //PeriphClkInitStruct.SdmmcClockSelection  = RCC_SDMMCCLKSOURCE_PLL;
    PeriphClkInitStruct.SdmmcClockSelection  = RCC_SDMMCCLKSOURCE_PLL2;
    PeriphClkInitStruct.Sai1ClockSelection   = RCC_SAI1CLKSOURCE_PLL3;
    PeriphClkInitStruct.Sai23ClockSelection  = RCC_SAI23CLKSOURCE_PLL3;
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
    PeriphClkInitStruct.Usart234578ClockSelection
        = RCC_USART234578CLKSOURCE_D2PCLK1;
    PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
    PeriphClkInitStruct.I2c123ClockSelection  = RCC_I2C123CLKSOURCE_D2PCLK1;
    PeriphClkInitStruct.I2c4ClockSelection    = RCC_I2C4CLKSOURCE_PLL3;
    PeriphClkInitStruct.UsbClockSelection     = RCC_USBCLKSOURCE_HSI48;
    PeriphClkInitStruct.AdcClockSelection     = RCC_ADCCLKSOURCE_PLL3;
    if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
}
