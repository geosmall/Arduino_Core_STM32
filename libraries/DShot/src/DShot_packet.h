// SPDX-License-Identifier: MIT
// Copyright (c) 2026 George Small
// See the LICENSE file in this library's root directory.

#ifndef DSHOT_PACKET_H
#define DSHOT_PACKET_H

#include <stdint.h>

namespace DShot {

// DShot bit timing constants (timer ticks at DShot clock rate)
static constexpr uint32_t BIT_0_DUTY  = 7;   // Bit 0: ~37% duty cycle
static constexpr uint32_t BIT_1_DUTY  = 15;  // Bit 1: 75% duty cycle (DShot spec)
static constexpr uint32_t BIT_PERIOD  = 19;  // Total period per bit
static constexpr int DMA_BUF_SIZE     = 18;  // 16 data bits + 2 zero-padding (frame reset)

// DShot special commands (sent in place of throttle, telemetry bit must be 1)
// Values 0-47 are commands; 48-2047 are throttle.
enum Command : uint8_t {
  CMD_MOTOR_STOP               = 0,
  CMD_BEEP1                    = 1,
  CMD_BEEP2                    = 2,
  CMD_BEEP3                    = 3,
  CMD_BEEP4                    = 4,
  CMD_BEEP5                    = 5,
  CMD_ESC_INFO                 = 6,
  CMD_SPIN_DIRECTION_1         = 7,
  CMD_SPIN_DIRECTION_2         = 8,
  CMD_3D_MODE_OFF              = 9,
  CMD_3D_MODE_ON               = 10,
  CMD_SAVE_SETTINGS            = 12,
  CMD_SPIN_DIRECTION_NORMAL    = 20,
  CMD_SPIN_DIRECTION_REVERSED  = 21,
};

static constexpr uint8_t CMD_MAX = 47;

// Number of times configuration commands must be repeated
static constexpr int CMD_REPEAT_COUNT = 10;

// Timing constants (microseconds)
static constexpr uint32_t CMD_INITIAL_DELAY_US = 10000;   // Before first command
static constexpr uint32_t CMD_REPEAT_DELAY_US  = 1000;    // Between repeats
static constexpr uint32_t CMD_BEEP_DELAY_US    = 100000;  // After beacon commands

// DShot speed variants (timer tick frequency in Hz)
enum Speed : uint32_t {
  DSHOT150  =  3000000,  //  3 MHz
  DSHOT300  =  6000000,  //  6 MHz
  DSHOT600  = 12000000,  // 12 MHz
  DSHOT1200 = 24000000   // 24 MHz
};

// Encode a DShot packet: 11-bit value + telemetry bit + 4-bit CRC
// Returns the full 16-bit packet ready for DMA buffer fill
static inline uint16_t encodePacket(uint16_t value, bool telemetry)
{
  uint16_t packet = (value << 1) | (telemetry ? 1 : 0);

  // CRC: XOR of three nibbles
  uint16_t csum = 0;
  uint16_t csum_data = packet;
  for (int i = 0; i < 3; i++) {
    csum ^= csum_data;
    csum_data >>= 4;
  }
  csum &= 0x0F;

  return (packet << 4) | csum;
}

// Fill a DMA buffer with timer compare values for a 16-bit DShot packet.
// Buffer must be at least DMA_BUF_SIZE (18) elements.
// Elements 0-15: bit duty cycles (MSB first)
// Elements 16-17: zero (frame reset / guard time)
static inline void fillDmaBuffer(uint32_t *buf, uint16_t packet)
{
  for (int i = 0; i < 16; i++) {
    buf[i] = (packet & 0x8000) ? BIT_1_DUTY : BIT_0_DUTY;
    packet <<= 1;
  }
  buf[16] = 0;
  buf[17] = 0;
}

} // namespace DShot

#endif // DSHOT_PACKET_H
