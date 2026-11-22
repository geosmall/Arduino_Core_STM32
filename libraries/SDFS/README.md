# SDFS - SD Card File System Library

FatFs R0.15 behind the Arduino `FS.h` interface, for the STM32 Arduino core.
SDFS's disk I/O goes through an `SDBlockSPI` card object (`libraries/SDBlock`)
that the sketch owns, so the same card can also serve another SDBlock user,
such as an SD log writer, while FatFs is idle.

## Features

- `FS.h` interface (`open`, `exists`, `mkdir`, `rename`, `remove`, `rmdir`,
  `format`, sizes), shared with LittleFS and the `Storage` library
- FatFs R0.15: FAT12/16/32, 8.3 names, `f_expand` (contiguous allocation)
  and `f_mkfs` (format) enabled
- Card access through SDBlock: polled register-level SPI with bounded waits;
  SDHC/SDXC cards (byte-addressed SDSC cards are refused)
- Static allocation; the caller supplies the card object

## Requirements

- STM32F4, F7 or G4 (SDBlock's SPI back end; H7 is not supported yet)
- SDHC or SDXC card, formatted FAT32 (or formatted by `format()`); exFAT
  and NTFS volumes do not mount
- SPI wiring: MOSI, MISO, SCLK, CS, with the SPI block named in the board's
  `StorageConfig`

## Basic Usage

```cpp
#include <SDFS.h>
#include "targets/NUCLEO_F411RE_SDFS.h"

static SDBlockSPI card(BoardConfig::storage.instance,
                       BoardConfig::storage.mosi_pin,
                       BoardConfig::storage.miso_pin,
                       BoardConfig::storage.sclk_pin,
                       BoardConfig::storage.cs_pin);
static SDFS_SPI sdfs;

void setup() {
  Serial.begin(115200);
  if (sdfs.begin(card, BoardConfig::storage.freq_hz)) {
    File file = sdfs.open("/test.txt", FILE_WRITE_BEGIN);
    if (file) {
      file.println("Hello SDFS!");
      file.close();
    }
  }
}

void loop() {}
```

Through the `Storage` library, `BoardStorage::begin(BoardConfig::storage)`
builds the card from the same `StorageConfig` and mounts it.

## Card compatibility with Betaflight and INav

A card formatted by `format()` is readable by Betaflight's and INav's SD
blackbox (their `asyncfatfs`, SPI driver), which require: an MBR partition of
type FAT16, FAT16-LBA, FAT32 or FAT32-LBA starting after sector 0; 512-byte sectors; **exactly two FATs**; a power-of-two cluster of at most
64 KB; FAT16 or FAT32 (no exFAT). `format()` writes FAT32 (`0x0C`) at sector
63 with two FATs. Their SPI card drivers set no capacity limit, so an SDXC
card formatted FAT32 this way is accepted by the code (Betaflight's wiki says
SDXC is not supported, which matches cards as shipped, in exFAT). Read from
[Betaflight](https://github.com/betaflight/betaflight/blob/master/src/main/io/asyncfatfs/asyncfatfs.c)
and [INav](https://github.com/iNavFlight/inav/blob/master/src/main/io/asyncfatfs/asyncfatfs.c)
master, 2026-11-22; not tested on a Betaflight or INav board.

## API

### `SDFS_SPI`

| Call | Description |
|---|---|
| `bool begin(SDBlockSPI &card, uint32_t clock_hz)` | Starts the card at `clock_hz` (capped at 25 MHz) if it has not completed `begin()`, makes it FatFs drive 0, mounts the volume |
| `SDBlockSPI *card()` | The card in use |
| `const char *getMediaName()` | `"SD Card (SPI)"` |
| `bool isMounted()` | Volume mounted |
| `SDFSERR getLastError()` | Last SDFS error |

### `SDFS` (FS.h)

`open`, `exists`, `mkdir`, `rename`, `remove`, `rmdir`, `usedSize`,
`totalSize`, `mediaPresent`, `name`, and:

| Call | Description |
|---|---|
| `bool format()` | `f_mkfs` with a 512 B work buffer, FAT type chosen by FatFs, two FATs |
| `bool format(void *work, uint32_t work_len, uint8_t fmt = FM_ANY)` | `f_mkfs` with the caller's work buffer (FatFs writes the FAT `work_len` bytes at a time, so a larger buffer formats a large card faster) and format bits (`FM_FAT`, `FM_FAT32`, `FM_ANY`). One MBR partition, two FATs; the data area is aligned to the card's allocation unit when the card reports it |

Long FatFs operations (a format, a large file) are made of the card's
waiting calls, which call `yield()` per block: a sketch with a watchdog can
feed it there.

## Configuration

`src/SDFSConfig.h` (override with `-D` or before including):

| Macro | Default | Meaning |
|---|---|---|
| `SDFS_NAME_MAX` | 255 | Path length limit used by the FS layer |
| `SDFS_MAX_OPEN_FILES` | 2 | Open files at once (also FatFs `FF_FS_LOCK`) |
| `SDFS_FILE_MAX` | 2147483647 | File size limit reported to FS users |

The SPI clock is the `clock_hz` given to `begin()` (the board's
`StorageConfig::freq_hz` by convention). FatFs settings are in
`src/fatfs/ffconf.h`; see `FatFs_Integration.md`.

## Files

| File | Role |
|---|---|
| `src/SDFS.h`, `src/SDFS.cpp` | FS.h classes over FatFs |
| `src/sdfs_diskio.cpp` | FatFs `disk_*` functions over the attached `SDBlockSPI` |
| `src/fatfs/` | ChaN FatFs R0.15 (`ff.c`, `ff.h`, `diskio.h`, `ffconf.h`) |

## Version History

| Version | Date | Changes |
|---------|------|---------|
| v2.0.0 | 2026-11-22 | Disk I/O over an `SDBlockSPI` card the caller owns; `begin(card, clock_hz)`; `format()` with a caller work buffer; `f_expand` enabled; SDSC cards no longer supported |
| v1.0.0 | 2025-09-13 | Release with configuration header, runtime detection |
| v0.9.0 | 2024-09-09 | SDHC optimization and directory enumeration fixes |
| v0.8.0 | 2024-08-21 | Initial implementation from STM32 FatFs v4.0.0 |

## License

SDFS's own files follow the core's licensing (`LICENSE.md` at the core's
root). FatFs in `src/fatfs/` is ChaN's, under its own terms (header of `ff.c`).
