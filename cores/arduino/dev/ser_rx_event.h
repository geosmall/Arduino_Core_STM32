// TODO: make this adjustable
#define SERRX_BUFFER_LEN 128
#define SERRX_PACKET_LEN 0x20
#define SERRX_CMD_CODE 0x40
#define SERRX_NUM_CHAN 14

namespace daisy
{

enum SerRxMessageType
{
    RxNotInit,
    RxValidPacket,
    RxFailSafePacket,
};

/** Struct containing Rx not initialized message
*/
struct RxNotInitEvent
{
    const uint8_t      length = SERRX_PACKET_LEN;
    const uint8_t      cmd_code = SERRX_CMD_CODE;
    uint16_t           data[SERRX_NUM_CHAN*2];
    uint16_t           crc;
};


/** Struct containing Rx valid packet message
*/
struct RxValidPacketEvent
{
    const uint8_t      length = SERRX_PACKET_LEN;
    const uint8_t      cmd_code = SERRX_CMD_CODE;
    uint16_t           data[SERRX_NUM_CHAN*2];
    uint16_t           crc;
};

/** Struct containing Rx failesafe packet message
*/
struct RxFailSafePacketEvent
{
    const uint8_t      length = SERRX_PACKET_LEN;
    const uint8_t      cmd_code = SERRX_CMD_CODE;
    uint16_t           data[SERRX_NUM_CHAN*2];
    uint16_t           crc;
};

/** Simple SerRxEvent with message type, channel, and data[2] members.
*/
struct SerRxEvent
{
    SerRxMessageType   type;                         /**< & */
    uint8_t            sysex_data[SERRX_BUFFER_LEN]; /**< & */
    uint8_t            sysex_message_len;

    /** Returns the data within the SerRxEvent as a RxNotInitEvent struct */
    RxNotInitEvent AsRxNotInit()
    {
        RxNotInitEvent m;
        return m;
    }

    /** Returns the data within the SerRxEvent as a RxValidPacketEvent struct */
    RxValidPacketEvent AsRxValidPacket()
    {
        RxValidPacketEvent m;
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
