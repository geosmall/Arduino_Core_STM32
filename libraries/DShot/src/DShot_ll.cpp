#include <Arduino.h>
#include "DShot_ll.h"
#include "PeripheralPins.h"
#include "timer.h"
#include "pinmap.h"

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
void initGPIO(uint32_t pin, TIM_TypeDef *timer, uint32_t ll_channel)
{
  PinName pin_name = digitalPinToPinName(pin);

  // Find the PinMap_TIM entry matching both this physical pin AND the
  // specified timer.  pinmap_function() returns the first pin match, which
  // is wrong when a pin has multiple timer functions listed under ALT names
  // (e.g., PB0: TIM1_CH2N at AF1, TIM3_CH3 at AF2 via PB_0_ALT1).
  uint32_t function = 0;
  const PinMap *map = PinMap_TIM;
  while (map->pin != NC) {
    if (map->peripheral == timer &&
        STM_PORT(map->pin) == STM_PORT(pin_name) &&
        STM_PIN(map->pin) == STM_PIN(pin_name)) {
      function = map->function;
      break;
    }
    map++;
  }
  if (function == 0) {
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

// F4/F7: Fixed stream↔peripheral mapping from reference manuals
// Table: {timer, channel_index, DMA, stream, channel_select}
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

// G4/H7: DMAMUX allows flexible routing — allocate sequentially
static uint32_t next_dma_resource = 0;

// Get the DMAMUX request ID for a timer channel
static uint32_t getDMAMUXRequest(TIM_TypeDef *timer, uint8_t ch_index)
{
#if defined(STM32H7xx)
  // H7: DMAMUX1 request IDs
#if defined(TIM1_BASE)
  if (timer == TIM1) {
    static const uint32_t reqs[] = {
      LL_DMAMUX1_REQ_TIM1_CH1, LL_DMAMUX1_REQ_TIM1_CH2,
      LL_DMAMUX1_REQ_TIM1_CH3, LL_DMAMUX1_REQ_TIM1_CH4
    };
    return reqs[ch_index];
  }
#endif
#if defined(TIM3_BASE)
  if (timer == TIM3) {
    static const uint32_t reqs[] = {
      LL_DMAMUX1_REQ_TIM3_CH1, LL_DMAMUX1_REQ_TIM3_CH2,
      LL_DMAMUX1_REQ_TIM3_CH3, LL_DMAMUX1_REQ_TIM3_CH4
    };
    return reqs[ch_index];
  }
#endif
#if defined(TIM4_BASE)
  if (timer == TIM4) {
    static const uint32_t reqs[] = {
      LL_DMAMUX1_REQ_TIM4_CH1, LL_DMAMUX1_REQ_TIM4_CH2,
      LL_DMAMUX1_REQ_TIM4_CH3, 0  // TIM4_CH4 not available on some H7
    };
    return reqs[ch_index];
  }
#endif
#if defined(TIM5_BASE)
  if (timer == TIM5) {
    static const uint32_t reqs[] = {
      LL_DMAMUX1_REQ_TIM5_CH1, LL_DMAMUX1_REQ_TIM5_CH2,
      LL_DMAMUX1_REQ_TIM5_CH3, LL_DMAMUX1_REQ_TIM5_CH4
    };
    return reqs[ch_index];
  }
#endif
#if defined(TIM8_BASE)
  if (timer == TIM8) {
    static const uint32_t reqs[] = {
      LL_DMAMUX1_REQ_TIM8_CH1, LL_DMAMUX1_REQ_TIM8_CH2,
      LL_DMAMUX1_REQ_TIM8_CH3, LL_DMAMUX1_REQ_TIM8_CH4
    };
    return reqs[ch_index];
  }
#endif

#elif defined(STM32G4xx)
  // G4: DMAMUX request IDs
#if defined(TIM1_BASE)
  if (timer == TIM1) {
    static const uint32_t reqs[] = {
      LL_DMAMUX_REQ_TIM1_CH1, LL_DMAMUX_REQ_TIM1_CH2,
      LL_DMAMUX_REQ_TIM1_CH3, LL_DMAMUX_REQ_TIM1_CH4
    };
    return reqs[ch_index];
  }
#endif
#if defined(TIM3_BASE)
  if (timer == TIM3) {
    static const uint32_t reqs[] = {
      LL_DMAMUX_REQ_TIM3_CH1, LL_DMAMUX_REQ_TIM3_CH2,
      LL_DMAMUX_REQ_TIM3_CH3, LL_DMAMUX_REQ_TIM3_CH4
    };
    return reqs[ch_index];
  }
#endif
#if defined(TIM4_BASE)
  if (timer == TIM4) {
    static const uint32_t reqs[] = {
      LL_DMAMUX_REQ_TIM4_CH1, LL_DMAMUX_REQ_TIM4_CH2,
      LL_DMAMUX_REQ_TIM4_CH3, LL_DMAMUX_REQ_TIM4_CH4
    };
    return reqs[ch_index];
  }
#endif
#if defined(TIM8_BASE)
  if (timer == TIM8) {
    static const uint32_t reqs[] = {
      LL_DMAMUX_REQ_TIM8_CH1, LL_DMAMUX_REQ_TIM8_CH2,
      LL_DMAMUX_REQ_TIM8_CH3, LL_DMAMUX_REQ_TIM8_CH4
    };
    return reqs[ch_index];
  }
#endif
#if defined(TIM16_BASE)
  if (timer == TIM16) {
    // TIM16 only has CH1
    if (ch_index == 0) return LL_DMAMUX_REQ_TIM16_CH1;
  }
#endif
#endif // STM32G4xx

  return 0;
}

bool resolveDMA(MotorHW *motor)
{
  uint32_t dmamux_req = getDMAMUXRequest(motor->timer, motor->channel_index);
  if (dmamux_req == 0) return false;

#if defined(STM32H7xx)
  // H7: use DMA1 streams sequentially
  // DMA1 has streams 0-7
  if (next_dma_resource >= 8) return false;

  motor->dma = DMA1;
  motor->dma_stream = next_dma_resource;  // LL_DMA_STREAM_x == stream number
  next_dma_resource++;

#elif defined(STM32G4xx)
  // G4: use DMA1 channels sequentially
  // DMA1 has channels 1-8 (LL uses 1-based: LL_DMA_CHANNEL_1 etc.)
  if (next_dma_resource >= 8) return false;

  motor->dma = DMA1;
  // G4 LL_DMA_CHANNEL_x are 0-based: LL_DMA_CHANNEL_1=0, LL_DMA_CHANNEL_2=1, etc.
  motor->dma_stream = next_dma_resource;
  next_dma_resource++;
#endif

  // DMAMUX configuration is done in initDMA()
  return true;
}

#endif // family selection

// ---------------------------------------------------------------------------
// DMA initialization
// ---------------------------------------------------------------------------
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

#if defined(STM32F4xx) || defined(STM32F7xx)
  // F4/F7: Stream-based DMA with fixed channel selection

  LL_DMA_DisableStream(motor->dma, motor->dma_stream);
  LL_DMA_DeInit(motor->dma, motor->dma_stream);

  LL_DMA_InitTypeDef dma_init;
  LL_DMA_StructInit(&dma_init);

  dma_init.Channel = motor->dma_channel_sel;
  dma_init.MemoryOrM2MDstAddress = (uint32_t)motor->dma_buffer;
  dma_init.PeriphOrM2MSrcAddress = (uint32_t)ccr_addr;
  dma_init.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  dma_init.NbData = DMA_BUF_SIZE;
  dma_init.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  dma_init.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dma_init.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
  dma_init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
  dma_init.Mode = LL_DMA_MODE_NORMAL;
  dma_init.Priority = LL_DMA_PRIORITY_HIGH;
  dma_init.FIFOMode = LL_DMA_FIFOMODE_ENABLE;
  dma_init.FIFOThreshold = LL_DMA_FIFOTHRESHOLD_1_4;
  dma_init.MemBurst = LL_DMA_MBURST_SINGLE;
  dma_init.PeriphBurst = LL_DMA_PBURST_SINGLE;

  LL_DMA_Init(motor->dma, motor->dma_stream, &dma_init);

#elif defined(STM32H7xx)
  // H7: Stream-based DMA with DMAMUX

  LL_DMA_DisableStream(motor->dma, motor->dma_stream);
  LL_DMA_DeInit(motor->dma, motor->dma_stream);

  LL_DMA_InitTypeDef dma_init;
  LL_DMA_StructInit(&dma_init);

  dma_init.PeriphRequest = getDMAMUXRequest(motor->timer, motor->channel_index);
  dma_init.MemoryOrM2MDstAddress = (uint32_t)motor->dma_buffer;
  dma_init.PeriphOrM2MSrcAddress = (uint32_t)ccr_addr;
  dma_init.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  dma_init.NbData = DMA_BUF_SIZE;
  dma_init.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  dma_init.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dma_init.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
  dma_init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
  dma_init.Mode = LL_DMA_MODE_NORMAL;
  dma_init.Priority = LL_DMA_PRIORITY_HIGH;
  dma_init.FIFOMode = LL_DMA_FIFOMODE_DISABLE;
  dma_init.PeriphBurst = LL_DMA_PBURST_SINGLE;
  dma_init.MemBurst = LL_DMA_MBURST_SINGLE;

  LL_DMA_Init(motor->dma, motor->dma_stream, &dma_init);

#elif defined(STM32G4xx)
  // G4: Channel-based DMA with DMAMUX

  LL_DMA_DisableChannel(motor->dma, motor->dma_stream);
  LL_DMA_DeInit(motor->dma, motor->dma_stream);

  LL_DMA_InitTypeDef dma_init;
  LL_DMA_StructInit(&dma_init);

  dma_init.PeriphRequest = getDMAMUXRequest(motor->timer, motor->channel_index);
  dma_init.MemoryOrM2MDstAddress = (uint32_t)motor->dma_buffer;
  dma_init.PeriphOrM2MSrcAddress = (uint32_t)ccr_addr;
  dma_init.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  dma_init.NbData = DMA_BUF_SIZE;
  dma_init.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  dma_init.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dma_init.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
  dma_init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
  dma_init.Mode = LL_DMA_MODE_NORMAL;
  dma_init.Priority = LL_DMA_PRIORITY_HIGH;

  LL_DMA_Init(motor->dma, motor->dma_stream, &dma_init);
#endif

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
  // G4: GI (Global Interrupt) clears all flags for the channel
  switch (stream_or_channel) {
    case LL_DMA_CHANNEL_1: LL_DMA_ClearFlag_GI1(dma); break;
    case LL_DMA_CHANNEL_2: LL_DMA_ClearFlag_GI2(dma); break;
    case LL_DMA_CHANNEL_3: LL_DMA_ClearFlag_GI3(dma); break;
    case LL_DMA_CHANNEL_4: LL_DMA_ClearFlag_GI4(dma); break;
    case LL_DMA_CHANNEL_5: LL_DMA_ClearFlag_GI5(dma); break;
    case LL_DMA_CHANNEL_6: LL_DMA_ClearFlag_GI6(dma); break;
    case LL_DMA_CHANNEL_7: LL_DMA_ClearFlag_GI7(dma); break;
    case LL_DMA_CHANNEL_8: LL_DMA_ClearFlag_GI8(dma); break;
  }
#else
  // F4/F7/H7: clear TC flag for the stream (required before re-enabling)
  switch (stream_or_channel) {
    case LL_DMA_STREAM_0: LL_DMA_ClearFlag_TC0(dma); break;
    case LL_DMA_STREAM_1: LL_DMA_ClearFlag_TC1(dma); break;
    case LL_DMA_STREAM_2: LL_DMA_ClearFlag_TC2(dma); break;
    case LL_DMA_STREAM_3: LL_DMA_ClearFlag_TC3(dma); break;
    case LL_DMA_STREAM_4: LL_DMA_ClearFlag_TC4(dma); break;
    case LL_DMA_STREAM_5: LL_DMA_ClearFlag_TC5(dma); break;
    case LL_DMA_STREAM_6: LL_DMA_ClearFlag_TC6(dma); break;
    case LL_DMA_STREAM_7: LL_DMA_ClearFlag_TC7(dma); break;
  }
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
  // Flush DCache to ensure DMA reads current buffer contents (H7, some F7)
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
    if (LL_DMA_IsEnabledChannel(motors[i].dma, motors[i].dma_stream))
      return false;
#else
    if (LL_DMA_IsEnabledStream(motors[i].dma, motors[i].dma_stream))
      return false;
#endif
  }
  return true;
}

// ---------------------------------------------------------------------------
// DMAR burst mode — F4/F7 only
// ---------------------------------------------------------------------------
#if defined(STM32F4xx) || defined(STM32F7xx)

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

bool resolveDMABurst(BurstGroup *group)
{
  // Look up the trigger channel's DMA stream from the fixed F4/F7 map
  for (size_t i = 0; i < sizeof(dma_map) / sizeof(dma_map[0]); i++) {
    if (dma_map[i].timer == group->timer &&
        dma_map[i].ch_index == group->trigger_ch_index) {
      group->dma = dma_map[i].dma;
      group->dma_stream = dma_map[i].stream;
      group->dma_channel_sel = dma_map[i].channel_sel;
      return true;
    }
  }
  return false;
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

void initDMABurst(BurstGroup *group)
{
  enableDMAClk(group->dma);

  LL_DMA_DisableStream(group->dma, group->dma_stream);
  LL_DMA_DeInit(group->dma, group->dma_stream);

  LL_DMA_InitTypeDef dma_init;
  LL_DMA_StructInit(&dma_init);

  dma_init.Channel = group->dma_channel_sel;
  dma_init.MemoryOrM2MDstAddress = (uint32_t)group->burst_buffer;
  dma_init.PeriphOrM2MSrcAddress = (uint32_t)&group->timer->DMAR;
  dma_init.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  dma_init.NbData = DMA_BUF_SIZE * group->burst_length;
  dma_init.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  dma_init.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dma_init.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
  dma_init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
  dma_init.Mode = LL_DMA_MODE_NORMAL;
  dma_init.Priority = LL_DMA_PRIORITY_HIGH;
  dma_init.FIFOMode = LL_DMA_FIFOMODE_ENABLE;
  dma_init.FIFOThreshold = LL_DMA_FIFOTHRESHOLD_1_4;
  dma_init.MemBurst = LL_DMA_MBURST_SINGLE;
  dma_init.PeriphBurst = LL_DMA_PBURST_SINGLE;

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
  LL_DMA_EnableStream(group->dma, group->dma_stream);

  // Re-set DCR in case anything modified it (matches INav's pattern)
  LL_TIM_ConfigDMABurst(group->timer,
                        LL_TIM_DMABURST_BASEADDR_CCR1,
                        burstLengthToLL(group->burst_length));

  // Enable DMA request for the trigger channel's CC event
  enableTimDMAReq(group->timer, group->trigger_ch_index);
}

void cleanupPreviousBurstTransfer(BurstGroup *group)
{
  disableTimDMAReq(group->timer, group->trigger_ch_index);
  LL_DMA_DisableStream(group->dma, group->dma_stream);
  clearDMAFlags(group->dma, group->dma_stream);
}

bool burstTransferComplete(const BurstGroup *group)
{
  return !LL_DMA_IsEnabledStream(group->dma, group->dma_stream);
}

#endif // STM32F4xx || STM32F7xx

} // namespace DShot
