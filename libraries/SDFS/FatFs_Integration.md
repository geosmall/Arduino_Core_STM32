# FatFs Integration

How SDFS embeds ChaN's FatFs and connects it to the card.

## Layers

```
Sketch / Storage / minIniStorage
        │  FS.h (File, open, exists, ...)
        ▼
SDFS, SDFSFile          src/SDFS.cpp
        │  f_open, f_read, f_write, f_mkfs, f_expand, ...
        ▼
FatFs R0.15             src/fatfs/ff.c
        │  disk_read, disk_write, disk_ioctl
        ▼
sdfs_diskio.cpp         drive 0 → the attached SDBlockSPI
        │  readBlocks, writeBlocks, sync, blockCount, auBlocks
        ▼
SDBlockSPI              libraries/SDBlock (polled SPI, bounded waits)
```

`SDFS_SPI::begin(card, clock_hz)` attaches the card (`sdfs_diskio_attach()`)
and mounts drive `0:`. FatFs uses only the card's waiting calls, so it runs
while the card has no stream open; an SDBlock stream in progress makes those
calls return `StreamOpen`, which FatFs sees as a disk error.

## Disk I/O mapping (`src/sdfs_diskio.cpp`)

| FatFs call | Card call |
|---|---|
| `disk_initialize`, `disk_status` | `ready()` (the card has completed `begin()`) |
| `disk_read(sector, count)` | `readBlocks()` (CMD17 per block) |
| `disk_write(sector, count)` | `writeBlocks()` (CMD24 for one block, ACMD23 + CMD25 for more) |
| `CTRL_SYNC` | `sync(BUSY_TIMEOUT_US)` |
| `GET_SECTOR_COUNT` | `blockCount()` (from the CSD) |
| `GET_SECTOR_SIZE` | 512 |
| `GET_BLOCK_SIZE` | `auBlocks()` (allocation unit from the SD status, ACMD13), or 1 when unknown; `f_mkfs` aligns the data area to it |
| `get_fattime` | fixed 2024-01-01 12:00:00 (no RTC) |

## FatFs source

- Version R0.15, from STMicroelectronics' FatFs package v4.0.0; author ChaN
  (http://elm-chan.org/fsw/ff/), under the terms in the header of `ff.c`.
- Files in `src/fatfs/`: `ff.c`, `ff.h`, `ffconf.h`, `diskio.h`, `integer.h`.

### `ffconf.h` settings changed from the template

| Option | Value | Why |
|---|---|---|
| `FF_USE_MKFS` | 1 | `format()` |
| `FF_USE_EXPAND` | 1 | `f_expand`: a log file reserved contiguously before writing |
| `FF_USE_FASTSEEK` | 1 | Fast seek in large files |
| `FF_USE_STRFUNC` | 2 | `f_printf` and friends |
| `FF_FS_LOCK` | `SDFS_MAX_OPEN_FILES` | Open-file limit shared with the FS layer |

Unchanged and relevant: `FF_FS_EXFAT 0` (FAT12/16/32 only), `FF_LBA64 0`
(32-bit sector numbers, cards up to 2 TB), `FF_USE_LFN 0` (8.3 names),
`FF_FS_REENTRANT 0` (one caller at a time), `FF_MIN_SS = FF_MAX_SS = 512`.

## Updating FatFs

1. Download the release from http://elm-chan.org/fsw/ff/ and unpack it.
2. Replace `ff.c`, `ff.h`, `diskio.h` (and `integer.h` if present) in
   `src/fatfs/`.
3. Merge `ffconf.h`: take the new template, then reapply the settings in the
   table above.
4. Read the release's `updates.txt` for API and `diskio.h` changes; adjust
   `SDFS.cpp` and `sdfs_diskio.cpp`.
5. Compile `examples/SDFS_Test` and the HIL tests (`tests/SDFS_IT`,
   `tests/Storage_SDFS_IT`, `tests/minIni_SDFS_IT`), run them on the SD rig,
   and update the version tables here and in `README.md`.

### Version history

| Date | FatFs | SDFS | Notes |
|------|-------|------|-------|
| 2026-11-22 | R0.15 (80286) | v2.0.0 | Disk I/O over SDBlock; `FF_USE_EXPAND` on |
| 2025-09-13 | R0.15 (80286) | v1.0.0 | Configuration header, runtime detection |
| 2024-09-09 | R0.15 (80286) | v0.9.0 | SDHC optimization, directory enumeration fixes |
| 2024-08-21 | R0.15 (80286) | v0.8.0 | Initial implementation from STM32 FatFs v4.0.0 |
