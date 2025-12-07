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
    , dma_enabled_(false) {
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

    // Create parser based on protocol
    switch (protocol_) {
    case IBUS:
        parser_ = new IBusParser();
        break;
    case SBUS:
        parser_ = new SBusParser();
        break;
    case NONE:
    default:
        return false;
    }

    if (parser_ == nullptr) {
        return false;
    }

    // Initialize serial port (DMA or interrupt mode)
    dma_enabled_ = false;
    if (config.use_dma && config.dma_rx_buf != nullptr && config.dma_rx_size > 0) {
        // Try DMA mode - reduces interrupt overhead for continuous streams
        if (serial_->beginDMA(config.baudrate, config.dma_rx_buf, config.dma_rx_size)) {
            dma_enabled_ = true;
        } else {
            // DMA failed (wrong buffer region on H7, unsupported UART, etc.)
            // Fall back to interrupt mode
            serial_->begin(config.baudrate);
        }
    } else {
        // Standard interrupt mode
        serial_->begin(config.baudrate);
    }

    return true;
}

void SerialRx::end() {
    if (serial_ != nullptr) {
        if (dma_enabled_) {
            serial_->endDMA();
            dma_enabled_ = false;
        }
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

            // After idle, first byte MUST be frame start (0x20 for IBus)
            if (byte != 0x20) {
                // Not a valid frame start after idle → discard byte
                continue;
            }
        }

        if (parser_->ParseByte(byte)) {
            // Complete valid message parsed (checksum validated)
            last_message_time_ = millis();
        }
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

bool SerialRx::sendTelemetry(uint8_t* data, size_t len) {
    // Future implementation for bi-directional telemetry
    if (serial_ == nullptr || data == nullptr || len == 0) {
        return false;
    }

    // TODO: Implement protocol-specific telemetry framing
    serial_->write(data, len);
    return true;
}
