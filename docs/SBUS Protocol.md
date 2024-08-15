# From: uzh-rpg

https://github.com/uzh-rpg/rpg_quadrotor_control/wiki/SBUS-Protocol

The SBUS Protocol is a serial protocol that was developed by Futaba for hobby remote control applications.
It is derived from the RS232 protocol but the voltage levels are inverted.
The protocol provides 16 channels of 11 bits each, two digital channels, and two flags for "frame lost" and "failsafe".

#### For SBUS, a serial port has to be configured as follows:
* 100'000 Baud rate (this is a non standard baud rate!)
* 8E2 configuration, i.e.:
  - 1 start bit
  - 8 Data bits
  - 1 Even parity bit
  - 2 Stop bits

**Note:** The voltage levels of SBUS are inverted. So while a `0` with a normal serial port is encoded with a `low` voltage, it is encoded with a `high` voltage with SBUS.

**Note:** Even parity means that, for a given set of bits, the occurrences of bits whose value is 1 is counted. If that count is odd, the parity bit value is set to 1, making the total count of occurrences of 1s in the whole set (including the parity bit) an even number. If the count of 1s in a given set of bits is already even, the parity bit's value is 0.

#### SBUS Protocol
A single SBUS message is 25 bytes long an therefore, with the configuration described above, takes 3ms to be transmitted.
It consists of the following bytes:
* 1 Header byte 00001111b (0x0F)
* 16 * 11 bit channels -> 22 bytes
* 1 Byte with two digital channels (channel 17 and 18) and "frame lost" and "failsafe" flags
* 1 Footer byte 00000000b (0x00)

Each byte is composed of 8 bits with IDs as follows [7 6 5 4 3 2 1 0] where bit 0 is the least significant bit.
The data of the 16 channels are distributed onto the 22 data bytes starting with the least significant bit of channel 1 as follows (using the notation CHANNEL.BIT_ID):
* data byte 0: [1.7  1.6  1.5   1.4  1.3  1.2   1.1  1.0]
* data byte 1: [2.4  2.3  2.2   2.1  2.0  1.10  1.9  1.8]
* data byte 2: [3.1  3.0  2.10  2.9  2.8  2.7   2.6  2.5]
* data byte 3: ...

The digital channels and flag bytes is composed as:
* flag byte: [0  0  0  0  failsafe  frame_lost  ch18 ch17]

Since the least significant bit is sent first over the serial port, the following bit sequence is transmitted:
```
shhhhhhhhpss | s 1.0 1.1 1.2 1.3 1.4 1.5 1.6 1.7 pss | s 1.8 1.9 1.10 2.1 2.2 2.3 2.4 pss | ...
```

#### Channel Values
* Each of the 16 channels use values in the range of 192 - 1792 which are mapped linearly in the [[Betaflight Firmware|Betaflight-Firmware]] to values in the range 1000 - 2000.
These values in the range [1000, 2000] are also what can be observed in the Betaflight Configurator's Receiver tab.
Note that e.g. a [[Taranis|RC-Setup]] transmitter sends values in a slightly larger range than [192, 1792] but these values will later be cropped.


# From: Bolderflight

https://github.com/bolderflight/sbus/blob/main/README.md

## Description
SBUS is a bus protocol for receivers to send commands to servos. Unlike PWM, SBUS uses a bus architecture where a single serial line can be connected with up to 16 servos with each receiving a unique command.

The SBUS protocol uses an inverted serial logic with a baud rate of 100000, 8 data bits, even parity, and 2 stop bits. The SBUS packet is 25 bytes long consisting of:
   * Byte[0]: SBUS header, 0x0F
   * Byte[1 -22]: 16 servo channels, 11 bits each
   * Byte[23]
      * Bit 0: channel 17 (0x01)
      * Bit 1: channel 18 (0x02)
      * Bit 2: frame lost (0x04)
      * Bit 3: failsafe activated (0x08)
   * Byte[24]: SBUS footer

Note that lost frame is indicated when a frame is lost between the transmitter and receiver. Failsafe activation typically requires that many frames are lost in a row and indicates that the receiver has moved into failsafe mode. Packets are sent approximately every 10 ms or 20 ms, depending on the system configuration.

A variation on SBUS called "Fast SBUS" has started to be used. This uses a baudrate of 200000 and a quicker update rate.

**Note on CH17 and CH18:** Channel 17 and channel 18 are digital on/off channels. These are not universally available on all SBUS receivers and servos.

FrSky receivers will output a range of 172 - 1811 with channels set to a range of -100% to +100%. Using extended limits of -150% to +150% outputs a range of 0 to 2047, which is the maximum range acheivable with 11 bits of data.

Because SBUS is a digital bus format, it is an excellent means of receiving pilot commands from a transmitter and an SBUS capable receiver. If SBUS servos are used in the aircraft, SBUS is also an excellent means of sending actuator commands - servo commands can often be sent with lower latency and, by only using a single pin to command up to 16 servos, additional microcontroller pins are freed for other uses.