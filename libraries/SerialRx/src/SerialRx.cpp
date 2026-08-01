// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 George Small
// See the LICENSE file in this library's root directory.

/*
 * SerialRx - Arduino library for serial RC protocols
 *
 * Transport layer implementation
 */

#include "SerialRx.h"

SerialRx::SerialRx(Protocol protocol)
    : serial_(nullptr)
    , parser_(nullptr)
    , protocol_(protocol)
    , timeout_ms_(1000)
    , last_message_time_(0)
    , idle_threshold_us_(0)
    , last_byte_time_us_(0)
    , expect_frame_start_(false)
    , channelState_{}
    , rxSignalReceived_(false)
    , rxFlightChannelsValid_(false)
    , lastSignalStatus_(SignalStatus::TIMEOUT)
    , frameTimeoutMs_(100)
    , channelExpiryMs_(300)
    , validPulseMin_(885)
    , validPulseMax_(2115) {
}

SerialRx::~SerialRx() {
    if (parser_ != nullptr) {
        delete parser_;
        parser_ = nullptr;
    }
}

bool SerialRx::begin(const Config& config) {
    if (config.serial == nullptr) {
        return false;
    }

    serial_ = config.serial;
    protocol_ = config.rx_protocol;
    timeout_ms_ = config.timeout_ms;
    idle_threshold_us_ = config.idle_threshold_us;
    last_message_time_ = millis();
    last_byte_time_us_ = micros();
    expect_frame_start_ = false;

    // Failsafe configuration
    frameTimeoutMs_ = config.frame_timeout_ms;
    channelExpiryMs_ = config.channel_expiry_ms;
    validPulseMin_ = config.valid_pulse_min;
    validPulseMax_ = config.valid_pulse_max;
    rxSignalReceived_ = false;
    rxFlightChannelsValid_ = false;
    lastSignalStatus_ = SignalStatus::TIMEOUT;
    memset(channelState_, 0, sizeof(channelState_));

    // Create parser based on protocol
    switch (protocol_) {
    case IBUS:
        parser_ = new IBusParser();
        break;
    case SBUS:
        parser_ = new SBusParser();
        break;
    case CRSF:
        parser_ = new CRSFParser();
        break;
    case NONE:
    default:
        return false;
    }

    if (parser_ == nullptr) {
        return false;
    }

    // SBUS requires 8E2 framing (8 data bits, even parity, 2 stop bits)
    // IBus uses standard 8N1 framing
    uint32_t serial_config = (protocol_ == SBUS) ? SERIAL_8E2 : SERIAL_8N1;
    serial_->begin(config.baudrate, serial_config);

    // Configure RX signal inversion if requested (required for SBUS)
    // Hardware support: STM32F7, H7, G4, L4 (USART_CR2_RXINV bit)
    // Not supported: STM32F4 - requires external inverter circuit
    if (config.invert_rx) {
#if defined(USART_CR2_RXINV)
        // Hardware RX inversion supported - set RXINV bit in CR2
        // Note: RXINV can only be modified when UART is disabled (UE=0)
        UART_HandleTypeDef* huart = serial_->getHandle();
        if (huart != nullptr && huart->Instance != nullptr) {
            // Disable UART
            huart->Instance->CR1 &= ~USART_CR1_UE;
            // Set RXINV bit
            huart->Instance->CR2 |= USART_CR2_RXINV;
            // Re-enable UART
            huart->Instance->CR1 |= USART_CR1_UE;
        }
#endif
        // F4 and other unsupported families: no action, external inverter required
    }

    return true;
}

void SerialRx::end() {
    if (serial_ != nullptr) {
        serial_->end();
    }

    if (parser_ != nullptr) {
        parser_->ResetParser();
    }
}

void SerialRx::update() {
    if (serial_ == nullptr || parser_ == nullptr) {
        return;
    }

    uint32_t now = micros();

    // Software idle line detection (optional feature)
    // Only trigger idle detection when:
    // 1) Buffer is empty (no pending data)
    // 2) Sufficient time has passed since last byte
    // This prevents false idle detection when buffer contains partial frames
    if (idle_threshold_us_ > 0 && !serial_->available()) {
        uint32_t idle_time = now - last_byte_time_us_;

        if (idle_time > idle_threshold_us_) {
            if (!expect_frame_start_) {
                // Idle period detected with empty buffer → prepare for guaranteed frame start
                parser_->ResetParser();
                expect_frame_start_ = true;
            }
        }
    }

    // Poll serial buffer and feed bytes to parser
    while (serial_->available()) {
        uint8_t byte = serial_->read();
        last_byte_time_us_ = micros();  // Update timestamp

        // Validate frame start after idle detection
        if (expect_frame_start_) {
            expect_frame_start_ = false;

            // After idle, first byte MUST be frame start (protocol-specific)
            uint8_t expected_header;
            switch (protocol_) {
            case SBUS: expected_header = 0x0F; break;
            case CRSF: expected_header = 0xC8; break;
            default:   expected_header = 0x20; break;  // IBus
            }
            if (byte != expected_header) {
                // Not a valid frame start after idle → discard byte
                continue;
            }
        }

        if (parser_->ParseByte(byte)) {
            // Complete valid message parsed (checksum validated)
            last_message_time_ = millis();

            // Process failsafe state from latest frame
            updateFailsafeState(parser_->GetLatestFrame());
        }
    }

    // Layer 2: Frame timeout check (runs every update, not just on new frames)
    if ((millis() - last_message_time_) > frameTimeoutMs_) {
        rxSignalReceived_ = false;
        lastSignalStatus_ = SignalStatus::TIMEOUT;
    }
}

bool SerialRx::available() const {
    if (parser_ == nullptr) {
        return false;
    }
    return parser_->Listener();
}

bool SerialRx::getMessage(RCMessage* msg) {
    if (parser_ == nullptr || msg == nullptr) {
        return false;
    }
    return parser_->GetMessageFromFIFO(msg);
}

bool SerialRx::timeout(uint32_t threshold_ms) const {
    return (millis() - last_message_time_) > threshold_ms;
}

uint32_t SerialRx::timeSinceLastMessage() const {
    return millis() - last_message_time_;
}

uint32_t SerialRx::getFramesReceived() const {
    if (parser_ == nullptr) return 0;
    return parser_->GetFramesReceived();
}

uint32_t SerialRx::getFramesFailed() const {
    if (parser_ == nullptr) return 0;
    return parser_->GetFramesFailed();
}

float SerialRx::getFrameLossPercent() const {
    if (parser_ == nullptr) return 0.0f;
    return parser_->GetFrameLossPercent();
}

void SerialRx::resetStatistics() {
    if (parser_ != nullptr) {
        parser_->ResetStatistics();
    }
}

uint16_t SerialRx::toPWM(uint16_t raw, Protocol protocol) {
    if (protocol == SBUS || protocol == CRSF) {
        // iNav formula derived from OpenTX/FrSky X4R measurements:
        // http://www.wolframalpha.com/input/?i=linear+fit+%7B173%2C+988%7D%2C+%7B1812%2C+2012%7D%2C+%7B993%2C+1500%7D
        // pwm = (sbus * 5 / 8) + 880
        // Input range: 0-2047 (11-bit), typical 173-1812
        // Output range: 880-2159, typical 988-2012
        uint16_t clamped = (raw > 2047) ? 2047 : raw;
        return (5 * clamped / 8) + 880;
    } else {
        // IBus raw values are already in PWM µs — pass through unmodified
        // No clamping: endpoint trick sends ~880 µs which must reach failsafe
        // range checking (Layer 3) unmodified
        return raw;
    }
}

uint16_t SerialRx::channelToPWM(uint16_t raw) const {
    return toPWM(raw, protocol_);
}

bool SerialRx::sendTelemetry(uint8_t* data, size_t len) {
    // Future implementation for bi-directional telemetry
    if (serial_ == nullptr || data == nullptr || len == 0) {
        return false;
    }

    // TODO: Implement protocol-specific telemetry framing
    serial_->write(data, len);
    return true;
}

// --- Failsafe Implementation ---

bool SerialRx::isSignalLost() {
    // INAV pattern: signal valid only when BOTH conditions true
    return !(rxSignalReceived_ && rxFlightChannelsValid_);
}

SerialRx::SignalStatus SerialRx::getSignalStatus() const {
    return lastSignalStatus_;
}

const char* SerialRx::getSignalStatusString() const {
    switch (lastSignalStatus_) {
    case SignalStatus::OK:             return "OK";
    case SignalStatus::TIMEOUT:        return "TIMEOUT";
    case SignalStatus::FAILSAFE_FLAG:  return "FAILSAFE_FLAG";
    case SignalStatus::OUT_OF_RANGE:   return "OUT_OF_RANGE";
    case SignalStatus::EXPIRED:        return "EXPIRED";
    default:                           return "UNKNOWN";
    }
}

uint16_t SerialRx::getLastValidPWM(uint8_t channel) const {
    if (channel >= 4) return 0;
    return channelState_[channel].lastValidPWM;
}

bool SerialRx::isChannelExpired(uint8_t channel) const {
    if (channel >= 4) return true;
    return millis() > channelState_[channel].expiresAt;
}

void SerialRx::updateFailsafeState(const RCMessage& frame) {
    // Layer 1: Protocol failsafe flag (SBUS only; IBus error_flags always 0)
    if (frame.error_flags & 0x02) {
        rxSignalReceived_ = false;
        lastSignalStatus_ = SignalStatus::FAILSAFE_FLAG;
        return;
    }

    rxSignalReceived_ = true;

    // Layer 3 & 4: Process AETR channels (0-3)
    rxFlightChannelsValid_ = true;
    for (uint8_t ch = 0; ch < 4; ch++) {
        processChannel(ch, channelToPWM(frame.channels[ch]));
    }

    // If all checks passed, signal is OK
    if (rxSignalReceived_ && rxFlightChannelsValid_) {
        lastSignalStatus_ = SignalStatus::OK;
    }
}

void SerialRx::processChannel(uint8_t ch, uint16_t pwm) {
    if (isValidPulse(pwm)) {
        // Valid pulse — update state and reset expiry
        channelState_[ch].lastValidPWM = pwm;
        channelState_[ch].expiresAt = millis() + channelExpiryMs_;
    } else {
        // Invalid pulse (e.g., ~880 µs from endpoint trick)
        // Mark out of range immediately
        rxFlightChannelsValid_ = false;
        lastSignalStatus_ = SignalStatus::OUT_OF_RANGE;

        // Also check if channel has expired (Layer 4)
        if (millis() > channelState_[ch].expiresAt) {
            lastSignalStatus_ = SignalStatus::EXPIRED;
        }
        // Channel holds lastValidPWM until caller checks (INAV behavior)
    }
}

bool SerialRx::isValidPulse(uint16_t pwm) const {
    return (pwm >= validPulseMin_ && pwm <= validPulseMax_);
}
