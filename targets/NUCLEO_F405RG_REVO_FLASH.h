#pragma once

// Custom NUCLEO-F405RG with its SPI flash declared — test header.
//
// NUCLEO_F405RG_REVO.h leaves the W25Q undeclared because its chip select,
// PB3, is also the SWO trace pin. This header adds the storage entry for
// builds that address the flash (SPIFlash/FlashLog rig tests). Such a build
// drives PB3 as chip select and runs no SWO trace.
//
// The fitted part is identified at run time by its JEDEC id, not by this
// header.

#include "NUCLEO_F405RG_REVO.h"

namespace BoardConfig {
  // Storage: SPI NOR flash on SPI3, wired as on the Revo
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PC12, PC11, PC10, PB3, 21000000, SPI3};
}
