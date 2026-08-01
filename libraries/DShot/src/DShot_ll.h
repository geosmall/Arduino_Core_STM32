// SPDX-License-Identifier: MIT
// Copyright (c) 2026 George Small
// See the LICENSE file in this library's root directory.

#ifndef DSHOT_LL_H
#define DSHOT_LL_H

#include <Arduino.h>
#include "stm32yyxx_ll.h"
#include "DShot_packet.h"

namespace DShot {

static constexpr int MAX_MOTORS = 8;

// DMA resource descriptor — used for external overrides (future allocator)
struct DMAResource {
  DMA_TypeDef *dma;
  uint32_t stream;             // LL_DMA_STREAM_x (F4/F7/H7) or LL_DMA_CHANNEL_x (G4)
#if defined(STM32F4xx) || defined(STM32F7xx)
  uint32_t channel_sel;        // LL_DMA_CHANNEL_x (F4/F7 request mux)
#endif
};

// Per-motor hardware state
struct MotorHW {
  // Timer
  TIM_TypeDef *timer;
  uint32_t ll_channel;         // LL_TIM_CHANNEL_CHx (compare register index)
  uint8_t  channel_index;      // 0-3 (CH1-CH4)
  bool     n_channel;          // true if pin AF maps to complementary output (CHxN)

  // DMA
  DMA_TypeDef *dma;
  uint32_t dma_stream;         // LL_DMA_STREAM_x (F4/F7/H7) or LL_DMA_CHANNEL_x (G4)
#if defined(STM32F4xx) || defined(STM32F7xx)
  uint32_t dma_channel_sel;    // LL_DMA_CHANNEL_x (F4/F7 request mux)
#endif
  bool dma_resolved;           // true if DMA set via override (skip resolveDMA)

  // DMA buffer
  uint32_t dma_buffer[DMA_BUF_SIZE];

  // Motor state
  uint16_t throttle;
  bool     telemetry;
};

// Initialize GPIO alternate function for timer output.
// Returns true if the resolved AF maps to a complementary output (CHxN).
bool initGPIO(Pin pin, TIM_TypeDef *timer, uint32_t ll_channel);

// Initialize timer for DShot: prescaler for target speed, ARR=BIT_PERIOD, PWM1 mode
void initTimer(TIM_TypeDef *timer, uint32_t ll_channel, Speed speed);

// Initialize DMA for a motor channel
void initDMA(MotorHW *motor);

// Trigger DMA transfer for a single motor
void triggerDMA(MotorHW *motor);

// Get timer clock frequency (handles APBx prescaler doubling)
uint32_t getTimerClockFreq(TIM_TypeDef *timer);

// Enable the timer peripheral clock
void enableTimerClk(TIM_TypeDef *timer);

// Enable DMA controller clock
void enableDMAClk(DMA_TypeDef *dma);

// Resolve DMA resource for a (timer, channel) pair.
// Populates motor->dma, dma_stream, and dma_channel_sel (F4/F7).
// Returns true on success.
bool resolveDMA(MotorHW *motor);

// Cleanup from previous DMA transfer: disable timer DMA request, clear flags.
// Called at the start of each Send() cycle before triggering new transfers.
void cleanupPreviousTransfer(MotorHW *motor);

// Check if all motors in an array have completed their DMA transfers.
// Polls hardware: in normal mode, DMA stream/channel auto-disables on completion.
bool allTransfersComplete(const MotorHW *motors, int count);

// ---------------------------------------------------------------------------
// DMAR burst mode — all families
// ---------------------------------------------------------------------------
// Uses a single DMA stream per timer group, writing to TIMx->DMAR which
// distributes to CCR1..CCRn sequentially. On F4/F7, solves the TIM1
// CH1/CH2/CH3 stream conflict where all three map to DMA2_Stream6.
// On G4/H7 (DMAMUX), reduces DMA consumption from one stream per channel
// to one stream per timer group for stream conservation.

// Per-timer-group burst state
struct BurstGroup {
  TIM_TypeDef *timer;
  DMA_TypeDef *dma;
  uint32_t dma_stream;
#if defined(STM32F4xx) || defined(STM32F7xx)
  uint32_t dma_channel_sel;
#endif
  uint32_t burst_buffer[DMA_BUF_SIZE * 4]; // interleaved, max 4 channels
  uint8_t  burst_length;      // highest (channel_index + 1), stride for interleaving
  uint8_t  trigger_ch_index;  // lowest active channel, its CC event triggers DMA
};

// Fill one motor's packet into the interleaved burst buffer.
// ch_index: 0-3 (which column in the interleaved layout).
// burst_length: stride between consecutive bits for the same channel.
void fillDmaBurstBuffer(uint32_t *burst_buffer, uint8_t ch_index,
                        uint8_t burst_length, uint16_t packet);

// Resolve DMA resource for DMAR burst (one stream per timer group).
// F4/F7: uses trigger channel's entry in dma_map[].
// G4/H7: scans for first unclaimed stream (DMAMUX routes the request).
bool resolveDMABurst(BurstGroup *group);

// Initialize DMA for DMAR burst mode (target = TIMx->DMAR, burst via DCR).
void initDMABurst(BurstGroup *group);

// Trigger DMAR burst DMA transfer for a timer group.
void triggerDMABurst(BurstGroup *group);

// Cleanup from previous burst transfer.
void cleanupPreviousBurstTransfer(BurstGroup *group);

// Check if burst DMA transfer is complete.
bool burstTransferComplete(const BurstGroup *group);

} // namespace DShot

#endif // DSHOT_LL_H
