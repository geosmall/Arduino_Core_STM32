/**
 * DShot_Burst_Verification - DMAR burst mode loopback test
 *
 * Validates DMAR burst DMA by adding multiple motors to TIM1, which
 * triggers burst mode (F4/F7: stream conflict, G4/H7: stream conservation).
 * Captures one channel's output via DMA input capture on TIM2 and
 * verifies the packet matches the expected encoding.
 *
 * Different throttle values per channel prove correct interleaving:
 * if the stride-based buffer fill were wrong, the captured channel
 * would show another channel's data.
 *
 * Wiring (Nucleo-64: F411RE, G474RE):
 *   PA8  (TIM1_CH1, D7) --> PA0 (TIM2_CH1, A0)   [captured channel]
 *   PA9  (TIM1_CH2, D8)     not connected          [output only]
 *   PA10 (TIM1_CH3, D2)     not connected          [output only]
 *
 * Wiring (Nucleo-144: F722ZE, H753ZI):
 *   PE11 (TIM1_CH2, D5) --> PA0 (TIM2_CH1)        [captured channel]
 *   PE13 (TIM1_CH3)         not connected          [output only]
 *
 * Same jumper wiring as DShot_Verification Pass 1.
 */

#include <DShot.h>
#include "stm32yyxx_ll.h"

// --- Board-specific motor configuration ---
struct MotorDef {
  uint32_t pin;
  uint32_t channel;   // timer channel 1-4
};

#if defined(ARDUINO_NUCLEO_F411RE) || defined(ARDUINO_NUCLEO_G474RE)
// 3 motors on TIM1 CH1-CH3
// F4: all map to DMA2_Stream6 → burst via conflict
// G4: 2+ motors on same timer → burst for stream conservation
static const MotorDef tim1_motors[] = {
  {PA8,  1},  // TIM1_CH1 — captured via jumper to PA0
  {PA9,  2},  // TIM1_CH2
  {PA10, 3},  // TIM1_CH3
};
static constexpr int CAPTURED_MOTOR = 0;  // index into tim1_motors[]

#elif defined(ARDUINO_NUCLEO_F722ZE) || defined(ARDUINO_NUCLEO_H753ZI)
// 2 motors on TIM1 CH2-CH3
// F7: both map to DMA2_Stream6 → burst via conflict
// H7: 2+ motors on same timer → burst for stream conservation
static const MotorDef tim1_motors[] = {
  {PE11, 2},  // TIM1_CH2 — captured via jumper to PA0
  {PE13, 3},  // TIM1_CH3
};
static constexpr int CAPTURED_MOTOR = 0;

#else
  #error "DShot_Burst_Verification: no pin config for this board"
#endif

static constexpr int NUM_TIM1_MOTORS = sizeof(tim1_motors) / sizeof(tim1_motors[0]);

// --- Test throttle values ---
// Each motor gets a different throttle to prove correct interleaving.
// captured_throttle is what we verify; others are decoys.
static const uint16_t test_throttles[] = {0, 48, 500, 1000, 2047};
static constexpr int NUM_TESTS = sizeof(test_throttles) / sizeof(test_throttles[0]);

// Offset applied to non-captured motors so each channel has unique data.
// If interleaving is wrong, the captured packet will contain the wrong value.
static constexpr uint16_t DECOY_OFFSET_1 = 300;
static constexpr uint16_t DECOY_OFFSET_2 = 700;

// --- Input capture buffer ---
static constexpr int CAPTURE_BUF_SIZE = 40;
static constexpr int MIN_EDGES = 32;
static volatile uint32_t capture_buf[CAPTURE_BUF_SIZE];

// Forward declarations
static void initCaptureTimer(void);
static void initCaptureDMA(void);
static void armCapture(void);
static int stopAndCountCaptures(void);
static bool verifyPacket(uint16_t expected_throttle, int num_captured);

void setup()
{
  Serial.begin(115200);
  while (!Serial) { }
  Serial.println("DShot_Burst_Verification: DMAR burst mode loopback test");
  Serial.print("  TIM1 motors: ");
  Serial.println(NUM_TIM1_MOTORS);

  // --- Set up capture infrastructure (TIM2 + DMA) ---
  initCaptureTimer();
  initCaptureDMA();

  // --- Add all TIM1 motors (triggers DMAR burst on all families) ---
  DShotOutput dshot;
  for (int i = 0; i < NUM_TIM1_MOTORS; i++) {
    int result = dshot.AddMotor(TIM1, tim1_motors[i].pin,
                                tim1_motors[i].channel, DShot::DSHOT600);
    if (result < 0) {
      Serial.print("FAIL: AddMotor() failed for motor ");
      Serial.println(i);
      Serial.println("*STOP*");
      while (1) { }
    }
    Serial.print("  Motor ");
    Serial.print(i);
    Serial.print(": TIM1_CH");
    Serial.println(tim1_motors[i].channel);
  }

  // First Send() triggers initAllDMA() which detects stream conflicts
  dshot.SetAllThrottle(0);
  dshot.Send();
  uint32_t deadline = millis() + 100;
  while (!dshot.IsTransferComplete() && millis() < deadline) { }

  // Check if DMAR burst was activated by reading TIM1->DCR
  bool burst_detected = (TIM1->DCR != 0);
  Serial.print("  DMAR burst mode: ");
  Serial.println(burst_detected ? "ACTIVE" : "INACTIVE");
  if (!burst_detected) {
    Serial.println("FAIL: Expected DMAR burst with multiple TIM1 channels");
    Serial.println("*STOP*");
    while (1) { }
  }

  // --- Run verification tests ---
  int pass_count = 0;
  int fail_count = 0;

  for (int t = 0; t < NUM_TESTS; t++) {
    uint16_t captured_throttle = test_throttles[t];

    // Set different throttle per motor to prove correct interleaving
    dshot.SetThrottle(CAPTURED_MOTOR, captured_throttle);
    for (int m = 0; m < NUM_TIM1_MOTORS; m++) {
      if (m == CAPTURED_MOTOR) continue;
      // Decoy values: different from captured, clamped to 0-2047
      uint16_t decoy = captured_throttle +
          ((m == ((CAPTURED_MOTOR + 1) % NUM_TIM1_MOTORS)) ? DECOY_OFFSET_1 : DECOY_OFFSET_2);
      if (decoy > 2047) decoy -= 2048;
      dshot.SetThrottle(m, decoy);
    }

    armCapture();
    dshot.Send();

    deadline = millis() + 100;
    while (!dshot.IsTransferComplete() && millis() < deadline) { }
    delayMicroseconds(100);

    int num_captured = stopAndCountCaptures();

    if (num_captured < MIN_EDGES) {
      Serial.print("FAIL: Only captured ");
      Serial.print(num_captured);
      Serial.print(" edges for throttle=");
      Serial.println(captured_throttle);
      fail_count++;
      continue;
    }

    if (verifyPacket(captured_throttle, num_captured)) {
      Serial.print("PASS: throttle=");
      Serial.println(captured_throttle);
      pass_count++;
    } else {
      Serial.print("FAIL: throttle=");
      Serial.println(captured_throttle);
      fail_count++;
    }
  }

  // --- Results ---
  Serial.println();
  Serial.println("=== Results ===");
  Serial.print(pass_count);
  Serial.print(" passed, ");
  Serial.print(fail_count);
  Serial.println(" failed");
  Serial.print("DMAR burst: ");
  Serial.print(NUM_TIM1_MOTORS);
  Serial.println(" channels on TIM1");

  if (fail_count == 0) {
    Serial.println("*STOP*");
  }
}

void loop()
{
  delay(1000);
}

// ---------------------------------------------------------------------------
// TIM2 capture timer init (32-bit free-running counter)
// ---------------------------------------------------------------------------
static void initCaptureTimer(void)
{
  __HAL_RCC_TIM2_CLK_ENABLE();
  LL_TIM_DisableCounter(TIM2);

  LL_TIM_InitTypeDef tim_init;
  LL_TIM_StructInit(&tim_init);
  tim_init.Prescaler = 0;
  tim_init.Autoreload = 0xFFFFFFFF;
  tim_init.CounterMode = LL_TIM_COUNTERMODE_UP;
  LL_TIM_Init(TIM2, &tim_init);
  LL_TIM_EnableCounter(TIM2);
}

// ---------------------------------------------------------------------------
// Input capture channel + DMA setup (PA0 = TIM2_CH1)
// ---------------------------------------------------------------------------
static void initCaptureDMA(void)
{
  // GPIO: PA0 as TIM2_CH1 alternate function
  __HAL_RCC_GPIOA_CLK_ENABLE();
  LL_GPIO_InitTypeDef gpio_init;
  LL_GPIO_StructInit(&gpio_init);
  gpio_init.Pin = LL_GPIO_PIN_0;
  gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
  gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  gpio_init.Pull = LL_GPIO_PULL_DOWN;
  gpio_init.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOA, &gpio_init);

  // Input capture: both edges on TIM2_CH1
  LL_TIM_IC_InitTypeDef ic_init;
  LL_TIM_IC_StructInit(&ic_init);
  ic_init.ICActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;
  ic_init.ICPrescaler = LL_TIM_ICPSC_DIV1;
  ic_init.ICFilter = LL_TIM_IC_FILTER_FDIV1;
  ic_init.ICPolarity = LL_TIM_IC_POLARITY_BOTHEDGE;
  LL_TIM_IC_Init(TIM2, LL_TIM_CHANNEL_CH1, &ic_init);
  LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);

  // DMA: TIM2_CH1 → capture_buf
  // Use DMA2 on G4/H7 to avoid conflict with DShot burst on DMA1
  LL_DMA_InitTypeDef dma_init;
  LL_DMA_StructInit(&dma_init);

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

#if defined(STM32F4xx) || defined(STM32F7xx)
  // TIM2_CH1 → DMA1_Stream5/CH3
  __HAL_RCC_DMA1_CLK_ENABLE();
  LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_5);
  LL_DMA_DeInit(DMA1, LL_DMA_STREAM_5);
  dma_init.Channel = LL_DMA_CHANNEL_3;
  dma_init.FIFOMode = LL_DMA_FIFOMODE_DISABLE;
  dma_init.MemBurst = LL_DMA_MBURST_SINGLE;
  dma_init.PeriphBurst = LL_DMA_PBURST_SINGLE;
  LL_DMA_Init(DMA1, LL_DMA_STREAM_5, &dma_init);

#elif defined(STM32H7xx)
  // TIM2_CH1 → DMA2_Stream0 via DMAMUX (avoids DShot burst on DMA1)
  __HAL_RCC_DMA2_CLK_ENABLE();
#if defined(__HAL_RCC_DMAMUX1_CLK_ENABLE)
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
#endif
  LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);
  LL_DMA_DeInit(DMA2, LL_DMA_STREAM_0);
  dma_init.PeriphRequest = LL_DMAMUX1_REQ_TIM2_CH1;
  dma_init.FIFOMode = LL_DMA_FIFOMODE_DISABLE;
  dma_init.PeriphBurst = LL_DMA_PBURST_SINGLE;
  dma_init.MemBurst = LL_DMA_MBURST_SINGLE;
  LL_DMA_Init(DMA2, LL_DMA_STREAM_0, &dma_init);

#elif defined(STM32G4xx)
  // TIM2_CH1 → DMA2_Channel1 via DMAMUX (avoids DShot burst on DMA1)
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_1);
  LL_DMA_DeInit(DMA2, LL_DMA_CHANNEL_1);
  dma_init.PeriphRequest = LL_DMAMUX_REQ_TIM2_CH1;
  LL_DMA_Init(DMA2, LL_DMA_CHANNEL_1, &dma_init);
#endif
}

// ---------------------------------------------------------------------------
// Arm capture DMA
// ---------------------------------------------------------------------------
static void armCapture(void)
{
  for (int i = 0; i < CAPTURE_BUF_SIZE; i++) {
    capture_buf[i] = 0;
  }

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
  SCB_CleanDCache_by_Addr((uint32_t *)capture_buf, sizeof(capture_buf));
#endif

#if defined(STM32F4xx) || defined(STM32F7xx)
  LL_DMA_ClearFlag_TC5(DMA1);
  LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, CAPTURE_BUF_SIZE);
  LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_5);
#elif defined(STM32H7xx)
  LL_DMA_ClearFlag_TC0(DMA2);
  LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, CAPTURE_BUF_SIZE);
  LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);
#elif defined(STM32G4xx)
  LL_DMA_ClearFlag_GI1(DMA2);
  LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_1, CAPTURE_BUF_SIZE);
  LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_1);
#endif

  LL_TIM_SetCounter(TIM2, 0);
  LL_TIM_EnableDMAReq_CC1(TIM2);
}

// ---------------------------------------------------------------------------
// Stop capture DMA, return edge count
// ---------------------------------------------------------------------------
static int stopAndCountCaptures(void)
{
  int remaining = 0;

#if defined(STM32F4xx) || defined(STM32F7xx)
  LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_5);
  remaining = LL_DMA_GetDataLength(DMA1, LL_DMA_STREAM_5);
#elif defined(STM32H7xx)
  LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);
  remaining = LL_DMA_GetDataLength(DMA2, LL_DMA_STREAM_0);
#elif defined(STM32G4xx)
  LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_1);
  remaining = LL_DMA_GetDataLength(DMA2, LL_DMA_CHANNEL_1);
#endif

  LL_TIM_DisableDMAReq_CC1(TIM2);
  return CAPTURE_BUF_SIZE - remaining;
}

// ---------------------------------------------------------------------------
// Packet verification from captured edge timestamps
// ---------------------------------------------------------------------------
static bool verifyPacket(uint16_t expected_throttle, int num_captured)
{
  uint16_t expected = DShot::encodePacket(expected_throttle, false);

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
  SCB_InvalidateDCache_by_Addr((uint32_t *)capture_buf, sizeof(capture_buf));
#endif

  uint32_t tim_clk = DShot::getTimerClockFreq(TIM2);
  uint32_t bit0_ticks = (uint32_t)((uint64_t)DShot::BIT_0_DUTY * tim_clk / DShot::DSHOT600);
  uint32_t bit1_ticks = (uint32_t)((uint64_t)DShot::BIT_1_DUTY * tim_clk / DShot::DSHOT600);
  uint32_t threshold = (bit0_ticks + bit1_ticks) / 2;

  // Find first non-zero capture
  int start = 0;
  while (start < num_captured && capture_buf[start] == 0) start++;
  if (start + 32 > num_captured) {
    Serial.print("  Not enough valid edges (start=");
    Serial.print(start);
    Serial.println(")");
    return false;
  }

  // Reconstruct 16 bits from edge pairs
  uint16_t reconstructed = 0;
  int bits_decoded = 0;

  for (int i = 0; i < 32 && (start + i + 1) < num_captured; i += 2) {
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

  // Check bit period timing tolerance (+/-20%)
  uint32_t bit_period_ticks = (uint32_t)((uint64_t)DShot::BIT_PERIOD * tim_clk / DShot::DSHOT600);

  for (int i = 0; i < 15 && (start + 2 * (i + 1)) < num_captured; i++) {
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
