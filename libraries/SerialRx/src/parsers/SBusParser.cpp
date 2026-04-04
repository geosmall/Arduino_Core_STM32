/*
 * SerialRx - Arduino library for serial RC protocols
 *
 * SBUS protocol parser implementation
 */

#include "SBusParser.h"
#include <string.h>

SBusParser::SBusParser() {
    ResetParser();
}

bool SBusParser::ParseByte(uint8_t byte) {
    bool did_parse = false;

    switch (pstate_) {
    case WaitingForHeader:
        // Check for valid header byte
        if (byte == SBUS_HEADER) {
            byte_count_ = 0;
            msg_.error_flags = 0;
            pstate_ = AccumulateChannels;
        }
        break;

    case AccumulateChannels:
        // Store channel data bytes (22 bytes total)
        if (byte_count_ < SBUS_CHANNEL_DATA_LEN) {
            raw_data_[byte_count_] = byte;
            byte_count_++;

            if (byte_count_ >= SBUS_CHANNEL_DATA_LEN) {
                pstate_ = ReadFlags;
            }
        }
        break;

    case ReadFlags:
        // Store flags byte
        flags_ = byte;
        raw_data_[22] = byte;  // Store for reference
        pstate_ = ValidateFooter;
        break;

    case ValidateFooter:
        // Validate footer byte
        if (byte == SBUS_FOOTER) {
            // Valid frame - unpack channels
            UnpackChannels();

            // Set error flags based on SBUS flags
            msg_.error_flags = 0;
            if (flags_ & SBUS_FLAG_FRAME_LOST) {
                msg_.error_flags |= 0x01;  // Frame lost flag
            }
            if (flags_ & SBUS_FLAG_FAILSAFE) {
                msg_.error_flags |= 0x02;  // Failsafe flag
            }

            ParserNotify();  // Good frame received
            did_parse = true;
        } else {
            ParserNotifyFailed();  // Invalid footer byte
        }
        // Go back to start (valid or invalid footer)
        ResetParser();
        break;

    default:
        break;
    }

    return did_parse;
}

void SBusParser::ResetParser() {
    // Reset the parser state
    pstate_ = WaitingForHeader;
    byte_count_ = 0;
    flags_ = 0;

    memset(raw_data_, 0, sizeof(raw_data_));
    memset(msg_.channels, 0, sizeof(msg_.channels));
    msg_.error_flags = 0;
}

void SBusParser::UnpackChannels() {
    // Unpack first 14 analog channels from 22 bytes of packed 11-bit data
    // SBUS protocol has 16 channels but channels 15-16 are digital (CH17/CH18)
    // and already captured in the flags byte — no need to store in channels[]
    // SBUS uses little-endian bit packing

    msg_.channels[0]  = (raw_data_[0]    | raw_data_[1]<<8)                     & 0x07FF;
    msg_.channels[1]  = (raw_data_[1]>>3 | raw_data_[2]<<5)                     & 0x07FF;
    msg_.channels[2]  = (raw_data_[2]>>6 | raw_data_[3]<<2 | raw_data_[4]<<10)  & 0x07FF;
    msg_.channels[3]  = (raw_data_[4]>>1 | raw_data_[5]<<7)                     & 0x07FF;
    msg_.channels[4]  = (raw_data_[5]>>4 | raw_data_[6]<<4)                     & 0x07FF;
    msg_.channels[5]  = (raw_data_[6]>>7 | raw_data_[7]<<1 | raw_data_[8]<<9)   & 0x07FF;
    msg_.channels[6]  = (raw_data_[8]>>2 | raw_data_[9]<<6)                     & 0x07FF;
    msg_.channels[7]  = (raw_data_[9]>>5 | raw_data_[10]<<3)                    & 0x07FF;
    msg_.channels[8]  = (raw_data_[11]   | raw_data_[12]<<8)                    & 0x07FF;
    msg_.channels[9]  = (raw_data_[12]>>3| raw_data_[13]<<5)                    & 0x07FF;
    msg_.channels[10] = (raw_data_[13]>>6| raw_data_[14]<<2 | raw_data_[15]<<10)& 0x07FF;
    msg_.channels[11] = (raw_data_[15]>>1| raw_data_[16]<<7)                    & 0x07FF;
    msg_.channels[12] = (raw_data_[16]>>4| raw_data_[17]<<4)                    & 0x07FF;
    msg_.channels[13] = (raw_data_[17]>>7| raw_data_[18]<<1 | raw_data_[19]<<9) & 0x07FF;
    // Channels 15-16 (SBUS CH15/CH16) intentionally not unpacked —
    // RCMessage has 14 slots; digital CH17/CH18 are in flags byte
}
