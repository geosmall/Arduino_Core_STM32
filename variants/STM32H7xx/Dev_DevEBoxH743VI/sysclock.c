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
  * VCI2RGE_0: The PLLx input (refx_ck) clock range frequency is between 1 and 2 MHz (default after reset)
  * VCI2RGE_1: The PLLx input (refx_ck) clock range frequency is between 2 and 4 MHz
  * VCI2RGE_2: The PLLx input (refx_ck) clock range frequency is between 4 and 8 MHz
  * VCI2RGE_3: The PLLx input (refx_ck) clock range frequency is between 8 and 16 MHz
  */
void SystemClock_Config(uint32_t plln_val, uint32_t FLatency)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Macro to configure the PLL clock source
    */
    __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 5;
    RCC_OscInitStruct.PLL.PLLN = plln_val;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 5;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                                  | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLatency) != HAL_OK)
    {
        Error_Handler();
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB | RCC_PERIPHCLK_QSPI
            | RCC_PERIPHCLK_SDMMC | RCC_PERIPHCLK_ADC
            | RCC_PERIPHCLK_LPUART1 | RCC_PERIPHCLK_USART16
            | RCC_PERIPHCLK_USART234578 | RCC_PERIPHCLK_I2C123
            | RCC_PERIPHCLK_I2C4 | RCC_PERIPHCLK_SPI123
            | RCC_PERIPHCLK_SPI45 | RCC_PERIPHCLK_SPI6;

    PeriphClkInitStruct.PLL2.PLL2M = 5;
    PeriphClkInitStruct.PLL2.PLL2N = 120;
    PeriphClkInitStruct.PLL2.PLL2P = 24;
    PeriphClkInitStruct.PLL2.PLL2Q = 6;
    PeriphClkInitStruct.PLL2.PLL2R = 3;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;

    PeriphClkInitStruct.PLL3.PLL3M = 5;
    PeriphClkInitStruct.PLL3.PLL3N = 192;
    PeriphClkInitStruct.PLL3.PLL3P = 20;
    PeriphClkInitStruct.PLL3.PLL3Q = 20;
    PeriphClkInitStruct.PLL3.PLL3R = 30;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_2;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;

    /* PLL1 q clk used for USB 48 Mhz */
    /* PLL1 q clk also used for FMC, QUADSPI, SDMMC, RNG, SAI */
    /* PLL2 p clk is needed for adc max 80 Mhz (p,q,r same) */
    /* PLL2 p clk also used for LP timers 2,3,4,5, SPI 1,2,3 */
    /* PLL2 q clk is needed for uart, can, spi4,5,6 80 Mhz */
    /* PLL3 r clk is needed for i2c 80 Mhz (p,q,r same) */
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL3;
    PeriphClkInitStruct.QspiClockSelection = RCC_QSPICLKSOURCE_PLL2;
    PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL2;
    PeriphClkInitStruct.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PLL2;
    PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_PLL2;
    PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_PLL2;
    PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_PLL3;
    PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PLL3;
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
    PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_PLL2;
    PeriphClkInitStruct.Spi6ClockSelection = RCC_SPI6CLKSOURCE_PLL2;
    if ( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInitStruct ) != HAL_OK )
    {
        Error_Handler();
    }
}
