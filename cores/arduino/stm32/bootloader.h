#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_

#include <stdint.h>
#include "stm32_def.h"  /* For NVIC_SystemReset() */

/* Ensure DTR_TOGGLING_SEQ enabled */
#if defined(BL_LEGACY_LEAF) || defined(BL_HID)
  #ifndef DTR_TOGGLING_SEQ
    #define DTR_TOGGLING_SEQ
  #endif /* DTR_TOGGLING_SEQ || BL_HID */
#endif /* BL_LEGACY_LEAF */

/*
 * TinyUF2 bootloader support
 *
 * Board variant MUST define TINYUF2_DBL_TAP_MAGIC and TINYUF2_DBL_TAP_ADDR
 * to match the TinyUF2 bootloader build. Source of truth:
 *   https://github.com/geosmall/tinyuf2
 *   - Magic value: src/board_api.h (DBL_TAP_MAGIC)
 *   - RAM address: ports/stm32f4/boards.h (TINYUF2_DBL_TAP_REG)
 *
 * Example for F4 (64KB RAM):
 *   #define TINYUF2_DBL_TAP_MAGIC  0xf01669efUL
 *   #define TINYUF2_DBL_TAP_ADDR   0x2000FFFCUL
 */
#if defined(BL_TINYUF2)
  #if !defined(TINYUF2_DBL_TAP_MAGIC) || !defined(TINYUF2_DBL_TAP_ADDR)
    #error "BL_TINYUF2 requires variant to define TINYUF2_DBL_TAP_MAGIC and TINYUF2_DBL_TAP_ADDR"
  #endif

  /**
   * @brief Enter UF2 bootloader from running application
   *
   * Writes magic value to RAM and resets. Bootloader sees
   * magic value and stays in DFU mode instead of jumping to app.
   *
   * Usage: Call from CLI command handler, e.g.:
   *   if (cmd == "bl") { enterBootloader(); }
   */
  static inline void enterBootloader(void) {
    *((volatile uint32_t *)TINYUF2_DBL_TAP_ADDR) = TINYUF2_DBL_TAP_MAGIC;
    NVIC_SystemReset();
  }
#endif /* BL_TINYUF2 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _BOOTLOADER_H_ */
