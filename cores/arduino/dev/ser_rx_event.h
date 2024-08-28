#pragma once

#include <cstring>

#define SERRX_PACKET_LEN 0x20
#define SERRX_CMD_CODE 0x40
#define SERRX_NUM_CHAN 10

namespace daisy
{

enum SerRxMessageType : uint8_t
{
    RxValidPacket,
    RxFailSafePacket,
    RxMessageLast,
};

/** Struct containing Rx valid packet message
*/
struct RxValidPacketEvent
{
    uint16_t           channels[SERRX_NUM_CHAN];
};

/** Struct containing Rx failesafe packet message
*/
struct RxFailSafePacketEvent
{
    uint16_t           channels[SERRX_NUM_CHAN];
};

// Calculate the message length at compile time
constexpr uint8_t MESSAGE_LEN()
{
    return sizeof(SerRxMessageType) + (SERRX_NUM_CHAN * sizeof(uint16_t)) + sizeof(uint8_t);
}

/** Simple SerRxEvent with message type, channel, and data[2] members.
*/
struct SerRxEvent
{
    SerRxMessageType   type;
    uint16_t           channels[SERRX_NUM_CHAN];
    uint32_t           message_len;

    /** Returns the data within the SerRxEvent as a RxValidPacketEvent struct */
    RxValidPacketEvent AsRxValidPacket()
    {
        RxValidPacketEvent m;
        memcpy(m.channels, channels, sizeof(m.channels));
        message_len = MESSAGE_LEN();
        return m;
    }

    /** Returns the data within the SerRxEvent as a RxFailSafePacket struct */
    RxFailSafePacketEvent AsRxFailSafePacket()
    {
        RxFailSafePacketEvent m;
        return m;
    }

};

} //namespace daisy
