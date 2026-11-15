#pragma once

/*
 * SPIFlashParts.h — supported SPI NOR parts, identified by JEDEC id (9Fh).
 *
 * Every figure is taken from the part's own datasheet, cited per entry.
 * max_clock_hz is the lower of the part's general clock limit (fC) and its
 * Read Data (03h) limit (fR): the driver runs every instruction at one clock.
 * Timeouts are the datasheet maximums.
 */

#include <stdint.h>

struct SPIFlashPart {
  uint32_t    jedec_id;          // manufacturer << 16 | memory type << 8 | capacity
  const char *name;
  uint32_t    size_bytes;
  uint32_t    max_clock_hz;      // min(fC, fR)
  uint32_t    page_program_max_us;   // tPP max
  uint32_t    erase_64k_max_ms;      // tBE2 max
  uint32_t    chip_erase_max_ms;     // tCE max
  bool        has_ep_fail;       // status register reports program/erase failure
};

// Winbond W25Q128FV — datasheet Revision L (2015-08-24), "AC Electrical
// Characteristics" (2.7-3.6 V, industrial): fC1 104 MHz, fR (03h) 50 MHz;
// tPP 0.7 ms typ / 3 ms max; tBE2 150 ms typ / 2,000 ms max; tCE 40 s typ /
// 200 s max. No program/erase fail bit in its status registers.
// The W25Q128JV reports the same JEDEC id (0xEF4018), so 9Fh cannot tell the
// two apart; this entry carries the FV's figures for both.
// Puya PY25Q128HA — datasheet V1.9 (2024-02-02; figures unchanged since
// v1.6), Table 5-3-1 AC Parameters (Grade I, 2.7-3.6 V): fC 133 MHz,
// fR (READ) 80 MHz; tPP 0.5 ms typ / 2.4 ms max (up to 256 B); tBE2 0.3 s typ
// / 1.2 s max; tCE 50 s typ / 120 s max. EP_FAIL is status bit S10 (read
// with 35h).
static constexpr SPIFlashPart SPIFLASH_PARTS[] = {
  {0xEF4018, "W25Q128",    16u * 1024u * 1024u, 50000000u, 3000u, 2000u, 200000u, false},
  {0x852018, "PY25Q128HA", 16u * 1024u * 1024u, 80000000u, 2400u, 1200u, 120000u, true},
};

static constexpr unsigned SPIFLASH_PART_COUNT =
    sizeof(SPIFLASH_PARTS) / sizeof(SPIFLASH_PARTS[0]);
