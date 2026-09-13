// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 George Small
// See the LICENSE file in this library's root directory.

/*
 * SerialRx - Arduino library for serial RC protocols
 *
 * RC Message structure definition
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

// Number of channels in the parsed message.
//
// 14 is the lowest common denominator across every supported protocol, chosen
// so all receivers are equally supported: a consumer may read channels[0..13]
// without knowing which protocol is configured, and every slot is populated
// from wire data for all of them.
//
//   IBus  14 proportional  -> all 14 delivered (protocol maximum)
//   SBUS  16 proportional  -> first 14 delivered, CH15/CH16 truncated
//   CRSF  16 proportional  -> first 14 delivered, CH15/CH16 truncated
//
// Raising this to 16 would leave the last two slots unsourced for IBus, making
// the array's meaning protocol-dependent. Truncating instead keeps it uniform.
constexpr size_t RC_NUM_CHANNELS = 14;

/**
 * @brief Parsed RC message structure
 * @details Contains channel data and error flags from RC receiver
 */
struct RCMessage {
    uint16_t channels[RC_NUM_CHANNELS];  // Channel data (1000-2000 us typical range)
    uint32_t error_flags;                 // Error flags (bitmask)

    // Constructor to initialize fields
    RCMessage() : channels{0}, error_flags(0) {}
};
