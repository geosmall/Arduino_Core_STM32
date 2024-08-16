#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <algorithm>
#include "per/uart.h"
#include "util/FIFO.h"
#include "sys/dma.h"
#include "sys/system.h"
#include "serial_rx/ibus_parser.h"

namespace daisy
{
/** @brief   Transport layer for sending and receiving IBUS data over UART
 *  @details This is the mode of communication used for serial Rx comms
 *           There is an additional 2kB of RAM data used within this class
 *           for processing bulk data from the UART peripheral
 *  @ingroup ibus
 */
class SerRxUartTransport
{
  public:
    typedef void (*SerRxParseCallback)(uint8_t* data,
                                       size_t   size,
                                       void*    context);

    SerRxUartTransport() : rx_buffer(nullptr), rx_buffer_size(0), parse_context_(nullptr), parse_callback_(nullptr) {}
    ~SerRxUartTransport() {}

    /** @brief Configuration structure for UART IBUS */
    struct Config
    {
        UartHandler::Config::Peripheral periph;
        dsy_gpio_pin                    rx;
        dsy_gpio_pin                    tx;

        /** Pointer to buffer for DMA UART rx byte transfer in background.
         *
         *  @details By default this uses a shared buffer in DMA_BUFFER_MEM_SECTION,
         *           which can only be utilized for a single UART peripheral. To
         *           use IBUS with multiple UART peripherals, you must provide your own
         *           buffer, allocated to a DMA-capable memory section.
         */
        uint8_t* rx_buffer;

        /** Size in bytes of rx_buffer.
         *
         *  @details This size determines the maximum Rx bytes readable by the UART in the background.
         *           By default it's set to the size of the default shared rx_buffer (256 bytes).
         *           While much smaller sizes can be used, data can get missed if the buffer is too small.
         */
        size_t rx_buffer_size;

        Config() : rx_buffer(nullptr), rx_buffer_size(256) {}
    };

    /** @brief Initialization of UART using config struct */
    inline void Init(const Config* config)
    {
        UartHandler::Config uart_config;

        // defaults
        uart_config.baudrate   = 115200;
        uart_config.stopbits   = UartHandler::Config::StopBits::BITS_1;
        uart_config.parity     = UartHandler::Config::Parity::NONE;
        uart_config.mode       = UartHandler::Config::Mode::TX_RX;
        uart_config.wordlength = UartHandler::Config::WordLength::BITS_8;

        // user settings
        uart_config.periph        = config->periph;
        uart_config.pin_config.rx = config->rx;
        uart_config.pin_config.tx = config->tx;

        rx_buffer      = config->rx_buffer;
        rx_buffer_size = config->rx_buffer_size;

        /** zero the buffer to ensure emptiness regardless of source memory */
        std::fill(rx_buffer, rx_buffer + rx_buffer_size, 0);

        uart_.Init(uart_config);
    }

    /** @brief Start the UART peripheral in listening mode.
     *  This will fill an internal data structure in the background */
    inline void StartRx(SerRxParseCallback parse_callback, void* context)
    {
        parse_context_  = context;
        parse_callback_ = parse_callback;
        dsy_dma_clear_cache_for_buffer((uint8_t*)this,
                                       sizeof(SerRxUartTransport));
        uart_.DmaListenStart(
            rx_buffer, rx_buffer_size, SerRxUartTransport::rxCallback, this);
    }

    /** @brief returns whether the UART peripheral is actively listening in the background or not */
    inline bool RxActive() const { return uart_.IsListening(); }

    /** @brief This is a no-op for UART transport - Rx is via DMA callback with circular buffer */
    inline void FlushRx() {}

    /** @brief sends the buffer of bytes out of the UART peripheral */
    inline void Tx(uint8_t* buff, size_t size) { uart_.PollTx(buff, size); }

  private:
    UartHandler        uart_;
    uint8_t*           rx_buffer;
    size_t             rx_buffer_size;
    void*              parse_context_;
    SerRxParseCallback parse_callback_;

    /** Static callback for Uart IBUS that occurs when
     *  new data is available from the peripheral.
     *  The new data is transferred from the peripheral to the
     *  IBUS instance's byte FIFO that feeds the IBUS parser.
     *
     *  TODO: Handle UartHandler errors better/at all.
     *  (If there is a UART error, there's not really any recovery
     *  option at the moment)
     */
    static void rxCallback(uint8_t*            data,
                           size_t              size,
                           void*               context,
                           UartHandler::Result res)
    {
        /** Read context as transport type */
        SerRxUartTransport* transport
            = reinterpret_cast<SerRxUartTransport*>(context);
        if(res == UartHandler::Result::OK)
        {
            if(transport->parse_callback_)
            {
                transport->parse_callback_(
                    data, size, transport->parse_context_);
            }
        }
    }
};

/**
    @brief Simple IBUS Handler \n
    Parses bytes from an input into valid IBusEvents. \n
    The IBusEvents fill a FIFO queue that the user can pop messages from.
*/
class IBusHandler
{
  public:
    IBusHandler() {}
    ~IBusHandler() {}

    struct Config
    {
        SerRxUartTransport::Config transport_config;
    };

    /** Initializes the IBusHandler
     *  \param config Configuration structure used to define specifics to the IBUS Handler.
     */
    inline void Init(const Config* config)
    {
        config_ = *config;
        transport_.Init(&config_.transport_config);
        parser_.Init();
    }

    /** Starts listening on the selected input mode(s).
     * IBusEvent Queue will begin to fill, and can be checked with HasEvents() */
    inline void StartReceive()
    {
        transport_.StartRx(IBusHandler::ParseCallback, this);
    }

    /** Start listening */
    inline void Listen()
    {
        // In case of UART Error, (particularly
        //  overrun error), UART disables itself.
        // Flush the buff, and restart.
        if(!transport_.RxActive())
        {
            parser_.Reset();
            transport_.FlushRx();
            StartReceive();
        }
    }

    /** Checks if there are unhandled messages in the queue
    \return True if there are events to be handled, else false.
     */
    inline bool HasEvents() const { return event_q_.GetNumElements() > 0; }

    /** Pops the oldest unhandled SerRxEvent from the internal queue
    \return The event to be handled
     */
    inline SerRxEvent PopEvent() { return event_q_.PopFront(); }

    /** SendMessage
    Send raw bytes as message
    */
    inline void SendMessage(uint8_t* bytes, size_t size)
    {
        transport_.Tx(bytes, size);
    }

    /** Feed in bytes to parser state machine from an external source.
        Populates internal FIFO queue with IBUS Messages.

        \note  Normally application code won't need to use this method directly.
        \param byte IBUS byte to be parsed
    */
    inline void Parse(uint8_t byte)
    {
        SerRxEvent event;
        if(parser_.Parse(byte, &event))
        {
            event_q_.PushBack(event);
        }
    }

  private:
    Config                config_;
    SerRxUartTransport    transport_;
    IBusParser            parser_;
    FIFO<SerRxEvent, 256> event_q_;

    static inline void ParseCallback(uint8_t* data, size_t size, void* context)
    {
        IBusHandler* handler = reinterpret_cast<IBusHandler*>(context);
        for(size_t i = 0; i < size; i++)
        {
            handler->Parse(data[i]);
        }
    }
};

/**
 *  @{
 *  @ingroup ibus
 *  @brief shorthand accessors for IBUS Handlers
 * */
using IBusRxHandler = IBusHandler;
/** @} */
} // namespace daisy
