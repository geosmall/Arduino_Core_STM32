#ifndef DSHOT_LL_H
#define DSHOT_LL_H

#include <Arduino.h>
#include "stm32yyxx_ll.h"
#include "DShot_packet.h"

namespace DShot {

static constexpr int MAX_MOTORS = 8;

// Per-motor hardware state
struct MotorHW {
  // Timer
  TIM_TypeDef *timer;
  uint32_t ll_channel;         // LL_TIM_CHANNEL_CHx
  uint8_t  channel_index;      // 0-3 (CH1-CH4)

  // DMA
  DMA_TypeDef *dma;
  uint32_t dma_stream;         // LL_DMA_STREAM_x (F4/F7/H7) or LL_DMA_CHANNEL_x (G4)
#if defined(STM32F4xx) || defined(STM32F7xx)
  uint32_t dma_channel_sel;    // LL_DMA_CHANNEL_x (F4/F7 request mux)
#endif

  // DMA buffer
  uint32_t dma_buffer[DMA_BUF_SIZE];

  // Motor state
  uint16_t throttle;
  bool     telemetry;
};

// Initialize GPIO alternate function for timer output
void initGPIO(uint32_t pin, TIM_TypeDef *timer, uint32_t ll_channel);

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

} // namespace DShot

#endif // DSHOT_LL_H
