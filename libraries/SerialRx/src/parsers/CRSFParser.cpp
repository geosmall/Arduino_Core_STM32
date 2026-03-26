/*
 * SerialRx - Arduino library for serial RC protocols
 *
 * CRSF (Crossfire/ELRS) protocol parser implementation
 */

#include "CRSFParser.h"
#include <string.h>

CRSFParser::CRSFParser()
    : uplink_lq_(0xFF) {
    ResetParser();
}

bool CRSFParser::ParseByte(uint8_t byte) {
    bool did_parse = false;

    switch (pstate_) {
    case WaitingForSync:
        if (byte == CRSF_SYNC_BYTE) {
            pstate_ = ReadLength;
        }
        break;

    case ReadLength:
        frame_length_ = byte;
        if (frame_length_ < 2 || frame_length_ > CRSF_FRAME_LENGTH_MAX) {
            // Invalid length: minimum is 2 (type + CRC), maximum is 62
            ResetParser();
        } else {
            byte_count_ = 0;
            pstate_ = AccumulatePayload;
        }
        break;

    case AccumulatePayload:
        // Accumulate type + payload + CRC bytes (total = frame_length_)
        if (byte_count_ < frame_length_) {
            frame_data_[byte_count_] = byte;
            byte_count_++;

            if (byte_count_ >= frame_length_) {
                // Complete frame received
                if (validateCRC()) {
                    did_parse = processFrame();
                } else {
                    ParserNotifyFailed();
                }
                ResetParser();
            }
        }
        break;

    default:
        break;
    }

    return did_parse;
}

void CRSFParser::ResetParser() {
    pstate_ = WaitingForSync;
    frame_length_ = 0;
    byte_count_ = 0;
    // uplink_lq_ intentionally NOT reset — persists across frames
}

uint8_t CRSFParser::crc8_dvb_s2(uint8_t crc, uint8_t a) {
    crc ^= a;
    for (uint8_t i = 0; i < 8; i++) {
        crc = (crc & 0x80) ? (crc << 1) ^ 0xD5 : (crc << 1);
    }
    return crc;
}

bool CRSFParser::validateCRC() const {
    // CRC covers type + payload (all bytes except the last one which is the CRC)
    uint8_t crc = 0;
    for (uint8_t i = 0; i < frame_length_ - 1; i++) {
        crc = crc8_dvb_s2(crc, frame_data_[i]);
    }
    return crc == frame_data_[frame_length_ - 1];
}

bool CRSFParser::processFrame() {
    uint8_t type = frame_data_[0];
    // Payload starts at frame_data_[1], length = frame_length_ - 2 (minus type and CRC)
    uint8_t payload_length = frame_length_ - 2;

    if (type == CRSF_FRAMETYPE_RC_CHANNELS_PACKED &&
        payload_length == CRSF_RC_CHANNELS_PAYLOAD_SIZE) {
        // Unpack 16 channels from packed 11-bit data
        unpackChannels();

        // Layer 1 failsafe: check uplink LQ from most recent link statistics
        msg_.error_flags = 0;
        if (uplink_lq_ == 0) {
            msg_.error_flags |= 0x02;  // Failsafe flag (same as SBUS)
        }

        ParserNotify();
        return true;
    }

    if (type == CRSF_FRAMETYPE_LINK_STATISTICS &&
        payload_length == CRSF_LINK_STATISTICS_PAYLOAD_SIZE) {
        // Extract uplink LQ (3rd byte of link statistics payload)
        uplink_lq_ = frame_data_[1 + CRSF_LQ_OFFSET];
        // Valid frame but not RC channels — count it but don't push to queue
        if (frames_received_ < UINT32_MAX) frames_received_++;
        return false;
    }

    // Other frame types: valid CRC but not relevant — count and discard
    if (frames_received_ < UINT32_MAX) frames_received_++;
    return false;
}

void CRSFParser::unpackChannels() {
    // Unpack first 14 channels from 22 bytes of packed 11-bit data
    // CRSF protocol has 16 channels but RCMessage stores 14 (RC_NUM_CHANNELS)
    // Channels 15-16 intentionally not unpacked — same approach as SBusParser
    // Bit packing is identical to SBUS (11-bit little-endian)
    // Payload starts at frame_data_[1] (after type byte)
    const uint8_t* raw = &frame_data_[1];

    msg_.channels[0]  = (raw[0]     | raw[1]<<8)                    & 0x07FF;
    msg_.channels[1]  = (raw[1]>>3  | raw[2]<<5)                    & 0x07FF;
    msg_.channels[2]  = (raw[2]>>6  | raw[3]<<2  | raw[4]<<10)      & 0x07FF;
    msg_.channels[3]  = (raw[4]>>1  | raw[5]<<7)                    & 0x07FF;
    msg_.channels[4]  = (raw[5]>>4  | raw[6]<<4)                    & 0x07FF;
    msg_.channels[5]  = (raw[6]>>7  | raw[7]<<1  | raw[8]<<9)       & 0x07FF;
    msg_.channels[6]  = (raw[8]>>2  | raw[9]<<6)                    & 0x07FF;
    msg_.channels[7]  = (raw[9]>>5  | raw[10]<<3)                   & 0x07FF;
    msg_.channels[8]  = (raw[11]    | raw[12]<<8)                   & 0x07FF;
    msg_.channels[9]  = (raw[12]>>3 | raw[13]<<5)                   & 0x07FF;
    msg_.channels[10] = (raw[13]>>6 | raw[14]<<2 | raw[15]<<10)     & 0x07FF;
    msg_.channels[11] = (raw[15]>>1 | raw[16]<<7)                   & 0x07FF;
    msg_.channels[12] = (raw[16]>>4 | raw[17]<<4)                   & 0x07FF;
    msg_.channels[13] = (raw[17]>>7 | raw[18]<<1 | raw[19]<<9)      & 0x07FF;
}
