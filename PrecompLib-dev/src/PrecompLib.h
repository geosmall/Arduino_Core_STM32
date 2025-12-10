/**
 * PrecompLib.h - Precompiled Library Example for STM32
 *
 * Demonstrates Arduino's precompiled library feature (precompiled=true)
 * for binary-only distribution.
 *
 * Functionality: CRC-16 (CCITT) checksum calculation
 * - Polynomial: 0x1021 (x^16 + x^12 + x^5 + 1)
 * - Initial value: 0xFFFF
 * - Standard test vector: "123456789" -> 0x29B1
 *
 * Supported targets:
 *   - F4 + G4: cortex-m4 with fpv4-sp-d16 hard float
 *   - F7 + H7: cortex-m7 with fpv4-sp-d16 hard float
 */

#ifndef PRECOMPLIB_H
#define PRECOMPLIB_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Calculate CRC-16 CCITT checksum over a data buffer
 *
 * @param data   Pointer to data buffer
 * @param length Number of bytes in buffer
 * @return       16-bit CRC value
 */
uint16_t crc16_calculate(const uint8_t* data, size_t length);

/**
 * Update running CRC-16 with a single byte
 *
 * Usage for streaming data:
 *   uint16_t crc = 0xFFFF;
 *   crc = crc16_update(crc, byte1);
 *   crc = crc16_update(crc, byte2);
 *   // ... final CRC in 'crc'
 *
 * @param crc    Current CRC value (start with 0xFFFF)
 * @param byte   Next byte to include in checksum
 * @return       Updated CRC value
 */
uint16_t crc16_update(uint16_t crc, uint8_t byte);

/**
 * Get library version string
 *
 * @return Version string (e.g., "1.0.0")
 */
const char* precomplib_version(void);

#ifdef __cplusplus
}
#endif

#endif // PRECOMPLIB_H
