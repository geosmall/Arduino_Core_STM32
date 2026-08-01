// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 George Small
// See the LICENSE file in this library's root directory.

/*
 * SerialRx - Arduino library for serial RC protocols
 *
 * IBus protocol parser
 */

#pragma once

#include "../ProtocolParser.h"

/**
 * @brief IBus protocol parser
 * @details State machine parser for FlySky IBus protocol
 *
 * Protocol specification:
 * - Baudrate: 115200
 * - Frame format: 32 bytes total
 *   - Header: 0x20 0x40 (2 bytes)
 *   - Channel data: 14 channels × 2 bytes = 28 bytes (little-endian)
 *   - Checksum: 2 bytes (0xFFFF - sum of all previous bytes)
 *
 * Example frame (14 channels):
 *   20 40 DB 5 DC 5 54 5 DC 5 E8 3 D0 7 D2 5 E8 3 DC 5 DC 5 DC 5 DC 5 DC 5 DC 5 DA F3
 *
 * Channel value range: 1000-2000 us typical (0x3E8-0x7D0)
 */

/**
 * @brief IBus frame structure (14 channels max)
 */
struct IBusFrame {
    uint16_t channels[14];
    uint16_t checksum;
};

// Length of IBus data frame minus checksum
constexpr size_t IBUS_FRAME_LEN_MINUS_CHECKSUM = 30;

class IBusParser : public ProtocolParser {
public:
    IBusParser();
    ~IBusParser() override = default;

    /**
     * @brief Parse a single byte of IBus protocol
     * @param byte The byte to parse
     * @return true if a complete valid message was parsed
     */
    bool ParseByte(uint8_t byte) override;

    /**
     * @brief Reset parser to initial state
     */
    void ResetParser() override;

private:
    /**
     * @brief Parser state machine states
     */
    enum ParserState {
        WaitingForHeader0,   // Waiting for first header byte (0x20)
        ParserHasHeader0,    // Found first header byte
        ParserHasHeader1,    // Found second header byte (0x40)
        ParserHasFrame,      // Received full frame, waiting for checksum
        ParserHasCheckSum0,  // Has first checksum byte
    };

    ParserState pstate_;         // Current parser state
    uint32_t byte_count_;        // Byte counter within frame
    uint16_t running_checksum_;  // Running checksum calculation
    uint16_t frame_checksum_;    // Received checksum from frame
};
