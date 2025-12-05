# WIRE_DMA.md - DMA Read Capability for Wire Library

## Overview

Add non-blocking DMA read capability to the Wire library for efficient multi-byte I2C sensor reads.

**Status**: Phase 2 Implemented ✅ (Pending Hardware Validation on MATEK_H743VI)
**Reference**: UVOS_Duino Wire library DMA implementation

## Implementation Phases

| Phase | Target | Status | Scope |
|-------|--------|--------|-------|
| **Phase 1** | STM32F4xx | ✅ Complete | Core DMA implementation, no cache concerns |
| **Phase 2** | STM32H7xx | ✅ Implemented | Non-cached D2 SRAM3 via MPU, pending hardware validation |

## Motivation

Current Wire library uses blocking reads - CPU polls until I2C transfer completes. For slow multi-byte reads (barometers, EEPROMs, multi-register sensor bursts), DMA frees the CPU during transfer.

## API Design

```cpp
// New TwoWire methods (polling-based, non-blocking)
bool requestFromDMA(uint8_t address, uint8_t* buf, uint32_t len, bool stopBit = true);
bool dmaTransferDone() const;
```

**Usage Pattern (F4):**
```cpp
uint8_t sensorData[16];
Wire.requestFromDMA(0x77, sensorData, 16);
while (!Wire.dmaTransferDone()) {
    // Do other work...
}
// Data now in sensorData[]
```

**Usage Pattern (H7 - Phase 2):**
```cpp
WIRE_DMA_BUFFER uint8_t sensorData[16];  // Must be in non-cached memory
Wire.requestFromDMA(0x77, sensorData, 16);
while (!Wire.dmaTransferDone()) {
    // Do other work...
}
```

## Architecture

### Current Wire Stack
```
TwoWire (Wire.h/cpp)     ← Arduino API
    ↓
twi.c/h                  ← HAL wrapper (blocking)
    ↓
HAL_I2C_Master_Receive_IT()  ← Interrupt-based, but code blocks waiting
```

### With DMA Addition
```
TwoWire (Wire.h/cpp)     ← Arduino API + new DMA methods
    ↓
twi.c/h                  ← HAL wrapper + DMA functions
    ↓
HAL_I2C_Master_Receive_DMA() ← True non-blocking with DMA
    ↓
DMA IRQ → Callback       ← Signals completion
```

---

## Phase 1: STM32F4xx Implementation

### Files to Modify

| File | Changes |
|------|---------|
| `libraries/Wire/src/utility/twi.h` | Add DMA state to `i2c_s` struct |
| `libraries/Wire/src/utility/twi.c` | Add DMA init, read, IRQ, callbacks |
| `libraries/Wire/src/Wire.h` | Declare requestFromDMA(), dmaTransferDone() |
| `libraries/Wire/src/Wire.cpp` | Implement new methods |

### 1. DMA State (twi.h)

Add to `struct i2c_s`:
```c
// DMA RX state
DMA_HandleTypeDef hdma_rx;
volatile uint8_t *dma_rx_buf;
volatile uint16_t dma_rx_len;
volatile bool dma_rx_busy;
volatile i2c_status_e dma_rx_result;
```

### 2. F4xx DMA Channel Mapping

| I2C | DMA | Stream | Channel |
|-----|-----|--------|---------|
| I2C1 | DMA1 | Stream 0 | Channel 1 |
| I2C2 | DMA1 | Stream 2 | Channel 7 |
| I2C3 | DMA1 | Stream 2 | Channel 3 |

### 3. Core Functions (twi.c)

```c
// Initialize DMA for I2C RX (lazy init on first use)
static void i2c_dma_rx_init(i2c_t *obj);

// Start DMA read - returns immediately
i2c_status_e i2c_master_read_dma(i2c_t *obj, uint8_t dev_address,
                                  uint8_t *data, uint16_t size);

// Check if DMA transfer complete
bool i2c_dma_rx_done(i2c_t *obj);

// Get result of last DMA transfer
i2c_status_e i2c_dma_rx_result(i2c_t *obj);
```

### 4. IRQ Handlers

```c
// DMA stream IRQ handler
void DMA1_Stream0_IRQHandler(void);  // I2C1 RX

// HAL completion callbacks (weak, we override)
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c);
```

### 5. Callback Implementation (F4 - Simple)

```c
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    i2c_t *obj = get_i2c_obj(hi2c);
    obj->dma_rx_busy = false;
    obj->dma_rx_result = I2C_OK;
    // No cache ops needed on F4
}
```

### 6. Arduino API Layer

```cpp
// Wire.h additions
bool requestFromDMA(uint8_t address, uint8_t* buf, uint32_t len, bool stopBit = true);
bool dmaTransferDone() const;

// Wire.cpp implementation
bool TwoWire::requestFromDMA(uint8_t address, uint8_t* buf,
                              uint32_t len, bool stopBit) {
    if (!i2c_dma_rx_done(&_i2c)) return false;  // Previous DMA still running
    return i2c_master_read_dma(&_i2c, address << 1, buf, len) == I2C_OK;
}

bool TwoWire::dmaTransferDone() const {
    return i2c_dma_rx_done(&_i2c);
}
```

### Phase 1 Hardware Validation Results (NUCLEO_F411RE)

**Test Date**: 2025-12-05
**Hardware**: NUCLEO_F411RE + DPS310 barometer (I2C @ 400kHz)
**Test Sketch**: `libraries/xensiv-dps3xx/examples/i2c_dma_test/`

| Test | Result | Details |
|------|--------|---------|
| DPS3xx detection | ✅ PASS | Product ID: 0x10 |
| DMA read product ID | ✅ PASS | Matches blocking read (0x10) |
| Multi-byte DMA (6 bytes) | ✅ PASS | Data integrity verified |
| Timing comparison | ✅ PASS | ~232 µs/read (blocking and DMA equivalent when waiting) |
| Idle state check | ✅ PASS | `dmaTransferDone()` returns true when idle |
| Busy rejection | ✅ PASS | Second DMA correctly rejected while first busy |

**All 6 tests passed.**

---

## Phase 2: STM32H7xx Implementation

### Problem: D-Cache Coherency

H7 has D-Cache - DMA writes to RAM bypass cache, causing coherency issues. CPU sees stale cached data instead of DMA-written data.

**Solution**: Configure D2 SRAM3 region as non-cached via MPU, place DMA buffers there.

### STM32H743 D2 SRAM Memory Map

| Region | Address | Size | Usage |
|--------|---------|------|-------|
| D2 SRAM1 | 0x30000000 - 0x3001FFFF | 128KB | General purpose (cached) |
| D2 SRAM2 | 0x30020000 - 0x3003FFFF | 128KB | General purpose (cached) |
| **D2 SRAM3** | **0x30040000 - 0x30047FFF** | **32KB** | **DMA buffers (non-cached)** |

**Why SRAM3?**
- **Clean separation**: Entire region is non-cached, no fragmentation of SRAM1/SRAM2
- **Dedicated purpose**: SRAM3 often unused, perfect for DMA buffer pool
- **Sufficient size**: 32KB plenty for I2C/SPI/UART DMA buffers (typical: 16-4096 bytes each)

### Files to Modify

| File | Changes |
|------|---------|
| `cores/arduino/stm32/hw_config.c` | MPU config for 32KB non-cached D2 SRAM3 |
| `variants/STM32H7xx/.../ldscript.ld` | Add `.dmabuf` section, add RAM_D2_SRAM3 memory region |
| `libraries/Wire/src/utility/twi.c` | Update buffer validation for SRAM3 address range |

**Phase 2 Implementation Status:**
- `WIRE_DMA_BUFFER` macro in Wire.h ✅
- H7 DMA channel mapping in twi.c ✅
- MPU configuration in hw_config.c ✅
- Buffer validation in twi.c ✅ (SRAM3: 0x30040000-0x30048000)
- Linker scripts updated ✅:
  - `H742V.../ldscript.ld` (MATEK_H743VI)
  - `H742Z.../ldscript.ld` (NUCLEO_H743ZI, NUCLEO_H753ZI)

### Design Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| MPU placement | `hw_config.c` | Central Arduino init, called after HAL_Init(), no ST file mods |
| Region location | D2 SRAM3 (0x30040000) | Clean separation, no SRAM1/SRAM2 fragmentation |
| Region size | 32KB | Matches SRAM3 exactly, sufficient for DMA buffers |

### H7xx DMA Channel Mapping

| I2C | DMA | Stream | Request |
|-----|-----|--------|---------|
| I2C1 | DMA1 | Stream 0 | DMA_REQUEST_I2C1_RX |
| I2C2 | DMA1 | Stream 2 | DMA_REQUEST_I2C2_RX |
| I2C4 | BDMA | Channel 1 | BDMA_REQUEST_I2C4_RX |

### 1. MPU Configuration (hw_config.c)

```c
#if defined(STM32H7xx) && defined(HAL_DMA_MODULE_ENABLED)
static void MPU_Config_DMA_Region(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  HAL_MPU_Disable();

  /* D2 SRAM3 (32KB): 0x30040000 - 0x30047FFF as non-cached, shareable */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x30040000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}
#endif
```

### 2. Linker Script (variants/.../ldscript.ld)

Add memory region and section:
```ld
/* Memory regions */
MEMORY
{
  ...
  RAM_D2 (xrw)       : ORIGIN = 0x30000000, LENGTH = 288K
  RAM_D2_SRAM3 (xrw) : ORIGIN = 0x30040000, LENGTH = 32K   /* DMA buffer pool */
  ...
}

/* Sections */
SECTIONS
{
  ...
  /* DMA buffer section in D2 SRAM3 (non-cached via MPU for DMA coherency) */
  .dmabuf (NOLOAD) :
  {
    . = ALIGN(4);
    *(.dmabuf)
    *(.dmabuf*)
    . = ALIGN(4);
  } >RAM_D2_SRAM3
  ...
}
```

### 3. Buffer Validation (twi.c)

```c
#if defined(STM32H7xx)
  /* D2 SRAM3 (non-cached region): 0x30040000 - 0x30047FFF (32KB) */
  uint32_t buf_addr = (uint32_t)data;
  if (buf_addr < 0x30040000 || buf_addr >= 0x30048000) {
    return I2C_ERROR; /* Buffer not in DMA-safe region */
  }
#endif
```

### 4. Buffer Macro (Wire.h - already implemented)

```c
#if defined(STM32H7xx)
  #define WIRE_DMA_BUFFER __attribute__((section(".dmabuf")))
#else
  #define WIRE_DMA_BUFFER  // No-op on F4
#endif
```

### Phase 2 Test Plan (MATEK_H743VI)

| Test | Description |
|------|-------------|
| Rejection test | Verify `requestFromDMA()` returns false for normal buffers |
| DMA read test | DPS310 product ID with `WIRE_DMA_BUFFER` |
| Multi-byte test | 6-byte pressure/temp burst read |
| Data integrity | Compare DMA vs blocking reads |
| Cache stress | Repeated reads with D-Cache active |

---

## Risks & Mitigations

| Risk | Mitigation |
|------|------------|
| DMA stream conflicts | Use streams not typically claimed by SPI/UART |
| H7 wrong buffer | Runtime address check returns error (not silent corruption) |
| IRQ priority issues | Match existing I2C IRQ priorities |
| Breaking existing code | Additive API only, no changes to existing functions |

## Estimated Effort

| Component | Lines | Effort |
|-----------|-------|--------|
| twi.h additions | ~15 | Small |
| twi.c DMA functions (F4) | ~150 | Medium |
| Wire.h/cpp additions | ~40 | Small |
| **Phase 1 Total** | **~200** | **Medium** |
| H7 additions (twi.c) | ~30 | Small |
| Linker script + MPU | ~20 | Small |
| **Phase 2 Total** | **~50** | **Small** |

## Reference Implementation

UVOS_Duino Wire library: `/home/geo/src/UVOS_Duino/libraries/Wire`
- `Wire.h:44-55` - API declarations
- `Wire.cpp:138-161` - requestFromDMA implementation
- `cores/arduino/per/i2c.cpp:562-632` - HAL DMA integration
- `cores/arduino/sys/dma.c` - Cache helpers
- `cores/arduino/uvos_core.h:28` - DMA_BUFFER_MEM_SECTION macro

## Future Extensions

- TX DMA for large writes (EEPROM programming)
- Callback-based completion notification
