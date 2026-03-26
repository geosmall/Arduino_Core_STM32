/*
 * SerialRx - Arduino library for serial RC protocols
 *
 * CRSF (Crossfire/ELRS) protocol parser
 */

#pragma once

#include "../ProtocolParser.h"

/**
 * @brief CRSF protocol parser
 * @details State machine parser for TBS Crossfire / ExpressLRS protocol
 *
 * Protocol specification:
 * - Baudrate: 420000 (not inverted)
 * - Frame format: variable length, max 64 bytes total
 *   - Sync byte: 0xC8 (device address / flight controller)
 *   - Frame length: 1 byte (includes type, payload, and CRC)
 *   - Type: 1 byte
 *   - Payload: variable length
 *   - CRC: 1 byte (CRC8-DVB-S2 over type + payload)
 *
 * RC Channels frame (type 0x16):
 *   - Payload: 22 bytes (16 channels x 11 bits = 176 bits, packed)
 *   - Channel packing identical to SBUS (11-bit little-endian)
 *   - Channel value range: 0-2047 (11-bit resolution)
 *   - Typical: 172-1811 (1000-2000 us range)
 *
 * Link Statistics frame (type 0x14):
 *   - Payload: 10 bytes (RSSI, LQ, SNR, RF mode, TX power)
 *   - Uplink LQ at payload offset 2: 0-100%
 *   - LQ=0 triggers Layer 1 failsafe (error_flags |= 0x02)
 *
 * Big-endian for multi-byte header fields, but channel bit packing
 * is identical to SBUS on little-endian ARM platforms.
 */

// CRSF frame constants
constexpr uint8_t CRSF_SYNC_BYTE = 0xC8;
constexpr uint8_t CRSF_FRAME_SIZE_MAX = 64;
constexpr uint8_t CRSF_PAYLOAD_SIZE_MAX = CRSF_FRAME_SIZE_MAX - 6;
constexpr uint8_t CRSF_FRAME_LENGTH_MAX = 62;  // Max value of length field

// CRSF frame types
constexpr uint8_t CRSF_FRAMETYPE_LINK_STATISTICS = 0x14;
constexpr uint8_t CRSF_FRAMETYPE_RC_CHANNELS_PACKED = 0x16;

// CRSF payload sizes
constexpr uint8_t CRSF_RC_CHANNELS_PAYLOAD_SIZE = 22;
constexpr uint8_t CRSF_LINK_STATISTICS_PAYLOAD_SIZE = 10;

// Link statistics payload offsets
constexpr uint8_t CRSF_LQ_OFFSET = 2;  // uplinkLQ is 3rd byte in link stats payload

class CRSFParser : public ProtocolParser {
public:
    CRSFParser();
    ~CRSFParser() override = default;

    /**
     * @brief Parse a single byte of CRSF protocol
     * @param byte The byte to parse
     * @return true if a complete valid RC channels message was parsed
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
        WaitingForSync,      // Waiting for sync byte (0xC8)
        ReadLength,          // Reading frame length byte
        AccumulatePayload,   // Accumulating type + payload + CRC bytes
    };

    /**
     * @brief Compute CRC8-DVB-S2 for a single byte
     * @param crc Running CRC value
     * @param a Byte to process
     * @return Updated CRC value
     */
    static uint8_t crc8_dvb_s2(uint8_t crc, uint8_t a);

    /**
     * @brief Validate frame CRC
     * @return true if CRC matches
     */
    bool validateCRC() const;

    /**
     * @brief Process a complete, CRC-validated frame by type
     * @return true if RC channels were unpacked
     */
    bool processFrame();

    /**
     * @brief Unpack 11-bit channels from packed byte array
     * @details Extracts first 14 of 16 channels from 22 bytes of packed data
     *          Bit packing is identical to SBUS (11-bit little-endian)
     */
    void unpackChannels();

    ParserState pstate_;                        // Current parser state
    uint8_t frame_length_;                      // Frame length from header
    uint8_t byte_count_;                        // Bytes received in current section
    uint8_t frame_data_[CRSF_PAYLOAD_SIZE_MAX]; // Type + payload buffer (no sync/length/CRC)
    uint8_t uplink_lq_;                         // Uplink link quality (0-100, 0xFF = no data)
};
