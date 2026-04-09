# STM32 Arduino Core Serial Communication Architecture

## Deep Dive: Arduino API → STM32 HAL → Hardware

This document provides a comprehensive analysis of serial UART communication in the STM32 Arduino Core, tracing the path from user-facing Arduino API down to hardware registers and interrupts.

---

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Layer 1: Arduino API (HardwareSerial)](#layer-1-arduino-api-hardwareserial)
3. [Layer 2: Core UART Wrapper](#layer-2-core-uart-wrapper)
4. [Layer 3: ST HAL Driver](#layer-3-st-hal-driver)
5. [Layer 4: Hardware Interrupts](#layer-4-hardware-interrupts)
6. [Data Flow: RX Path](#data-flow-rx-path)
7. [Data Flow: TX Path](#data-flow-tx-path)
8. [Buffer Management](#buffer-management)
9. [Interrupt Priority & Timing](#interrupt-priority--timing)
10. [Critical Findings for RC Receiver Implementation](#critical-findings-for-rc-receiver-implementation)

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                      User Application                           │
│              Serial1.available(), Serial1.read()                │
└────────────────────────┬────────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────────┐
│          Layer 1: Arduino HardwareSerial Class                  │
│  File: cores/arduino/HardwareSerial.{h,cpp}                     │
│  • Ring buffer management (RX: 64 bytes, TX: 64 bytes)          │
│  • available(), read(), write() API                             │
│  • Buffer indices: rx_head, rx_tail, tx_head, tx_tail           │
└────────────────────────┬────────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────────┐
│          Layer 2: Core UART Wrapper (serial_t)                  │
│  File: cores/arduino/stm32/uart.{h,c}                           │
│  • uart_init(), uart_attach_rx_callback()                       │
│  • Interrupt handlers: _rx_complete_irq(), _tx_complete_irq()   │
│  • HAL API calls: HAL_UART_Receive_IT(), HAL_UART_Transmit_IT() │
└────────────────────────┬────────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────────┐
│          Layer 3: ST HAL Driver (UART_HandleTypeDef)            │
│  File: system/Drivers/STM32xxxx_HAL_Driver/Src/stm32xxxx_hal_uart.c │
│  • HAL_UART_IRQHandler() - Decodes interrupt source             │
│  • HAL_UART_RxCpltCallback() - RX complete callback             │
│  • HAL_UART_TxCpltCallback() - TX complete callback             │
│  • HAL_UART_ErrorCallback() - Error handling                    │
└────────────────────────┬────────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────────┐
│          Layer 4: Hardware (USART Peripheral Registers)         │
│  • USARTx->DR (Data Register) - Holds received/transmitted byte│
│  • USARTx->SR (Status Register) - RXNE, TXE, ORE, FE, PE flags │
│  • USARTx->CR1 (Control Register 1) - RXNEIE, TXEIE enables    │
│  • NVIC (Nested Vectored Interrupt Controller)                 │
│    - USARTx_IRQn interrupt vector                              │
│    - Priority: UART_IRQ_PRIO (default 1)                       │
│    - Subpriority: UART_IRQ_SUBPRIO (default 0)                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Layer 1: Arduino API (HardwareSerial)

### Files
- `cores/arduino/HardwareSerial.h`
- `cores/arduino/HardwareSerial.cpp`

### Key Data Structures

```cpp
class HardwareSerial : public Stream {
  protected:
    bool _written;                              // Has any byte been written?

    // Ring buffers for RX and TX
    unsigned char _rx_buffer[SERIAL_RX_BUFFER_SIZE];  // Default: 64 bytes
    unsigned char _tx_buffer[SERIAL_TX_BUFFER_SIZE];  // Default: 64 bytes

    serial_t _serial;  // Core wrapper structure (see Layer 2)

  private:
    bool _rx_enabled;
    uint8_t _config;      // SERIAL_8N1, SERIAL_8E1, etc.
    unsigned long _baud;
};
```

### Buffer Configuration

```cpp
// Default buffer sizes (can be overridden at compile time)
#define SERIAL_RX_BUFFER_SIZE 64
#define SERIAL_TX_BUFFER_SIZE 64

// Index types (uint8_t for buffers ≤256, uint16_t for >256)
#if (SERIAL_RX_BUFFER_SIZE > 256)
  typedef uint16_t rx_buffer_index_t;
#else
  typedef uint8_t rx_buffer_index_t;
#endif
```

### Key Methods

#### Initialization
```cpp
void HardwareSerial::begin(unsigned long baud, byte config)
{
    // Parse config byte (databits, parity, stopbits)
    // ...

    // Initialize UART peripheral via Layer 2
    uart_init(&_serial, baud, databits, parity, stopbits);

    // Attach RX interrupt callback
    uart_attach_rx_callback(&_serial, _rx_complete_irq);
}
```

#### Reading Data (Polled by User)
```cpp
int HardwareSerial::available(void)
{
    // Calculate bytes in ring buffer
    return ((unsigned int)(SERIAL_RX_BUFFER_SIZE + _serial.rx_head - _serial.rx_tail))
           % SERIAL_RX_BUFFER_SIZE;
}

int HardwareSerial::read(void)
{
    // Return -1 if buffer empty
    if (_serial.rx_head == _serial.rx_tail) {
        return -1;
    }

    // Read from tail (oldest byte)
    unsigned char c = _serial.rx_buff[_serial.rx_tail];
    _serial.rx_tail = (rx_buffer_index_t)(_serial.rx_tail + 1) % SERIAL_RX_BUFFER_SIZE;
    return c;
}
```

#### RX Interrupt Handler (Called from Layer 2)
```cpp
void HardwareSerial::_rx_complete_irq(serial_t *obj)
{
    unsigned char c;

    // Get byte from UART via Layer 2
    if (uart_getc(obj, &c) == 0) {
        // Calculate next head position
        rx_buffer_index_t i = (unsigned int)(obj->rx_head + 1) % SERIAL_RX_BUFFER_SIZE;

        // Store if not full (don't overwrite unread data)
        if (i != obj->rx_tail) {
            obj->rx_buff[obj->rx_head] = c;
            obj->rx_head = i;
        }
        // else: buffer overflow, byte discarded
    }
}
```

**Critical Observation**: The RX interrupt handler is **simple and fast**. It:
1. Retrieves the byte from hardware
2. Stores it in the ring buffer (if space available)
3. Updates the head pointer
4. **Does NOT process data or call user callbacks**

This means the interrupt latency is minimal (~1-2 µs typical).

---

## Layer 2: Core UART Wrapper

### Files
- `cores/arduino/stm32/uart.h`
- `cores/arduino/stm32/uart.c` (1198 lines)

### Key Data Structure

```cpp
struct serial_s {
    USART_TypeDef *uart;           // Hardware USART peripheral (USART1, USART2, etc.)
    UART_HandleTypeDef handle;     // ST HAL handle

    // Callback function pointers
    void (*rx_callback)(serial_t *);  // Called on RX complete
    int  (*tx_callback)(serial_t *);  // Called on TX complete

    // Pin configuration
    PinName pin_tx;
    PinName pin_rx;
    PinName pin_rts;  // Hardware flow control
    PinName pin_cts;

    // Interrupt configuration
    IRQn_Type irq;     // Interrupt number (USART1_IRQn, etc.)
    uint8_t index;     // UART instance index (0-based)

    // Single-byte receive buffer (used by HAL_UART_Receive_IT)
    uint8_t recv;

    // Ring buffer pointers (point to HardwareSerial buffers)
    uint8_t *rx_buff;
    uint8_t *tx_buff;

    // Ring buffer indices
    uint16_t rx_tail;
    uint16_t tx_head;
    volatile uint16_t rx_head;  // volatile: written by ISR
    volatile uint16_t tx_tail;  // volatile: written by ISR

    size_t tx_size;  // Current TX transfer size
};
```

### Initialization

```cpp
void uart_init(serial_t *obj, uint32_t baudrate, uint32_t databits,
               uint32_t parity, uint32_t stopbits)
{
    // 1. Determine peripheral from pin mapping
    USART_TypeDef *uart_tx = pinmap_peripheral(obj->pin_tx, PinMap_UART_TX);
    USART_TypeDef *uart_rx = pinmap_peripheral(obj->pin_rx, PinMap_UART_RX);
    obj->uart = pinmap_merge_peripheral(uart_tx, uart_rx);

    // 2. Enable clock for USART peripheral
    // Example for USART1:
    __HAL_RCC_USART1_FORCE_RESET();
    __HAL_RCC_USART1_RELEASE_RESET();
    __HAL_RCC_USART1_CLK_ENABLE();
    obj->index = UART1_INDEX;
    obj->irq = USART1_IRQn;

    // 3. Configure GPIO pins (AF mode)
    pinmap_pinout(obj->pin_tx, PinMap_UART_TX);
    pinmap_pinout(obj->pin_rx, PinMap_UART_RX);

    // 4. Configure UART via HAL
    UART_HandleTypeDef *huart = &(obj->handle);
    huart->Instance = (USART_TypeDef *)(obj->uart);
    huart->Init.BaudRate = baudrate;
    huart->Init.WordLength = databits;
    huart->Init.StopBits = stopbits;
    huart->Init.Parity = parity;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;

    // 5. Set NVIC priority
    HAL_NVIC_SetPriority(obj->irq, UART_IRQ_PRIO, UART_IRQ_SUBPRIO);

    // 6. Initialize HAL
    HAL_UART_Init(huart);
}
```

### Attach RX Callback (Start Interrupt-Driven Reception)

```cpp
void uart_attach_rx_callback(serial_t *obj, void (*callback)(serial_t *))
{
    // Store callback function pointer
    obj->rx_callback = callback;

    // Disable IRQ to prevent race condition
    HAL_NVIC_DisableIRQ(obj->irq);

    // Start interrupt-driven reception (1 byte at a time)
    HAL_UART_Receive_IT(uart_handlers[obj->index], &(obj->recv), 1);

    // Enable IRQ
    HAL_NVIC_EnableIRQ(obj->irq);
}
```

**Key Point**: `HAL_UART_Receive_IT(&obj->recv, 1)` tells the HAL to:
1. Enable the RXNE (RX Not Empty) interrupt
2. When a byte arrives, copy it to `obj->recv`
3. Call `HAL_UART_RxCpltCallback()` (which calls our `obj->rx_callback`)

### Get Received Byte

```cpp
int uart_getc(serial_t *obj, unsigned char *c)
{
    // Check if RX transaction is ongoing
    if (serial_rx_active(obj)) {
        return -1; // Busy
    }

    // Copy byte from single-byte buffer
    *c = (unsigned char)(obj->recv);

    // Restart RX interrupt for next byte
    HAL_UART_Receive_IT(uart_handlers[obj->index], &(obj->recv), 1);

    return 0;
}
```

**Critical Flow**:
1. HAL receives byte into `obj->recv` via interrupt
2. HAL calls `HAL_UART_RxCpltCallback()` → `obj->rx_callback()` (which is `HardwareSerial::_rx_complete_irq`)
3. `_rx_complete_irq()` calls `uart_getc()` to retrieve `obj->recv` and store in ring buffer
4. `uart_getc()` re-arms the interrupt with `HAL_UART_Receive_IT()` for the next byte

This creates a **perpetual interrupt loop** where each byte automatically triggers reception of the next byte.

### USART IRQ Handlers

```cpp
// Example for USART1
void USART1_IRQHandler(void)
{
    HAL_NVIC_ClearPendingIRQ(USART1_IRQn);
    HAL_UART_IRQHandler(uart_handlers[UART1_INDEX]);
}

// Similar handlers for USART2, USART3, UART4, UART5, USART6, etc.
```

Each USART peripheral has its own IRQ handler that:
1. Clears the pending interrupt flag
2. Calls the HAL IRQ handler with the appropriate `UART_HandleTypeDef`

---

## Layer 3: ST HAL Driver

### Files
- `system/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c` (for STM32F4)
- Similar for F7, H7, G4, L4, etc.

### HAL_UART_Receive_IT (Start Interrupt Reception)

```c
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
    // Store buffer pointer and size
    huart->pRxBuffPtr = pData;
    huart->RxXferSize = Size;
    huart->RxXferCount = Size;

    // Set state to busy
    huart->RxState = HAL_UART_STATE_BUSY_RX;

    // Enable RXNE interrupt (fires when data register has data)
    __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);

    // Enable error interrupts
    __HAL_UART_ENABLE_IT(huart, UART_IT_PE);   // Parity error
    __HAL_UART_ENABLE_IT(huart, UART_IT_ERR);  // Frame, noise, overrun errors

    return HAL_OK;
}
```

### HAL_UART_IRQHandler (Interrupt Dispatcher)

```c
void HAL_UART_IRQHandler(UART_HandleTypeDef *huart)
{
    uint32_t isrflags = READ_REG(huart->Instance->SR);  // Status Register
    uint32_t cr1its = READ_REG(huart->Instance->CR1);   // Control Register 1
    uint32_t errorflags;

    // Check for errors first
    errorflags = (isrflags & (UART_FLAG_PE | UART_FLAG_FE | UART_FLAG_ORE | UART_FLAG_NE));
    if (errorflags != 0) {
        // Handle errors...
        HAL_UART_ErrorCallback(huart);
        return;
    }

    // Check if RXNE (RX Not Empty) interrupt
    if (((isrflags & UART_FLAG_RXNE) != 0) && ((cr1its & UART_IT_RXNE) != 0)) {
        // Read data register
        *huart->pRxBuffPtr = (uint8_t)(huart->Instance->DR & 0x00FF);
        huart->pRxBuffPtr++;
        huart->RxXferCount--;

        // If all bytes received
        if (huart->RxXferCount == 0) {
            // Disable RXNE interrupt
            __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);

            // Set state to ready
            huart->RxState = HAL_UART_STATE_READY;

            // Call user callback
            HAL_UART_RxCpltCallback(huart);
        }
    }

    // Check for TXE (TX Empty) interrupt...
    // (similar logic for transmission)
}
```

### HAL Callbacks (Weak Functions - Overridden by Core)

```c
// Called when RX transfer completes
__weak void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // Default: do nothing
    // Overridden in uart.c to call obj->rx_callback()
}

// Called when TX transfer completes
__weak void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    // Default: do nothing
    // Overridden in uart.c to call obj->tx_callback()
}

// Called on UART errors
__weak void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    // Default: do nothing
    // Overridden in uart.c to clear errors and restart RX
}
```

The core overrides these weak functions in `uart.c`:

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    serial_t *obj = get_serial_obj(huart);
    if (obj) {
        obj->rx_callback(obj);  // Calls HardwareSerial::_rx_complete_irq
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    // Clear error flags
    __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_PEF);  // Parity
    __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_FEF);  // Frame
    __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_NEF);  // Noise
    __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF); // Overrun

    // Restart receive interrupt
    serial_t *obj = get_serial_obj(huart);
    if (obj && !serial_rx_active(obj)) {
        HAL_UART_Receive_IT(huart, &(obj->recv), 1);
    }
}
```

---

## Layer 4: Hardware Interrupts

### USART Peripheral Registers (STM32F411 Example)

```
USART1 Base Address: 0x40011000

Key Registers:
├── SR (Status Register) @ offset 0x00
│   ├── Bit 0: PE (Parity Error)
│   ├── Bit 1: FE (Framing Error)
│   ├── Bit 2: NF (Noise Flag)
│   ├── Bit 3: ORE (Overrun Error)
│   ├── Bit 4: IDLE (IDLE line detected)
│   ├── Bit 5: RXNE (Read Data Register Not Empty) ← RX interrupt source
│   ├── Bit 6: TC (Transmission Complete)
│   └── Bit 7: TXE (Transmit Data Register Empty) ← TX interrupt source
│
├── DR (Data Register) @ offset 0x04
│   └── Bits 0-8: Data value (read for RX, write for TX)
│
├── BRR (Baud Rate Register) @ offset 0x08
│   └── Divider for baud rate generation
│
└── CR1 (Control Register 1) @ offset 0x0C
    ├── Bit 5: RXNEIE (RXNE Interrupt Enable) ← Enable RX interrupt
    ├── Bit 6: TCIE (Transmission Complete Interrupt Enable)
    └── Bit 7: TXEIE (TXE Interrupt Enable) ← Enable TX interrupt
```

### Interrupt Flow

```
1. Byte arrives at USART RX pin
   └─> USART peripheral latches data into shift register

2. When shift register complete (8 bits received):
   └─> Data moved from shift register to DR (Data Register)
   └─> SR.RXNE flag set (RX Not Empty)

3. If CR1.RXNEIE is enabled (set by HAL_UART_Receive_IT):
   └─> NVIC signals USARTx_IRQn interrupt

4. CPU vectors to USARTx_IRQHandler()
   └─> HAL_UART_IRQHandler(uart_handlers[UARTx_INDEX])

5. HAL reads DR register (clears RXNE flag automatically)
   └─> Stores byte in huart->pRxBuffPtr (points to obj->recv)
   └─> Calls HAL_UART_RxCpltCallback(huart)

6. HAL_UART_RxCpltCallback() (overridden in uart.c)
   └─> Calls obj->rx_callback (HardwareSerial::_rx_complete_irq)

7. _rx_complete_irq():
   └─> Calls uart_getc() to retrieve byte from obj->recv
   └─> Stores byte in ring buffer (obj->rx_buff[obj->rx_head])
   └─> Increments rx_head
   └─> Calls HAL_UART_Receive_IT() to re-arm interrupt for next byte

8. Interrupt returns, CPU resumes main code
```

### NVIC Configuration

```cpp
// Set in uart_init()
HAL_NVIC_SetPriority(obj->irq, UART_IRQ_PRIO, UART_IRQ_SUBPRIO);

// Default priorities (can be overridden at compile time)
#ifndef UART_IRQ_PRIO
#define UART_IRQ_PRIO       1    // Group priority (0 = highest, 15 = lowest)
#endif
#ifndef UART_IRQ_SUBPRIO
#define UART_IRQ_SUBPRIO    0    // Subpriority within group
#endif
```

**STM32F4 NVIC Priority Grouping**: Configured in `system_stm32f4xx.c`:
```c
NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
// 4 bits for preemption priority (0-15), 0 bits for subpriority
```

This means:
- Priority 0 = highest (can preempt everything)
- Priority 1 = UART default (can preempt priorities 2-15)
- Priority 15 = lowest (can be preempted by everything)

---

## Data Flow: RX Path

### Complete RX Flow Diagram

```
User Code                HardwareSerial            uart.c                 HAL                  Hardware
─────────                ───────────────           ──────                 ───                  ────────

                                                                                               BYTE → RX Pin
                                                                                                  │
                                                                                                  ▼
                                                                                            [Shift Register]
                                                                                                  │
                                                                                                  ▼
                                                                                              DR Register
                                                                                                  │
                                                                                                  ▼
                                                                                              SR.RXNE = 1
                                                                                                  │
                                                                                                  ▼
                                                                                          USARTx_IRQn fires
                                                                                                  │
                                                                                                  ▼
                                                                             USARTx_IRQHandler()
                                                                                                  │
                                                                                                  ▼
                                                                             HAL_UART_IRQHandler()
                                                                             - Read DR (→ obj->recv)
                                                                             - RXNE auto-cleared
                                                                                                  │
                                                                                                  ▼
                                                                             HAL_UART_RxCpltCallback()
                                                                                                  │
                                                                                                  ▼
                                                obj->rx_callback()
                                                       │
                                                       ▼
                       _rx_complete_irq(obj)
                       - uart_getc(obj, &c)
                       - Store c in rx_buffer[rx_head]
                       - rx_head++
                       - HAL_UART_Receive_IT() ← Re-arm
                                ↑
                                │
Serial1.available()             │
  ↓                             │
if (rx_head != rx_tail)         │
  return count                  │
                                │
Serial1.read()                  │
  ↓                             │
c = rx_buffer[rx_tail]          │
rx_tail++                       │
return c ─────────────────────────> [User gets byte]
```

### Timing Analysis (STM32F411 @ 100 MHz)

**From hardware RX pin to ring buffer**:
1. Byte reception (8 bits @ 115200 baud): **~87 µs**
2. USART → DR register: **< 1 µs** (hardware)
3. Interrupt latency (NVIC): **~12-20 cycles** = **~0.2 µs** @100MHz
4. HAL_UART_IRQHandler execution: **~50 cycles** = **~0.5 µs**
5. _rx_complete_irq execution: **~100 cycles** = **~1 µs**
6. HAL_UART_Receive_IT re-arm: **~50 cycles** = **~0.5 µs**

**Total interrupt overhead per byte: ~2.2 µs**

**Buffer capacity**: With 64-byte buffer and 115200 baud (11520 bytes/sec):
- Buffer fills in: 64 bytes / 11520 B/s = **~5.6 ms**
- If user polls every 1ms, buffer utilization is ~11 bytes

---

## Data Flow: TX Path

### TX Flow (Simplified)

```
User Code              HardwareSerial           uart.c                HAL              Hardware
─────────              ───────────────          ──────                ───              ────────

Serial1.write(data)
    │
    ▼
write(buffer, size)
- Copy to tx_buffer[tx_head]
- tx_head += size
    │
    ▼
uart_attach_tx_callback()
- Disable IRQ
- HAL_UART_Transmit_IT(tx_buffer[tx_tail], size)
- Enable IRQ
                                                      │
                                                      ▼
                                          HAL_UART_Transmit_IT()
                                          - Enable TXE interrupt
                                                      │
                                                      ▼
                                          TXE interrupt fires
                                          (when DR empty)
                                                      │
                                                      ▼
                                          HAL_UART_IRQHandler()
                                          - Write next byte to DR
                                          - If done:
                                            HAL_UART_TxCpltCallback()
                                                      │
                                                      ▼
                         _tx_complete_irq(obj)
                         - tx_tail += tx_size
                         - If more data:
                           uart_attach_tx_callback()
```

**Key Difference from RX**:
- TX uses **burst mode**: Sends multiple bytes per interrupt attachment
- TX callback handles buffer advancement automatically
- User doesn't poll TX (fire-and-forget)

---

## Buffer Management

### Ring Buffer Mechanics

```
RX Buffer State Example (64 bytes):

Index:  0  1  2  3  4  5  6  7  8  9 ... 62 63
       [A][B][C][D][E][ ][ ][ ][ ][ ]...[ ][ ]
        ↑           ↑
      tail        head

available() = (64 + head - tail) % 64 = (64 + 5 - 0) % 64 = 5 bytes

read() returns A, tail advances to 1:
       [ ][B][C][D][E][ ][ ][ ][ ][ ]...[ ][ ]
           ↑        ↑
         tail     head
```

### Buffer Overflow Handling

```cpp
void HardwareSerial::_rx_complete_irq(serial_t *obj)
{
    unsigned char c;
    if (uart_getc(obj, &c) == 0) {
        rx_buffer_index_t i = (obj->rx_head + 1) % SERIAL_RX_BUFFER_SIZE;

        // Check if buffer full
        if (i != obj->rx_tail) {
            obj->rx_buff[obj->rx_head] = c;
            obj->rx_head = i;
        }
        // else: OVERFLOW - byte is DISCARDED (no error flag set!)
    }
}
```

**Critical Issue**: **Silent data loss** on overflow!
- No error flag or counter
- User has no way to detect dropped bytes
- **Solution for RC receiver**: Must poll fast enough to prevent overflow

---

## Interrupt Priority & Timing

### Priority Considerations

**Default UART Priority**: 1 (high, but not highest)
```cpp
#define UART_IRQ_PRIO       1
#define UART_IRQ_SUBPRIO    0
```

**Other Peripheral Priorities** (for reference):
- SysTick: Usually 15 (lowest) - for millis()
- TIMx (PWM): Varies, often 1-5
- SPI: Often 1-3
- I2C: Often 1-3

### Interrupt Contention Scenarios

**Scenario 1: UART + TIM3 PWM**
- If TIM3 priority = 2 (lower than UART)
- UART interrupt can preempt TIM3 ISR
- TIM3 cannot preempt UART ISR

**Scenario 2: UART + SPI (same priority = 1)**
- First ISR to execute runs to completion
- Second ISR waits (tail-chaining)
- Execution order depends on which flag was set first

**Scenario 3: High-frequency interrupts**
- If TIM interrupt fires every 1ms (1kHz)
- And UART receives at 115200 baud (~86µs per byte)
- Multiple UART ISRs can execute between TIM ISRs

### Interrupt Latency Budget

**For 115200 baud** (worst case):
- Byte time: 86.8 µs (1 start + 8 data + 1 stop bit @ 115200 baud)
- UART ISR execution: ~2.2 µs
- **Available time before next byte**: 86.8 - 2.2 = **84.6 µs**

This means user code can run for 84.6 µs between UART interrupts without missing data (assuming buffer doesn't overflow).

---

## Critical Findings for RC Receiver Implementation

### 1. **Interrupt-Driven RX is Already Implemented**

✅ **Good News**: Don't need to implement DMA or custom interrupt handling
- Arduino Core already provides interrupt-driven, byte-by-byte RX
- Ring buffer automatically accumulates data
- User code just polls `Serial.available()` and `Serial.read()`

### 2. **Polling Strategy**

**Recommended approach**:
```cpp
void loop() {
    rc_receiver.update();  // Poll Serial.available() and process bytes
    // Other tasks...
}
```

**Why polling works**:
- Interrupt handler fills ring buffer automatically
- 64-byte buffer provides ~5.6ms cushion @ 115200 baud
- Typical loop() iteration: 1-2 ms
- No risk of buffer overflow if loop() runs at reasonable rate

### 3. **No DMA Needed**

**UVOS used DMA** because:
- Custom HAL required explicit buffer management
- DMA reduces CPU load for high-speed transfers

**Arduino doesn't need DMA** because:
- Interrupt-driven reception is efficient (~2.2 µs per byte)
- Ring buffer provides automatic buffering
- CPU load is minimal (0.002ms / 0.087ms = **2.5% per byte**)

### 4. **Buffer Size is Adequate**

**For IBus @ 115200 baud**:
- Frame size: 32 bytes (14 channels)
- Frame time: 32 bytes × 86.8 µs/byte = **2.78 ms**
- Buffer capacity: 64 bytes = **2 frames**
- Polling every 1ms → buffer utilization ~ 11 bytes (17%)

**Conclusion**: No need to increase `SERIAL_RX_BUFFER_SIZE`

### 5. **Parser Integration**

**Simple integration**:
```cpp
void SerialRCReceiver::update()
{
    while (serial_->available()) {
        uint8_t byte = serial_->read();
        if (parser_->ParseByte(byte)) {
            // Message complete, enqueued in parser's FIFO
            last_message_time_ = millis();
        }
    }
}
```

**Why this works**:
- `available()` returns immediately (reads `rx_head - rx_tail`)
- `read()` returns immediately (reads from buffer, no blocking)
- `ParseByte()` is pure computation (no I/O)
- Total time: ~5-10 µs per byte

### 6. **Error Handling**

**Current HAL behavior**:
- Parity, framing, noise, overrun errors are caught by `HAL_UART_ErrorCallback()`
- Errors are cleared and RX is restarted
- **BUT**: No error counter or flag exposed to user

**Recommendation for RC receiver**:
- Implement timeout detection (already planned)
- Consider adding checksum validation (IBus has checksum)
- Don't rely on UART error flags (they're cleared silently)

### 7. **Interrupt Latency is Not a Concern**

**Analysis**:
- UART ISR: ~2.2 µs (very fast)
- Byte arrival: every 86.8 µs @ 115200 baud
- ISR overhead: 2.5% of byte time
- **Conclusion**: Interrupt latency will not cause data loss

### 8. **Multi-Instance Support**

**Arduino provides multiple Serial instances**:
```cpp
SerialRCReceiver rc1(SerialRCReceiver::IBUS);
SerialRCReceiver rc2(SerialRCReceiver::SBUS);

void setup() {
    rc1.begin(&Serial1, 115200);  // USART1
    rc2.begin(&Serial2, 100000);  // USART2
}
```

Each USART has independent:
- Ring buffers (64 bytes each)
- Interrupt handlers
- HAL state machines

**No conflict** between multiple instances.

### 9. **Direct HAL Access (Optional)**

**If needed**, can access HAL directly:
```cpp
HardwareSerial Serial1(USART1);
UART_HandleTypeDef* hal_handle = Serial1.getHandle();

// Example: Check error flags
uint32_t errors = hal_handle->ErrorCode;
if (errors & HAL_UART_ERROR_PE) { /* Parity error */ }
if (errors & HAL_UART_ERROR_FE) { /* Frame error */ }
if (errors & HAL_UART_ERROR_ORE) { /* Overrun error */ }
```

### 10. **Performance Comparison: Polling vs. DMA**

| Metric                  | Polling (Arduino) | DMA (UVOS)      |
|-------------------------|-------------------|-----------------|
| CPU per byte            | ~2.2 µs           | 0 (DMA handles) |
| Buffer management       | Automatic         | Manual          |
| Implementation complexity| Low (uses core)   | High (custom)   |
| Multi-instance support  | Easy (Serial1-N)  | Complex         |
| Error handling          | Built-in          | Manual          |
| **Recommended?**        | ✅ **YES**         | ❌ Overkill     |

---

## Recommendations for SerialRCReceiver Implementation

### Architecture

```cpp
class SerialRCReceiver {
public:
    enum Protocol { NONE, IBUS, SBUS };

    struct Config {
        HardwareSerial* serial;  // &Serial1, &Serial2, etc.
        uint32_t baudrate;
        uint32_t timeout_ms;
    };

    void begin(const Config& config) {
        serial_ = config.serial;
        serial_->begin(config.baudrate);
        // Parser already created in constructor
    }

    void update() {
        // Called in loop(), polls Serial
        while (serial_->available()) {
            uint8_t byte = serial_->read();
            if (parser_->ParseByte(byte)) {
                last_message_time_ = millis();
            }
        }
    }

    bool available() const {
        return parser_->Listener();  // Check FIFO queue
    }

    bool getMessage(RCMessage* msg) {
        return parser_->GetMessageFromFIFO(msg);
    }

    bool timeout(uint32_t threshold_ms) const {
        return (millis() - last_message_time_) > threshold_ms;
    }

private:
    HardwareSerial* serial_;
    ProtocolParser* parser_;
    uint32_t last_message_time_;
};
```

### Key Design Decisions

1. **Use Arduino HardwareSerial** (don't bypass it)
   - Leverages existing, tested interrupt-driven RX
   - No custom HAL code needed

2. **Poll in loop()** (don't use custom interrupts)
   - `update()` drains ring buffer every loop iteration
   - Fast enough for RC protocols

3. **Single-byte parsing** (not batch)
   - `ParseByte()` called for each byte
   - Parser state machine handles framing

4. **Message queue in parser** (not transport)
   - Parser's FIFO stores complete messages
   - Transport just feeds bytes to parser

5. **No buffer size changes needed**
   - 64-byte default is adequate
   - Prevents unnecessary core modifications

---

## Appendix: Register-Level Example (STM32F411 USART1)

### Register Addresses
```
USART1 Base: 0x40011000

SR   @ 0x40011000  Status Register
DR   @ 0x40011004  Data Register
BRR  @ 0x40011008  Baud Rate Register
CR1  @ 0x4001100C  Control Register 1
CR2  @ 0x40011010  Control Register 2
CR3  @ 0x40011014  Control Register 3
```

### Initialization Sequence (Pseudo-Register Operations)

```c
// 1. Enable USART1 clock
RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

// 2. Configure baud rate (115200 @ 100 MHz APB2)
// BRR = f_CK / (16 * baud) = 100,000,000 / (16 * 115200) = 54.25 ≈ 0x0364
USART1->BRR = 0x0364;

// 3. Enable USART, TX, RX
USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;

// 4. Enable RXNE interrupt
USART1->CR1 |= USART_CR1_RXNEIE;

// 5. Enable USART1 interrupt in NVIC
NVIC_SetPriority(USART1_IRQn, UART_IRQ_PRIO);
NVIC_EnableIRQ(USART1_IRQn);
```

### Interrupt Service Routine (Conceptual)

```c
void USART1_IRQHandler(void)
{
    uint32_t sr = USART1->SR;

    // Check if RXNE flag set (data available)
    if (sr & USART_SR_RXNE) {
        uint8_t data = USART1->DR & 0xFF;  // Read DR (clears RXNE)

        // Store in ring buffer
        rx_buffer[rx_head] = data;
        rx_head = (rx_head + 1) % BUFFER_SIZE;
    }

    // Check for errors
    if (sr & USART_SR_ORE) {
        volatile uint32_t dummy = USART1->DR;  // Clear ORE by reading DR
    }
}
```

---

## Serial Instance System and Pin Mapping

### Overview

The STM32 Arduino Core provides two ways to create `HardwareSerial` instances:

1. **Pre-defined instances** (`Serial1`, `Serial2`, etc.) - Global instances with variant-specific pin assignments
2. **Custom instances** - User-created instances with explicit pin specification

Understanding the difference is critical for reliable peripheral configuration.

---

### Pre-defined Serial Instances (Serial1, Serial2, etc.)

#### How They Work

Pre-defined instances are created through a multi-stage configuration system:

**Stage 1: Variant Configuration** (`variant_BOARD.h`)
```cpp
// Example: variant_NUCLEO_F411RE.h
#define SERIAL_UART_INSTANCE  2  // Maps 'Serial' to USART2 (ST-Link)
#define PIN_SERIAL_RX         PA3
#define PIN_SERIAL_TX         PA2
```

**Stage 2: Enable Logic** (`WSerial.h`)
```cpp
// If SERIAL_UART_INSTANCE == 2
#define ENABLE_HWSERIAL2
#define Serial Serial2  // Map 'Serial' alias to 'Serial2'

// Then check hardware availability
#if defined(USART2_BASE)
  #define HAVE_HWSERIAL2  // Enable instantiation
#endif
```

**Stage 3: Instance Creation** (`HardwareSerial.cpp`)
```cpp
#if defined(HAVE_HWSERIAL1)
  HardwareSerial Serial1(USART1);  // Global instance
#endif
#if defined(HAVE_HWSERIAL2)
  HardwareSerial Serial2(USART2);  // Global instance
#endif
```

**Stage 4: Pin Assignment** (`HardwareSerial::HardwareSerial(void *peripheral)`)

Pins are assigned via nested if-else chain (lines 144-285):

```cpp
#if defined(PIN_SERIAL1_TX) && defined(USART1_BASE)
  if (peripheral == USART1) {
    setRx(PIN_SERIAL1_RX);  // If defined in variant
    setTx(PIN_SERIAL1_TX);
  } else
#endif
{
  // FALLBACK: Auto-detect first available pins from PeripheralPins.c
  _serial.pin_rx = pinmap_pin(peripheral, PinMap_UART_RX);
  _serial.pin_tx = pinmap_pin(peripheral, PinMap_UART_TX);
}
```

#### The "Opaque Pins" Problem

**Question**: What pins does `Serial1.begin(115200)` use on NUCLEO_F411RE?

**Answer** (requires detective work):
1. Check `variant_NUCLEO_F411RE.h` for `PIN_SERIAL1_RX`/`PIN_SERIAL1_TX` → **Not defined**
2. Fallback to `pinmap_pin(USART1, PinMap_UART_RX)` in `PeripheralPins.c`
3. Returns **first occurrence** in `PinMap_UART_RX[]` array for USART1
4. For F411: **Likely PA10/PA9, but not guaranteed**

**Result**: Pin assignment is **opaque** - not visible in user code.

#### Example: NUCLEO_F411RE Serial Instances

| Instance | USART | Default Pins (best guess) | Source |
|----------|-------|---------------------------|--------|
| `Serial` | USART2 | PA3 (RX), PA2 (TX) | variant: `SERIAL_UART_INSTANCE=2` |
| `Serial1` | USART1 | PA10 (RX), PA9 (TX) | PeripheralPins.c (first match) |
| `Serial2` | USART2 | PA3 (RX), PA2 (TX) | Same as Serial |
| `Serial6` | USART6 | PA12 (RX), PA11 (TX) | PeripheralPins.c (first match) |

⚠️ **Warning**: Only `Serial` has guaranteed pins via `PIN_SERIAL_RX/TX`. Others depend on `PeripheralPins.c` ordering.

---

### Custom HardwareSerial Instances

#### Pin-Based Constructor

```cpp
HardwareSerial SerialRC(PA10, PA9);  // RX=PA10, TX=PA9
```

**How it works** (`HardwareSerial.cpp`):
```cpp
HardwareSerial::HardwareSerial(Pin _rx, Pin _tx, Pin _rts, Pin _cts)
{
  init(_rx.toPinName(), _tx.toPinName(),
       _rts.toPinName(), _cts.toPinName());
}
```

**Peripheral auto-detection** (`uart.c: uart_init()`):
```cpp
// 1. Look up which USART peripheral these pins map to
USART_TypeDef *uart_tx = pinmap_peripheral(obj->pin_tx, PinMap_UART_TX);
USART_TypeDef *uart_rx = pinmap_peripheral(obj->pin_rx, PinMap_UART_RX);
obj->uart = pinmap_merge_peripheral(uart_tx, uart_rx);

// 2. If PA10/PA9 provided, result is USART1
// 3. Configure and enable that peripheral
```

**Key Feature**: The core **automatically determines** which USART peripheral matches the specified pins via `PeripheralPins.c` lookup.

---

### Comparison: Pre-defined vs Custom

| Aspect | Pre-defined (`Serial1`) | Custom (`HardwareSerial(PA10, PA9)`) |
|--------|-------------------------|--------------------------------------|
| **Peripheral** | Hardcoded (e.g., USART1) | Auto-detected from pins |
| **Pins** | Opaque (variant or PeripheralPins.c) | Explicit in code |
| **Clarity** | Hidden pin assignment | Self-documenting |
| **Memory** | Global (always allocated if enabled) | Allocated when declared |
| **Flexibility** | Limited to predefined | Any valid pin combination |
| **Portability** | Variant-dependent | Works on any board with those pins |

---

### Design Pattern Consistency

Custom instances match the project's peripheral configuration pattern:

**SPI Example** (ICM42688P, SDFS):
```cpp
SPIClass spi(MOSI_PIN, MISO_PIN, SCLK_PIN, CS_PIN);  // Explicit pins
imu.begin(spi, CS_PIN, 1000000);
```

**UART Example** (SerialRx):
```cpp
HardwareSerial serial(RX_PIN, TX_PIN);  // Explicit pins
rc.begin(&serial, 115200);
```

**Why this pattern**:
- ✅ **Self-documenting**: Hardware wiring visible in code
- ✅ **Portable**: Works across board variants
- ✅ **Explicit control**: No hidden pin assignments
- ✅ **Matches SPIClass pattern**: Consistent API design

---

### Recommendation for Peripheral Libraries

**Use custom `HardwareSerial` instances with explicit pins**:

```cpp
// Recommended: Explicit pin specification
HardwareSerial SerialRC(PA10, PA9);  // RX=PA10, TX=PA9

void setup() {
  SerialRC.begin(115200);
  // Wiring is clear from code
}
```

**Avoid relying on pre-defined instances** (unless `Serial` for USB debug):
```cpp
// Not recommended: Opaque pin assignment
Serial1.begin(115200);  // Which pins? Need to check variant + PeripheralPins.c
```

**Exception**: `Serial` for debug output is acceptable, as it's explicitly configured in each variant to match the board's USB/ST-Link connection.

---

### BoardConfig Integration (Optional Enhancement)

For maximum clarity with multi-board support:

```cpp
// In targets/BOARD_NAME.h
namespace BoardConfig {
  namespace RC {
    static constexpr Pin rx_pin = PA10;
    static constexpr Pin tx_pin = PA9;
    static constexpr uint32_t baudrate = 115200;
  }
}

// In sketch
HardwareSerial SerialRC(BoardConfig::RC::rx_pin, BoardConfig::RC::tx_pin);
```

This combines explicit pin control with board-specific configuration.

---

## Enabling Pre-defined Serial Instances (Serial1, Serial2, etc.)

### Overview

By default, only the `Serial` instance (mapped via `SERIAL_UART_INSTANCE`) is enabled. Additional Serial instances (Serial1, Serial2, etc.) must be explicitly enabled even though the hardware peripherals (USART1, USART2, etc.) exist on the chip.

### Why Are They Disabled by Default?

**Memory Conservation**: Each enabled Serial instance allocates:
- 64-byte RX buffer
- 64-byte TX buffer
- Instance data structure
- **Total**: ~150 bytes per instance

For boards with limited RAM, enabling all instances would waste memory.

### Enabling Mechanisms (Tested on NUCLEO_F411RE)

Three methods to enable additional Serial instances, verified with clean cache builds:

---

#### **Method 1: build_opt.h** (Per-Sketch, No Core Modification)

**How It Works**:
1. Create `build_opt.h` file alongside your sketch `.ino` file
2. Add one `-D` define per line
3. Arduino CLI includes it during compilation via `@build_opt.h` response file

**Example** (`build_opt.h`):
```
-DENABLE_HWSERIAL1
```

**Test Results** (Clean Cache):
```bash
# Baseline: Serial1 NOT available
rm -rf ~/.cache/arduino
arduino-cli compile --fqbn ... sketch/
# Result: undefined reference to `Serial1`

# With build_opt.h containing -DENABLE_HWSERIAL1
rm -rf ~/.cache/arduino
arduino-cli compile --fqbn ... sketch/
# Result: ✅ Compiles successfully, Serial1 present in ELF (9016 bytes)
```

**Pins Used**: First occurrence in `PeripheralPins.c` for that peripheral
- **USART1**: PA10 (RX), PA9 (TX)
- **USART2**: PA3 (RX), PA2 (TX)
- **USART6**: PA12 (RX), PA11 (TX)

**Pros**:
- ✅ No core/variant modification required
- ✅ Per-sketch configuration
- ✅ Documented STM32duino feature
- ✅ Works for multiple instances (add multiple `-DENABLE_HWSERIALx` lines)

**Cons**:
- ⚠️ **Cache-sensitive**: Must delete `~/.cache/arduino` for reliable results
- ⚠️ **Opaque pins**: Pin assignment not visible in code
- ⚠️ **Per-sketch overhead**: Must add `build_opt.h` to each sketch needing Serial instances

**When to Use**:
- Testing different Serial configurations
- Sketches that need specific Serial instance (Serial1, Serial6, etc.)
- When core modification is not desired

---

#### **Method 2: Variant Modification** (Board-Wide, Permanent)

**How It Works**:
Edit `variant_BOARD.h` to enable instance and optionally override default pins.

**Example** (`variant_NUCLEO_F411RE.h`):
```cpp
// Add after PIN_SERIAL_TX definition (around line 131)

// Enable Serial1 instance with explicit pin assignment
#ifndef ENABLE_HWSERIAL1
  #define ENABLE_HWSERIAL1
#endif
#ifndef PIN_SERIAL1_RX
  #define PIN_SERIAL1_RX        PA10
#endif
#ifndef PIN_SERIAL1_TX
  #define PIN_SERIAL1_TX        PA9
#endif

// Enable Serial6 instance
#ifndef ENABLE_HWSERIAL6
  #define ENABLE_HWSERIAL6
#endif
#ifndef PIN_SERIAL6_RX
  #define PIN_SERIAL6_RX        PA12
#endif
#ifndef PIN_SERIAL6_TX
  #define PIN_SERIAL6_TX        PA11
#endif
```

**Test Results** (Clean Cache):
```bash
# With ENABLE_HWSERIAL1 only (no PIN defines)
rm -rf ~/.cache/arduino
arduino-cli compile --fqbn ... sketch/
# Result: ✅ Compiles, uses PeripheralPins.c defaults (PA10/PA9)

# With ENABLE_HWSERIAL1 + PIN_SERIAL1_RX/TX
rm -rf ~/.cache/arduino
arduino-cli compile --fqbn ... sketch/
# Result: ✅ Compiles, uses specified pins (PA10/PA9 in this case)

# With PIN_SERIAL1_RX/TX only (no ENABLE)
rm -rf ~/.cache/arduino
arduino-cli compile --fqbn ... sketch/
# Result: ❌ FAILS - undefined reference to `Serial1`
```

**Key Finding**: Both `ENABLE_HWSERIAL1` (instance creation) and `PIN_SERIAL1_RX/TX` (pin override) serve different purposes:
- `ENABLE_HWSERIALx` → Creates the Serial instance
- `PIN_SERIALx_RX/TX` → Overrides default pins (optional)

**Pros**:
- ✅ Available for all sketches on this board
- ✅ Explicit pin assignment visible in variant file
- ✅ No per-sketch configuration needed
- ✅ Survives cache clears

**Cons**:
- ⚠️ Modifies local core fork
- ⚠️ Increases memory footprint for ALL sketches (even if unused)
- ⚠️ Requires maintenance across core updates

**When to Use**:
- Board variant needs specific Serial instances enabled by default
- Custom board definitions
- When all sketches for a board use certain Serial instances

---

#### **Method 3: Custom HardwareSerial Instance** (Recommended for Libraries)

**How It Works**:
Create a `HardwareSerial` object with explicit pin specification. The core automatically detects which USART peripheral those pins map to.

**Example**:
```cpp
HardwareSerial SerialRC(PA10, PA9);  // RX=PA10, TX=PA9

void setup() {
  SerialRC.begin(115200);
  SerialRC.println("Explicit pins!");
}
```

**Peripheral Auto-Detection**:
```cpp
// Core determines: PA10+PA9 → USART1
// via pinmap_peripheral() lookup in PeripheralPins.c
```

**Pros**:
- ✅ **No core/variant modification**
- ✅ **No build_opt.h needed**
- ✅ **No cache issues**
- ✅ **Self-documenting** - pins visible in code
- ✅ **Portable** - works on any board variant
- ✅ **Works reliably every time**
- ✅ **Matches SPIClass pattern** - consistent with project

**Cons**:
- None for peripheral library use cases

**When to Use**:
- **Peripheral libraries** (RC receivers, GPS, LoRa, etc.)
- Multi-board support required
- Explicit hardware control desired
- **Recommended default approach**

---

### Comparison Matrix

| Aspect | build_opt.h | Variant Modification | Custom Instance |
|--------|-------------|---------------------|-----------------|
| **Core Modification** | No | Yes | No |
| **Cache Sensitivity** | High ⚠️ | None | None |
| **Pin Clarity** | Opaque | Explicit | Explicit |
| **Per-Sketch Config** | Yes | No | Yes |
| **Memory Impact** | When used | Always | When used |
| **Reliability** | Moderate | High | High |
| **Portability** | Moderate | Low | High |
| **Project Pattern** | ❌ | ❌ | ✅ |

---

### Verification Procedure

When testing Serial instance enabling, **always clear cache**:

```bash
# Before each test
rm -rf ~/.cache/arduino

# Compile
arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE sketch/

# Verify Serial1 symbol exists (if expecting success)
~/.arduino15/packages/STMicroelectronics/tools/xpack-arm-none-eabi-gcc/12.2.1-1.2/bin/arm-none-eabi-nm \
  ~/.cache/arduino/sketches/*/sketch.ino.elf | grep " Serial1$"
```

**Expected Output** (if enabled):
```
20000148 B Serial1
```

---

### PeripheralPins.c Pin Mapping Reference (NUCLEO_F411RE)

When using pre-defined instances (Serial1, Serial2, etc.) without explicit `PIN_SERIALx_RX/TX` defines, pins are determined by **first occurrence** in `PeripheralPins.c`:

**USART1** (Serial1):
- **RX Options**: PA10, PB3, PB7, PA12
- **TX Options**: PA9, PA15, PB6, PA10, PB3, PB7
- **Default**: PA10 (RX), PA9 (TX)

**USART2** (Serial2 / Serial):
- **RX Options**: PA3, PA15, PB4, PA12
- **TX Options**: PA2, PA9, PB3, PB6, PA12, PA15, PB4
- **Default**: PA3 (RX), PA2 (TX)

**USART6** (Serial6):
- **RX Options**: PA12, PC7
- **TX Options**: PA11, PC6
- **Default**: PA12 (RX), PA11 (TX)

**Note**: These are the available pins from `PeripheralPins.c`. The "Default" is the **first entry** in each array, used when `PIN_SERIALx_RX/TX` are not defined in the variant.

---

### Real-World Example: SerialRx Library

The SerialRx library (for RC receiver protocols like IBus) uses **Method 3** (custom instance):

```cpp
// libraries/SerialRx/examples/IBus_Basic/IBus_Basic.ino
HardwareSerial SerialRC(PA10, PA9);  // Explicit USART1 pins

SerialRx rc;

void setup() {
  SerialRx::Config config;
  config.serial = &SerialRC;
  config.protocol = SerialRx::IBUS;
  config.baudrate = 115200;

  rc.begin(config);
}

void loop() {
  rc.update();  // Polls SerialRC.available()
  // ...
}
```

**Why This Approach**:
1. No `build_opt.h` needed - works out of box
2. No variant modification - portable across boards
3. Pins are self-documenting in example code
4. No cache issues - compiles reliably
5. Matches project's SPIClass usage pattern

---

## Conclusion

The STM32 Arduino Core provides a **robust, interrupt-driven serial implementation** that is well-suited for RC receiver protocols:

✅ **Pros**:
- Interrupt-driven RX with automatic ring buffering
- Low CPU overhead (~2.5% @ 115200 baud)
- Multi-instance support (Serial1-10)
- Flexible pin-based constructors
- Automatic peripheral detection from pins
- No custom HAL code needed
- Tested and stable

❌ **Cons**:
- Silent buffer overflow (no error counter)
- Fixed buffer size (64 bytes default)
- Pre-defined instance pins can be opaque
- No built-in DMA support (but not needed for RC protocols)

**Recommendations**:
1. **Use custom `HardwareSerial` instances** with explicit pins for peripheral libraries
2. **Use polling-based `update()` method** for RC receiver protocols
3. **Reserve pre-defined instances** (`Serial`, `Serial1`, etc.) for debug output or simple use cases
4. **Document pin assignments** in code comments or BoardConfig

This provides the simplest, most reliable, and most explicit implementation for RC receiver and other peripheral libraries.

