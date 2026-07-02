#pragma once

// Peripheral device selection + resolution for BoardConfig.
//
// BoardConfig names peripherals by a compact device enum (SpiDev, ...) instead
// of a raw CMSIS pointer. This mirrors Betaflight (spiDevice_e + a resolver
// table) and, crucially, keeps peripheral selection usable inside
// `static constexpr` BoardConfig aggregates: a uint8_t enum is a constant
// expression, whereas the CMSIS macro `SPI1` == reinterpret_cast<SPI_TypeDef*>(..)
// is not — and a non-constexpr pointer cannot survive being nested in a
// `static constexpr` object (e.g. IMUConfig holding an SPIConfig).
//
// This header is the SOLE place that maps a device enum to its CMSIS pointer;
// the reinterpret_cast is quarantined in the (runtime) resolvers below. The
// device handles carry only the enum, so they are literal types.
//
// See PERIPHERAL_DEVICE_ENUM_PLAN.md and Arduino_Core_STM32/../doc/STM32_GOTCHAS.md.

#include "stm32/pinmap.h"  // CMSIS peripheral types + macros (SPI1, ...)

namespace BoardConfig {

// ---------------------------------------------------------------------------
// SPI
// ---------------------------------------------------------------------------
enum class SpiDev : uint8_t { None, Spi1, Spi2, Spi3, Spi4, Spi5, Spi6 };

inline SPI_TypeDef* spiResolve(SpiDev d) {
  switch (d) {
#ifdef SPI1
    case SpiDev::Spi1: return SPI1;
#endif
#ifdef SPI2
    case SpiDev::Spi2: return SPI2;
#endif
#ifdef SPI3
    case SpiDev::Spi3: return SPI3;
#endif
#ifdef SPI4
    case SpiDev::Spi4: return SPI4;
#endif
#ifdef SPI5
    case SpiDev::Spi5: return SPI5;
#endif
#ifdef SPI6
    case SpiDev::Spi6: return SPI6;
#endif
    default: return nullptr;
  }
}

// Device handle: names an SPI by enum, resolves to the CMSIS pointer at runtime.
// No default ctor -> naming is mandatory in BoardConfig (omitting it fails to
// compile). Implicit ctor from SpiDev lets targets write `SpiDev::Spi1`;
// implicit operator SPI_TypeDef* lets the SPIClass ctor consume it unchanged.
struct SpiInstance {
  SpiDev dev;
  constexpr SpiInstance(SpiDev d) : dev(d) {}
  operator SPI_TypeDef*() const { return spiResolve(dev); }
  explicit operator bool() const { return dev != SpiDev::None; }
};

// ---------------------------------------------------------------------------
// UART / USART / LPUART  (CMSIS declares LPUARTn as (USART_TypeDef*)<base>)
// ---------------------------------------------------------------------------
enum class UartDev : uint8_t { None, Usart1, Usart2, Usart3, Uart4, Uart5, Usart6, Uart7, Uart8, Lpuart1 };

inline USART_TypeDef* uartResolve(UartDev d) {
  switch (d) {
#ifdef USART1
    case UartDev::Usart1: return USART1;
#endif
#ifdef USART2
    case UartDev::Usart2: return USART2;
#endif
#ifdef USART3
    case UartDev::Usart3: return USART3;
#endif
#ifdef UART4
    case UartDev::Uart4: return UART4;
#endif
#ifdef UART5
    case UartDev::Uart5: return UART5;
#endif
#ifdef USART6
    case UartDev::Usart6: return USART6;
#endif
#ifdef UART7
    case UartDev::Uart7: return UART7;
#endif
#ifdef UART8
    case UartDev::Uart8: return UART8;
#endif
#ifdef LPUART1
    case UartDev::Lpuart1: return LPUART1;
#endif
    default: return nullptr;
  }
}

struct UartInstance {
  UartDev dev;
  constexpr UartInstance(UartDev d) : dev(d) {}
  operator USART_TypeDef*() const { return uartResolve(dev); }
  explicit operator bool() const { return dev != UartDev::None; }
};

// ---------------------------------------------------------------------------
// I2C
// ---------------------------------------------------------------------------
enum class I2CDev : uint8_t { None, I2c1, I2c2, I2c3, I2c4 };

inline I2C_TypeDef* i2cResolve(I2CDev d) {
  switch (d) {
#ifdef I2C1
    case I2CDev::I2c1: return I2C1;
#endif
#ifdef I2C2
    case I2CDev::I2c2: return I2C2;
#endif
#ifdef I2C3
    case I2CDev::I2c3: return I2C3;
#endif
#ifdef I2C4
    case I2CDev::I2c4: return I2C4;
#endif
    default: return nullptr;
  }
}

struct I2CInstance {
  I2CDev dev;
  constexpr I2CInstance(I2CDev d) : dev(d) {}
  operator I2C_TypeDef*() const { return i2cResolve(dev); }
  explicit operator bool() const { return dev != I2CDev::None; }
};

// ---------------------------------------------------------------------------
// TIM (motor/servo timers)
// ---------------------------------------------------------------------------
enum class TimDev : uint8_t {
  None, Tim1, Tim2, Tim3, Tim4, Tim5, Tim6, Tim7, Tim8,
  Tim12, Tim13, Tim14, Tim15, Tim16, Tim17
};

inline TIM_TypeDef* timResolve(TimDev d) {
  switch (d) {
#ifdef TIM1
    case TimDev::Tim1: return TIM1;
#endif
#ifdef TIM2
    case TimDev::Tim2: return TIM2;
#endif
#ifdef TIM3
    case TimDev::Tim3: return TIM3;
#endif
#ifdef TIM4
    case TimDev::Tim4: return TIM4;
#endif
#ifdef TIM5
    case TimDev::Tim5: return TIM5;
#endif
#ifdef TIM6
    case TimDev::Tim6: return TIM6;
#endif
#ifdef TIM7
    case TimDev::Tim7: return TIM7;
#endif
#ifdef TIM8
    case TimDev::Tim8: return TIM8;
#endif
#ifdef TIM12
    case TimDev::Tim12: return TIM12;
#endif
#ifdef TIM13
    case TimDev::Tim13: return TIM13;
#endif
#ifdef TIM14
    case TimDev::Tim14: return TIM14;
#endif
#ifdef TIM15
    case TimDev::Tim15: return TIM15;
#endif
#ifdef TIM16
    case TimDev::Tim16: return TIM16;
#endif
#ifdef TIM17
    case TimDev::Tim17: return TIM17;
#endif
    default: return nullptr;
  }
}

// Mandatory: a motor/servo always names its timer.
struct TimInstance {
  TimDev dev;
  constexpr TimInstance(TimDev d) : dev(d) {}
  operator TIM_TypeDef*() const { return timResolve(dev); }
  explicit operator bool() const { return dev != TimDev::None; }
};

// ---------------------------------------------------------------------------
// DMA controller. Optional: DmaCtrl::None (the default) means "auto-resolve"
// (the DShot lib derives the stream/channel from the timer, Betaflight-compatible).
// ---------------------------------------------------------------------------
enum class DmaCtrl : uint8_t { None, Dma1, Dma2 };

inline DMA_TypeDef* dmaResolveCtrl(DmaCtrl d) {
  switch (d) {
#ifdef DMA1
    case DmaCtrl::Dma1: return DMA1;
#endif
#ifdef DMA2
    case DmaCtrl::Dma2: return DMA2;
#endif
    default: return nullptr;
  }
}

struct DmaInstance {
  DmaCtrl ctrl = DmaCtrl::None;
  constexpr DmaInstance() = default;
  constexpr DmaInstance(DmaCtrl c) : ctrl(c) {}
  operator DMA_TypeDef*() const { return dmaResolveCtrl(ctrl); }
  explicit operator bool() const { return ctrl != DmaCtrl::None; }
};

}  // namespace BoardConfig
