# PrecompLib - Precompiled Library Example

A demonstration of Arduino's `precompiled=true` feature for binary-only distribution on STM32 targets.

## Purpose & Context

PrecompLib is an **educational reference implementation** demonstrating Arduino's precompiled library feature.

**Role in Project:**
- Technical demonstration of `precompiled=true` workflow
- Validates multi-architecture binary distribution (cortex-m4, cortex-m7)

## Overview

PrecompLib provides CRC-16 CCITT checksum calculation as a precompiled static library.

**Key Features:**
- Binary-only distribution
- Supports F4, G4, F7, and H7 STM32 families
- Two binaries cover all targets (cortex-m4, cortex-m7)
- No HAL/CMSIS dependencies (pure computation)

## Supported Targets

| Family | MCU | Binary | Example Boards |
|--------|-----|--------|----------------|
| **F4** | cortex-m4 | `cortex-m4/fpv4-sp-d16-hard` | NUCLEO_F411RE, BLACKPILL_F411CE, JHEF_JHEF411 |
| **G4** | cortex-m4 | `cortex-m4/fpv4-sp-d16-hard` | NUCLEO_G431RB, NUCLEO_G474RE |
| **F7** | cortex-m7 | `cortex-m7/fpv4-sp-d16-hard` | BKMN_NERO, NUCLEO_F746ZG |
| **H7** | cortex-m7 | `cortex-m7/fpv4-sp-d16-hard` | MATEK_H743VI, NUCLEO_H743ZI |

## Installation

This library is included with the STM32 Robotics Arduino Core. No additional installation required.

## API Reference

### Functions

```cpp
#include <PrecompLib.h>

// Calculate CRC-16 over a buffer
uint16_t crc16_calculate(const uint8_t* data, size_t length);

// Update running CRC with a single byte (for streaming)
uint16_t crc16_update(uint16_t crc, uint8_t byte);

// Get library version
const char* precomplib_version(void);
```

### CRC-16 CCITT Parameters

| Parameter | Value |
|-----------|-------|
| Polynomial | 0x1021 (x^16 + x^12 + x^5 + 1) |
| Initial Value | 0xFFFF |
| Final XOR | None |
| Bit Order | MSB-first |

### Standard Test Vector

```cpp
// "123456789" -> 0x29B1
uint8_t data[] = {'1','2','3','4','5','6','7','8','9'};
uint16_t crc = crc16_calculate(data, 9);  // Returns 0x29B1
```

## Usage Examples

### Buffer CRC Calculation

```cpp
#include <PrecompLib.h>

void setup() {
    Serial.begin(115200);

    uint8_t packet[] = {0x01, 0x02, 0x03, 0x04};
    uint16_t crc = crc16_calculate(packet, sizeof(packet));

    Serial.print("CRC: 0x");
    Serial.println(crc, HEX);
}
```

### Streaming CRC (Byte-by-Byte)

```cpp
#include <PrecompLib.h>

void setup() {
    Serial.begin(115200);

    // Start with initial value
    uint16_t crc = 0xFFFF;

    // Process bytes as they arrive
    crc = crc16_update(crc, 0x01);
    crc = crc16_update(crc, 0x02);
    crc = crc16_update(crc, 0x03);

    Serial.print("Streaming CRC: 0x");
    Serial.println(crc, HEX);
}
```

### Packet Validation

```cpp
#include <PrecompLib.h>

bool validatePacket(uint8_t* data, size_t len) {
    // Assume last 2 bytes are CRC (big-endian)
    if (len < 3) return false;

    uint16_t received_crc = (data[len-2] << 8) | data[len-1];
    uint16_t calculated_crc = crc16_calculate(data, len - 2);

    return (received_crc == calculated_crc);
}
```

## Error Handling

| Input | Behavior |
|-------|----------|
| NULL pointer | Returns initial CRC (0xFFFF) |
| Zero length | Returns initial CRC (0xFFFF) |
| Valid data | Returns calculated CRC |

## Hardware Validation

Tested and validated on:

| Target | MCU | Architecture | Result |
|--------|-----|--------------|--------|
| JHEF_JHEF411 | STM32F411RE | cortex-m4 | 3/3 PASS |
| BKMN_NERO | STM32F722RE | cortex-m7 | 3/3 PASS |
| MATEK_H743VI | STM32H743ZI | cortex-m7 | 3/3 PASS |

Both RTT (HIL testing) and Serial (Arduino IDE) output modes verified.

## Building from Source

The source code is maintained separately (not distributed with the Arduino core).

The precompiled archives are:
- `cortex-m4/fpv4-sp-d16-hard/libPrecompLib.a` (F4 + G4)
- `cortex-m7/fpv4-sp-d16-hard/libPrecompLib.a` (F7 + H7)

## Library Structure

```
libraries/PrecompLib/
├── library.properties          # precompiled=true, ldflags=-lPrecompLib
├── README.md                   # This file
├── src/
│   ├── PrecompLib.h           # Public API header
│   ├── cortex-m4/
│   │   └── fpv4-sp-d16-hard/
│   │       └── libPrecompLib.a    # F4 + G4 binary
│   └── cortex-m7/
│       └── fpv4-sp-d16-hard/
│           └── libPrecompLib.a    # F7 + H7 binary
└── examples/
    └── CRC16_Demo/
        └── CRC16_Demo.ino     # Example with HIL support
```

## How Precompiled Libraries Work

Arduino's `precompiled=true` in `library.properties` tells the build system to:

1. **Skip source compilation** - No `.cpp` files are compiled
2. **Link the static archive** - Uses `ldflags=-lPrecompLib` to link `libPrecompLib.a`
3. **Select correct binary** - Matches `{build.mcu}` and FPU settings to directory structure

This enables binary-only distribution while maintaining full Arduino IDE compatibility.

## Related Documentation

- [Arduino Library Specification - Precompiled Libraries](https://arduino.github.io/arduino-cli/latest/library-specification/#precompiled-binaries)
- [PRECOMPILED_LIB.md](PRECOMPILED_LIB.md) - Detailed implementation plan and HAL/CMSIS considerations

## License

Part of the STM32 Robotics Arduino Core.
