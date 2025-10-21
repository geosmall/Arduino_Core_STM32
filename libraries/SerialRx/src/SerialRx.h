/*
 * SerialRx - Arduino library for serial RC protocols
 *
 * Main API for serial RC receiver communication
 */

#pragma once

#include <Arduino.h>
#include "RCMessage.h"
#include "ProtocolParser.h"
#include "parsers/IBusParser.h"
#include "parsers/SBusParser.h"

/**
 * @brief Serial RC Receiver transport layer
 * @details Manages Arduino HardwareSerial interface and protocol parsing
 */
class SerialRx {
public:
    /**
     * @brief Supported RC protocols
     */
    enum Protocol {
        NONE = 0,
        IBUS,
        SBUS,  // Future
    };

    /**
     * @brief Configuration structure
     */
    struct Config {
        HardwareSerial* serial;    // Pointer to Serial1, Serial2, etc.
        Protocol rx_protocol;       // RC receiver protocol type
        uint32_t baudrate;         // Serial baudrate
        uint32_t timeout_ms;       // Message timeout in milliseconds
        uint32_t idle_threshold_us; // Idle line detection threshold (0 = disabled)

        // Default constructor
        Config()
            : serial(nullptr)
            , rx_protocol(NONE)
            , baudrate(115200)
            , timeout_ms(1000)
            , idle_threshold_us(0) {}  // Disabled by default
    };

    /**
     * @brief Constructor
     * @param protocol Protocol type (default: IBUS)
     */
    SerialRx(Protocol protocol = IBUS);

    /**
     * @brief Destructor
     */
    ~SerialRx();

    /**
     * @brief Initialize serial receiver
     * @param config Configuration structure
     * @return true if successful
     */
    bool begin(const Config& config);

    /**
     * @brief Update receiver (call in loop())
     * @details Polls Serial.available() and feeds bytes to parser
     */
    void update();

    /**
     * @brief Check if messages are available
     * @return true if messages in queue
     */
    bool available() const;

    /**
     * @brief Get next message from queue
     * @param msg Pointer to RCMessage to fill
     * @return true if message retrieved
     */
    bool getMessage(RCMessage* msg);

    /**
     * @brief Check if receiver has timed out
     * @param threshold_ms Timeout threshold in milliseconds
     * @return true if time since last message > threshold
     */
    bool timeout(uint32_t threshold_ms) const;

    /**
     * @brief Get time since last valid message
     * @return Time in milliseconds
     */
    uint32_t timeSinceLastMessage() const;

    /**
     * @brief Send telemetry data (future)
     * @param data Telemetry data buffer
     * @param len Data length
     * @return true if sent successfully
     */
    bool sendTelemetry(uint8_t* data, size_t len);

private:
    HardwareSerial* serial_;         // Serial port pointer
    ProtocolParser* parser_;         // Protocol parser instance
    Protocol protocol_;              // Current protocol
    uint32_t timeout_ms_;            // Configured timeout
    uint32_t last_message_time_;     // millis() of last valid message
    uint32_t idle_threshold_us_;     // Idle detection threshold (0 = disabled)
    uint32_t last_byte_time_us_;     // micros() of last received byte
    bool expect_frame_start_;        // Next byte should be frame start after idle
};
