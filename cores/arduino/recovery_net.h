#pragma once

/*
 * recovery_net.h — keeps a board with no SWD and no BOOT0 pad recoverable
 * while it runs a test image. Active only in UF2-bootloader builds
 * (BL_BOOTUF2); compiles to nothing elsewhere.
 *
 *  - "bl" on Serial (USB CDC) enters the UF2 bootloader; "dfu" enters the ST
 *    ROM DFU bootloader (requestSystemBootloader()), for restoring a board's
 *    original full-flash image.
 *  - The independent watchdog runs (prescaler /256, reload 1000: ~8 s at
 *    the LSI's 32 kHz typical, ~4.6 s at its 47 kHz maximum). A hang stops
 *    the feeding; after the watchdog reset, recovery_begin() enters the UF2
 *    bootloader instead of running the test again.
 *  - The bootloader is entered by a software reset, which returns the
 *    watchdog to its reset (stopped) state (RM0383 §6.1.1: a system reset
 *    sets all registers to their reset values), so the bootloader runs
 *    without it.
 *
 * How a watchdog reset is told apart: by IWDGRSTF where the reset flags reach
 * the app. Started through the UF2 bootloader on F7, G4 or H7 they do not
 * (its HAL_RCC_DeInit() clears them, doc/STM32_GOTCHAS.md), so no flag at all
 * means: ask the marker, a word in RTC backup register RECOVERY_BKP_INDEX
 * written by recovery_begin() and cleared before every intended exit. It
 * survives a reset and the bootloader, and is lost with the power (on boards
 * without a battery on VBAT). The word is different for each build, so a new
 * image is not mistaken for a hung one. Any unplanned reset counts as a hang
 * there: the reset button, a J-Link "r" + "g", or a J-Link being plugged in
 * (it pulses NRST), enters the bootloader once;
 * its "reset" command starts the app.
 *
 * Use: recovery_begin() first in setup(); recovery_wait_start() (or
 * recovery_wait_host() to start without a 'g') before the first output;
 * recovery_feed() in every wait and every loop that can last over a second;
 * recovery_poll() in loop(). To leave on purpose: recovery_bootloader() or
 * recovery_dfu(); before any other deliberate reset, recovery_disarm().
 */

#include <Arduino.h>
#include <bootloader.h>

#if defined(BL_BOOTUF2)

#include <backup.h>

#if !defined(RECOVERY_BKP_INDEX)
#define RECOVERY_BKP_INDEX LL_RTC_BKP_DR5
#endif

#if defined(STM32H7xx)
#define RECOVERY_IWDG IWDG1
#define RECOVERY_RSR (RCC->RSR)
#define RECOVERY_RMVF RCC_RSR_RMVF
#define RECOVERY_IWDGRSTF RCC_RSR_IWDG1RSTF
#define RECOVERY_FLAGS 0xFFFE0000u   // RSR bits above RMVF
#else
#define RECOVERY_IWDG IWDG
#define RECOVERY_RSR (RCC->CSR)
#define RECOVERY_RMVF RCC_CSR_RMVF
#define RECOVERY_IWDGRSTF RCC_CSR_IWDGRSTF
#define RECOVERY_FLAGS 0xFE000000u   // CSR bits 31:25
#endif

// FNV-1a of the build time: different for each build of the sketch.
constexpr uint32_t recovery_marker()
{
  const char *const parts[2] = {__DATE__, __TIME__};
  uint32_t h = 2166136261u;
  for (const char *p : parts) {
    while (*p) {
      h = (h ^ uint8_t(*p++)) * 16777619u;
    }
  }
  return h | 1u;   // never 0, the value of a cleared register
}

inline void recovery_backup_open()
{
  enableBackupDomain();
#if defined(RCC_APB4ENR_RTCAPBEN)
  RCC->APB4ENR |= RCC_APB4ENR_RTCAPBEN;
  (void)RCC->APB4ENR;
#elif defined(RCC_APB1ENR_RTCAPBEN)
  RCC->APB1ENR |= RCC_APB1ENR_RTCAPBEN;
  (void)RCC->APB1ENR;
#endif
}

// False if the marker did not stick: hangs are then not caught where the
// reset flags are cleared.
inline bool recovery_arm()
{
  recovery_backup_open();
  setBackupRegister(RECOVERY_BKP_INDEX, recovery_marker());
  return getBackupRegister(RECOVERY_BKP_INDEX) == recovery_marker();
}

inline void recovery_disarm()
{
  recovery_backup_open();
  setBackupRegister(RECOVERY_BKP_INDEX, 0);
}

inline void recovery_bootloader()
{
  recovery_disarm();
  enterBootloader();
}

inline void recovery_dfu()
{
  recovery_disarm();
  requestSystemBootloader();
}

inline void recovery_begin()
{
  const uint32_t flags = RECOVERY_RSR & RECOVERY_FLAGS;
  RECOVERY_RSR |= RECOVERY_RMVF;   // clear the reset flags
  recovery_backup_open();
  const bool marked = getBackupRegister(RECOVERY_BKP_INDEX) == recovery_marker();
  if ((flags & RECOVERY_IWDGRSTF) || (flags == 0 && marked)) {
    recovery_bootloader();
  }
  recovery_arm();
  RECOVERY_IWDG->KR = 0xCCCC;          // start
  RECOVERY_IWDG->KR = 0x5555;          // unlock PR/RLR
  RECOVERY_IWDG->PR = 6;               // /256
  RECOVERY_IWDG->RLR = 1000;
  while (RECOVERY_IWDG->SR != 0) { }
  RECOVERY_IWDG->KR = 0xAAAA;          // reload
}

inline void recovery_feed()
{
  RECOVERY_IWDG->KR = 0xAAAA;
}

inline void recovery_poll()
{
  static char line[8];
  static uint8_t len = 0;
  recovery_feed();
  while (Serial.available()) {
    const char ch = (char)Serial.read();
    if (ch == '\r' || ch == '\n') {
      line[len] = '\0';
      if (strcmp(line, "bl") == 0) {
        Serial.println("Entering UF2 bootloader...");
        Serial.flush();
        delay(100);
        recovery_bootloader();
      } else if (strcmp(line, "dfu") == 0) {
        Serial.println("Entering ROM DFU...");
        Serial.flush();
        delay(100);
        recovery_dfu();
      }
      len = 0;
    } else if (len < sizeof(line) - 1) {
      line[len++] = ch;
    }
  }
}

// Waits for the host to open the port (USB CDC), feeding the watchdog; with
// no host within 60 s, falls back to the UF2 bootloader.
inline void recovery_wait_host()
{
  while (!Serial) {
    recovery_feed();
    delay(10);
    if (millis() > 60000) {
      recovery_bootloader();
    }
  }
  delay(500);
}

// recovery_wait_host(), then waits for 'g': opening the port is not a start
// signal, since other host software may open a new USB serial port.
inline void recovery_wait_start()
{
  recovery_wait_host();
  // 'g' starts; "bl" and "dfu" lines still work while waiting.
  char line[8];
  uint8_t len = 0;
  for (;;) {
    recovery_feed();
    const int c = Serial.read();
    if (c < 0) {
      delay(10);
      continue;
    }
    if (c == 'g' && len == 0) {
      return;
    }
    if (c == '\r' || c == '\n') {
      line[len] = '\0';
      if (strcmp(line, "bl") == 0) {
        recovery_bootloader();
      } else if (strcmp(line, "dfu") == 0) {
        recovery_dfu();
      }
      len = 0;
    } else if (len < sizeof(line) - 1) {
      line[len++] = char(c);
    }
  }
}

#else

inline void recovery_begin() {}
inline void recovery_feed() {}
inline void recovery_poll() {}
inline void recovery_disarm() {}
inline void recovery_wait_host()
{
  while (!Serial) {
    delay(10);
  }
  delay(200);
}
inline void recovery_wait_start() { recovery_wait_host(); }

#endif
