/*
 * sdfs_diskio.cpp — FatFs disk I/O over an SDBlock card (drive 0).
 * Uses the card's waiting calls only; FatFs runs while disarmed.
 */

#include "sdfs_diskio.h"
#include "fatfs/ff.h"
#include "fatfs/diskio.h"

static SDBlockDevice *s_card = nullptr;

void sdfs_diskio_attach(SDBlockDevice *card)
{
  s_card = card;
}

SDBlockDevice *sdfs_diskio_card()
{
  return s_card;
}

static bool usable(BYTE pdrv)
{
  return pdrv == 0 && s_card != nullptr && s_card->ready();
}

extern "C" {

DSTATUS disk_initialize(BYTE pdrv)
{
  return usable(pdrv) ? 0 : STA_NOINIT;
}

DSTATUS disk_status(BYTE pdrv)
{
  return usable(pdrv) ? 0 : STA_NOINIT;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
  if (!usable(pdrv)) {
    return RES_NOTRDY;
  }
  return s_card->readBlocks(sector, buff, count) == SDBlockDevice::Result::Ok ? RES_OK : RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
  if (!usable(pdrv)) {
    return RES_NOTRDY;
  }
  return s_card->writeBlocks(sector, buff, count) == SDBlockDevice::Result::Ok ? RES_OK : RES_ERROR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
  if (!usable(pdrv)) {
    return RES_NOTRDY;
  }
  switch (cmd) {
    case CTRL_SYNC:
      return s_card->sync(SDBlockDevice::BUSY_TIMEOUT_US) == SDBlockDevice::Result::Ok ? RES_OK : RES_ERROR;
    case GET_SECTOR_COUNT:
      *(LBA_t *)buff = s_card->blockCount();
      return RES_OK;
    case GET_SECTOR_SIZE:
      *(WORD *)buff = SDBlockDevice::BLOCK_SIZE;
      return RES_OK;
    case GET_BLOCK_SIZE:
      // Erase block size in sectors: the card's allocation unit, or 1
      // (unknown) when not read. f_mkfs aligns the data area to it.
      *(DWORD *)buff = s_card->auBlocks() ? s_card->auBlocks() : 1;
      return RES_OK;
    default:
      return RES_PARERR;
  }
}

DWORD get_fattime(void)
{
  // No RTC: a fixed time stamp (2024-01-01 12:00:00).
  return ((DWORD)(2024 - 1980) << 25) | (1UL << 21) | (1UL << 16) | (12UL << 11);
}

}  // extern "C"
