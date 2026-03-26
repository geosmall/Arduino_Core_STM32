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
#include "parsers/CRSFParser.h"

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
        SBUS,
        CRSF,
    };

    /**
     * @brief Signal status for failsafe diagnostics
     */
    enum class SignalStatus : uint8_t {
        OK,              // Signal good
        TIMEOUT,         // No frames received (Layer 2)
        FAILSAFE_FLAG,   // Protocol failsafe flag set (Layer 1, SBUS only)
        OUT_OF_RANGE,    // Channel value outside valid range (Layer 3)
        EXPIRED          // Channel value stale (Layer 4)
    };

    /**
     * @brief Per-channel failsafe state
     */
    struct ChannelState {
        uint16_t lastValidPWM;   // Last known good value (PWM µs)
        uint32_t expiresAt;      // millis() when this channel expires
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
        // Signal inversion (required for SBUS)
        bool invert_rx;            // Enable UART RX signal inversion (default: false)
                                   // Supported on STM32F7, H7, G4 (hardware RXINV)
                                   // F4 requires external inverter circuit
        // Failsafe configuration
        uint32_t frame_timeout_ms;  // Frame timeout in ms (default: 100)
        uint32_t channel_expiry_ms; // Per-channel expiry in ms (default: 300)
        uint16_t valid_pulse_min;   // Minimum valid pulse µs (default: 885, INAV rx_min_usec)
        uint16_t valid_pulse_max;   // Maximum valid pulse µs (default: 2115, INAV rx_max_usec)

        // Default constructor
        Config()
            : serial(nullptr)
            , rx_protocol(NONE)
            , baudrate(115200)
            , timeout_ms(1000)
            , idle_threshold_us(0)
            , invert_rx(false)
            , frame_timeout_ms(100)
            , channel_expiry_ms(300)
            , valid_pulse_min(885)
            , valid_pulse_max(2115) {}
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
     * @brief Stop serial receiver and cleanup
     */
    void end();

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
     * @brief Get count of successfully received frames
     * @return Number of valid frames
     */
    uint32_t getFramesReceived() const;

    /**
     * @brief Get count of frames that failed validation
     * @return Number of failed frames (checksum errors)
     */
    uint32_t getFramesFailed() const;

    /**
     * @brief Get frame loss percentage
     * @return Loss rate as percentage (0.0 - 100.0)
     */
    float getFrameLossPercent() const;

    /**
     * @brief Reset frame statistics counters
     */
    void resetStatistics();

    /**
     * @brief Convert raw channel value to PWM microseconds
     * @param raw Raw channel value (protocol-specific range)
     * @param protocol Protocol type (SBUS or IBUS)
     * @return PWM value in microseconds (~988-2012 range)
     * @details Static method for manual conversion with explicit protocol.
     *          SBUS: Uses iNav formula (5 * raw / 8) + 880
     *          IBus: Already in PWM µs, passed through unmodified
     */
    static uint16_t toPWM(uint16_t raw, Protocol protocol);

    /**
     * @brief Convert raw channel value to PWM using configured protocol
     * @param raw Raw channel value from RCMessage
     * @return PWM value in microseconds (~988-2012 range)
     * @details Instance method that uses the protocol set in begin()
     */
    uint16_t channelToPWM(uint16_t raw) const;

    /**
     * @brief Get current protocol
     * @return Configured protocol type
     */
    Protocol getProtocol() const { return protocol_; }

    // --- Failsafe API ---

    /**
     * @brief Check if RC signal is lost (primary failsafe API)
     * @return true if signal lost (any layer triggered)
     */
    bool isSignalLost();

    /**
     * @brief Get detailed signal status for diagnostics
     * @return Most recent SignalStatus value
     */
    SignalStatus getSignalStatus() const;

    /**
     * @brief Get signal status as human-readable string
     * @return "OK", "TIMEOUT", "FAILSAFE_FLAG", "OUT_OF_RANGE", or "EXPIRED"
     */
    const char* getSignalStatusString() const;

    /**
     * @brief Set frame timeout (Layer 2)
     * @param ms Timeout in milliseconds (default: 100)
     */
    void setFrameTimeoutMs(uint32_t ms) { frameTimeoutMs_ = ms; }

    /**
     * @brief Set per-channel expiry time (Layer 4)
     * @param ms Expiry in milliseconds (default: 300)
     */
    void setChannelExpiryMs(uint32_t ms) { channelExpiryMs_ = ms; }

    /**
     * @brief Set valid pulse range (Layer 3)
     * @param min Minimum valid pulse µs (default: 885, INAV rx_min_usec)
     * @param max Maximum valid pulse µs (default: 2115, INAV rx_max_usec)
     */
    void setValidPulseRange(uint16_t min, uint16_t max) { validPulseMin_ = min; validPulseMax_ = max; }

    /**
     * @brief Get last valid PWM value for a channel (held during failsafe)
     * @param channel Channel index (0-3 for AETR)
     * @return Last valid PWM in µs, or 0 if channel out of range
     */
    uint16_t getLastValidPWM(uint8_t channel) const;

    /**
     * @brief Check if a specific channel has expired
     * @param channel Channel index (0-3 for AETR)
     * @return true if expired, or true if channel out of range
     */
    bool isChannelExpired(uint8_t channel) const;

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

    // Failsafe state
    ChannelState channelState_[4];   // AETR channels only
    bool rxSignalReceived_;          // Frames arriving, no protocol failsafe
    bool rxFlightChannelsValid_;     // All AETR channels valid and not expired
    SignalStatus lastSignalStatus_;  // Most recent status for diagnostics
    uint32_t frameTimeoutMs_;        // Frame timeout (Layer 2)
    uint32_t channelExpiryMs_;       // Per-channel expiry (Layer 4)
    uint16_t validPulseMin_;         // Minimum valid pulse µs (Layer 3)
    uint16_t validPulseMax_;         // Maximum valid pulse µs (Layer 3)

    // Failsafe helpers
    void updateFailsafeState(const RCMessage& frame);
    void processChannel(uint8_t ch, uint16_t pwm);
    bool isValidPulse(uint16_t pwm) const;
};
