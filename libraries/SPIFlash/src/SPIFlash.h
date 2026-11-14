#pragma once

/*
 * SPIFlash.h — raw polled SPI NOR flash driver (PROVISIONAL INTERFACE).
 *
 * The interface may change until the flight-log user's craft bench passes.
 *
 * - The part is identified by its JEDEC id from a table (SPIFlashParts.h),
 *   never assumed from a board header. An unknown id is refused: begin()
 *   returns UnknownPart and jedecId() names it.
 * - The SPI clock is an explicit argument, capped at the part's limit; the
 *   clock actually in use is reported by clockHz().
 * - Every SPI transfer goes through one function (xfer). Page programs use a
 *   transmit-only loop that keeps the transmit register full, so the bus runs
 *   at wire rate; reads use a one-byte-in-flight loop. Every hardware wait in
 *   those loops is bounded by a spin limit.
 * - pageProgram() and erase64k() return once the command is on the bus; they
 *   never wait for the chip. The caller polls isBusy() before the next write.
 *   waitIdle() is the only call that waits, with a timeout, for the erase path
 *   and tests.
 * - Static allocation only; the bus is assumed dedicated to the flash.
 *
 * Supported SPI blocks: STM32F4, F7, G4 (TXE/RXNE flags). H7-class SPI is not
 * supported yet.
 */

#include <Arduino.h>
#include <SPI.h>
#include "SPIFlashParts.h"

class SPIFlash {
public:
  enum class Result : uint8_t {
    Ok,
    NotFound,      // JEDEC read returned 0x000000 or 0xFFFFFF
    UnknownPart,   // id not in the part table
    NotReady,      // begin() not completed
    BadArgument,   // range outside the chip, or a program crossing a page
    Timeout,       // a bounded wait expired
    ProgramFail,   // the part reported a program/erase failure (EP_FAIL)
  };

  static constexpr uint32_t PAGE_SIZE = 256;
  static constexpr uint32_t BLOCK_64K = 65536;

  SPIFlash(SPI_TypeDef *instance, Pin mosi, Pin miso, Pin sclk, Pin cs);

  // Reads the JEDEC id, looks it up in `table`, sets the clock to
  // min(clock_hz, part limit). `table` defaults to SPIFLASH_PARTS; a test may
  // pass a table without the fitted part to exercise the refusal.
  Result begin(uint32_t clock_hz,
               const SPIFlashPart *table = SPIFLASH_PARTS,
               unsigned table_count = SPIFLASH_PART_COUNT);

  uint32_t jedecId() const { return _jedec; }
  const SPIFlashPart *part() const { return _part; }
  uint32_t clockHz() const { return _clock_hz; }
  uint32_t size() const { return _part ? _part->size_bytes : 0; }

  // Status register 1 read (05h): true while a program or erase runs.
  // A transfer timeout reports busy and sets the returned result.
  bool isBusy(Result *result = nullptr);

  // Polled read (03h) of n bytes.
  Result read(uint32_t addr, uint8_t *dst, uint32_t n);

  // Write enable (06h) then page program (02h) of 1..256 bytes, not crossing
  // a page boundary. Returns once the bytes are on the bus; does not check or
  // wait for busy.
  Result pageProgram(uint32_t addr, const uint8_t *src, uint32_t n);

  // Write enable then 64 KB block erase (D8h) at a 64 KB-aligned address.
  // Returns once the command is on the bus.
  Result erase64k(uint32_t addr);

  // Waits for busy to clear, up to timeout_us. Then, for parts with an
  // EP_FAIL bit, reports ProgramFail if the last program/erase failed.
  Result waitIdle(uint32_t timeout_us);

  // For parts with EP_FAIL: reads it (35h). Ok on parts without the bit.
  Result lastProgramResult();

  // The one transfer function. tx == nullptr sends 0xFF; rx == nullptr uses
  // the transmit-only loop and clears the receive flags afterwards.
  // Public for timing tests; callers frame chip select themselves.
  Result xfer(const uint8_t *tx, uint8_t *rx, uint32_t n);

  void select();
  void deselect();

private:
  Result command(uint8_t op);
  Result commandAddr(uint8_t op, uint32_t addr);
  static uint32_t busClockHz(SPI_TypeDef *spi);

  SPIClass _spi;
  SPI_TypeDef *_instance;
  Pin _cs;
  GPIO_TypeDef *_cs_port = nullptr;
  uint32_t _cs_mask = 0;
  uint32_t _jedec = 0;
  const SPIFlashPart *_part = nullptr;
  uint32_t _clock_hz = 0;
};
