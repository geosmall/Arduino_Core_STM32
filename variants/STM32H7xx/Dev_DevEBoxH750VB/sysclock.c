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
void SystemClock_Config(uint32_t FLatency)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {};

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    /* PLL1 pclk is sysclk 480 Mhz */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    // 480 Mhz
    RCC_OscInitStruct.PLL.PLLM = 5; // M div 5
    RCC_OscInitStruct.PLL.PLLN = 96; // N mul 96
    RCC_OscInitStruct.PLL.PLLP = 1; // P div 1
    RCC_OscInitStruct.PLL.PLLQ = 10; // Q div 10 - USB 48 Mhz
    RCC_OscInitStruct.PLL.PLLR = 10; // R unused
    // 360 Mhz
    /*
    RCC_OscInitStruct.PLL.PLLM = 5; // M div 10
    RCC_OscInitStruct.PLL.PLLN = 144; // N mul 144 (add 1 to N)
    RCC_OscInitStruct.PLL.PLLP = 2; // P div 2 (add 1 to PLLP) must be even
    RCC_OscInitStruct.PLL.PLLQ = 15; // Q div 10 (add 1 to PLLQ) - USB 48 Mhz
    RCC_OscInitStruct.PLL.PLLR = 15; // R unused
    */

    /*
     * RCC_PLL1VCIRANGE_0  Clock range frequency between 1 and 2 MHz
     * RCC_PLL1VCIRANGE_1  Clock range frequency between 2 and 4 MHz
     * RCC_PLL1VCIRANGE_2  Clock range frequency between 4 and 8 MHz
     * RCC_PLL1VCIRANGE_3  Clock range frequency between 8 and 16 MHz */
    /* PLLRGE: RCC_PLL1VCIRANGE_2 Clock range frequency between 4 and 8 MHz  */
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    /* VCOSEL PLL1VCOWIDE 2-16 Mhz */
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    if ( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK )
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
    if ( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_4 ) != HAL_OK )
    {
        Error_Handler();
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB | RCC_PERIPHCLK_QSPI
            | RCC_PERIPHCLK_SDMMC | RCC_PERIPHCLK_ADC
            | RCC_PERIPHCLK_LPUART1 | RCC_PERIPHCLK_USART16
            | RCC_PERIPHCLK_USART234578 | RCC_PERIPHCLK_I2C123
            | RCC_PERIPHCLK_I2C4 | RCC_PERIPHCLK_SPI123
            | RCC_PERIPHCLK_SPI45 | RCC_PERIPHCLK_SPI6;

    /* PLL1 qclk used for USB 48 Mhz */
    /* PLL1 qclk also used for FMC, QUADSPI, SDMMC, RNG, SAI */
    /* PLL2 pclk is needed for adc max 80 Mhz (p,q,r same) */
    /* PLL2 pclk also used for LP timers 2,3,4,5, SPI 1,2,3 */
    /* PLL2 qclk is needed for uart, can, spi4,5,6 80 Mhz */
    /* PLL3 r clk is needed for i2c 80 Mhz (p,q,r same) */
    PeriphClkInitStruct.PLL2.PLL2M = 15; // M DIV 15 vco 25 / 15 ~ 1.667 Mhz
    PeriphClkInitStruct.PLL2.PLL2N = 96; // N MUL 96
    PeriphClkInitStruct.PLL2.PLL2P = 2;  // P div 2
    PeriphClkInitStruct.PLL2.PLL2Q = 2;  // Q div 2
    PeriphClkInitStruct.PLL2.PLL2R = 2;  // R div 2
    // RCC_PLL1VCIRANGE_0  Clock range frequency between 1 and 2 MHz
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    PeriphClkInitStruct.PLL3.PLL3M = 15; // M DIV 15 vco 25 / 15 ~ 1.667 Mhz
    PeriphClkInitStruct.PLL3.PLL3N = 96; // N MUL 96
    PeriphClkInitStruct.PLL3.PLL3P = 2;  // P div 2
    PeriphClkInitStruct.PLL3.PLL3Q = 2;  // Q div 2
    PeriphClkInitStruct.PLL3.PLL3R = 2;  // R div 2
    // RCC_PLL1VCIRANGE_0  Clock range frequency between 1 and 2 MHz
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOMEDIUM;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    // ADC from PLL2 pclk
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
    // USB from PLL1 qclk
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
    // QSPI from PLL1 qclk
    PeriphClkInitStruct.QspiClockSelection = RCC_QSPICLKSOURCE_PLL;
    // SDMMC from PLL1 qclk
    PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
    // LPUART from PLL2 qclk
    PeriphClkInitStruct.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PLL2;
    // USART from PLL2 qclk
    PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_PLL2;
    // USART from PLL2 qclk
    PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_PLL2;
    // I2C123 from PLL3 rclk
    PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_PLL3;
    // I2C4 from PLL3 rclk
    PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PLL3;
    // SPI123 from PLL2 pclk
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
    // SPI45 from PLL2 qclk
    PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_PLL2;
    // SPI6 from PLL2 qclk
    PeriphClkInitStruct.Spi6ClockSelection = RCC_SPI6CLKSOURCE_PLL2;


    if ( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInitStruct ) != HAL_OK )
    {
        Error_Handler();
    }
}
