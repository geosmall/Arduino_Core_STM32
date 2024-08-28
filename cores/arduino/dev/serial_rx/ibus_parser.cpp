#include "ibus_parser.h"

using namespace daisy;

bool IBusParser::Parse(uint8_t byte, SerRxEvent* event_out)
{
    // reset parser when status byte is received
    bool did_parse = false;
    uint32_t char_count;
    uint16_t running_checksum;
    uint16_t frame_checksum;
    uint8_t i;

    switch (pstate_)
    {
    case ParserEmpty:
        // check byte for valid first header byte
        if (byte == 0x20)
        {
            char_count = 1;
            running_checksum = 0xFFFF - byte;
            pstate_ = ParserHasHeader0; // we need to get the 2nd byte yet
        }
        break;
    case ParserHasHeader0:
        if (byte == 0x40)
        {
            char_count = 2;
            running_checksum -= byte;
            pstate_ = ParserHasHeader1; // we need to get the 2nd byte yet
        }
        else
        {
            // invalid message go back to start
            char_count = 0;
            pstate_ = ParserEmpty;
        }
        break;
    case ParserHasHeader1:
        char_count++;
        // Store the byte in the channel array
        // odd bytes are high byte, even bytes are low
        // channels array index is char_count / 2 - 1
        i = (char_count - 3u) / 2u;
        if (i < SERRX_NUM_CHAN)
        {
            if (char_count % 2u)
            {
                event_message_.channels[i] = byte; // odd = low byte
            }
            else
            {
                event_message_.channels[i] |= byte << 8u; // even = high bytr
            }
        }
        running_checksum -= byte;
        if (char_count >= 30u)
        {
            pstate_ = ParserHasFrame;
        }
        break;
    case ParserHasFrame:
        frame_checksum = byte;
        pstate_ = ParserHasCheckSum0;
        break;
    case ParserHasCheckSum0:
        frame_checksum = (byte << 8) | frame_checksum;
        if (frame_checksum == running_checksum)
        {
            event_message_.type = RxValidPacket;

            if (event_out != nullptr)
            {
                *event_out = event_message_;
            }
            did_parse = true;
        }
        // Go back to start
        char_count = 0;
        pstate_ = ParserEmpty;
        break;
    default:
        break;
    }

    return did_parse;
}

void IBusParser::Reset()
{
    pstate_ = ParserEmpty;
    event_message_.type = RxMessageLast;
}
