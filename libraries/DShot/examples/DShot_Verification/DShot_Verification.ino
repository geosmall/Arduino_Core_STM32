/**
 * DShot_Verification - DMA-based input capture loopback test
 *
 * Verifies DShot output timing using a jumper wire from an output pin
 * to an input capture pin on a different timer.
 *
 * Wiring (Nucleo F411RE or Nucleo G474RE):
 *   PB4 (TIM3_CH1, DShot output) --> PA0 (TIM2_CH1, input capture)
 *
 * The test:
 * 1. Sends a known DShot600 packet on PB4 via DMA
 * 2. Captures all edges on PA0 via DMA-driven input capture
 * 3. Reconstructs the packet from captured timestamps
 * 4. Verifies it matches the expected encoding
 *
 * Capture completion is detected by polling the DMA TC flag (no ISR
 * needed — the DShot library uses cleanup-at-start-of-Send() and
 * never enables DMA TC interrupts).
 */

#include <DShot.h>
#include "stm32yyxx_ll.h"

// --- Output: DShot on PB4 (TIM3_CH1) ---
DShotOutput dshot;

// --- Input Capture: TIM2_CH1 on PA0 ---
// 16 bits × 2 edges/bit = 32 edges + margin
static constexpr int CAPTURE_BUF_SIZE = 40;
static volatile uint32_t capture_buf[CAPTURE_BUF_SIZE];

static void initInputCapture(void);
static void armCapture(void);
static bool pollCaptureComplete(void);
static bool verifyPacket(uint16_t expected_throttle, bool expected_telemetry);

void setup()
{
  Serial.begin(115200);
  while (!Serial) { }
  Serial.println("DShot_Verification: DMA loopback test");

  // Configure DShot output (single motor)
  dshot.AddMotor(TIM3, PB4, 1, DShot::DSHOT600);

  // Configure input capture on TIM2_CH1 (PA0)
  initInputCapture();

  Serial.println("Setup complete. Running verification...");
  Serial.println();

  // Test several known packets
  static const uint16_t test_throttles[] = {0, 1, 48, 1000, 2047};
  int pass_count = 0;
  int fail_count = 0;

  for (size_t t = 0; t < sizeof(test_throttles) / sizeof(test_throttles[0]); t++) {
    uint16_t throttle = test_throttles[t];

    // Arm input capture DMA
    armCapture();

    // Send the DShot packet
    dshot.SetThrottle(0, throttle, false);
    dshot.Send();

    // Poll for capture DMA completion
    uint32_t deadline = millis() + 100;
    bool captured = false;
    while (millis() < deadline) {
      if (pollCaptureComplete()) {
        captured = true;
        break;
      }
    }

    // Wait for DShot output DMA to finish too
    while (!dshot.IsTransferComplete() && millis() < deadline) { }

    if (!captured) {
      Serial.print("FAIL: Capture timeout for throttle=");
      Serial.println(throttle);
      fail_count++;
      continue;
    }

    // Brief gap between test packets
    delayMicroseconds(100);

    if (verifyPacket(throttle, false)) {
      Serial.print("PASS: throttle=");
      Serial.println(throttle);
      pass_count++;
    } else {
      Serial.print("FAIL: throttle=");
      Serial.println(throttle);
      fail_count++;
    }
  }

  Serial.println();
  Serial.print("Results: ");
  Serial.print(pass_count);
  Serial.print(" passed, ");
  Serial.print(fail_count);
  Serial.println(" failed");
}

void loop()
{
  delay(1000);
}

// ---------------------------------------------------------------------------
// Input Capture Setup — TIM2_CH1 on PA0, DMA-driven, polled completion
// ---------------------------------------------------------------------------
static void initInputCapture(void)
{
  // GPIO: PA0 as TIM2_CH1 alternate function
  __HAL_RCC_GPIOA_CLK_ENABLE();

  LL_GPIO_InitTypeDef gpio_init;
  LL_GPIO_StructInit(&gpio_init);
  gpio_init.Pin = LL_GPIO_PIN_0;
  gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
  gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  gpio_init.Pull = LL_GPIO_PULL_DOWN;
  gpio_init.Alternate = LL_GPIO_AF_1;  // AF1 = TIM2 on F4/F7/G4/H7
  LL_GPIO_Init(GPIOA, &gpio_init);

  // Timer: TIM2 at full clock, 32-bit counter, input capture both edges
  __HAL_RCC_TIM2_CLK_ENABLE();
  LL_TIM_DisableCounter(TIM2);

  LL_TIM_InitTypeDef tim_init;
  LL_TIM_StructInit(&tim_init);
  tim_init.Prescaler = 0;
  tim_init.Autoreload = 0xFFFFFFFF;  // TIM2 is 32-bit
  tim_init.CounterMode = LL_TIM_COUNTERMODE_UP;
  LL_TIM_Init(TIM2, &tim_init);

  LL_TIM_IC_InitTypeDef ic_init;
  LL_TIM_IC_StructInit(&ic_init);
  ic_init.ICActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;
  ic_init.ICPrescaler = LL_TIM_ICPSC_DIV1;
  ic_init.ICFilter = LL_TIM_IC_FILTER_FDIV1;
  ic_init.ICPolarity = LL_TIM_IC_POLARITY_BOTHEDGE;
  LL_TIM_IC_Init(TIM2, LL_TIM_CHANNEL_CH1, &ic_init);

  LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
  LL_TIM_EnableCounter(TIM2);

  // DMA: capture CCR1 values into buffer
  __HAL_RCC_DMA1_CLK_ENABLE();

#if defined(STM32F4xx) || defined(STM32F7xx)
  // TIM2_CH1 → DMA1_Stream5, Channel 3 (F4/F7 reference manual)
  LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_5);
  LL_DMA_DeInit(DMA1, LL_DMA_STREAM_5);

  LL_DMA_InitTypeDef dma_init;
  LL_DMA_StructInit(&dma_init);
  dma_init.Channel = LL_DMA_CHANNEL_3;
  dma_init.PeriphOrM2MSrcAddress = (uint32_t)&TIM2->CCR1;
  dma_init.MemoryOrM2MDstAddress = (uint32_t)capture_buf;
  dma_init.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  dma_init.NbData = CAPTURE_BUF_SIZE;
  dma_init.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  dma_init.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dma_init.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
  dma_init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
  dma_init.Mode = LL_DMA_MODE_NORMAL;
  dma_init.Priority = LL_DMA_PRIORITY_HIGH;
  dma_init.FIFOMode = LL_DMA_FIFOMODE_DISABLE;
  dma_init.MemBurst = LL_DMA_MBURST_SINGLE;
  dma_init.PeriphBurst = LL_DMA_PBURST_SINGLE;

  LL_DMA_Init(DMA1, LL_DMA_STREAM_5, &dma_init);
  // No TC interrupt — we poll the TC flag instead

#elif defined(STM32H7xx)
  __HAL_RCC_DMA2_CLK_ENABLE();
  LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);
  LL_DMA_DeInit(DMA2, LL_DMA_STREAM_0);

  LL_DMA_InitTypeDef dma_init;
  LL_DMA_StructInit(&dma_init);
  dma_init.PeriphRequest = LL_DMAMUX1_REQ_TIM2_CH1;
  dma_init.PeriphOrM2MSrcAddress = (uint32_t)&TIM2->CCR1;
  dma_init.MemoryOrM2MDstAddress = (uint32_t)capture_buf;
  dma_init.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  dma_init.NbData = CAPTURE_BUF_SIZE;
  dma_init.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  dma_init.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dma_init.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
  dma_init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
  dma_init.Mode = LL_DMA_MODE_NORMAL;
  dma_init.Priority = LL_DMA_PRIORITY_HIGH;
  dma_init.FIFOMode = LL_DMA_FIFOMODE_DISABLE;
  dma_init.PeriphBurst = LL_DMA_PBURST_SINGLE;
  dma_init.MemBurst = LL_DMA_MBURST_SINGLE;

  LL_DMA_Init(DMA2, LL_DMA_STREAM_0, &dma_init);

#elif defined(STM32G4xx)
  // G4: channel-based DMA with DMAMUX
  // TIM2_CH1 capture → DMA1_CHANNEL_2 (CHANNEL_1 taken by DShot motor)
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
  LL_DMA_DeInit(DMA1, LL_DMA_CHANNEL_2);

  LL_DMA_InitTypeDef dma_init;
  LL_DMA_StructInit(&dma_init);
  dma_init.PeriphRequest = LL_DMAMUX_REQ_TIM2_CH1;
  dma_init.PeriphOrM2MSrcAddress = (uint32_t)&TIM2->CCR1;
  dma_init.MemoryOrM2MDstAddress = (uint32_t)capture_buf;
  dma_init.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  dma_init.NbData = CAPTURE_BUF_SIZE;
  dma_init.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  dma_init.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dma_init.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
  dma_init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
  dma_init.Mode = LL_DMA_MODE_NORMAL;
  dma_init.Priority = LL_DMA_PRIORITY_HIGH;

  LL_DMA_Init(DMA1, LL_DMA_CHANNEL_2, &dma_init);
#endif
}

static void armCapture(void)
{
  for (int i = 0; i < CAPTURE_BUF_SIZE; i++) {
    capture_buf[i] = 0;
  }

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
  SCB_CleanDCache_by_Addr((uint32_t *)capture_buf, sizeof(capture_buf));
#endif

#if defined(STM32F4xx) || defined(STM32F7xx)
  LL_DMA_ClearFlag_TC5(DMA1);  // Clear any stale TC flag
  LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, CAPTURE_BUF_SIZE);
  LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_5);
#elif defined(STM32H7xx)
  LL_DMA_ClearFlag_TC0(DMA2);
  LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, CAPTURE_BUF_SIZE);
  LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);
#elif defined(STM32G4xx)
  LL_DMA_ClearFlag_TC2(DMA1);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, CAPTURE_BUF_SIZE);
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
#endif

  LL_TIM_SetCounter(TIM2, 0);
  LL_TIM_EnableDMAReq_CC1(TIM2);
}

static bool pollCaptureComplete(void)
{
#if defined(STM32F4xx) || defined(STM32F7xx)
  if (LL_DMA_IsActiveFlag_TC5(DMA1)) {
    LL_DMA_ClearFlag_TC5(DMA1);
    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_5);
    LL_TIM_DisableDMAReq_CC1(TIM2);
    return true;
  }
#elif defined(STM32H7xx)
  if (LL_DMA_IsActiveFlag_TC0(DMA2)) {
    LL_DMA_ClearFlag_TC0(DMA2);
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);
    LL_TIM_DisableDMAReq_CC1(TIM2);
    return true;
  }
#elif defined(STM32G4xx)
  if (LL_DMA_IsActiveFlag_TC2(DMA1)) {
    LL_DMA_ClearFlag_TC2(DMA1);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    LL_TIM_DisableDMAReq_CC1(TIM2);
    return true;
  }
#endif
  return false;
}

// ---------------------------------------------------------------------------
// Packet verification from captured edge timestamps
// ---------------------------------------------------------------------------
static bool verifyPacket(uint16_t expected_throttle, bool expected_telemetry)
{
  uint16_t expected = DShot::encodePacket(expected_throttle, expected_telemetry);

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
  SCB_InvalidateDCache_by_Addr((uint32_t *)capture_buf, sizeof(capture_buf));
#endif

  // Both-edge capture produces alternating timestamps:
  //   [0] rising, [1] falling, [2] rising, [3] falling, ...
  // High time for bit i = capture[2i+1] - capture[2i]
  // Classify: high_time > midpoint threshold → bit 1, else bit 0

  uint32_t tim_clk = DShot::getTimerClockFreq(TIM2);
  uint32_t bit0_ticks = (uint32_t)((uint64_t)DShot::BIT_0_DUTY * tim_clk / DShot::DSHOT600);
  uint32_t bit1_ticks = (uint32_t)((uint64_t)DShot::BIT_1_DUTY * tim_clk / DShot::DSHOT600);
  uint32_t threshold = (bit0_ticks + bit1_ticks) / 2;

  // Find the first non-zero capture (first edge)
  int start = 0;
  while (start < CAPTURE_BUF_SIZE && capture_buf[start] == 0) start++;
  if (start + 32 > CAPTURE_BUF_SIZE) {
    Serial.println("  Not enough captured edges");
    return false;
  }

  // Reconstruct 16 bits
  uint16_t reconstructed = 0;
  int bits_decoded = 0;

  for (int i = 0; i < 32 && (start + i + 1) < CAPTURE_BUF_SIZE; i += 2) {
    uint32_t rising = capture_buf[start + i];
    uint32_t falling = capture_buf[start + i + 1];
    if (falling == 0 || rising == 0) break;

    uint32_t high_time = falling - rising;
    reconstructed <<= 1;
    if (high_time > threshold) {
      reconstructed |= 1;
    }
    bits_decoded++;
  }

  if (bits_decoded < 16) {
    Serial.print("  Only decoded ");
    Serial.print(bits_decoded);
    Serial.println(" bits");
    return false;
  }

  if (reconstructed != expected) {
    Serial.print("  Expected: 0x");
    Serial.print(expected, HEX);
    Serial.print("  Got: 0x");
    Serial.println(reconstructed, HEX);
    return false;
  }

  // Check bit period timing tolerance (±20%)
  uint32_t bit_period_ticks = (uint32_t)((uint64_t)DShot::BIT_PERIOD * tim_clk / DShot::DSHOT600);

  for (int i = 0; i < 15 && (start + 2 * (i + 1)) < CAPTURE_BUF_SIZE; i++) {
    uint32_t period = capture_buf[start + 2 * (i + 1)] - capture_buf[start + 2 * i];
    if (period == 0) break;

    uint32_t lower = bit_period_ticks * 80 / 100;
    uint32_t upper = bit_period_ticks * 120 / 100;
    if (period < lower || period > upper) {
      Serial.print("  Timing out of spec at bit ");
      Serial.print(i);
      Serial.print(": period=");
      Serial.print(period);
      Serial.print(" expected ~");
      Serial.println(bit_period_ticks);
      return false;
    }
  }

  return true;
}
