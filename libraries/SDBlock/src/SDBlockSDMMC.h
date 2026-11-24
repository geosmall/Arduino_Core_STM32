#pragma once

/*
 * SDBlockSDMMC.h — SD card block device on SDMMC1, 4-bit, with the SDMMC's
 * own DMA (IDMA). STM32H7 (PROVISIONAL INTERFACE, see SDBlockDevice.h).
 *
 * Pins, fixed: CK PC12, CMD PD2, D0-D3 PC8-PC11 (AF12). Cards: SDHC/SDXC.
 * Registers only: the HAL SD driver's calls busy-wait for their responses.
 *
 * STREAM. Submitted blocks collect in the back end's buffer and are sent in
 * chunks of chunkBlocks() (the last one of a stream may be shorter). A chunk
 * is one CMD25 with an IDMA transfer and CMD12 (CMD24 for a single block),
 * then CMD13 until the card is back in the transfer state without errors. An
 * open CMD25 cannot be paced block by block: the SDMMC starts data only with
 * a data command (RM0433: DTEN must not be used with SD cards), and the IDMA
 * does not wait for a buffer to be filled. While a chunk is under way
 * blockFree() is false. Each step() starts or polls one phase of the chunk
 * and waits for none; a step is a register access or two, apart from the
 * block copies, which are the caller's, and the cache clean of one block in
 * submit(). busyMaxUs() is the longest chunk, from its first command to the
 * card ready (data transfer included).
 *
 * BUFFER. SDMMC1's IDMA reaches AXI SRAM and not DTCM or the D2/D3 SRAMs, so
 * the object must be in AXI SRAM (the .bss of the H743 linker scripts);
 * begin() refuses it elsewhere. The buffer is cached write-back, cleaned
 * before writes and invalidated after reads. Callers' buffers may be
 * anywhere: all data is copied through the back end's buffer.
 */

#include <Arduino.h>
#include "SDBlockDevice.h"

#if defined(STM32H7xx) && defined(SDMMC1) && defined(SDMMC_IDMA_IDMAEN)
#define SDBLOCK_HAS_SDMMC 1

class SDBlockSDMMC final : public SDBlockDevice {
public:
  static constexpr uint32_t MAX_CHUNK_BLOCKS = 16;
  static constexpr uint32_t INIT_CLOCK_HZ = 400000;

  // Not constexpr: a constant-initialised object would put the buffer's image
  // in flash.
  SDBlockSDMMC() {}

  Result begin(uint32_t clock_hz) override;

  bool ready() const override { return _ready; }
  uint32_t blockCount() const override { return _blocks; }
  uint32_t auBlocks() const override { return _au_blocks; }
  uint32_t clockHz() const override { return _clock_hz; }
  Result fault() const override { return _fault; }

  // Blocks per chunk (1 to MAX_CHUNK_BLOCKS), and ACMD23 (pre-erase count)
  // before each chunk of more than one block. BadState in a stream.
  Result setChunk(uint32_t blocks, bool pre_erase);
  uint32_t chunkBlocks() const { return _chunk; }

  // For diagnosing a failure: the last command, the last R1 and the SDMMC
  // status when the last failure was seen.
  uint8_t lastCommand() const { return _last_cmd; }
  uint32_t lastR1() const { return _last_r1; }
  uint32_t lastStatus() const { return _last_sta; }

  Result readBlocks(uint32_t lba, uint8_t *dst, uint32_t n) override;
  Result writeBlocks(uint32_t lba, const uint8_t *src, uint32_t n) override;
  Result sync(uint32_t timeout_us) override;

  Result streamOpen(uint32_t lba, uint32_t count) override;
  uint8_t *block() override { return _buf + _queued * BLOCK_SIZE; }
  bool blockFree() const override { return _phase == Phase::None && _queued < _chunk; }
  Result submit() override;
  Result step() override;
  Result streamClose() override;

  bool streaming() const override { return _state != State::Idle; }
  bool idle() const override { return _state == State::Idle && _phase == Phase::None; }
  uint32_t blocksWritten() const override { return _blocks_written; }
  uint32_t busyMaxUs() const override { return _busy_max_us; }
  void resetBusyMax() override { _busy_max_us = 0; }

private:
  enum class State : uint8_t { Idle, Streaming, Closing };
  // Phase of the chunk under way.
  enum class Phase : uint8_t { None, App, PreErase, Write, StopResp, StopBusy, Status };

  Result checkUsable(bool allow_stream) const;
  Result setFault(Result r);
  Result fail(Result r);
  Result recover(Result r);
  void setClock(uint32_t max_hz, bool wide);
  void startCommand(uint32_t idx, uint32_t arg, uint32_t flags);
  Result pollResponse();
  Result waitResponse(bool check_crc);
  Result command(uint32_t idx, uint32_t arg);
  Result waitTransfer(uint32_t timeout_us);
  Result readTransfer(uint32_t idx, uint32_t arg, uint32_t bytes, uint32_t size_log2);
  void startChunk(uint32_t lba, uint32_t n);
  void startWrite();
  Result advance();
  Result advanceTimed();

  bool _ready = false;
  Result _fault = Result::Ok;
  uint32_t _blocks = 0;
  uint32_t _au_blocks = 0;
  uint32_t _clock_hz = 0;
  uint32_t _kernel_hz = 0;
  uint32_t _rca = 0;            // in bits 31:16
  uint32_t _chunk = MAX_CHUNK_BLOCKS;
  bool _pre_erase = false;
  uint8_t _last_cmd = 0;
  uint32_t _last_r1 = 0;
  uint32_t _last_sta = 0;

  State _state = State::Idle;
  Phase _phase = Phase::None;
  uint32_t _queued = 0;         // blocks in the buffer, not yet on the card
  uint32_t _stream_lba = 0;     // where the next chunk goes
  uint32_t _stream_left = 0;    // blocks the stream still takes
  uint32_t _chunk_lba = 0;
  uint32_t _chunk_n = 0;
  uint32_t _chunk_t0 = 0;
  uint32_t _blocks_written = 0;
  uint32_t _busy_max_us = 0;

  // Whole cache lines, so maintenance touches nothing else.
  alignas(32) uint8_t _buf[MAX_CHUNK_BLOCKS * BLOCK_SIZE];
};

#endif  // STM32H7xx
