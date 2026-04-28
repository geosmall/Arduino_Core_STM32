#include <Arduino.h>
#include "DShot_ll.h"
#include "PeripheralPins.h"
#include "timer.h"
#include "pinmap.h"
#include "dma.h"

namespace DShot {

// ---------------------------------------------------------------------------
// Timer clock frequency
// ---------------------------------------------------------------------------
uint32_t getTimerClockFreq(TIM_TypeDef *timer)
{
  uint32_t timer_clock = 0;
  uint8_t clkSrc = getTimerClkSrc(timer);

  if (clkSrc == 2) {
    // APB2 timer
    timer_clock = HAL_RCC_GetPCLK2Freq();
#if defined(STM32H7xx)
    if ((RCC->D2CFGR & RCC_D2CFGR_D2PPRE2) != 0) {
      timer_clock *= 2;
    }
#else
    if ((RCC->CFGR & RCC_CFGR_PPRE2) != 0) {
      timer_clock *= 2;
    }
#endif
  } else {
    // APB1 timer
    timer_clock = HAL_RCC_GetPCLK1Freq();
#if defined(STM32H7xx)
    if ((RCC->D2CFGR & RCC_D2CFGR_D2PPRE1) != 0) {
      timer_clock *= 2;
    }
#else
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != 0) {
      timer_clock *= 2;
    }
#endif
  }

  return timer_clock;
}

// ---------------------------------------------------------------------------
// Enable timer peripheral clock
// ---------------------------------------------------------------------------
void enableTimerClk(TIM_TypeDef *timer)
{
  // Use a temporary HAL handle to leverage the core's enableTimerClock()
  TIM_HandleTypeDef htim;
  htim.Instance = timer;
  enableTimerClock(&htim);
}

// ---------------------------------------------------------------------------
// Enable DMA controller clock
// ---------------------------------------------------------------------------
void enableDMAClk(DMA_TypeDef *dma)
{
#if defined(DMA1)
  if (dma == DMA1) {
    __HAL_RCC_DMA1_CLK_ENABLE();
  }
#endif
#if defined(DMA2)
  if (dma == DMA2) {
    __HAL_RCC_DMA2_CLK_ENABLE();
  }
#endif
#if defined(__HAL_RCC_DMAMUX1_CLK_ENABLE)
  // G4/H7: DMAMUX has a separate clock gate
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
#endif
}

// ---------------------------------------------------------------------------
// GPIO init — configure pin AF for timer output
// ---------------------------------------------------------------------------
void initGPIO(Pin pin, TIM_TypeDef *timer, uint32_t ll_channel)
{
  PinName pin_name = pin.toPinName();

  // Peripheral-aware AF lookup: matches both pin AND timer instance,
  // correctly resolving ALT entries (e.g., PB0: TIM1_CH2N at AF1 vs
  // TIM3_CH3 at AF2 via PB_0_ALT1).
  uint32_t function = pinmap_function_for_peripheral(pin_name, timer, PinMap_TIM);
  if (function == (uint32_t)NC) {
    // Fallback: first pin match (original behavior)
    function = pinmap_function(pin_name, PinMap_TIM);
  }

  // Configure the pin: sets GPIO mode to AF, assigns the correct AF number
  pin_function(pin_name, function);

  // Set output speed to high for better signal integrity
  GPIO_TypeDef *port = get_GPIO_Port(STM_PORT(pin_name));
  LL_GPIO_SetPinSpeed(port, STM_LL_GPIO_PIN(pin_name), LL_GPIO_SPEED_FREQ_HIGH);
}

// ---------------------------------------------------------------------------
// Timer init — prescaler, ARR, PWM1 mode, output compare
// ---------------------------------------------------------------------------
void initTimer(TIM_TypeDef *timer, uint32_t ll_channel, Speed speed)
{
  enableTimerClk(timer);
  LL_TIM_DisableCounter(timer);

  // Time base
  LL_TIM_InitTypeDef tim_init;
  LL_TIM_StructInit(&tim_init);

  uint32_t tim_clk = getTimerClockFreq(timer);
  tim_init.Prescaler = (uint16_t)((tim_clk / (uint32_t)speed) - 1);
  tim_init.Autoreload = BIT_PERIOD;
  tim_init.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  tim_init.RepetitionCounter = 0;
  tim_init.CounterMode = LL_TIM_COUNTERMODE_UP;

  LL_TIM_Init(timer, &tim_init);

  // Output compare — configured per-channel by the caller after this
  // (initTimer is called once per timer group, OC init per channel)

  LL_TIM_EnableARRPreload(timer);
}

// Helper: init one OC channel on an already-initialized timer
static void initOC(TIM_TypeDef *timer, uint32_t ll_channel)
{
  LL_TIM_OC_InitTypeDef oc_init;
  LL_TIM_OC_StructInit(&oc_init);

  oc_init.OCMode = LL_TIM_OCMODE_PWM1;
  oc_init.OCState = LL_TIM_OCSTATE_ENABLE;
  oc_init.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
  oc_init.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  oc_init.CompareValue = 0;

  LL_TIM_OC_Init(timer, ll_channel, &oc_init);
  LL_TIM_OC_EnablePreload(timer, ll_channel);
  LL_TIM_CC_EnableChannel(timer, ll_channel);
}

// ---------------------------------------------------------------------------
// DMA resource resolution — family-specific
// ---------------------------------------------------------------------------

#if defined(STM32F4xx) || defined(STM32F7xx)

// F4/F7: Fixed stream-peripheral mapping from reference manuals
// Table: {timer, channel_index, DMA, stream, channel_select}
//
// F411 DMA streams shared with UART RX (claimed by uart.c via dma_set_handler):
//   DMA1_Stream5 — USART2_RX (also TIM3_CH2, but DShot skips via claim check)
//   DMA2_Stream1 — USART6_RX (also TIM1_CH1 alt, not in table)
//   DMA2_Stream2 — USART1_RX (also TIM1_CH2 alt, not in table)
// These streams are NOT excluded from the table. Instead, dma_is_claimed()
// detects conflicts at init time and auto-upgrades to DMAR burst if needed.
// See initAllDMA() in DShotOutput.cpp for the resolve -> decide -> claim flow.
struct DMAMapping {
  TIM_TypeDef *timer;
  uint8_t ch_index;      // 0=CH1, 1=CH2, 2=CH3, 3=CH4
  DMA_TypeDef *dma;
  uint32_t stream;
  uint32_t channel_sel;
};

static const DMAMapping dma_map[] = {
  // TIM1
  {TIM1, 0, DMA2, LL_DMA_STREAM_6, LL_DMA_CHANNEL_0},  // TIM1_CH1
  {TIM1, 1, DMA2, LL_DMA_STREAM_6, LL_DMA_CHANNEL_0},  // TIM1_CH2 (alt: stream 2)
  {TIM1, 2, DMA2, LL_DMA_STREAM_6, LL_DMA_CHANNEL_6},  // TIM1_CH3
  {TIM1, 3, DMA2, LL_DMA_STREAM_4, LL_DMA_CHANNEL_6},  // TIM1_CH4
  // TIM3
  {TIM3, 0, DMA1, LL_DMA_STREAM_4, LL_DMA_CHANNEL_5},  // TIM3_CH1
  {TIM3, 1, DMA1, LL_DMA_STREAM_5, LL_DMA_CHANNEL_5},  // TIM3_CH2
  {TIM3, 2, DMA1, LL_DMA_STREAM_7, LL_DMA_CHANNEL_5},  // TIM3_CH3
  {TIM3, 3, DMA1, LL_DMA_STREAM_2, LL_DMA_CHANNEL_5},  // TIM3_CH4
  // TIM4
  {TIM4, 0, DMA1, LL_DMA_STREAM_0, LL_DMA_CHANNEL_2},  // TIM4_CH1
  {TIM4, 1, DMA1, LL_DMA_STREAM_3, LL_DMA_CHANNEL_2},  // TIM4_CH2
  {TIM4, 2, DMA1, LL_DMA_STREAM_7, LL_DMA_CHANNEL_2},  // TIM4_CH3
  // TIM5
  {TIM5, 0, DMA1, LL_DMA_STREAM_2, LL_DMA_CHANNEL_6},  // TIM5_CH1
  {TIM5, 1, DMA1, LL_DMA_STREAM_4, LL_DMA_CHANNEL_6},  // TIM5_CH2
  {TIM5, 2, DMA1, LL_DMA_STREAM_0, LL_DMA_CHANNEL_6},  // TIM5_CH3
  {TIM5, 3, DMA1, LL_DMA_STREAM_1, LL_DMA_CHANNEL_6},  // TIM5_CH4
#if defined(TIM8_BASE)
  // TIM8
  {TIM8, 0, DMA2, LL_DMA_STREAM_2, LL_DMA_CHANNEL_7},  // TIM8_CH1
  {TIM8, 1, DMA2, LL_DMA_STREAM_3, LL_DMA_CHANNEL_7},  // TIM8_CH2
  {TIM8, 2, DMA2, LL_DMA_STREAM_4, LL_DMA_CHANNEL_7},  // TIM8_CH3
  {TIM8, 3, DMA2, LL_DMA_STREAM_7, LL_DMA_CHANNEL_7},  // TIM8_CH4
#endif
};

// TIM_UP DMA mapping for F4/F7 burst mode — paced by the timer UPDATE event
// (Betaflight-compatible). Keyed by timer only. Source: Betaflight
// timer_def.h:236-243 (F4) / :339-346 (F7), verified against RM0383 (F411)
// Table 28 and RM0431 (F722) Table 27 DMA request maps.
struct TimUpMapping {
  TIM_TypeDef *timer;
  DMA_TypeDef *dma;
  uint32_t stream;
  uint32_t channel_sel;
};

static const TimUpMapping tim_up_map[] = {
#if defined(TIM1_BASE)
  {TIM1, DMA2, LL_DMA_STREAM_5, LL_DMA_CHANNEL_6},  // TIM1_UP
#endif
#if defined(TIM2_BASE)
  {TIM2, DMA1, LL_DMA_STREAM_7, LL_DMA_CHANNEL_3},  // TIM2_UP
#endif
#if defined(TIM3_BASE)
  {TIM3, DMA1, LL_DMA_STREAM_2, LL_DMA_CHANNEL_5},  // TIM3_UP
#endif
#if defined(TIM4_BASE)
  {TIM4, DMA1, LL_DMA_STREAM_6, LL_DMA_CHANNEL_2},  // TIM4_UP
#endif
#if defined(TIM5_BASE)
  {TIM5, DMA1, LL_DMA_STREAM_0, LL_DMA_CHANNEL_6},  // TIM5_UP
#endif
#if defined(TIM8_BASE)
  {TIM8, DMA2, LL_DMA_STREAM_1, LL_DMA_CHANNEL_7},  // TIM8_UP
#endif
};

bool resolveDMA(MotorHW *motor)
{
  for (size_t i = 0; i < sizeof(dma_map) / sizeof(dma_map[0]); i++) {
    if (dma_map[i].timer == motor->timer &&
        dma_map[i].ch_index == motor->channel_index) {
      motor->dma = dma_map[i].dma;
      motor->dma_stream = dma_map[i].stream;
      motor->dma_channel_sel = dma_map[i].channel_sel;
      return true;
    }
  }
  return false;
}

#elif defined(STM32G4xx) || defined(STM32H7xx)

// G4/H7: DMAMUX allows flexible routing — scan for first unclaimed stream

// DMAMUX request lookup table — same pattern as dma_map[] above for F4/F7
struct DMAMUXMapping {
  TIM_TypeDef *timer;
  uint8_t ch_index;      // 0=CH1, 1=CH2, 2=CH3, 3=CH4
  uint32_t request_id;
};

#if defined(STM32H7xx)
static const DMAMUXMapping dmamux_map[] = {
#if defined(TIM1_BASE)
  {TIM1, 0, LL_DMAMUX1_REQ_TIM1_CH1}, {TIM1, 1, LL_DMAMUX1_REQ_TIM1_CH2},
  {TIM1, 2, LL_DMAMUX1_REQ_TIM1_CH3}, {TIM1, 3, LL_DMAMUX1_REQ_TIM1_CH4},
#endif
#if defined(TIM3_BASE)
  {TIM3, 0, LL_DMAMUX1_REQ_TIM3_CH1}, {TIM3, 1, LL_DMAMUX1_REQ_TIM3_CH2},
  {TIM3, 2, LL_DMAMUX1_REQ_TIM3_CH3}, {TIM3, 3, LL_DMAMUX1_REQ_TIM3_CH4},
#endif
#if defined(TIM4_BASE)
  {TIM4, 0, LL_DMAMUX1_REQ_TIM4_CH1}, {TIM4, 1, LL_DMAMUX1_REQ_TIM4_CH2},
  {TIM4, 2, LL_DMAMUX1_REQ_TIM4_CH3},  // TIM4_CH4 not available on some H7
#endif
#if defined(TIM5_BASE)
  {TIM5, 0, LL_DMAMUX1_REQ_TIM5_CH1}, {TIM5, 1, LL_DMAMUX1_REQ_TIM5_CH2},
  {TIM5, 2, LL_DMAMUX1_REQ_TIM5_CH3}, {TIM5, 3, LL_DMAMUX1_REQ_TIM5_CH4},
#endif
#if defined(TIM8_BASE)
  {TIM8, 0, LL_DMAMUX1_REQ_TIM8_CH1}, {TIM8, 1, LL_DMAMUX1_REQ_TIM8_CH2},
  {TIM8, 2, LL_DMAMUX1_REQ_TIM8_CH3}, {TIM8, 3, LL_DMAMUX1_REQ_TIM8_CH4},
#endif
};
#elif defined(STM32G4xx)
static const DMAMUXMapping dmamux_map[] = {
#if defined(TIM1_BASE)
  {TIM1, 0, LL_DMAMUX_REQ_TIM1_CH1}, {TIM1, 1, LL_DMAMUX_REQ_TIM1_CH2},
  {TIM1, 2, LL_DMAMUX_REQ_TIM1_CH3}, {TIM1, 3, LL_DMAMUX_REQ_TIM1_CH4},
#endif
#if defined(TIM3_BASE)
  {TIM3, 0, LL_DMAMUX_REQ_TIM3_CH1}, {TIM3, 1, LL_DMAMUX_REQ_TIM3_CH2},
  {TIM3, 2, LL_DMAMUX_REQ_TIM3_CH3}, {TIM3, 3, LL_DMAMUX_REQ_TIM3_CH4},
#endif
#if defined(TIM4_BASE)
  {TIM4, 0, LL_DMAMUX_REQ_TIM4_CH1}, {TIM4, 1, LL_DMAMUX_REQ_TIM4_CH2},
  {TIM4, 2, LL_DMAMUX_REQ_TIM4_CH3}, {TIM4, 3, LL_DMAMUX_REQ_TIM4_CH4},
#endif
#if defined(TIM8_BASE)
  {TIM8, 0, LL_DMAMUX_REQ_TIM8_CH1}, {TIM8, 1, LL_DMAMUX_REQ_TIM8_CH2},
  {TIM8, 2, LL_DMAMUX_REQ_TIM8_CH3}, {TIM8, 3, LL_DMAMUX_REQ_TIM8_CH4},
#endif
#if defined(TIM16_BASE)
  {TIM16, 0, LL_DMAMUX_REQ_TIM16_CH1},
#endif
};
#endif

static uint32_t getDMAMUXRequest(TIM_TypeDef *timer, uint8_t ch_index)
{
  for (size_t i = 0; i < sizeof(dmamux_map) / sizeof(dmamux_map[0]); i++) {
    if (dmamux_map[i].timer == timer && dmamux_map[i].ch_index == ch_index)
      return dmamux_map[i].request_id;
  }
  return 0;
}

// Burst mode uses timer UPDATE event as DMA trigger (Betaflight-compatible).
// Maps timer -> its DMAMUX UPDATE request ID.
static uint32_t getDMAMUXRequestUpdate(TIM_TypeDef *timer)
{
#if defined(STM32H7xx)
  #define _DSHOT_UP_REQ(n) LL_DMAMUX1_REQ_TIM ## n ## _UP
#else
  #define _DSHOT_UP_REQ(n) LL_DMAMUX_REQ_TIM ## n ## _UP
#endif
#if defined(TIM1_BASE)
  if (timer == TIM1) return _DSHOT_UP_REQ(1);
#endif
#if defined(TIM2_BASE)
  if (timer == TIM2) return _DSHOT_UP_REQ(2);
#endif
#if defined(TIM3_BASE)
  if (timer == TIM3) return _DSHOT_UP_REQ(3);
#endif
#if defined(TIM4_BASE)
  if (timer == TIM4) return _DSHOT_UP_REQ(4);
#endif
#if defined(TIM8_BASE)
  if (timer == TIM8) return _DSHOT_UP_REQ(8);
#endif
  #undef _DSHOT_UP_REQ
  return 0;
}

bool resolveDMA(MotorHW *motor)
{
  uint32_t dmamux_req = getDMAMUXRequest(motor->timer, motor->channel_index);
  if (dmamux_req == 0) return false;

  // Scan for the first unclaimed stream/channel across DMA1, then DMA2.
  // Skips streams already claimed by UART or other DMA consumers.
  DMA_TypeDef *controllers[] = { DMA1
#if defined(DMA2)
    , DMA2
#endif
  };
  for (size_t c = 0; c < sizeof(controllers) / sizeof(controllers[0]); c++) {
    for (uint32_t s = 0; s < 8; s++) {
      if (!dma_is_claimed(controllers[c], s)) {
        motor->dma = controllers[c];
        motor->dma_stream = s;
        // DMAMUX configuration is done in initDMA()
        return true;
      }
    }
  }
  return false;  // all streams/channels exhausted
}

#endif // family selection

// ---------------------------------------------------------------------------
// DMA initialization
// ---------------------------------------------------------------------------

// Common DMA init fields shared across all families and both per-channel
// and burst modes.  Callers set family-specific fields after this returns.
static void fillDMAInitCommon(LL_DMA_InitTypeDef *init,
                              uint32_t mem_addr, uint32_t periph_addr,
                              uint32_t nb_data)
{
  LL_DMA_StructInit(init);
  init->MemoryOrM2MDstAddress      = mem_addr;
  init->PeriphOrM2MSrcAddress      = periph_addr;
  init->NbData                     = nb_data;
  init->Direction                  = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  init->PeriphOrM2MSrcIncMode      = LL_DMA_PERIPH_NOINCREMENT;
  init->MemoryOrM2MDstIncMode      = LL_DMA_MEMORY_INCREMENT;
  init->PeriphOrM2MSrcDataSize     = LL_DMA_PDATAALIGN_WORD;
  init->MemoryOrM2MDstDataSize     = LL_DMA_MDATAALIGN_WORD;
  init->Mode                       = LL_DMA_MODE_NORMAL;
  init->Priority                   = LL_DMA_PRIORITY_HIGH;
}

void initDMA(MotorHW *motor)
{
  enableDMAClk(motor->dma);

  // Get the CCR address for this channel
  volatile uint32_t *ccr_addr;
  switch (motor->channel_index) {
    case 0: ccr_addr = &motor->timer->CCR1; break;
    case 1: ccr_addr = &motor->timer->CCR2; break;
    case 2: ccr_addr = &motor->timer->CCR3; break;
    case 3: ccr_addr = &motor->timer->CCR4; break;
    default: return;
  }

  LL_DMA_InitTypeDef dma_init;
  fillDMAInitCommon(&dma_init, (uint32_t)motor->dma_buffer,
                    (uint32_t)ccr_addr, DMA_BUF_SIZE);

#if defined(STM32F4xx) || defined(STM32F7xx)
  LL_DMA_DisableStream(motor->dma, motor->dma_stream);
  LL_DMA_DeInit(motor->dma, motor->dma_stream);
  dma_init.Channel = motor->dma_channel_sel;
  dma_init.FIFOMode = LL_DMA_FIFOMODE_ENABLE;
  dma_init.FIFOThreshold = LL_DMA_FIFOTHRESHOLD_1_4;

#elif defined(STM32H7xx)
  LL_DMA_DisableStream(motor->dma, motor->dma_stream);
  LL_DMA_DeInit(motor->dma, motor->dma_stream);
  dma_init.PeriphRequest = getDMAMUXRequest(motor->timer, motor->channel_index);

#elif defined(STM32G4xx)
  LL_DMA_DisableChannel(motor->dma, motor->dma_stream);
  LL_DMA_DeInit(motor->dma, motor->dma_stream);
  dma_init.PeriphRequest = getDMAMUXRequest(motor->timer, motor->channel_index);
#endif

  LL_DMA_Init(motor->dma, motor->dma_stream, &dma_init);

  // No DMA TC interrupts — completion is handled by cleanup-at-start-of-Send().
  // Normal-mode DMA auto-disables the stream/channel on transfer completion.
  // This avoids IRQ handler conflicts with uart.c strong handlers on F4.
}

// ---------------------------------------------------------------------------
// Timer DMA request enable/disable helpers
// ---------------------------------------------------------------------------
static void enableTimDMAReq(TIM_TypeDef *timer, uint8_t ch_index)
{
  switch (ch_index) {
    case 0: LL_TIM_EnableDMAReq_CC1(timer); break;
    case 1: LL_TIM_EnableDMAReq_CC2(timer); break;
    case 2: LL_TIM_EnableDMAReq_CC3(timer); break;
    case 3: LL_TIM_EnableDMAReq_CC4(timer); break;
  }
}

static void disableTimDMAReq(TIM_TypeDef *timer, uint8_t ch_index)
{
  switch (ch_index) {
    case 0: LL_TIM_DisableDMAReq_CC1(timer); break;
    case 1: LL_TIM_DisableDMAReq_CC2(timer); break;
    case 2: LL_TIM_DisableDMAReq_CC3(timer); break;
    case 3: LL_TIM_DisableDMAReq_CC4(timer); break;
  }
}

// ---------------------------------------------------------------------------
// DMA flag clearing — family-specific
// ---------------------------------------------------------------------------
static void clearDMAFlags(DMA_TypeDef *dma, uint32_t stream_or_channel)
{
#if defined(STM32G4xx)
  // G4: channel-based. GI clear bit at position (4 * ch) clears all flags.
  // Channels 1-8 are LL values 0-7; CGIF positions: 0, 4, 8, ..., 28.
  WRITE_REG(dma->IFCR, 0xFU << (4U * stream_or_channel));
#else
  // F4/F7/H7: stream-based. TC bit positions are non-uniform within each
  // half-register: {5, 11, 21, 27} for streams {0,1,2,3} and {4,5,6,7}.
  static const uint8_t tc_pos[] = {5, 11, 21, 27};
  volatile uint32_t *reg = (stream_or_channel < 4) ? &dma->LIFCR : &dma->HIFCR;
  WRITE_REG(*reg, 1UL << tc_pos[stream_or_channel & 3]);
#endif
}

// ---------------------------------------------------------------------------
// Cleanup from previous DMA transfer
// ---------------------------------------------------------------------------
void cleanupPreviousTransfer(MotorHW *motor)
{
  // Disable timer DMA request (may still be enabled from previous cycle)
  disableTimDMAReq(motor->timer, motor->channel_index);

  // Explicitly disable DMA before clearing flags and re-arming.
  // Normal mode auto-disables on completion, but explicit disable ensures
  // clean state for re-arm on all families (required on G4).
#if defined(STM32G4xx)
  LL_DMA_DisableChannel(motor->dma, motor->dma_stream);
#else
  LL_DMA_DisableStream(motor->dma, motor->dma_stream);
#endif

  // Clear DMA event flags (required before re-enabling stream/channel)
  clearDMAFlags(motor->dma, motor->dma_stream);
}

// ---------------------------------------------------------------------------
// Trigger DMA transfer
// ---------------------------------------------------------------------------
void triggerDMA(MotorHW *motor)
{
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
  // Flush DCache to ensure DMA reads current buffer contents (F7 / H7)
  SCB_CleanDCache_by_Addr(motor->dma_buffer, sizeof(motor->dma_buffer));
#endif

#if defined(STM32G4xx)
  LL_DMA_SetDataLength(motor->dma, motor->dma_stream, DMA_BUF_SIZE);
  LL_DMA_EnableChannel(motor->dma, motor->dma_stream);
#else
  // F4/F7/H7: stream-based
  LL_DMA_SetDataLength(motor->dma, motor->dma_stream, DMA_BUF_SIZE);
  LL_DMA_EnableStream(motor->dma, motor->dma_stream);
#endif

  enableTimDMAReq(motor->timer, motor->channel_index);
}

// ---------------------------------------------------------------------------
// Transfer complete check — polls hardware
// ---------------------------------------------------------------------------
bool allTransfersComplete(const MotorHW *motors, int count)
{
  // In normal mode, DMA auto-disables the stream/channel on transfer completion.
  // A disabled stream/channel means the transfer is done (or was never started).
  for (int i = 0; i < count; i++) {
#if defined(STM32G4xx)
    // G4: EN bit doesn't auto-clear because timer DMA requests keep re-triggering.
    // Check NDTR==0 instead — definitive indicator that all data was transferred.
    if (LL_DMA_GetDataLength(motors[i].dma, motors[i].dma_stream) != 0)
      return false;
#else
    if (LL_DMA_IsEnabledStream(motors[i].dma, motors[i].dma_stream))
      return false;
#endif
  }
  return true;
}

// ---------------------------------------------------------------------------
// DMAR burst mode — all families
// ---------------------------------------------------------------------------

void fillDmaBurstBuffer(uint32_t *burst_buffer, uint8_t ch_index,
                        uint8_t burst_length, uint16_t packet)
{
  for (int i = 0; i < 16; i++) {
    burst_buffer[ch_index + i * burst_length] =
        (packet & 0x8000) ? BIT_1_DUTY : BIT_0_DUTY;
    packet <<= 1;
  }
  // Reset frame (2 zero bits)
  burst_buffer[ch_index + 16 * burst_length] = 0;
  burst_buffer[ch_index + 17 * burst_length] = 0;
}

// Convert burst_length (1-4) to LL_TIM_DMABURST_LENGTH_xTRANSFERS enum
static uint32_t burstLengthToLL(uint8_t length)
{
  switch (length) {
    case 1:  return LL_TIM_DMABURST_LENGTH_1TRANSFER;
    case 2:  return LL_TIM_DMABURST_LENGTH_2TRANSFERS;
    case 3:  return LL_TIM_DMABURST_LENGTH_3TRANSFERS;
    default: return LL_TIM_DMABURST_LENGTH_4TRANSFERS;
  }
}

bool resolveDMABurst(BurstGroup *group)
{
#if defined(STM32F4xx) || defined(STM32F7xx)
  // Burst DMA is paced by the timer UPDATE event — look up the TIM_UP
  // stream for this timer (Betaflight-compatible). Keyed by timer only;
  // trigger_ch_index is unused on F4/F7 burst because the stream request
  // is TIM_UP, not any particular CC channel.
  for (size_t i = 0; i < sizeof(tim_up_map) / sizeof(tim_up_map[0]); i++) {
    if (tim_up_map[i].timer == group->timer) {
      group->dma = tim_up_map[i].dma;
      group->dma_stream = tim_up_map[i].stream;
      group->dma_channel_sel = tim_up_map[i].channel_sel;
      return true;
    }
  }
  return false;

#elif defined(STM32G4xx) || defined(STM32H7xx)
  // DMAMUX: scan for first unclaimed stream (any stream works)
  DMA_TypeDef *controllers[] = { DMA1
#if defined(DMA2)
    , DMA2
#endif
  };
  for (size_t c = 0; c < sizeof(controllers) / sizeof(controllers[0]); c++) {
    for (uint32_t s = 0; s < 8; s++) {
      if (!dma_is_claimed(controllers[c], s)) {
        group->dma = controllers[c];
        group->dma_stream = s;
        return true;
      }
    }
  }
  return false;  // all streams/channels exhausted
#endif
}

void initDMABurst(BurstGroup *group)
{
  enableDMAClk(group->dma);

  LL_DMA_InitTypeDef dma_init;
  fillDMAInitCommon(&dma_init, (uint32_t)group->burst_buffer,
                    (uint32_t)&group->timer->DMAR,
                    DMA_BUF_SIZE * group->burst_length);

#if defined(STM32F4xx) || defined(STM32F7xx)
  LL_DMA_DisableStream(group->dma, group->dma_stream);
  LL_DMA_DeInit(group->dma, group->dma_stream);
  dma_init.Channel = group->dma_channel_sel;
  dma_init.FIFOMode = LL_DMA_FIFOMODE_ENABLE;
  dma_init.FIFOThreshold = LL_DMA_FIFOTHRESHOLD_1_4;

#elif defined(STM32H7xx)
  LL_DMA_DisableStream(group->dma, group->dma_stream);
  LL_DMA_DeInit(group->dma, group->dma_stream);
  // Burst DMA is paced by the timer UPDATE event (one request per PWM period),
  // matching Betaflight. Required on TIM3/TIM4 to avoid draining all burst
  // words on a single CC event before preloaded CCRs can take effect.
  dma_init.PeriphRequest = getDMAMUXRequestUpdate(group->timer);

#elif defined(STM32G4xx)
  LL_DMA_DisableChannel(group->dma, group->dma_stream);
  LL_DMA_DeInit(group->dma, group->dma_stream);
  // Burst DMA is paced by the timer UPDATE event (one request per PWM period),
  // matching Betaflight. Required on TIM3/TIM4 to avoid draining all burst
  // words on a single CC event before preloaded CCRs can take effect.
  dma_init.PeriphRequest = getDMAMUXRequestUpdate(group->timer);
#endif

  LL_DMA_Init(group->dma, group->dma_stream, &dma_init);

  // Configure timer DMA burst: base address = CCR1, burst length = N registers
  LL_TIM_ConfigDMABurst(group->timer,
                        LL_TIM_DMABURST_BASEADDR_CCR1,
                        burstLengthToLL(group->burst_length));
}

void triggerDMABurst(BurstGroup *group)
{
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
  SCB_CleanDCache_by_Addr(group->burst_buffer,
                          DMA_BUF_SIZE * group->burst_length * sizeof(uint32_t));
#endif

  LL_DMA_SetDataLength(group->dma, group->dma_stream,
                       DMA_BUF_SIZE * group->burst_length);

#if defined(STM32G4xx)
  LL_DMA_EnableChannel(group->dma, group->dma_stream);
#else
  LL_DMA_EnableStream(group->dma, group->dma_stream);
#endif

  // Re-set DCR in case anything modified it (matches INav's pattern)
  LL_TIM_ConfigDMABurst(group->timer,
                        LL_TIM_DMABURST_BASEADDR_CCR1,
                        burstLengthToLL(group->burst_length));

  // Enable DMA request for the timer UPDATE event (burst is period-paced,
  // one DMA request per PWM period). Uniform across F4/F7/G4/H7 after
  // porting the G4 UPDATE-trigger fix to all families.
  LL_TIM_EnableDMAReq_UPDATE(group->timer);
}

void cleanupPreviousBurstTransfer(BurstGroup *group)
{
  LL_TIM_DisableDMAReq_UPDATE(group->timer);
  // Defensive: clear any CC DMA request bits that may have been left set by
  // a previous (pre-fix) binary running on the board — avoids stale-state
  // bugs on rapid re-flash without power cycle.
  CLEAR_BIT(group->timer->DIER, TIM_DIER_CC1DE | TIM_DIER_CC2DE
                              | TIM_DIER_CC3DE | TIM_DIER_CC4DE);

#if defined(STM32G4xx)
  LL_DMA_DisableChannel(group->dma, group->dma_stream);
#else
  LL_DMA_DisableStream(group->dma, group->dma_stream);
#endif

  clearDMAFlags(group->dma, group->dma_stream);
}

bool burstTransferComplete(const BurstGroup *group)
{
#if defined(STM32G4xx)
  return LL_DMA_GetDataLength(group->dma, group->dma_stream) == 0;
#else
  return !LL_DMA_IsEnabledStream(group->dma, group->dma_stream);
#endif
}

} // namespace DShot
