"""
DMA option resolution for Betaflight timer configurations.

Translates dma_opt values from TIMER_PIN_MAP entries to DMA controller/stream/channel
assignments. The translation rules differ by MCU family:

- F4/F7 (fixed mapping): dma_opt indexes into a per-timer-channel options array
  from Betaflight's dmaTimerMapping[] table (dma_reqmap_mcu.c)
- G4/H7 (DMAMUX): dma_opt is a flat index into all available DMA streams/channels
"""

from dataclasses import dataclass
from typing import Optional


@dataclass
class DMAAssignment:
    """Resolved DMA assignment for a motor."""
    controller: int   # 1 or 2
    stream: int       # 0-7 (stream number for F4/F7/H7, channel-1 for G4)
    channel_sel: int  # 0-7 (F4/F7 channel select index, 0 for G4/H7)


# F4/F7 DMA timer mapping table from Betaflight dma_reqmap_mcu.c
# Key: (timer_name, channel) -> list of (controller, stream, channel_select)
# dma_opt indexes into the list; dma_opt=0 selects [0], etc.
# Note: For TIM1 CH1-CH3, dma_opt=0 is TIM1_UP (burst mode stream).
# dma_opt=1+ are per-channel dedicated streams.
DMA_TIMER_MAP_F4F7 = {
    ('TIM1', 1): [(2, 6, 0), (2, 1, 6), (2, 3, 6)],
    ('TIM1', 2): [(2, 6, 0), (2, 2, 6)],
    ('TIM1', 3): [(2, 6, 0), (2, 6, 6)],
    ('TIM1', 4): [(2, 4, 6)],

    ('TIM2', 1): [(1, 5, 3)],
    ('TIM2', 2): [(1, 6, 3)],
    ('TIM2', 3): [(1, 1, 3)],
    ('TIM2', 4): [(1, 7, 3), (1, 6, 3)],

    ('TIM3', 1): [(1, 4, 5)],
    ('TIM3', 2): [(1, 5, 5)],
    ('TIM3', 3): [(1, 7, 5)],
    ('TIM3', 4): [(1, 2, 5)],

    ('TIM4', 1): [(1, 0, 2)],
    ('TIM4', 2): [(1, 3, 2)],
    ('TIM4', 3): [(1, 7, 2)],

    ('TIM5', 1): [(1, 2, 6)],
    ('TIM5', 2): [(1, 4, 6)],
    ('TIM5', 3): [(1, 0, 6)],
    ('TIM5', 4): [(1, 1, 6), (1, 3, 6)],

    ('TIM8', 1): [(2, 2, 0), (2, 2, 7)],
    ('TIM8', 2): [(2, 2, 0), (2, 3, 7)],
    ('TIM8', 3): [(2, 2, 0), (2, 4, 7)],
    ('TIM8', 4): [(2, 7, 7)],
}


def resolve_dma(mcu_type: str, timer: str, channel: int,
                dma_opt: int) -> Optional[DMAAssignment]:
    """
    Resolve a Betaflight dma_opt value to a DMA assignment.

    Args:
        mcu_type: MCU type string (e.g., 'STM32F411', 'STM32H743')
        timer: Timer name (e.g., 'TIM1', 'TIM3')
        channel: Timer channel (1-4)
        dma_opt: DMA option from TIMER_PIN_MAP (-1 = no DMA)

    Returns:
        DMAAssignment if resolved, None if dma_opt is -1 or invalid
    """
    if dma_opt < 0:
        return None

    family = _get_family(mcu_type)

    if family in ('F4', 'F7'):
        return _resolve_f4f7(timer, channel, dma_opt)
    elif family == 'G4':
        return _resolve_g4(dma_opt)
    elif family == 'H7':
        return _resolve_h7(dma_opt)

    return None


def _get_family(mcu_type: str) -> Optional[str]:
    """Map MCU type to family."""
    if 'F41' in mcu_type or 'F40' in mcu_type:
        return 'F4'
    if 'F7' in mcu_type:
        return 'F7'
    if 'G4' in mcu_type:
        return 'G4'
    if 'H7' in mcu_type:
        return 'H7'
    return None


def _resolve_f4f7(timer: str, channel: int,
                  dma_opt: int) -> Optional[DMAAssignment]:
    """Resolve DMA for F4/F7 (fixed stream mapping via lookup table)."""
    key = (timer, channel)
    options = DMA_TIMER_MAP_F4F7.get(key)
    if not options or dma_opt >= len(options):
        return None

    controller, stream, channel_sel = options[dma_opt]
    return DMAAssignment(controller=controller, stream=stream,
                         channel_sel=channel_sel)


def _resolve_g4(dma_opt: int) -> Optional[DMAAssignment]:
    """Resolve DMA for G4 (DMAMUX, channel-based).

    dma_opt is a flat index: 0-7 = DMA1 CH1-CH8, 8-15 = DMA2 CH1-CH8.
    MotorConfig stores (channel - 1) as the stream value (0-7).
    """
    if dma_opt >= 16:
        return None
    controller = (dma_opt // 8) + 1
    stream = dma_opt % 8  # 0-7, maps to LL_DMA_CHANNEL_(x+1) = x
    return DMAAssignment(controller=controller, stream=stream, channel_sel=0)


def _resolve_h7(dma_opt: int) -> Optional[DMAAssignment]:
    """Resolve DMA for H7 (DMAMUX, stream-based).

    dma_opt is a flat index: 0-7 = DMA1 S0-S7, 8-15 = DMA2 S0-S7.
    """
    if dma_opt >= 16:
        return None
    controller = (dma_opt // 8) + 1
    stream = dma_opt % 8
    return DMAAssignment(controller=controller, stream=stream, channel_sel=0)
