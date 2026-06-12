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
 * BootUF2 bootloader support
 *
 * Board variant MUST define BOOTUF2_DBL_TAP_MAGIC and BOOTUF2_DBL_TAP_ADDR
 * to match the BootUF2 bootloader build. Source of truth:
 *   https://github.com/geosmall/bootuf2
 *   - Magic value: src/board_api.h (DBL_TAP_MAGIC)
 *   - RAM address: ports/stm32f4/boards.h (BOOTUF2_DBL_TAP_REG)
 *
 * Example for F4 (64KB RAM):
 *   #define BOOTUF2_DBL_TAP_MAGIC  0xf01669efUL
 *   #define BOOTUF2_DBL_TAP_ADDR   0x2000FFFCUL
 */
#if defined(BL_BOOTUF2)
  #if !defined(BOOTUF2_DBL_TAP_MAGIC) || !defined(BOOTUF2_DBL_TAP_ADDR)
    #error "BL_BOOTUF2 requires variant to define BOOTUF2_DBL_TAP_MAGIC and BOOTUF2_DBL_TAP_ADDR"
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
    *((volatile uint32_t *)BOOTUF2_DBL_TAP_ADDR) = BOOTUF2_DBL_TAP_MAGIC;
    NVIC_SystemReset();
  }
#endif /* BL_BOOTUF2 */

/*
 * Enter the STM32 factory system-memory DFU bootloader (USB 0483:df11).
 *
 * Jumps directly to the mask-ROM bootloader's vector table. The system bootloader
 * is always present regardless of what occupies main flash, so this is the way to
 * put a board back into ST DFU for reflashing (e.g. reverting a bootuf2 board to
 * stock Betaflight) without touching the BOOT0 pin.
 *
 * Per-family system-memory base (MSP at [0], reset vector at [1]). Addresses match
 * Betaflight's verified jump targets and ST AN2606:
 *   F4 (F4xx)      0x1FFF0000
 *   F7 (F72x/F73x) 0x1FF00000   (note: NOT 0x1FFF0000)
 *   G4 (G4xx)      0x1FFF0000
 *   H7 (H74x/H75x) 0x1FF09800
 *
 * Deinit order mirrors Betaflight system_stm32g4xx.c: HAL_RCC_DeInit() runs while
 * interrupts are still enabled (its timeout loops need the SysTick tick), then
 * SysTick and interrupts are torn down, then the jump.
 *
 * USB note: the caller should detach USB CDC first (e.g. Serial.end()) so the host
 * cleanly re-enumerates the device as DFU. HAL_RCC_DeInit() also stops the USB clock.
 *
 * Does not return.
 */
static inline void enterSystemBootloader(void) {
#if defined(STM32F4xx)
  const uint32_t sysmem_base = 0x1FFF0000UL;
#elif defined(STM32F7xx)
  const uint32_t sysmem_base = 0x1FF00000UL;
#elif defined(STM32G4xx)
  const uint32_t sysmem_base = 0x1FFF0000UL;
#elif defined(STM32H7xx)
  const uint32_t sysmem_base = 0x1FF09800UL;
#else
  #error "enterSystemBootloader: unsupported STM32 family"
#endif

  /* Reset clocks to their post-reset (HSI) state. Keep interrupts enabled here:
     HAL_RCC_DeInit() uses HAL_GetTick() timeouts that depend on the SysTick IRQ. */
  HAL_RCC_DeInit();

  /* Stop SysTick, then mask and clear all peripheral interrupts. */
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL  = 0;

  __disable_irq();
  for (uint32_t i = 0; i < (sizeof(NVIC->ICER) / sizeof(NVIC->ICER[0])); i++) {
    NVIC->ICER[i] = 0xFFFFFFFFUL;
    NVIC->ICPR[i] = 0xFFFFFFFFUL;
  }

#if defined(STM32H7xx)
  /* Cortex-M7 caches must be off before handing control to the ROM bootloader. */
  SCB_DisableDCache();
  SCB_DisableICache();
#endif

  /* Map system memory to 0x00000000, then jump. Only F4/G4 expose (and need) the
     runtime remap macro; F7/H7 jump via the absolute system-memory addresses with no
     remap, matching Betaflight (F7's remap is a no-op, H7 needs none). */
  __HAL_RCC_SYSCFG_CLK_ENABLE();
#if defined(STM32F4xx) || defined(STM32G4xx)
  __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();
#endif

  const uint32_t *vt = (const uint32_t *)sysmem_base;
  __set_MSP(vt[0]);
  __DSB();
  __ISB();
  ((void (*)(void))vt[1])();

  while (1) { } /* unreachable */
}

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _BOOTLOADER_H_ */
