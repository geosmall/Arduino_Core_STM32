#include "spi_util.h"

uint32_t getClkFreqInst(SPI_TypeDef* spi_inst)
{
    uint32_t spi_freq = SystemCoreClock;
    if (spi_inst != NULL)
    {
#if defined(SPI1_BASE)
        if (spi_inst == SPI1)
        {
#if defined(RCC_PERIPHCLK_SPI1) || defined(RCC_PERIPHCLK_SPI123)
#ifdef RCC_PERIPHCLK_SPI1
            spi_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI1);
#else
            spi_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI123);
#endif /* RCC_PERIPHCLK_SPI1 */
            if (spi_freq == 0)
#endif /* defined(RCC_PERIPHCLK_SPI1) || defined(RCC_PERIPHCLK_SPI123) */
            {
                /* SPI1, SPI4, SPI5 and SPI6. Source CLK is PCKL2 */
                spi_freq = HAL_RCC_GetPCLK2Freq();
            }
        }
#endif // SPI1_BASE
#if defined(SPI2_BASE)
        if (spi_inst == SPI2)
        {
#if defined(RCC_PERIPHCLK_SPI2) || defined(RCC_PERIPHCLK_SPI123) || defined(RCC_PERIPHCLK_SPI23)
#ifdef RCC_PERIPHCLK_SPI2
            spi_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI2);
#elif defined(RCC_PERIPHCLK_SPI123)
            spi_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI123);
#else
            spi_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI23);
#endif
            if (spi_freq == 0)
#endif
            {
                /* SPI_2 and SPI_3. Source CLK is PCKL1 */
                spi_freq = HAL_RCC_GetPCLK1Freq();
            }
        }
#endif // SPI2_BASE
#if defined(SPI3_BASE)
        if (spi_inst == SPI3)
        {
#if defined(RCC_PERIPHCLK_SPI3) || defined(RCC_PERIPHCLK_SPI123) || defined(RCC_PERIPHCLK_SPI23)
#ifdef RCC_PERIPHCLK_SPI3
            spi_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI3);
#elif defined(RCC_PERIPHCLK_SPI123)
            spi_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI123);
#else
            spi_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI23);
#endif
            if (spi_freq == 0)
#endif
            {
                /* SPI_2 and SPI_3. Source CLK is PCKL1 */
                spi_freq = HAL_RCC_GetPCLK1Freq();
            }
        }
#endif // SPI3_BASE
#if defined(SPI4_BASE)
        if (spi_inst == SPI4)
        {
#if defined(RCC_PERIPHCLK_SPI4) || defined(RCC_PERIPHCLK_SPI45)
#ifdef RCC_PERIPHCLK_SPI4
            spi_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI4);
#else
            spi_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI45);
#endif
            if (spi_freq == 0)
#endif
            {
                /* SPI1, SPI4, SPI5 and SPI6. Source CLK is PCKL2 */
                spi_freq = HAL_RCC_GetPCLK2Freq();
            }
        }
#endif // SPI4_BASE
#if defined(SPI5_BASE)
        if (spi_inst == SPI5)
        {
#if defined(RCC_PERIPHCLK_SPI5) || defined(RCC_PERIPHCLK_SPI45)
#ifdef RCC_PERIPHCLK_SPI5
            spi_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI5);
#else
            spi_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI45);
#endif
            if (spi_freq == 0)
#endif
            {
                /* SPI1, SPI4, SPI5 and SPI6. Source CLK is PCKL2 */
                spi_freq = HAL_RCC_GetPCLK2Freq();
            }
        }
#endif // SPI5_BASE
#if defined(SPI6_BASE)
        if (spi_inst == SPI6)
        {
#if defined(RCC_PERIPHCLK_SPI6)
            spi_freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI6);
            if (spi_freq == 0)
#endif
            {
                /* SPI1, SPI4, SPI5 and SPI6. Source CLK is PCKL2 */
                spi_freq = HAL_RCC_GetPCLK2Freq();
            }
        }
#endif // SPI6_BASE
    }
    return spi_freq;
}