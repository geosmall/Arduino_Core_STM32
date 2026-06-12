#include "bootloader.h"

#include "stm32_def.h"
#include "backup.h"

#ifdef BL_LEGACY_LEAF
void dtr_togglingHook(uint8_t *buf, uint32_t *len)
{
  /**
   * Four byte is the magic pack "1EAF" that puts the MCU into bootloader.
   * Check if the incoming contains the string "1EAF".
   * If yes, put the MCU into the bootloader mode.
   */
  if ((*len >= 4) && (buf[0] == '1') && (buf[1] == 'E') && (buf[2] == 'A') && (buf[3] == 'F')) {
    NVIC_SystemReset();
  }
}
#endif /* BL_LEGACY_LEAF */

#ifdef BL_HID
void dtr_togglingHook(uint8_t *buf, uint32_t *len)
{
  /**
   * Four byte is the magic pack "1EAF" that puts the MCU into bootloader.
   * Check if the incoming contains the string "1EAF".
   * If yes, put the MCU into the bootloader mode.
   */
  if ((*len >= 4) && (buf[0] == '1') && (buf[1] == 'E') && (buf[2] == 'A') && (buf[3] == 'F')) {
    enableBackupDomain();
    /* New HID Bootloader (ver 2.2+) */
    setBackupRegister(HID_MAGIC_NUMBER_BKP_INDEX, HID_MAGIC_NUMBER_BKP_VALUE);
#ifdef HID_OLD_MAGIC_NUMBER_BKP_INDEX
    /* Compatibility to the old HID Bootloader (ver <= 2.1) */
    setBackupRegister(HID_OLD_MAGIC_NUMBER_BKP_INDEX, HID_MAGIC_NUMBER_BKP_VALUE);
#endif
    NVIC_SystemReset();
  }
}
#endif /* BL_HID */

/* ---- Reset-based ST system DFU bootloader entry (all families) ---- */

/* Marker magic, distinct from bootuf2's double-tap magic (0xf01669ef). */
#define SYSTEM_DFU_MAGIC  0xDF11B007UL

/* Reset-surviving marker: the .noinit section (system/ldscript.ld, NOLOAD, INSERT AFTER
   .bss) is not zeroed by startup and survives a warm reset (NVIC_SystemReset retains SRAM),
   so requestSystemBootloader()'s value is readable from the early premain() hook. */
__attribute__((section(".noinit"))) static volatile uint32_t s_sysdfu_marker;

/* Per-family system-memory base (MSP at [0], reset vector at [1]); matches AN2606. */
#if defined(STM32F4xx)
  #define SYSDFU_SYSMEM_BASE  0x1FFF0000UL
#elif defined(STM32F7xx)
  #define SYSDFU_SYSMEM_BASE  0x1FF00000UL
#elif defined(STM32G4xx)
  #define SYSDFU_SYSMEM_BASE  0x1FFF0000UL
#elif defined(STM32H7xx)
  #define SYSDFU_SYSMEM_BASE  0x1FF09800UL
#else
  #error "system DFU: unsupported STM32 family"
#endif

void requestSystemBootloader(void)
{
  s_sysdfu_marker = SYSTEM_DFU_MAGIC;
  __DSB();
  NVIC_SystemReset();
  while (1) { } /* unreachable */
}

void systemDFU_checkAndJump(void)
{
  if (s_sysdfu_marker != SYSTEM_DFU_MAGIC) {
    return;
  }
  s_sysdfu_marker = 0;   /* clear so we only jump once */
  __DSB();

  /* Minimal jump from the clean post-reset state (premain() runs this first: clocks at
     reset HSI default, HAL not initialized, caches off, interrupts enabled). Interrupts
     are LEFT ENABLED on purpose: of F4/F7/G4/H7 only G4 disables them in Betaflight (and
     tolerates either), while F4/F7/H7 require them on for the ROM bootloader's USB. No
     HAL_RCC_DeInit (clocks already at reset default; calling it before HAL_Init would also
     stall on HAL_GetTick timeouts). */
  __HAL_RCC_SYSCFG_CLK_ENABLE();
#if defined(STM32F4xx) || defined(STM32G4xx)
  __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();
#endif

  const uint32_t *vt = (const uint32_t *)SYSDFU_SYSMEM_BASE;
  __set_MSP(vt[0]);
  __DSB();
  __ISB();
  ((void (*)(void))vt[1])();

  while (1) { } /* unreachable */
}
