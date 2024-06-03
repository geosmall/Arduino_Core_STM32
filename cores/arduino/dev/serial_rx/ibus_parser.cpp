#include "ibus_parser.h"

using namespace daisy;

bool IBusParser::Parse(uint8_t byte, SerRxEvent* event_out)
{
    // reset parser when status byte is received
    bool did_parse = false;

    if((byte & kStatusByteMask) && pstate_ != ParserSysEx)
    {
        pstate_ = ParserEmpty;
    }
    switch(pstate_)
    {
        case ParserEmpty:
            // check byte for valid Status Byte
            if(byte == 20) {
                pstate_ = ParserHasData0; // we need to get the 2nd byte yet
            }
            break;
        case ParserHasData0:
            if(byte == 40) {
                did_parse = true;
            } else  {
                // invalid message go back to start
                pstate_ = ParserEmpty;
            }
            // Regardless, of whether the data was valid or not we go back to empty
            // because either the message is queued for handling or its not.
            pstate_ = ParserEmpty;
            break;
        case ParserSysEx:
            // end of sysex
            if(byte == 0xf7) {
                pstate_ = ParserEmpty;
                if(event_out != nullptr) {
                    *event_out = incoming_message_;
                }
                did_parse = true;
            }
            break;
        default:
            break;
    }

    return did_parse;
}

void IBusParser::Reset()
{
    pstate_                = ParserEmpty;
    incoming_message_.type = RxNotInit;
}
