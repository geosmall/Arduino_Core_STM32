/*
 * SerialRx - Arduino library for serial RC protocols
 *
 * RC Message structure definition
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

// Number of channels in the parsed message
constexpr size_t RC_NUM_CHANNELS = 10;

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
