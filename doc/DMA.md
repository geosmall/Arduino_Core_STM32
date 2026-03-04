# STM32 DMA Reference

Technical reference for Direct Memory Access (DMA) on STM32 microcontrollers in the Arduino STM32 Core.

## Overview

DMA enables data transfers between memory and peripherals without CPU intervention. This reduces interrupt overhead for timing-critical outputs like DShot motor control.

## DMA Architecture by Family

### STM32F4xx / STM32F7xx (Fixed Channel Assignment)

DMA1 and DMA2, each with 8 streams. Each stream has fixed channel mappings.

```
DMA Controller → Stream (0-7) → Channel (0-7) → Peripheral
```

### STM32G4xx (DMAMUX - Channel Based)

DMA1 and DMA2, each with up to 8 channels. DMAMUX routes any peripheral request to any channel.

```
Peripheral → DMAMUX → DMA Channel → Memory
```

### STM32H7xx (DMAMUX - Stream Based)

DMA1 and DMA2, each with 8 streams. DMAMUX routes any peripheral request to any stream.

```
Peripheral → DMAMUX → DMA Stream → Memory
```

## DMA Consumers

DShot motor output is currently the only DMA consumer in this codebase. The `dma.c`/`dma.h` dispatch layer provides centralized ownership tracking via `dma_claim()` / `dma_is_claimed()` and IRQ callback dispatch via `dma_set_handler()`.

UART DMA was removed in Phase 9 — interrupt-mode UART is sufficient for all current serial protocols at ≤115200 baud. See `UART_DSHOT_DMA_CLEANUP_PLAN.md` in the workspace root for rationale and re-implementation guidance if UART DMA is needed in the future.

## STM32H7 Cache Coherency

H7 has a D-Cache that can cause DMA coherency issues. DMA buffers on H7 must be placed in non-cached memory. The core provides D2 SRAM3 (0x30040000-0x30047FFF, 32KB) configured as non-cached via MPU.

## References

- **STM32 AN3109:** "Communication peripheral FIFO emulation with DMA"
- **STM32F4 Reference Manual:** DMA controller chapter
- **STM32H7 Reference Manual:** DMAMUX chapter
