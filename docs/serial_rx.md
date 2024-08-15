geosmall: serial_rx has class SerRxUartTransport that is meant to implement a universal serial bus receiver that decodes a streaming RC serial protocol (IBus, SBus, etc).  The present implementation uses a Transport and a template to implement.  Is there a better, more flexible way to implement this module to accomodate multiple protocols using modern C++ concepts?

GitHub Copilot: To make the SerRxUartTransport class more flexible and accommodate multiple protocols using modern C++ concepts, we can use polymorphism and the Strategy design pattern. This approach allows us to define a common interface for different protocols and implement specific protocol logic in separate classes. Here's a step-by-step plan:

1. **Define a common interface for protocols**: Create an abstract base class `IProtocol` that defines the methods required for parsing and handling data.
2. **Implement specific protocols**: Create concrete classes that implement the `IProtocol` interface for each protocol (e.g., `IBusProtocol`, `SBusProtocol`).
3. **Modify SerRxUartTransport to use the protocol interface**: Update the SerRxUartTransport class to accept an `IProtocol` object and delegate protocol-specific operations to it.

### Step 1: Define a common interface for protocols

```cpp
#pragma once

#include <cstdint>
#include <cstddef>

class IProtocol
{
public:
    virtual ~IProtocol() = default;

    virtual void Init() = 0;
    virtual void Parse(uint8_t* data, size_t size) = 0;
    virtual void Reset() = 0;
};
```

### Step 2: Implement specific protocols

```cpp
#pragma once

#include "IProtocol.h"
#include "ibus_parser.h"
#include "FIFO.h"

class IBusProtocol : public IProtocol
{
public:
    void Init() override
    {
        parser_.Init();
    }

    void Parse(uint8_t* data, size_t size) override
    {
        for(size_t i = 0; i < size; i++)
        {
            SerRxEvent event;
            if(parser_.Parse(data[i], &event))
            {
                event_q_.PushBack(event);
            }
        }
    }

    void Reset() override
    {
        parser_.Reset();
    }

    bool HasEvents() const
    {
        return event_q_.GetNumElements() > 0;
    }

    SerRxEvent PopEvent()
    {
        return event_q_.PopFront();
    }

private:
    IBusParser parser_;
    FIFO<SerRxEvent, 256> event_q_;
};

// Similarly, you can implement SBusProtocol, etc.
```

### Step 3: Modify SerRxUartTransport to use the protocol interface

```cpp
#pragma once

#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include "per/uart.h"
#include "sys/dma.h"
#include "sys/system.h"
#include "IProtocol.h"

namespace daisy
{
class SerRxUartTransport
{
public:
    typedef void (*SerRxParseCallback)(uint8_t* data, size_t size, void* context);

    SerRxUartTransport(IProtocol* protocol) : protocol_(protocol) {}
    ~SerRxUartTransport() {}

    struct Config
    {
        UartHandler::Config::Peripheral periph;
        dsy_gpio_pin rx;
        dsy_gpio_pin tx;
        uint8_t* rx_buffer;
        size_t rx_buffer_size;

        Config();
    };

    void Init(Config config)
    {
        UartHandler::Config uart_config;
        uart_config.baudrate = 115200;
        uart_config.stopbits = UartHandler::Config::StopBits::BITS_1;
        uart_config.parity = UartHandler::Config::Parity::NONE;
        uart_config.mode = UartHandler::Config::Mode::TX_RX;
        uart_config.wordlength = UartHandler::Config::WordLength::BITS_8;

        uart_config.periph = config.periph;
        uart_config.pin_config.rx = config.rx;
        uart_config.pin_config.tx = config.tx;

        rx_buffer = config.rx_buffer;
        rx_buffer_size = config.rx_buffer_size;

        std::fill(rx_buffer, rx_buffer + rx_buffer_size, 0);

        uart_.Init(uart_config);
        protocol_->Init();
    }

    void StartRx(SerRxParseCallback parse_callback, void* context)
    {
        parse_context_ = context;
        parse_callback_ = parse_callback;
        dsy_dma_clear_cache_for_buffer((uint8_t*)this, sizeof(SerRxUartTransport));
        uart_.DmaListenStart(rx_buffer, rx_buffer_size, SerRxUartTransport::rxCallback, this);
    }

    bool RxActive() { return uart_.IsListening(); }

    void FlushRx() {}

    void Tx(uint8_t* buff, size_t size) { uart_.PollTx(buff, size); }

private:
    UartHandler uart_;
    uint8_t* rx_buffer;
    size_t rx_buffer_size;
    void* parse_context_;
    SerRxParseCallback parse_callback_;
    IProtocol* protocol_;

    static void rxCallback(uint8_t* data, size_t size, void* context, UartHandler::Result res)
    {
        SerRxUartTransport* transport = reinterpret_cast<SerRxUartTransport*>(context);
        if(res == UartHandler::Result::OK)
        {
            if(transport->parse_callback_)
            {
                transport->parse_callback_(data, size, transport->parse_context_);
            }
            transport->protocol_->Parse(data, size);
        }
    }
};
} // namespace daisy
```

### Usage Example

```cpp
#include "serial_rx.h"
#include "IBusProtocol.h"

int main()
{
    daisy::SerRxUartTransport::Config config;
    // Initialize config with appropriate values

    IBusProtocol ibus_protocol;
    daisy::SerRxUartTransport transport(&ibus_protocol);

    transport.Init(config);
    transport.StartRx(nullptr, nullptr);

    // Main loop
    while(true)
    {
        if(ibus_protocol.HasEvents())
        {
            SerRxEvent event = ibus_protocol.PopEvent();
            // Handle event
        }
    }

    return 0;
}
```

This approach makes the SerRxUartTransport class more flexible and allows you to easily add support for new protocols by implementing the `IProtocol` interface.

geosmall: Could you revise your answer to accomodate two protocols (IBusProtocol and SBusProtocol)?

GitHub Copilot: Sure, let's revise the implementation to accommodate both `IBusProtocol` and `SBusProtocol` using the Strategy design pattern. We'll create an interface for protocols, implement the specific protocols, and modify the SerRxUartTransport class to use the protocol interface.

### Step 1: Define a common interface for protocols

```cpp
#pragma once

#include <cstdint>
#include <cstddef>

class IProtocol
{
public:
    virtual ~IProtocol() = default;

    virtual void Init() = 0;
    virtual void Parse(uint8_t* data, size_t size) = 0;
    virtual void Reset() = 0;
};
```

### Step 2: Implement specific protocols

#### IBusProtocol

```cpp
#pragma once

#include "IProtocol.h"
#include "serial_rx/ibus_parser.h"
#include "util/FIFO.h"

class IBusProtocol : public IProtocol
{
public:
    void Init() override
    {
        parser_.Init();
    }

    void Parse(uint8_t* data, size_t size) override
    {
        for(size_t i = 0; i < size; i++)
        {
            SerRxEvent event;
            if(parser_.Parse(data[i], &event))
            {
                event_q_.PushBack(event);
            }
        }
    }

    void Reset() override
    {
        parser_.Reset();
    }

    bool HasEvents() const
    {
        return event_q_.GetNumElements() > 0;
    }

    SerRxEvent PopEvent()
    {
        return event_q_.PopFront();
    }

private:
    IBusParser parser_;
    FIFO<SerRxEvent, 256> event_q_;
};
```

#### SBusProtocol

```cpp
#pragma once

#include "IProtocol.h"
// Include necessary headers for SBus parsing

class SBusProtocol : public IProtocol
{
public:
    void Init() override
    {
        // Initialize SBus parser
    }

    void Parse(uint8_t* data, size_t size) override
    {
        // Parse SBus data
    }

    void Reset() override
    {
        // Reset SBus parser
    }

    bool HasEvents() const
    {
        // Return true if there are events
        return false;
    }

    SerRxEvent PopEvent()
    {
        // Pop and return the next event
        return SerRxEvent();
    }

private:
    // SBus parser and event queue
};
```

### Step 3: Modify SerRxUartTransport to use the protocol interface

```cpp
#pragma once

#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include "per/uart.h"
#include "sys/dma.h"
#include "sys/system.h"
#include "IProtocol.h"

namespace daisy
{
class SerRxUartTransport
{
public:
    typedef void (*RxParseCallback)(uint8_t* data, size_t size, void* context);

    SerRxUartTransport(IProtocol* protocol) : protocol_(protocol) {}
    ~SerRxUartTransport() {}

    struct Config
    {
        UartHandler::Config::Peripheral periph;
        dsy_gpio_pin rx;
        dsy_gpio_pin tx;
        uint8_t* rx_buffer;
        size_t rx_buffer_size;

        Config();
    };

    void Init(Config config)
    {
        UartHandler::Config uart_config;
        uart_config.baudrate = 115200;
        uart_config.stopbits = UartHandler::Config::StopBits::BITS_1;
        uart_config.parity = UartHandler::Config::Parity::NONE;
        uart_config.mode = UartHandler::Config::Mode::TX_RX;
        uart_config.wordlength = UartHandler::Config::WordLength::BITS_8;

        uart_config.periph = config.periph;
        uart_config.pin_config.rx = config.rx;
        uart_config.pin_config.tx = config.tx;

        rx_buffer = config.rx_buffer;
        rx_buffer_size = config.rx_buffer_size;

        std::fill(rx_buffer, rx_buffer + rx_buffer_size, 0);

        uart_.Init(uart_config);
        protocol_->Init();
    }

    void StartRx(RxParseCallback parse_callback, void* context)
    {
        parse_context_ = context;
        parse_callback_ = parse_callback;
        dsy_dma_clear_cache_for_buffer((uint8_t*)this, sizeof(SerRxUartTransport));
        uart_.DmaListenStart(rx_buffer, rx_buffer_size, SerRxUartTransport::rxCallback, this);
    }

    bool RxActive() { return uart_.IsListening(); }

    void FlushRx() {}

    void Tx(uint8_t* buff, size_t size) { uart_.PollTx(buff, size); }

private:
    UartHandler uart_;
    uint8_t* rx_buffer;
    size_t rx_buffer_size;
    void* parse_context_;
    RxParseCallback parse_callback_;
    IProtocol* protocol_;

    static void rxCallback(uint8_t* data, size_t size, void* context, UartHandler::Result res)
    {
        SerRxUartTransport* transport = reinterpret_cast<SerRxUartTransport*>(context);
        if(res == UartHandler::Result::OK)
        {
            if(transport->parse_callback_)
            {
                transport->parse_callback_(data, size, transport->parse_context_);
            }
            transport->protocol_->Parse(data, size);
        }
    }
};
} // namespace daisy
```

### Step 4: Update IBusHandlerto use the new SerRxUartTransport

```cpp
#pragma once

#include "serial_rx.h"
#include "IProtocol.h"

namespace daisy
{
template <typename Protocol>
class IBusHandler
{
public:
    IBusHandler() : protocol_(new Protocol()), transport_(protocol_) {}
    ~IBusHandler() { delete protocol_; }

    struct Config
    {
        typename SerRxUartTransport::Config transport_config;
    };

    void Init(Config config)
    {
        config_ = config;
        transport_.Init(config_.transport_config);
        protocol_->Init();
    }

    void StartReceive()
    {
        transport_.StartRx(IBusHandler::ParseCallback, this);
    }

    void Listen()
    {
        if(!transport_.RxActive())
        {
            protocol_->Reset();
            transport_.FlushRx();
            StartReceive();
        }
    }

    bool HasEvents() const { return protocol_->HasEvents(); }

    SerRxEvent PopEvent() { return protocol_->PopEvent(); }

    void SendMessage(uint8_t* bytes, size_t size)
    {
        transport_.Tx(bytes, size);
    }

    void Parse(uint8_t byte)
    {
        protocol_->Parse(&byte, 1);
    }

private:
    Config config_;
    Protocol* protocol_;
    SerRxUartTransport transport_;

    static void ParseCallback(uint8_t* data, size_t size, void* context)
    {
        IBusHandler* handler = reinterpret_cast<IBusHandler*>(context);
        for(size_t i = 0; i < size; i++)
        {
            handler->Parse(data[i]);
        }
    }
};

using IBusRxHandler = IBusHandler<IBusProtocol>;
using SBusRxHandler = IBusHandler<SBusProtocol>;
} // namespace daisy
```

### Usage Example

```cpp
#include "serial_rx.h"
#include "IBusProtocol.h"
#include "SBusProtocol.h"

int main()
{
    daisy::SerRxUartTransport::Config config;
    // Initialize config with appropriate values

    daisy::IBusRxHandler ibus_handler;
    ibus_handler.Init({config});
    ibus_handler.StartReceive();

    daisy::SBusRxHandler sbus_handler;
    sbus_handler.Init({config});
    sbus_handler.StartReceive();

    // Main loop
    while(true)
    {
        if(ibus_handler.HasEvents())
        {
            SerRxEvent event = ibus_handler.PopEvent();
            // Handle IBus event
        }

        if(sbus_handler.HasEvents())
        {
            SerRxEvent event = sbus_handler.PopEvent();
            // Handle SBus event
        }
    }

    return 0;
}
```

This approach makes the SerRxUartTransport class more flexible and allows you to easily add support for new protocols by implementing the `IProtocol` interface.
