#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Power saving mode, MCU runs significantly cooler
 * SysClock 240 Mhz, bus clocks 120 Mhz
 */
void SysClkHalfSpeed(void);

/*
 * Full speed - sysclk from PLL1 P - 480 Mhz
 * SysClock 480 Mhz, bus clocks 240 Mhz
 */
void SysClkFullSpeed(void);

#ifdef __cplusplus
}
#endif