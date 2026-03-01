/**
 * DShot_Verification - DMA-based input capture loopback test
 *
 * Verifies DShot output timing using jumper wires from output pins
 * to input capture pins on TIM2. Tests both an advanced timer (TIM1)
 * and a general-purpose timer (TIM3) sequentially.
 *
 * Wiring (Nucleo-64: F411RE, G474RE):
 *   PA8 (TIM1_CH1, D7) --> PA0 (TIM2_CH1, A0)   [Pass 1: advanced timer]
 *   PB0 (TIM3_CH3, A3) --> PB10 (TIM2_CH3, D6)  [Pass 2: general-purpose timer]
 *
 * Wiring (Nucleo-144: F722ZE, H753ZI):
 *   PE11 (TIM1_CH2, D5) --> PA0 (TIM2_CH1)       [Pass 1: advanced timer]
 *   PB0 (TIM3_CH3)      --> PB10 (TIM2_CH3)      [Pass 2: general-purpose timer]
 *
 * The test (per pass):
 * 1. Sends known DShot600 packets on the output pin via DMA
 * 2. Captures all edges on the capture pin via DMA-driven input capture
 * 3. Reconstructs each packet from captured timestamps
 * 4. Verifies it matches the expected encoding
 *
 * Edge count: 16 DShot bits x 2 edges/bit = 32 edges per packet.
 * The capture buffer is oversized (40); completion is detected by
 * waiting for the output DMA to finish, then reading the remaining
 * DMA count to see how many edges were actually captured.
 */

#include <DShot.h>
#include "stm32yyxx_ll.h"

// --- Input Capture buffer ---
// 16 bits x 2 edges/bit = 32 edges; buffer oversized for safety
static constexpr int CAPTURE_BUF_SIZE = 40;
static constexpr int MIN_EDGES = 32;
static volatile uint32_t capture_buf[CAPTURE_BUF_SIZE];

// --- Test configuration ---
struct TestPass {
  // DShot output
  TIM_TypeDef *timer;
  uint32_t pin;
  uint32_t channel;        // timer channel (1-4)
  const char *label;
  // Input capture (all use TIM2, different channels)
  GPIO_TypeDef *cap_port;
  uint32_t cap_ll_pin;     // LL_GPIO_PIN_x
  uint32_t cap_af;         // GPIO alternate function
  uint32_t cap_tim_ch;     // TIM2 channel index: 1 or 3
};

// Nucleo-64 boards: D7->A0 and A3->D6 jumpers
// PA8=D7 (TIM1_CH1 output), PA0=A0 (TIM2_CH1 capture)
// PB0=A3 (TIM3_CH3 output), PB10=D6 (TIM2_CH3 capture)
#if defined(ARDUINO_NUCLEO_F411RE) || defined(ARDUINO_NUCLEO_G474RE)
static const TestPass test_passes[] = {
  // Pass 1: D7->A0 jumper — TIM1_CH1 output, TIM2_CH1 capture
  {TIM1, PA8, 1, "TIM1/PA8 (advanced timer)",
   GPIOA, LL_GPIO_PIN_0, LL_GPIO_AF_1, 1},
  // Pass 2: A3->D6 jumper — TIM3_CH3 output, TIM2_CH3 capture
  {TIM3, PB0, 3, "TIM3/PB0 (general-purpose timer)",
   GPIOB, LL_GPIO_PIN_10, LL_GPIO_AF_1, 3},
};

// Nucleo-144 boards: PE11->PA0 and PB0->PB10 jumpers
// PE11=D5 (TIM1_CH2 output), PA0 (TIM2_CH1 capture)
// PB0 (TIM3_CH3 output), PB10 (TIM2_CH3 capture)
// Note: PE9 (TIM1_CH1, D6) requires SB28 solder bridge — PE11 is direct.
#elif defined(ARDUINO_NUCLEO_F722ZE) || defined(ARDUINO_NUCLEO_H753ZI)
static const TestPass test_passes[] = {
  // Pass 1: PE11->PA0 jumper — TIM1_CH2 output, TIM2_CH1 capture
  {TIM1, PE11, 2, "TIM1/PE11 (advanced timer)",
   GPIOA, LL_GPIO_PIN_0, LL_GPIO_AF_1, 1},
  // Pass 2: PB0->PB10 jumper — TIM3_CH3 output, TIM2_CH3 capture
  {TIM3, PB0, 3, "TIM3/PB0 (general-purpose timer)",
   GPIOB, LL_GPIO_PIN_10, LL_GPIO_AF_1, 3},
};

#else
  #error "DShot_Verification: no test passes defined for this board"
#endif
static constexpr int NUM_PASSES = sizeof(test_passes) / sizeof(test_passes[0]);

static const uint16_t test_throttles[] = {0, 1, 48, 1000, 2047};
static constexpr int NUM_THROTTLES = sizeof(test_throttles) / sizeof(test_throttles[0]);

// Current capture configuration (set before each pass)
static const TestPass *cur_pass;

static void initCaptureTimer(void);
static void initCaptureChannel(const TestPass &pass);
static void armCapture(void);
static int stopAndCountCaptures(void);
static bool verifyPacket(uint16_t expected_throttle, bool expected_telemetry, int num_captured);
static void disableMotorOutput(TIM_TypeDef *timer, uint32_t pin);
static int runTestPass(const TestPass &pass);

void setup()
{
  Serial.begin(115200);
  while (!Serial) { }
  Serial.println("DShot_Verification: DMA loopback test (v2)");
  Serial.print("  capture_buf @ 0x");
  Serial.println((uint32_t)capture_buf, HEX);

  // TIM2 timebase — shared across all passes (only the IC channel changes)
  initCaptureTimer();

  int total_pass = 0;
  int total_fail = 0;

  for (int p = 0; p < NUM_PASSES; p++) {
    Serial.println();
    Serial.print("--- Pass ");
    Serial.print(p + 1);
    Serial.print(": ");
    Serial.print(test_passes[p].label);
    Serial.println(" ---");

    cur_pass = &test_passes[p];
    initCaptureChannel(test_passes[p]);

    int fails = runTestPass(test_passes[p]);
    total_pass += (NUM_THROTTLES - fails);
    total_fail += fails;

    // Disable this motor output before next pass
    disableMotorOutput(test_passes[p].timer, test_passes[p].pin);
  }

  Serial.println();
  Serial.println("=== Final Results ===");
  Serial.print(total_pass);
  Serial.print(" passed, ");
  Serial.print(total_fail);
  Serial.println(" failed");

  if (total_fail == 0) {
    Serial.println("*STOP*");
  }
}

void loop()
{
  delay(1000);
}

// ---------------------------------------------------------------------------
// Run one test pass
// ---------------------------------------------------------------------------
static int runTestPass(const TestPass &pass)
{
  DShotOutput dshot;
  int result = dshot.AddMotor(pass.timer, pass.pin, pass.channel, DShot::DSHOT600);
  if (result < 0) {
    Serial.println("FAIL: AddMotor() failed");
    return NUM_THROTTLES;
  }

  int fail_count = 0;

  for (int t = 0; t < NUM_THROTTLES; t++) {
    uint16_t throttle = test_throttles[t];

    armCapture();
    uint32_t cr_after_arm = DMA2_Stream0->CR;

    dshot.SetThrottle(0, throttle, false);
    dshot.Send();

    uint32_t deadline = millis() + 100;
    while (!dshot.IsTransferComplete() && millis() < deadline) { }

    bool dma_ok = dshot.IsTransferComplete();
    delayMicroseconds(100);

    int num_captured = stopAndCountCaptures();

    // Debug: show DMA completion and register state
    if (num_captured < MIN_EDGES) {
      Serial.print("  [dbg] EN_after_arm=");
      Serial.print(cr_after_arm & 1);
      Serial.print(" NDTR=");
      Serial.print(DMA2_Stream0->NDTR);
      Serial.print(" LISR=0x");
      Serial.println(DMA2->LISR, HEX);
    }

    if (num_captured < MIN_EDGES) {
      Serial.print("FAIL: Only captured ");
      Serial.print(num_captured);
      Serial.print(" edges (need ");
      Serial.print(MIN_EDGES);
      Serial.print(") for throttle=");
      Serial.println(throttle);
      fail_count++;
      continue;
    }

    if (verifyPacket(throttle, false, num_captured)) {
      Serial.print("PASS: throttle=");
      Serial.println(throttle);
    } else {
      Serial.print("FAIL: throttle=");
      Serial.println(throttle);
      fail_count++;
    }
  }

  return fail_count;
}

// ---------------------------------------------------------------------------
// Disable a motor output
// ---------------------------------------------------------------------------
static void disableMotorOutput(TIM_TypeDef *timer, uint32_t pin)
{
  LL_TIM_DisableAllOutputs(timer);
  LL_TIM_DisableCounter(timer);
  pinMode(pin, INPUT);
}

// ---------------------------------------------------------------------------
// TIM2 timebase init — called once, shared across all capture channels
// ---------------------------------------------------------------------------
static void initCaptureTimer(void)
{
  __HAL_RCC_TIM2_CLK_ENABLE();
  LL_TIM_DisableCounter(TIM2);

  LL_TIM_InitTypeDef tim_init;
  LL_TIM_StructInit(&tim_init);
  tim_init.Prescaler = 0;
  tim_init.Autoreload = 0xFFFFFFFF;  // TIM2 is 32-bit
  tim_init.CounterMode = LL_TIM_COUNTERMODE_UP;
  LL_TIM_Init(TIM2, &tim_init);
  LL_TIM_EnableCounter(TIM2);

  __HAL_RCC_DMA1_CLK_ENABLE();
#if defined(__HAL_RCC_DMAMUX1_CLK_ENABLE)
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
#endif
#if defined(STM32H7xx)
  __HAL_RCC_DMA2_CLK_ENABLE();
#endif
}

// ---------------------------------------------------------------------------
// Configure IC channel + GPIO + DMA for a specific capture pin
// ---------------------------------------------------------------------------
static void initCaptureChannel(const TestPass &pass)
{
  // GPIO: capture pin as TIM2 alternate function
  LL_GPIO_InitTypeDef gpio_init;
  LL_GPIO_StructInit(&gpio_init);
  gpio_init.Pin = pass.cap_ll_pin;
  gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
  gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  gpio_init.Pull = LL_GPIO_PULL_DOWN;
  gpio_init.Alternate = pass.cap_af;

  // Enable GPIO clock for the capture port
  if (pass.cap_port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
  else if (pass.cap_port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();

  LL_GPIO_Init(pass.cap_port, &gpio_init);

  // Input capture: both edges
  uint32_t ll_ch = (pass.cap_tim_ch == 1) ? LL_TIM_CHANNEL_CH1 : LL_TIM_CHANNEL_CH3;

  LL_TIM_IC_InitTypeDef ic_init;
  LL_TIM_IC_StructInit(&ic_init);
  ic_init.ICActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;
  ic_init.ICPrescaler = LL_TIM_ICPSC_DIV1;
  ic_init.ICFilter = LL_TIM_IC_FILTER_FDIV1;
  ic_init.ICPolarity = LL_TIM_IC_POLARITY_BOTHEDGE;
  LL_TIM_IC_Init(TIM2, ll_ch, &ic_init);
  LL_TIM_CC_EnableChannel(TIM2, ll_ch);

  // DMA: capture CCR values into buffer
  volatile uint32_t *ccr = (pass.cap_tim_ch == 1) ? &TIM2->CCR1 : &TIM2->CCR3;

#if defined(STM32F4xx) || defined(STM32F7xx)
  // TIM2_CH1 → DMA1_Stream5/CH3,  TIM2_CH3 → DMA1_Stream1/CH3
  uint32_t stream = (pass.cap_tim_ch == 1) ? LL_DMA_STREAM_5 : LL_DMA_STREAM_1;

  LL_DMA_DisableStream(DMA1, stream);
  LL_DMA_DeInit(DMA1, stream);

  LL_DMA_InitTypeDef dma_init;
  LL_DMA_StructInit(&dma_init);
  dma_init.Channel = LL_DMA_CHANNEL_3;
  dma_init.PeriphOrM2MSrcAddress = (uint32_t)ccr;
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

  LL_DMA_Init(DMA1, stream, &dma_init);

#elif defined(STM32H7xx)
  LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);
  LL_DMA_DeInit(DMA2, LL_DMA_STREAM_0);

  LL_DMA_InitTypeDef dma_init;
  LL_DMA_StructInit(&dma_init);
  dma_init.PeriphRequest = (pass.cap_tim_ch == 1)
      ? LL_DMAMUX1_REQ_TIM2_CH1 : LL_DMAMUX1_REQ_TIM2_CH3;
  dma_init.PeriphOrM2MSrcAddress = (uint32_t)ccr;
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
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_8);
  LL_DMA_DeInit(DMA1, LL_DMA_CHANNEL_8);

  LL_DMA_InitTypeDef dma_init;
  LL_DMA_StructInit(&dma_init);
  dma_init.PeriphRequest = (cur_pass->cap_tim_ch == 1)
      ? LL_DMAMUX_REQ_TIM2_CH1 : LL_DMAMUX_REQ_TIM2_CH3;
  dma_init.PeriphOrM2MSrcAddress = (uint32_t)ccr;
  dma_init.MemoryOrM2MDstAddress = (uint32_t)capture_buf;
  dma_init.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  dma_init.NbData = CAPTURE_BUF_SIZE;
  dma_init.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  dma_init.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dma_init.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
  dma_init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
  dma_init.Mode = LL_DMA_MODE_NORMAL;
  dma_init.Priority = LL_DMA_PRIORITY_HIGH;

  LL_DMA_Init(DMA1, LL_DMA_CHANNEL_8, &dma_init);
#endif
}

// ---------------------------------------------------------------------------
// Arm capture DMA for current pass
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
  uint32_t stream = (cur_pass->cap_tim_ch == 1) ? LL_DMA_STREAM_5 : LL_DMA_STREAM_1;

  // Clear TC flag for the active stream
  if (stream == LL_DMA_STREAM_5) LL_DMA_ClearFlag_TC5(DMA1);
  else                           LL_DMA_ClearFlag_TC1(DMA1);

  LL_DMA_SetDataLength(DMA1, stream, CAPTURE_BUF_SIZE);
  LL_DMA_EnableStream(DMA1, stream);

#elif defined(STM32H7xx)
  LL_DMA_ClearFlag_TC0(DMA2);
  LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, CAPTURE_BUF_SIZE);
  LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);

#elif defined(STM32G4xx)
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_8);
  LL_DMA_ClearFlag_GI8(DMA1);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_8, CAPTURE_BUF_SIZE);
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_8);
#endif

  LL_TIM_SetCounter(TIM2, 0);

  if (cur_pass->cap_tim_ch == 1) LL_TIM_EnableDMAReq_CC1(TIM2);
  else                           LL_TIM_EnableDMAReq_CC3(TIM2);
}

// ---------------------------------------------------------------------------
// Stop capture DMA and return number of edges captured
// ---------------------------------------------------------------------------
static int stopAndCountCaptures(void)
{
  int remaining = 0;

#if defined(STM32F4xx) || defined(STM32F7xx)
  uint32_t stream = (cur_pass->cap_tim_ch == 1) ? LL_DMA_STREAM_5 : LL_DMA_STREAM_1;
  LL_DMA_DisableStream(DMA1, stream);
  remaining = LL_DMA_GetDataLength(DMA1, stream);

#elif defined(STM32H7xx)
  LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);
  remaining = LL_DMA_GetDataLength(DMA2, LL_DMA_STREAM_0);

#elif defined(STM32G4xx)
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_8);
  remaining = LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_8);
#endif

  if (cur_pass->cap_tim_ch == 1) LL_TIM_DisableDMAReq_CC1(TIM2);
  else                           LL_TIM_DisableDMAReq_CC3(TIM2);

  return CAPTURE_BUF_SIZE - remaining;
}

// ---------------------------------------------------------------------------
// Packet verification from captured edge timestamps
// ---------------------------------------------------------------------------
static bool verifyPacket(uint16_t expected_throttle, bool expected_telemetry, int num_captured)
{
  uint16_t expected = DShot::encodePacket(expected_throttle, expected_telemetry);

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
  SCB_InvalidateDCache_by_Addr((uint32_t *)capture_buf, sizeof(capture_buf));
#endif

  // Both-edge capture produces alternating timestamps:
  //   [0] rising, [1] falling, [2] rising, [3] falling, ...
  // High time for bit i = capture[2i+1] - capture[2i]
  // Classify: high_time > midpoint threshold -> bit 1, else bit 0

  uint32_t tim_clk = DShot::getTimerClockFreq(TIM2);
  uint32_t bit0_ticks = (uint32_t)((uint64_t)DShot::BIT_0_DUTY * tim_clk / DShot::DSHOT600);
  uint32_t bit1_ticks = (uint32_t)((uint64_t)DShot::BIT_1_DUTY * tim_clk / DShot::DSHOT600);
  uint32_t threshold = (bit0_ticks + bit1_ticks) / 2;

  // Find the first non-zero capture (first edge)
  int start = 0;
  while (start < num_captured && capture_buf[start] == 0) start++;
  if (start + 32 > num_captured) {
    Serial.print("  Not enough valid edges (start=");
    Serial.print(start);
    Serial.print(", captured=");
    Serial.print(num_captured);
    Serial.println(")");
    return false;
  }

  // Reconstruct 16 bits
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
