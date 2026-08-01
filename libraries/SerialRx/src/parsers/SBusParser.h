// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 George Small
// See the LICENSE file in this library's root directory.

/*
 * SerialRx - Arduino library for serial RC protocols
 *
 * SBUS protocol parser
 */

#pragma once

#include "../ProtocolParser.h"

/**
 * @brief SBUS protocol parser
 * @details State machine parser for FrSky/Futaba SBUS protocol
 *
 * Protocol specification:
 * - Baudrate: 100000 (inverted signal - requires hardware inverter or GPIO config)
 * - Frame format: 25 bytes total
 *   - Header: 0x0F (1 byte)
 *   - Channel data: 16 channels × 11 bits = 176 bits = 22 bytes (packed)
 *   - Flags: 1 byte (digital channels 17-18, frame lost, failsafe)
 *   - Footer: 0x00 (1 byte)
 *
 * Channel packing (11-bit little-endian):
 *   Byte 0:  [Ch1   LSB 8 bits]
 *   Byte 1:  [Ch2   LSB 5 bits] [Ch1   MSB 3 bits]
 *   Byte 2:  [Ch2   MSB 6 bits] [Ch3   LSB 2 bits]
 *   ...
 *   Byte 21: [Ch16  MSB 3 bits] [reserved]
 *
 * Channel value range: 0-2047 (11-bit resolution)
 * Typical: 172-1811 (1000-2000 µs range)
 * Center: 992 (1500 µs)
 *
 * Flags byte:
 *   Bit 0: Channel 17 (digital)
 *   Bit 1: Channel 18 (digital)
 *   Bit 2: Frame lost
 *   Bit 3: Failsafe active
 */

/**
 * @brief SBUS frame structure (16 channels + flags)
 */
struct SBusFrame {
    uint16_t channels[16];
    uint8_t flags;
};

// SBUS frame constants
constexpr uint8_t SBUS_HEADER = 0x0F;
constexpr uint8_t SBUS_FOOTER = 0x00;
constexpr size_t SBUS_CHANNEL_DATA_LEN = 22;

// SBUS flags bit masks
constexpr uint8_t SBUS_FLAG_CH17        = 0x01;
constexpr uint8_t SBUS_FLAG_CH18        = 0x02;
constexpr uint8_t SBUS_FLAG_FRAME_LOST  = 0x04;
constexpr uint8_t SBUS_FLAG_FAILSAFE    = 0x08;

class SBusParser : public ProtocolParser {
public:
    SBusParser();
    ~SBusParser() override = default;

    /**
     * @brief Parse a single byte of SBUS protocol
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
        WaitingForHeader,    // Waiting for header byte (0x0F)
        AccumulateChannels,  // Accumulating 22 channel data bytes
        ReadFlags,           // Reading flags byte
        ValidateFooter,      // Validating footer byte (0x00)
    };

    /**
     * @brief Unpack 11-bit channels from packed byte array
     * @details Extracts 16 channels from 22 bytes of packed data
     */
    void UnpackChannels();

    ParserState pstate_;         // Current parser state
    uint32_t byte_count_;        // Byte counter within frame
    uint8_t raw_data_[23];       // 22 channel bytes + 1 flag byte
    uint8_t flags_;              // Flags byte (Ch17, Ch18, frame lost, failsafe)
};
