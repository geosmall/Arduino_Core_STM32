/**
  ******************************************************************************
  * @file    dma.h
  * @brief   DMA IRQ callback dispatch
  *
  * Centralized DMA IRQ handler dispatch. Peripheral drivers register
  * callbacks per DMA stream/channel instead of defining strong IRQ handlers.
  ******************************************************************************
  */

#ifndef __DMA_H
#define __DMA_H

#include <stdbool.h>
#include "stm32_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(HAL_DMA_MODULE_ENABLED)

/**
 * @brief  DMA IRQ callback type
 * @param  context  User pointer passed at registration
 */
typedef void (*dma_irq_callback_t)(void *context);

/**
 * @brief  Register a callback for a DMA stream/channel IRQ
 * @param  dma       DMA controller (DMA1 or DMA2)
 * @param  stream    Stream/channel index: LL_DMA_STREAM_x (F4/F7/H7)
 *                   or LL_DMA_CHANNEL_x (G4). Values 0-7.
 * @param  callback  Function to call when the IRQ fires
 * @param  context   User pointer passed to callback
 * @retval 0 on success, -1 if already claimed or invalid
 */
int dma_set_handler(DMA_TypeDef *dma, uint32_t stream,
                    dma_irq_callback_t callback, void *context);

/**
 * @brief  Unregister a DMA stream/channel callback
 * @param  dma     DMA controller (DMA1 or DMA2)
 * @param  stream  Stream/channel index (0-7)
 */
void dma_clear_handler(DMA_TypeDef *dma, uint32_t stream);

/**
 * @brief  Claim a DMA stream/channel without registering an IRQ callback.
 *         Used by consumers that own a stream but don't need TC interrupts
 *         (e.g., unidirectional DShot).
 * @param  dma     DMA controller (DMA1 or DMA2)
 * @param  stream  Stream/channel index (0-7)
 * @retval 0 on success, -1 if already claimed or invalid
 */
int dma_claim(DMA_TypeDef *dma, uint32_t stream);

/**
 * @brief  Release (unclaim) a DMA stream/channel.
 *         Inverse of dma_claim(). Clears the claimed flag, callback,
 *         and context. Does NOT disable the DMA hardware — caller must
 *         disable the stream/channel before releasing.
 * @param  dma     DMA controller (DMA1 or DMA2)
 * @param  stream  Stream/channel index (0-7)
 * @retval 0 on success, -1 if invalid index
 */
int dma_release(DMA_TypeDef *dma, uint32_t stream);

/**
 * @brief  Query whether a DMA stream/channel is claimed
 * @param  dma     DMA controller (DMA1 or DMA2)
 * @param  stream  Stream/channel index (0-7)
 * @retval true if claimed (by dma_set_handler or dma_claim), false if free
 */
bool dma_is_claimed(DMA_TypeDef *dma, uint32_t stream);

#endif /* HAL_DMA_MODULE_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __DMA_H */
