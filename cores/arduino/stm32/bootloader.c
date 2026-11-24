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

/* The request is kept in an RTC backup register: it survives a reset and whatever the
   reset passes through (a UF2 bootloader in front of the app uses the same RAM as the
   app and may clear the reset flags), and reads 0 after a cold power-on (with a battery
   on VBAT, what was last written), so no reset-flag gate is needed. Not used by STM32RTC
   (DR1, DR6, DR7) or the HID bootloader (DR4, DR10). */
#define SYSDFU_BKP_INDEX  LL_RTC_BKP_DR8

/* Register access only: the check runs before HAL_Init(). */
static void sysdfu_rtcClockOn(void)
{
#if defined(RCC_APB4ENR_RTCAPBEN)      /* H7 */
  RCC->APB4ENR |= RCC_APB4ENR_RTCAPBEN;
  (void)RCC->APB4ENR;
#elif defined(RCC_APB1ENR1_RTCAPBEN)   /* G4 */
  RCC->APB1ENR1 |= RCC_APB1ENR1_RTCAPBEN;
  (void)RCC->APB1ENR1;
#endif
}

/* Opens the backup domain for writing. Returns 0 if it does not open. */
static int sysdfu_backupOpen(void)
{
#if defined(STM32F4xx)
  volatile uint32_t *const cr = &PWR->CR;
  const uint32_t dbp = PWR_CR_DBP;
#else
  volatile uint32_t *const cr = &PWR->CR1;
  const uint32_t dbp = PWR_CR1_DBP;
#endif
#if defined(__HAL_RCC_PWR_CLK_ENABLE)
  __HAL_RCC_PWR_CLK_ENABLE();
#endif
  *cr |= dbp;
  for (uint32_t spin = 0; spin < 100000; spin++) {
    if (*cr & dbp) {
      return 1;
    }
  }
  return 0;
}

/* Writes the register and reads it back. Returns 0 if it did not take. */
static int sysdfu_writeMarker(uint32_t value)
{
  sysdfu_rtcClockOn();
  if (!sysdfu_backupOpen()) {
    return 0;
  }
  setBackupRegister(SYSDFU_BKP_INDEX, value);
  __DSB();
  return getBackupRegister(SYSDFU_BKP_INDEX) == value;
}

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
  (void)sysdfu_writeMarker(SYSTEM_DFU_MAGIC);
  NVIC_SystemReset();
  while (1) { } /* unreachable */
}

void systemDFU_checkAndJump(void)
{
  sysdfu_rtcClockOn();
  if (getBackupRegister(SYSDFU_BKP_INDEX) != SYSTEM_DFU_MAGIC) {
    return;
  }

  /* One-shot. A marker that cannot be cleared is not honored: it would send every boot
     to the ROM. */
  if (!sysdfu_writeMarker(0)) {
    return;
  }

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
