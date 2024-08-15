#include "serial_rx.h"

namespace daisy
{
static constexpr size_t kDefaultSerRxRxBufferSize = 256;

/** @brief Shared buffer in DMA_BUFFER_MEM_SECTION
 *  @details DMA UART rx byte transfer in background. Can only
 *           be utilized for a single UART peripheral. To use
 *           multiple serial_rx (w/ multiple UART periphs), you
 *           must provide your own buffer which is allocated
 *           within a DMA-capable memory section.
 */
static uint8_t DMA_BUFFER_MEM_SECTION
    default_serial_rx_buffer[kDefaultSerRxRxBufferSize] = {0};

/** @brief Default constructor for SerRxUartTransport::Config
 *  @details Initializes the configuration struct with default values
 *           for the UART peripheral, rx and tx pins, rx_buffer,
 *           and rx_buffer_size.
 */
SerRxUartTransport::Config::Config()
{
    periph         = UartHandler::Config::Peripheral::USART_1;
    rx             = {DSY_GPIOB, 7};
    tx             = {DSY_GPIOB, 6};
    rx_buffer      = default_serial_rx_buffer;
    rx_buffer_size = kDefaultSerRxRxBufferSize;
}

} // namespace daisy
