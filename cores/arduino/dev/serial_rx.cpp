#include "serial_rx.h"

namespace daisy
{
static constexpr size_t kDefaultSerRxRxBufferSize = 256;

static uint8_t DMA_BUFFER_MEM_SECTION
    default_ibus_rx_buffer[kDefaultSerRxRxBufferSize] = {0};

SerRxUartTransport::Config::Config()
{
    periph         = UartHandler::Config::Peripheral::USART_1;
    rx             = {DSY_GPIOB, 7};
    tx             = {DSY_GPIOB, 6};
    rx_buffer      = default_ibus_rx_buffer;
    rx_buffer_size = kDefaultSerRxRxBufferSize;
}

} // namespace daisy
