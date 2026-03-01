/**
  ******************************************************************************
  * @file    dma.c
  * @brief   DMA IRQ callback dispatch
  *
  * Macro-generated IRQ handlers dispatch through a callback table.
  * Replaces per-driver strong handler definitions (e.g., the hardcoded
  * handlers previously in uart.c) with a shared registration mechanism.
  *
  * Callers are responsible for NVIC priority and enable/disable.
  ******************************************************************************
  */

#include "dma.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(HAL_DMA_MODULE_ENABLED)

/* 8 streams/channels per DMA controller */
#if defined(DMA2)
#define DMA_HANDLER_COUNT 16
#else
#define DMA_HANDLER_COUNT 8
#endif

static struct {
  dma_irq_callback_t callback;
  void *context;
} dma_handler_table[DMA_HANDLER_COUNT];

static inline uint32_t dma_get_index(DMA_TypeDef *dma, uint32_t stream)
{
  uint32_t base = 0;
#if defined(DMA2)
  if (dma == DMA2) {
    base = 8;
  }
#endif
  return base + stream;
}

int dma_set_handler(DMA_TypeDef *dma, uint32_t stream,
                    dma_irq_callback_t callback, void *context)
{
  uint32_t idx = dma_get_index(dma, stream);
  if (idx >= DMA_HANDLER_COUNT) {
    return -1;
  }
  if (dma_handler_table[idx].callback != NULL) {
    return -1;
  }
  dma_handler_table[idx].callback = callback;
  dma_handler_table[idx].context = context;
  return 0;
}

void dma_clear_handler(DMA_TypeDef *dma, uint32_t stream)
{
  uint32_t idx = dma_get_index(dma, stream);
  if (idx >= DMA_HANDLER_COUNT) {
    return;
  }
  dma_handler_table[idx].callback = NULL;
  dma_handler_table[idx].context = NULL;
}

/* --- IRQ handler dispatch --- */

static inline void dma_dispatch(uint32_t idx)
{
  if (dma_handler_table[idx].callback) {
    dma_handler_table[idx].callback(dma_handler_table[idx].context);
  }
}

#if defined(STM32G4xx)

/* G4: channel-based DMA (channels 1-8, LL values 0-7) */
#define DEFINE_DMA_HANDLER(dma_n, ch_n)                        \
  void DMA##dma_n##_Channel##ch_n##_IRQHandler(void) {         \
    dma_dispatch(((dma_n) - 1) * 8 + ((ch_n) - 1));           \
  }

DEFINE_DMA_HANDLER(1, 1)
DEFINE_DMA_HANDLER(1, 2)
DEFINE_DMA_HANDLER(1, 3)
DEFINE_DMA_HANDLER(1, 4)
DEFINE_DMA_HANDLER(1, 5)
DEFINE_DMA_HANDLER(1, 6)
DEFINE_DMA_HANDLER(1, 7)
DEFINE_DMA_HANDLER(1, 8)
#if defined(DMA2)
DEFINE_DMA_HANDLER(2, 1)
DEFINE_DMA_HANDLER(2, 2)
DEFINE_DMA_HANDLER(2, 3)
DEFINE_DMA_HANDLER(2, 4)
DEFINE_DMA_HANDLER(2, 5)
DEFINE_DMA_HANDLER(2, 6)
DEFINE_DMA_HANDLER(2, 7)
DEFINE_DMA_HANDLER(2, 8)
#endif

#else

/* F4/F7/H7: stream-based DMA (streams 0-7) */
#define DEFINE_DMA_HANDLER(dma_n, stream_n)                    \
  void DMA##dma_n##_Stream##stream_n##_IRQHandler(void) {      \
    dma_dispatch(((dma_n) - 1) * 8 + (stream_n));             \
  }

DEFINE_DMA_HANDLER(1, 0)
DEFINE_DMA_HANDLER(1, 1)
DEFINE_DMA_HANDLER(1, 2)
DEFINE_DMA_HANDLER(1, 3)
DEFINE_DMA_HANDLER(1, 4)
DEFINE_DMA_HANDLER(1, 5)
DEFINE_DMA_HANDLER(1, 6)
DEFINE_DMA_HANDLER(1, 7)
#if defined(DMA2)
DEFINE_DMA_HANDLER(2, 0)
DEFINE_DMA_HANDLER(2, 1)
DEFINE_DMA_HANDLER(2, 2)
DEFINE_DMA_HANDLER(2, 3)
DEFINE_DMA_HANDLER(2, 4)
DEFINE_DMA_HANDLER(2, 5)
DEFINE_DMA_HANDLER(2, 6)
DEFINE_DMA_HANDLER(2, 7)
#endif

#endif /* STM32G4xx */

#endif /* HAL_DMA_MODULE_ENABLED */

#ifdef __cplusplus
}
#endif
