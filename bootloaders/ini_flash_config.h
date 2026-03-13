/*
 * ini_flash_config.h — Shared INI config flash format and logic
 *
 * Append-log storage for INI configuration in internal flash.
 * Used by both the UF2 bootloader (file container write) and
 * the application (CLI save/load).
 *
 * Each port's board_flash.c defines port-specific primitives,
 * then includes this header to get the public API implementation.
 *
 * Required defines before including:
 *   INI_FLASH_ALIGN             Flash write alignment in bytes (4, 8, or 32)
 *
 * Required static functions before including:
 *   ini_erase_config()          Erase config region (handles flash lock/unlock)
 *   ini_program(addr, data, len) Program bytes to flash (handles flash lock/unlock,
 *                                pads trailing bytes to alignment)
 *
 * Optional defines:
 *   INI_CACHE_SYNC(addr, len)   D-Cache invalidation before read (default: no-op)
 *
 * Required from board.h (via board_api.h):
 *   BOARD_FLASH_CONFIG_START    Config region start address
 *   BOARD_FLASH_CONFIG_SIZE     Config region size in bytes
 */

#ifndef INI_FLASH_CONFIG_H
#define INI_FLASH_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Append-log entry header */
#define INI_ENTRY_MAGIC  0x494E4943UL  /* "INIC" */

typedef struct {
  uint32_t magic;   /* INI_ENTRY_MAGIC */
  uint32_t size;    /* payload size in bytes (actual, not padded) */
} IniEntryHeader;

#endif /* INI_FLASH_CONFIG_H */

/*
 * Implementation section — activated when the port defines INI_FLASH_ALIGN.
 * Kept outside the include guard so it can be processed on a second inclusion
 * (board_api.h includes this header for types only; board_flash.c includes it
 * again after defining INI_FLASH_ALIGN to get the function implementations).
 */
#if defined(INI_FLASH_ALIGN) && !defined(INI_FLASH_CONFIG_IMPL)
#define INI_FLASH_CONFIG_IMPL

/* Pad size up to flash write alignment */
#define INI_PAD(x)     (((x) + (INI_FLASH_ALIGN - 1)) & ~(INI_FLASH_ALIGN - 1))

/* Padded header size on flash (8 for F4/F7/G4, 32 for H7) */
#define INI_HDR_SIZE   INI_PAD(sizeof(IniEntryHeader))

#ifndef INI_CACHE_SYNC
#define INI_CACHE_SYNC(addr, len)  ((void)0)
#endif

/*--------------------------------------------------------------------+
 * Append-log scan helpers
 *--------------------------------------------------------------------*/

/* Find offset of next free slot from BOARD_FLASH_CONFIG_START.
 * Returns BOARD_FLASH_CONFIG_SIZE if region is full. */
static uint32_t ini_find_free_offset(void) {
  uint32_t off = 0;
  while (off + sizeof(IniEntryHeader) < BOARD_FLASH_CONFIG_SIZE) {
    INI_CACHE_SYNC(BOARD_FLASH_CONFIG_START + off, sizeof(IniEntryHeader));
    const IniEntryHeader *hdr =
        (const IniEntryHeader *)(BOARD_FLASH_CONFIG_START + off);
    if (hdr->magic != INI_ENTRY_MAGIC) break;
    if (hdr->size == 0 || hdr->size > BOARD_FLASH_CONFIG_SIZE) break;
    off += INI_HDR_SIZE + INI_PAD(hdr->size);
  }
  return off;
}

/* Find offset of the last valid entry. Sets *found = true if any exists. */
static uint32_t ini_find_last_entry(bool *found) {
  uint32_t off = 0;
  uint32_t last_off = 0;
  *found = false;
  while (off + sizeof(IniEntryHeader) < BOARD_FLASH_CONFIG_SIZE) {
    INI_CACHE_SYNC(BOARD_FLASH_CONFIG_START + off, sizeof(IniEntryHeader));
    const IniEntryHeader *hdr =
        (const IniEntryHeader *)(BOARD_FLASH_CONFIG_START + off);
    if (hdr->magic != INI_ENTRY_MAGIC) break;
    if (hdr->size == 0 || hdr->size > BOARD_FLASH_CONFIG_SIZE) break;
    last_off = off;
    *found = true;
    off += INI_HDR_SIZE + INI_PAD(hdr->size);
  }
  return last_off;
}

/*--------------------------------------------------------------------+
 * Public API
 *--------------------------------------------------------------------*/

/* Read the last (most recent) INI entry payload.
 * Returns number of bytes copied, 0 if no valid entry found. */
uint32_t board_flash_ini_read(uint8_t *buf, uint32_t buf_size) {
  bool found;
  uint32_t last_off = ini_find_last_entry(&found);
  if (!found) return 0;

  INI_CACHE_SYNC(BOARD_FLASH_CONFIG_START + last_off, sizeof(IniEntryHeader));
  const IniEntryHeader *hdr =
      (const IniEntryHeader *)(BOARD_FLASH_CONFIG_START + last_off);
  uint32_t copy_size = hdr->size;
  if (copy_size > buf_size) copy_size = buf_size;

  uint32_t payload_addr = BOARD_FLASH_CONFIG_START + last_off + INI_HDR_SIZE;
  INI_CACHE_SYNC(payload_addr, copy_size);
  memcpy(buf, (const void *)payload_addr, copy_size);
  return copy_size;
}

/* Write one block of INI data to config flash.
 * Called per UF2 file container block; handles multi-block reassembly. */
bool board_flash_ini_write_block(const uint8_t *data, uint32_t len,
                                  uint32_t offset, uint32_t file_size,
                                  uint32_t block_no, uint32_t num_blocks) {
  (void)num_blocks;

  if (block_no == 0) {
    /* First block: find free slot, maybe erase, write header */
    uint32_t free_off = ini_find_free_offset();
    uint32_t needed = INI_HDR_SIZE + INI_PAD(file_size);

    if (free_off + needed > BOARD_FLASH_CONFIG_SIZE) {
      ini_erase_config();
      free_off = 0;
    }

    /* Write header padded to flash alignment */
    uint8_t hdr_buf[INI_HDR_SIZE];
    memset(hdr_buf, 0xFF, INI_HDR_SIZE);
    IniEntryHeader hdr = { .magic = INI_ENTRY_MAGIC, .size = file_size };
    memcpy(hdr_buf, &hdr, sizeof(hdr));
    ini_program(BOARD_FLASH_CONFIG_START + free_off, hdr_buf, INI_HDR_SIZE);
  }

  /* Find the current (last valid) entry to write payload into */
  bool found;
  uint32_t last_off = ini_find_last_entry(&found);
  uint32_t payload_base = BOARD_FLASH_CONFIG_START + last_off + INI_HDR_SIZE;

#if INI_FLASH_ALIGN > 8
  /* Large flash-word alignment (H7: 32 bytes).
   * UF2 file container block boundaries may not be 32-byte aligned,
   * so pad offset and length to flash-word boundaries. */
  {
    uint32_t align_mask = INI_FLASH_ALIGN - 1;
    uint32_t aligned_offset = offset & ~align_mask;
    uint32_t write_addr = payload_base + aligned_offset;

    if ((offset & align_mask) == 0 && (len & align_mask) == 0) {
      ini_program(write_addr, data, len);
    } else {
      uint8_t align_buf[512];
      uint32_t total = INI_PAD(offset + len) - aligned_offset;
      if (total > sizeof(align_buf)) total = sizeof(align_buf);
      memset(align_buf, 0xFF, total);
      memcpy(align_buf + (offset - aligned_offset), data, len);
      ini_program(write_addr, align_buf, total);
    }
  }
#else
  /* Small flash-word alignment (F4/F7: 4 bytes, G4: 8 bytes).
   * ini_program() handles trailing-byte padding internally. */
  ini_program(payload_base + offset, data, len);
#endif

  return true;
}

#endif /* INI_FLASH_ALIGN && !INI_FLASH_CONFIG_IMPL */
