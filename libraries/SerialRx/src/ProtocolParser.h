// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 George Small
// See the LICENSE file in this library's root directory.

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

    /**
     * @brief Get count of successfully parsed frames
     * @return Number of valid frames received
     */
    inline uint32_t GetFramesReceived() const {
        return frames_received_;
    }

    /**
     * @brief Get count of frames that failed validation
     * @return Number of failed frames (checksum errors, etc.)
     */
    inline uint32_t GetFramesFailed() const {
        return frames_failed_;
    }

    /**
     * @brief Get frame loss percentage
     * @return Loss rate as percentage (0.0 - 100.0), or 0 if no frames processed
     */
    inline float GetFrameLossPercent() const {
        // Use uint64_t to avoid overflow when both counters are large
        uint64_t total = (uint64_t)frames_received_ + frames_failed_;
        if (total == 0) return 0.0f;
        return (frames_failed_ * 100.0f) / total;
    }

    /**
     * @brief Reset frame statistics counters
     */
    inline void ResetStatistics() {
        frames_received_ = 0;
        frames_failed_ = 0;
    }

    /**
     * @brief Access latest completed frame without consuming from FIFO
     * @return Reference to shadow copy of last successfully parsed frame
     */
    const RCMessage& GetLatestFrame() const { return latestFrame_; }

protected:
    // Working message buffer for parser
    RCMessage msg_;

    // Shadow copy of last completed frame (survives ResetParser)
    RCMessage latestFrame_;

    // Message queue (16-message depth)
    RingBuffer<RCMessage, 16> msg_q_;

    // Frame statistics
    uint32_t frames_received_ = 0;
    uint32_t frames_failed_ = 0;

    /**
     * @brief Notify that a message was successfully parsed
     * @details Call this from derived parser when complete message received.
     *          Saves a shadow copy before FIFO push for failsafe processing.
     */
    inline void ParserNotify() {
        latestFrame_ = msg_;  // Shadow copy before FIFO push
        msg_q_.PutWithOverwrite(msg_);
        if (frames_received_ < UINT32_MAX) frames_received_++;
    }

    /**
     * @brief Notify that a frame failed validation
     * @details Call this from derived parser when checksum or validation fails
     */
    inline void ParserNotifyFailed() {
        if (frames_failed_ < UINT32_MAX) frames_failed_++;
    }
};
