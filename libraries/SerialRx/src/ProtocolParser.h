/*
 * SerialRx - Arduino library for serial RC protocols
 *
 * Abstract base class for protocol parsers
 */

#pragma once

#include "RCMessage.h"
#include "RingBuffer.h"

/**
 * @brief Abstract base class for RC protocol parsers
 * @details Provides common interface for byte-by-byte parsing
 *          and message queue management
 */
class ProtocolParser {
public:
    virtual ~ProtocolParser() = default;

    /**
     * @brief Process a single byte from serial stream
     * @param byte The byte to parse
     * @return true if a complete message was parsed
     */
    virtual bool ParseByte(uint8_t byte) = 0;

    /**
     * @brief Reset parser state to initial condition
     */
    virtual void ResetParser() = 0;

    /**
     * @brief Check if messages are available in queue
     * @return true if queue has messages
     */
    inline bool Listener() const {
        return !msg_q_.IsEmpty();
    }

    /**
     * @brief Retrieve next message from queue
     * @param msg Pointer to RCMessage to fill
     * @return true if message retrieved, false if queue empty or null pointer
     */
    inline bool GetMessageFromFIFO(RCMessage* msg) {
        if (msg == nullptr) {
            return false;
        }
        return msg_q_.Get(*msg);
    }

    /**
     * @brief Get number of messages in queue
     * @return Message count
     */
    inline size_t GetMessageCount() const {
        return msg_q_.Count();
    }

protected:
    // Working message buffer for parser
    RCMessage msg_;

    // Message queue (16-message depth)
    RingBuffer<RCMessage, 16> msg_q_;

    /**
     * @brief Notify that a message was successfully parsed
     * @details Call this from derived parser when complete message received
     */
    inline void ParserNotify() {
        msg_q_.PutWithOverwrite(msg_);
    }
};
