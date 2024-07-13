// TODO: make this adjustable
#define SERRX_BUFFER_LEN 128
#define SERRX_PACKET_LEN 20
#define SERRX_CMD_CODE 40
#define SERRX_NUM_CHAN 14

namespace daisy
{

/** @brief   SerRx handlers, and message types
 *  @details max 14 channels in this lib (with messagelength of 0x20 there is room for 14 channels)

  Example set of bytes coming over the SerRx line for setting servos: 
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
