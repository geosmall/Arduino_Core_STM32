#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "../ser_rx_event.h"

namespace daisy
{
/** @brief   Utility class for parsing raw byte streams into IBUS messages
 *  @details Implemented as a state machine designed to parse one byte at a time
 *  supports max 14 channels in this lib (with messagelength of 0x20 there is room for 14 channels)

  Example set of bytes coming over the iBUS line for setting servos: 
    20 40 DB 5 DC 5 54 5 DC 5 E8 3 D0 7 D2 5 E8 3 DC 5 DC 5 DC 5 DC 5 DC 5 DC 5 DA F3
  Explanation
    Protocol length: 20
    Command code: 40 
    Channel 0: DB 5  -> value 0x5DB
    Channel 1: DC 5  -> value 0x5Dc
    Channel 2: 54 5  -> value 0x554
    Channel 3: DC 5  -> value 0x5DC
    Channel 4: E8 3  -> value 0x3E8
    Channel 5: D0 7  -> value 0x7D0
    Channel 6: D2 5  -> value 0x5D2
    Channel 7: E8 3  -> value 0x3E8
    Channel 8: DC 5  -> value 0x5DC
    Channel 9: DC 5  -> value 0x5DC
    Channel 10: DC 5 -> value 0x5DC
    Channel 11: DC 5 -> value 0x5DC
    Channel 12: DC 5 -> value 0x5DC
    Channel 13: DC 5 -> value 0x5DC
    Checksum: DA F3 -> calculated by adding up all previous bytes, total must be FFFF
 */

struct IBusFrame
{
    const uint8_t      length = 20;
    const uint8_t      cmd_code = 40;
    uint16_t           data[14*2];
    uint16_t           crc;
};

class IBusParser
{
  public:
    IBusParser(){};
    ~IBusParser() {}

    inline void Init() { Reset(); }

    /**
     * @brief Parse one IBUS byte. If the byte completes a parsed frame,
     *        its value will be assigned to the dereferenced output pointer.
     *        Otherwise, status is preserved in anticipation of the next sequential
     *        byte. Return value indicates if a new event was parsed or not.
     *
     * @param byte      Raw IBUS byte to parse
     * @param event_out Pointer to output event object, value assigned on parse success
     * @return true     If a new frame was parsed
     * @return false    If no new frame was parsed
     */
    bool Parse(uint8_t byte, SerRxEvent* frame_out);

    /**
     * @brief Reset parser to default state
     */
    void Reset();

  private:
    enum ParserState
    {
        ParserEmpty,          // Start, looking for header byte
        ParserHasHeader0,     // Found first header byte
        ParserHasHeader1,     // Found second header byte
        ParserHasFrame,       // Have received a full frame
        ParserHasCheckSum0,   // Has first checksum byte
        ParserHasValidFrame,  // Passed frame validity check
    };

    ParserState     pstate_ = ParserEmpty;
    SerRxEvent      incoming_message_;
    // MidiMessageType running_status_;

    // Masks to check for message type, and byte content
    const uint8_t kStatusByteMask     = 0x80;
    const uint8_t kMessageMask        = 0x70;
    const uint8_t kDataByteMask       = 0x7F;
    const uint8_t kChannelMask        = 0x0F;
    const uint8_t kSystemRealTimeMask = 0x07;
};

} // namespace daisy
