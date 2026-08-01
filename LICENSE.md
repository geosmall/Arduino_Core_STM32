# Licensing

This repository — the STM32_Robotics Arduino core, a fork of
[stm32duino/Arduino_Core_STM32](https://github.com/stm32duino/Arduino_Core_STM32)
— contains components under several licenses. There is no single license for
the whole tree; each component's terms are stated in its own files as listed
below. File headers govern where they differ from this summary.

| Component | Path | License |
|---|---|---|
| Arduino core API and stm32duino core files | `cores/arduino/` | LGPL-2.1-or-later (Arduino lineage; see file headers) |
| STM32 HAL/LL drivers | `system/Drivers/STM32*_HAL_Driver/` | BSD-3-Clause (STMicroelectronics; see each driver's `LICENSE.md`) |
| STM32 CMSIS device files | `system/Drivers/CMSIS/Device/` | STMicroelectronics terms per file headers |
| Fork-authored libraries | `libraries/SerialRx/`, `libraries/DShot/`, `libraries/BoardAlignment/`, `libraries/TimerPWM/` | MIT (see each library's `LICENSE`) |
| UF2 bootloader binaries | `bootloaders/` | MIT (BootUF2, TinyUF2/TinyUSB lineage; see `bootloaders/README.md`) |
| Bundled third-party libraries | `libraries/` (e.g. `Scheduler`, `EmbeddedCLI`, `libPrintf`, `minIniStorage`, `LittleFS`, `AUnit-1.7.1`) | Per-library terms — see each library's license file or file headers. Note `Scheduler` is GPL (INav/Cleanflight lineage); sketches that link it are subject to its terms. |
| Board variants and target configuration headers | `variants/`, `targets/` | Part of the core (see file headers where present) |

New original code contributed to this fork outside the components above is
MIT-licensed unless its file header states otherwise.
