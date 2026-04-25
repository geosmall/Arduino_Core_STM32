/**
 * DShot_Validation — multi-board DShot library loopback verification.
 *
 * Two-phase HIL test covering all four supported STM32 families on
 * Nucleo-64 and Nucleo-144 boards. Requires a permanent jumper
 * fixture (find pins by raw Pxx silkscreen label):
 *
 *   NUCLEO-G474RE (Nucleo-64, 4 jumpers):
 *     PB4 -> PA0,  PA8  -> PA1,  PB0 -> PB10,  PB6 -> PA10 (TIM2_CH4 AF10)
 *
 *   NUCLEO-F411RE (Nucleo-64, 3 jumpers):
 *     PB4 -> PA0,  PA8  -> PA1,  PB0 -> PB10
 *     (no TIM4 burst: only TIM2_CH4 alt is PA3 = VCOM RX)
 *
 *   NUCLEO-F722ZE (Nucleo-144 Zio, 3 jumpers):
 *     PB4 -> PA0,  PE11 -> PB3,  PB0 -> PB10
 *     (no TIM4 burst: F4/F7 DMA1 Stream 6 is silicon-shared between
 *      TIM4_UP and TIM2_CH2 capture, RM0431 Tbl 26)
 *
 *   NUCLEO-H753ZI (Nucleo-144 Zio, 4 jumpers):
 *     PB4 -> PA0,  PE11 -> PB3,  PB0 -> PB10,  PB6 -> PB11
 *
 * Decoy outputs (no jumper; drive only — force burst mode):
 *   PA9 (Nucleo-64 TIM1_CH2), PE13 (Nucleo-144 TIM1_CH3),
 *   PB5 (TIM3_CH2), PB7 (TIM4_CH2 where used).
 *
 * Phases:
 *   A — concurrent burst on TIM1 + TIM3 (+ TIM4 where fixture allows).
 *       Two motors per timer (captured + decoy) force burst mode; each
 *       decoy gets a distinct throttle so a stride/interleaving bug
 *       decodes the wrong value.
 *   B — per-channel (non-burst) DShot600 on TIM1 + TIM3_CH3. TIM1
 *       channel is CH1 on Nucleo-64, CH2 on Nucleo-144 (PE11).
 *
 * Each phase runs 5 throttle values (0, 48, 500, 1000, 2047). A full
 * run is 10 PASS rows. Sketch emits "*STOP*" on completion so the CI
 * harness can detect exit.
 *
 * Run (G474RE via ST-Link):
 *   ./ci/saflash_stlink.sh \
 *       Arduino_Core_STM32/libraries/DShot/examples/DShot_Validation \
 *       STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_G474RE --timeout 20
 *
 * Full fixture documentation, coverage matrix, expected output, and
 * troubleshooting: see README.md alongside this sketch.
 */

#include <DShot.h>
#include "stm32yyxx_ll.h"
#include "ci_log.h"

// ---------------------------------------------------------------------------
// Per-board fixture selection
//
// Nucleo-144 boards (F722ZE, H753ZI) route TIM1 burst through PE11/PE13
// instead of PA8/PA9 because PA8/PA9 only sit on the unpopulated morpho
// headers — PE11=D5/PE13=D3 are on the Arduino-compatible CN10 top.
// Same reason for TIM2_CH2 capture: PA1 is morpho-only on Nucleo-144,
// so we use PB3 (TIM2_CH2 alt mapping, AF1) on Zio.
// ---------------------------------------------------------------------------
#if defined(ARDUINO_NUCLEO_G474RE)
  #define BOARD_NAME         "NUCLEO-G474RE"
  #define HAS_TIM4_BURST     1
  #define NUM_CAP            4
  #define TIM1_PIN_A         PA8
  #define TIM1_CH_A          1
  #define TIM1_PIN_B         PA9
  #define TIM1_CH_B          2
  #define TIM1_LABEL_A       "TIM1_CH1"
  #define TIM1_LABEL_B       "TIM1_CH2(decoy)"

#elif defined(ARDUINO_NUCLEO_F411RE)
  #define BOARD_NAME         "NUCLEO-F411RE"
  #define HAS_TIM4_BURST     0
  #define NUM_CAP            3
  #define TIM1_PIN_A         PA8
  #define TIM1_CH_A          1
  #define TIM1_PIN_B         PA9
  #define TIM1_CH_B          2
  #define TIM1_LABEL_A       "TIM1_CH1"
  #define TIM1_LABEL_B       "TIM1_CH2(decoy)"

#elif defined(ARDUINO_NUCLEO_F722ZE)
  #define BOARD_NAME         "NUCLEO-F722ZE"
  // No TIM4 burst on F722: TIM4_UP and TIM2_CH2 capture both hard-wire to
  // DMA1 Stream 6 on F4/F7 (only stream mapping for either request) — can
  // not coexist. Same constraint as F411.
  #define HAS_TIM4_BURST     0
  #define NUM_CAP            3
  #define TIM1_PIN_A         PE11
  #define TIM1_CH_A          2
  #define TIM1_PIN_B         PE13
  #define TIM1_CH_B          3
  #define TIM1_LABEL_A       "TIM1_CH2"
  #define TIM1_LABEL_B       "TIM1_CH3(decoy)"

#elif defined(ARDUINO_NUCLEO_H753ZI)
  #define BOARD_NAME         "NUCLEO-H753ZI"
  #define HAS_TIM4_BURST     1
  #define NUM_CAP            4
  #define TIM1_PIN_A         PE11
  #define TIM1_CH_A          2
  #define TIM1_PIN_B         PE13
  #define TIM1_CH_B          3
  #define TIM1_LABEL_A       "TIM1_CH2"
  #define TIM1_LABEL_B       "TIM1_CH3(decoy)"

#else
  #error "DShot_Validation: unsupported board (expected NUCLEO-F411RE, NUCLEO-F722ZE, NUCLEO-H753ZI, or NUCLEO-G474RE)"
#endif

// ---------------------------------------------------------------------------
// Capture channel indexing: 0..NUM_CAP-1 map to TIM2_CH1..TIM2_CH(NUM_CAP)
// ---------------------------------------------------------------------------
enum CapCh {
  CAP_CH1 = 0,   // PA0  — captures PB4 (TIM3_CH1) in phase A
  CAP_CH2 = 1,   // PA1  — captures PA8 (TIM1_CH1) in both phases
  CAP_CH3 = 2,   // PB10 — captures PB0 (TIM3_CH3) in phase B
  CAP_CH4 = 3,   // PA10/PB11 — captures PB6 (TIM4_CH1) in phase A
};

// ---------------------------------------------------------------------------
// Motor tables — identical MCU pins on all 4 boards. Only the TIM4 entries
// are conditionally excluded on F411RE (no routable TIM2_CH4 pin).
// ---------------------------------------------------------------------------
struct MotorDef {
  TIM_TypeDef *timer;
  Pin pin;
  uint32_t channel;
  int cap_channel;   // -1 = decoy (output only, no jumper)
};

static const MotorDef phaseA_motors[] = {
  {TIM1, TIM1_PIN_A, TIM1_CH_A, CAP_CH2},  // TIM1_CHx captured -> CH2
  {TIM1, TIM1_PIN_B, TIM1_CH_B, -1},       // TIM1 decoy
  {TIM3, PB4,        1,         CAP_CH1},  // TIM3_CH1 -> PA0
  {TIM3, PB5,        2,         -1},       // TIM3_CH2 decoy
#if HAS_TIM4_BURST
  {TIM4, PB6,        1,         CAP_CH4},  // TIM4_CH1 -> PA10 / PB11
  {TIM4, PB7,        2,         -1},       // TIM4_CH2 decoy
#endif
};
static constexpr int NUM_A = sizeof(phaseA_motors) / sizeof(phaseA_motors[0]);

static const MotorDef phaseB_motors[] = {
  {TIM1, TIM1_PIN_A, TIM1_CH_A, CAP_CH2},  // TIM1_CHx -> CH2
  {TIM3, PB0,        3,         CAP_CH3},  // TIM3_CH3 -> PB10
};
static constexpr int NUM_B = sizeof(phaseB_motors) / sizeof(phaseB_motors[0]);

static const char *phaseA_label[] = {
  TIM1_LABEL_A, TIM1_LABEL_B, "TIM3_CH1", "TIM3_CH2(decoy)",
#if HAS_TIM4_BURST
  "TIM4_CH1", "TIM4_CH2(decoy)",
#endif
};
static const char *phaseB_label[NUM_B] = { TIM1_LABEL_A, "TIM3_CH3" };

// ---------------------------------------------------------------------------
// Throttle schedule + decoy offset
// ---------------------------------------------------------------------------
static const uint16_t test_throttles[] = {0, 48, 500, 1000, 2047};
static constexpr int NUM_TESTS = sizeof(test_throttles) / sizeof(test_throttles[0]);
static constexpr uint16_t DECOY_OFFSET = 300;

// ---------------------------------------------------------------------------
// Capture buffers (one per TIM2 channel) + edge-count thresholds
// ---------------------------------------------------------------------------
static constexpr int CAPTURE_BUF_SIZE = 40;
static constexpr int MIN_EDGES = 32;
static volatile uint32_t capture_buf[NUM_CAP][CAPTURE_BUF_SIZE];

// ---------------------------------------------------------------------------
// Per-board capture GPIO table (indexed by CapCh)
// ---------------------------------------------------------------------------
struct CapPin {
  GPIO_TypeDef *port;
  uint32_t     ll_pin;
  uint32_t     af;
};

#if defined(ARDUINO_NUCLEO_G474RE)
static const CapPin cap_pins[NUM_CAP] = {
  {GPIOA, LL_GPIO_PIN_0,  LL_GPIO_AF_1},   // TIM2_CH1
  {GPIOA, LL_GPIO_PIN_1,  LL_GPIO_AF_1},   // TIM2_CH2
  {GPIOB, LL_GPIO_PIN_10, LL_GPIO_AF_1},   // TIM2_CH3
  {GPIOA, LL_GPIO_PIN_10, LL_GPIO_AF_10},  // TIM2_CH4 (AF10 on G4)
};

#elif defined(ARDUINO_NUCLEO_F411RE)
static const CapPin cap_pins[NUM_CAP] = {
  {GPIOA, LL_GPIO_PIN_0,  LL_GPIO_AF_1},   // TIM2_CH1
  {GPIOA, LL_GPIO_PIN_1,  LL_GPIO_AF_1},   // TIM2_CH2
  {GPIOB, LL_GPIO_PIN_10, LL_GPIO_AF_1},   // TIM2_CH3
};

#elif defined(ARDUINO_NUCLEO_F722ZE)
// PA1 morpho-only on Nucleo-144 → use PB3 (TIM2_CH2 alt, AF1) on Zio.
// No TIM2_CH4 capture: TIM4 burst dropped from F722 fixture (DMA conflict).
static const CapPin cap_pins[NUM_CAP] = {
  {GPIOA, LL_GPIO_PIN_0,  LL_GPIO_AF_1},   // TIM2_CH1
  {GPIOB, LL_GPIO_PIN_3,  LL_GPIO_AF_1},   // TIM2_CH2 (alt — PB3 instead of PA1)
  {GPIOB, LL_GPIO_PIN_10, LL_GPIO_AF_1},   // TIM2_CH3
};

#else  // H753ZI — same Nucleo-144 PB3 alt for CH2; TIM4 burst kept (H7 captures on DMA2)
static const CapPin cap_pins[NUM_CAP] = {
  {GPIOA, LL_GPIO_PIN_0,  LL_GPIO_AF_1},   // TIM2_CH1
  {GPIOB, LL_GPIO_PIN_3,  LL_GPIO_AF_1},   // TIM2_CH2 (alt — PB3 instead of PA1)
  {GPIOB, LL_GPIO_PIN_10, LL_GPIO_AF_1},   // TIM2_CH3
  {GPIOB, LL_GPIO_PIN_11, LL_GPIO_AF_1},   // TIM2_CH4
};
#endif

static const uint32_t cap_ll_tim_ch[4] = {
  LL_TIM_CHANNEL_CH1, LL_TIM_CHANNEL_CH2,
  LL_TIM_CHANNEL_CH3, LL_TIM_CHANNEL_CH4,
};

// ---------------------------------------------------------------------------
// Per-family DMA allocation for captures
//
// F4/F7 — DMA1 streams (unique within DMA1; DShot burst uses DMA2 for TIM1,
//         DMA1 stream 4 for TIM3, DMA1 stream 0 for TIM4 — no collision):
//    CH1 Stream5/CH3   CH2 Stream6/CH3   CH3 Stream1/CH3   CH4 Stream7/CH3
// G4  — DMA2 channels via DMAMUX (DShot burst uses DMA1):
//    CH1 Ch1           CH2 Ch2           CH3 Ch3           CH4 Ch4
// H7  — DMA2 streams via DMAMUX (DShot burst uses DMA1):
//    CH1 Stream0       CH2 Stream1       CH3 Stream2       CH4 Stream3
// ---------------------------------------------------------------------------
#if defined(STM32F4xx) || defined(STM32F7xx)
static const uint32_t cap_dma_stream[4] = {
  LL_DMA_STREAM_5, LL_DMA_STREAM_6, LL_DMA_STREAM_1, LL_DMA_STREAM_7,
};
#define CAP_DMA              DMA1
#define CAP_DMA_CHANNEL_SEL  LL_DMA_CHANNEL_3

#elif defined(STM32G4xx)
static const uint32_t cap_dma_stream[4] = {
  LL_DMA_CHANNEL_1, LL_DMA_CHANNEL_2, LL_DMA_CHANNEL_3, LL_DMA_CHANNEL_4,
};
static const uint32_t cap_dmamux_req[4] = {
  LL_DMAMUX_REQ_TIM2_CH1, LL_DMAMUX_REQ_TIM2_CH2,
  LL_DMAMUX_REQ_TIM2_CH3, LL_DMAMUX_REQ_TIM2_CH4,
};
#define CAP_DMA              DMA2

#elif defined(STM32H7xx)
static const uint32_t cap_dma_stream[4] = {
  LL_DMA_STREAM_0, LL_DMA_STREAM_1, LL_DMA_STREAM_2, LL_DMA_STREAM_3,
};
static const uint32_t cap_dmamux_req[4] = {
  LL_DMAMUX1_REQ_TIM2_CH1, LL_DMAMUX1_REQ_TIM2_CH2,
  LL_DMAMUX1_REQ_TIM2_CH3, LL_DMAMUX1_REQ_TIM2_CH4,
};
#define CAP_DMA              DMA2
#endif

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------
static void initCaptureTimer(void);
static void initCaptureChannels(void);
static void initCaptureDMA(int ch);
static void armAllCaptures(void);
static int  stopAndCount(int ch);
static void enableAllCaptureDMAReqs(void);
static void disableAllCaptureDMAReqs(void);
static void clearCaptureDMAFlag(int ch);
static bool verifyPacket(int ch, uint16_t expected_throttle, int num_captured,
                         const char *label);
static int  runPhaseA(int &passes, int &fails);
static int  runPhaseB(int &passes, int &fails);

// ===========================================================================
// setup / loop
// ===========================================================================
void setup()
{
  Serial.begin(115200);
  while (!Serial) { }
  CI_LOGF("DShot_Validation (%s)\n", BOARD_NAME);

  initCaptureTimer();
  initCaptureChannels();
  for (int c = 0; c < NUM_CAP; c++) initCaptureDMA(c);

  int passes = 0, fails = 0;

#if HAS_TIM4_BURST
  CI_LOG("--- Phase A: concurrent burst on TIM1 + TIM3 + TIM4 ---\n");
#else
  CI_LOG("--- Phase A: concurrent burst on TIM1 + TIM3 ---\n");
#endif
  runPhaseA(passes, fails);

  CI_LOG("--- Phase B: per-channel on TIM1_CH1 + TIM3_CH3 ---\n");
  runPhaseB(passes, fails);

  CI_LOGF("=== Results: %d passed, %d failed ===\n", passes, fails);
  if (fails == 0) {
    CI_LOG("*STOP*\n");
  } else {
    CI_LOG("FAIL\n*STOP*\n");
  }
}

void loop() { delay(1000); }

// ===========================================================================
// Phase A — concurrent burst (captured + decoy motors per timer)
// ===========================================================================
static int runPhaseA(int &passes, int &fails)
{
  DShotOutput dshot;
  for (int i = 0; i < NUM_A; i++) {
    int r = dshot.AddMotor(phaseA_motors[i].timer, phaseA_motors[i].pin,
                           phaseA_motors[i].channel, DShot::DSHOT600);
    if (r < 0) {
      CI_LOGF("FAIL: AddMotor(%s) returned %d\n", phaseA_label[i], r);
      fails++;
      dshot.Release();
      return -1;
    }
  }

  // First Send() triggers library DMA allocation (burst vs per-channel).
  dshot.SetAllThrottle(0);
  dshot.Send();
  uint32_t deadline = millis() + 100;
  while (!dshot.IsTransferComplete() && millis() < deadline) { }

  bool tim1_burst = (TIM1->DCR != 0);
  bool tim3_burst = (TIM3->DCR != 0);
  CI_LOGF("  TIM1 DCR=0x%lX burst=%s\n", (unsigned long)TIM1->DCR,
          tim1_burst ? "ACTIVE" : "INACTIVE");
  CI_LOGF("  TIM3 DCR=0x%lX burst=%s\n", (unsigned long)TIM3->DCR,
          tim3_burst ? "ACTIVE" : "INACTIVE");
#if HAS_TIM4_BURST
  bool tim4_burst = (TIM4->DCR != 0);
  CI_LOGF("  TIM4 DCR=0x%lX burst=%s\n", (unsigned long)TIM4->DCR,
          tim4_burst ? "ACTIVE" : "INACTIVE");
  if (!(tim1_burst && tim3_burst && tim4_burst)) {
    CI_LOG("FAIL: expected burst active on TIM1+TIM3+TIM4\n");
    fails++;
    dshot.Release();
    return -1;
  }
#else
  if (!(tim1_burst && tim3_burst)) {
    CI_LOG("FAIL: expected burst active on TIM1+TIM3\n");
    fails++;
    dshot.Release();
    return -1;
  }
#endif

  for (int t = 0; t < NUM_TESTS; t++) {
    uint16_t throttle = test_throttles[t];

    // Captured motors get the test value; decoys get an offset-derived value
    // so a stride/interleaving bug would decode a mismatched throttle.
    for (int m = 0; m < NUM_A; m++) {
      if (phaseA_motors[m].cap_channel >= 0) {
        dshot.SetThrottle(m, throttle);
      } else {
        uint16_t decoy = throttle + DECOY_OFFSET + (uint16_t)(m * 50);
        if (decoy > 2047) decoy -= 2048;
        dshot.SetThrottle(m, decoy);
      }
    }

    armAllCaptures();
    dshot.Send();

    deadline = millis() + 100;
    while (!dshot.IsTransferComplete() && millis() < deadline) { }
    delayMicroseconds(100);

    disableAllCaptureDMAReqs();

    bool row_ok = true;
    for (int m = 0; m < NUM_A; m++) {
      int ch = phaseA_motors[m].cap_channel;
      if (ch < 0) continue;
      int n = stopAndCount(ch);
      if (n < MIN_EDGES) {
        CI_LOGF("  FAIL throttle=%u %s: only %d edges\n",
                throttle, phaseA_label[m], n);
        row_ok = false;
        continue;
      }
      if (!verifyPacket(ch, throttle, n, phaseA_label[m])) {
        row_ok = false;
      }
    }

    if (row_ok) {
#if HAS_TIM4_BURST
      CI_LOGF("  PASS throttle=%u  TIM1|TIM3|TIM4\n", throttle);
#else
      CI_LOGF("  PASS throttle=%u  TIM1|TIM3\n", throttle);
#endif
      passes++;
    } else {
      fails++;
    }
  }

  dshot.Release();
  return 0;
}

// ===========================================================================
// Phase B — per-channel on TIM1_CH1 + TIM3_CH3
// ===========================================================================
static int runPhaseB(int &passes, int &fails)
{
  // DShotOutput::Release() leaves TIM->DCR set from the prior burst run. Zero
  // it here so the mode indicator below is trustworthy. Stale DCR does not
  // corrupt per-channel output (the library ignores DCR on that path).
  TIM1->DCR = 0;
  TIM3->DCR = 0;

  DShotOutput dshot;
  for (int i = 0; i < NUM_B; i++) {
    int r = dshot.AddMotor(phaseB_motors[i].timer, phaseB_motors[i].pin,
                           phaseB_motors[i].channel, DShot::DSHOT600);
    if (r < 0) {
      CI_LOGF("FAIL: AddMotor(%s) returned %d\n", phaseB_label[i], r);
      fails++;
      dshot.Release();
      return -1;
    }
  }

  dshot.SetAllThrottle(0);
  dshot.Send();
  uint32_t deadline = millis() + 100;
  while (!dshot.IsTransferComplete() && millis() < deadline) { }

  CI_LOGF("  TIM1 DCR=0x%lX mode=%s\n", (unsigned long)TIM1->DCR,
          (TIM1->DCR == 0) ? "per-channel" : "burst");
  CI_LOGF("  TIM3 DCR=0x%lX mode=%s\n", (unsigned long)TIM3->DCR,
          (TIM3->DCR == 0) ? "per-channel" : "burst");

  for (int t = 0; t < NUM_TESTS; t++) {
    uint16_t throttle = test_throttles[t];
    for (int m = 0; m < NUM_B; m++) {
      dshot.SetThrottle(m, throttle);
    }

    armAllCaptures();
    dshot.Send();

    deadline = millis() + 100;
    while (!dshot.IsTransferComplete() && millis() < deadline) { }
    delayMicroseconds(100);

    disableAllCaptureDMAReqs();

    bool row_ok = true;
    for (int m = 0; m < NUM_B; m++) {
      int ch = phaseB_motors[m].cap_channel;
      int n = stopAndCount(ch);
      if (n < MIN_EDGES) {
        CI_LOGF("  FAIL throttle=%u %s: only %d edges\n",
                throttle, phaseB_label[m], n);
        row_ok = false;
        continue;
      }
      if (!verifyPacket(ch, throttle, n, phaseB_label[m])) {
        row_ok = false;
      }
    }

    if (row_ok) {
      CI_LOGF("  PASS throttle=%u  %s|TIM3_CH3\n", throttle, TIM1_LABEL_A);
      passes++;
    } else {
      fails++;
    }
  }

  dshot.Release();
  return 0;
}

// ===========================================================================
// TIM2 free-running 32-bit capture timebase
// ===========================================================================
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

// ===========================================================================
// Configure each capture pin as TIM2 alternate-function input, both-edge IC
// ===========================================================================
static void initCaptureChannels(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  LL_GPIO_InitTypeDef gpio_init;
  LL_GPIO_StructInit(&gpio_init);
  gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
  gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  gpio_init.Pull = LL_GPIO_PULL_DOWN;

  for (int c = 0; c < NUM_CAP; c++) {
    gpio_init.Pin = cap_pins[c].ll_pin;
    gpio_init.Alternate = cap_pins[c].af;
    LL_GPIO_Init(cap_pins[c].port, &gpio_init);
  }

  LL_TIM_IC_InitTypeDef ic_init;
  LL_TIM_IC_StructInit(&ic_init);
  ic_init.ICActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;
  ic_init.ICPrescaler = LL_TIM_ICPSC_DIV1;
  ic_init.ICFilter = LL_TIM_IC_FILTER_FDIV1;
  ic_init.ICPolarity = LL_TIM_IC_POLARITY_BOTHEDGE;

  for (int c = 0; c < NUM_CAP; c++) {
    LL_TIM_IC_Init(TIM2, cap_ll_tim_ch[c], &ic_init);
    LL_TIM_CC_EnableChannel(TIM2, cap_ll_tim_ch[c]);
  }
}

// ===========================================================================
// Per-capture DMA init — family branches
// ===========================================================================
static void initCaptureDMA(int ch)
{
  // TIM2->CCR1..CCR4 are contiguous — index by channel.
  volatile uint32_t *ccr = &TIM2->CCR1 + ch;

  LL_DMA_InitTypeDef dma_init;
  LL_DMA_StructInit(&dma_init);
  dma_init.PeriphOrM2MSrcAddress = (uint32_t)ccr;
  dma_init.MemoryOrM2MDstAddress = (uint32_t)capture_buf[ch];
  dma_init.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  dma_init.NbData = CAPTURE_BUF_SIZE;
  dma_init.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  dma_init.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dma_init.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
  dma_init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
  dma_init.Mode = LL_DMA_MODE_NORMAL;
  dma_init.Priority = LL_DMA_PRIORITY_HIGH;

#if defined(STM32F4xx) || defined(STM32F7xx)
  __HAL_RCC_DMA1_CLK_ENABLE();
  LL_DMA_DisableStream(CAP_DMA, cap_dma_stream[ch]);
  LL_DMA_DeInit(CAP_DMA, cap_dma_stream[ch]);
  dma_init.Channel = CAP_DMA_CHANNEL_SEL;
  dma_init.FIFOMode = LL_DMA_FIFOMODE_DISABLE;
  dma_init.MemBurst = LL_DMA_MBURST_SINGLE;
  dma_init.PeriphBurst = LL_DMA_PBURST_SINGLE;
  LL_DMA_Init(CAP_DMA, cap_dma_stream[ch], &dma_init);

#elif defined(STM32G4xx)
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  LL_DMA_DisableChannel(CAP_DMA, cap_dma_stream[ch]);
  LL_DMA_DeInit(CAP_DMA, cap_dma_stream[ch]);
  dma_init.PeriphRequest = cap_dmamux_req[ch];
  LL_DMA_Init(CAP_DMA, cap_dma_stream[ch], &dma_init);

#elif defined(STM32H7xx)
  __HAL_RCC_DMA2_CLK_ENABLE();
#if defined(__HAL_RCC_DMAMUX1_CLK_ENABLE)
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
#endif
  LL_DMA_DisableStream(CAP_DMA, cap_dma_stream[ch]);
  LL_DMA_DeInit(CAP_DMA, cap_dma_stream[ch]);
  dma_init.PeriphRequest = cap_dmamux_req[ch];
  dma_init.FIFOMode = LL_DMA_FIFOMODE_DISABLE;
  dma_init.MemBurst = LL_DMA_MBURST_SINGLE;
  dma_init.PeriphBurst = LL_DMA_PBURST_SINGLE;
  LL_DMA_Init(CAP_DMA, cap_dma_stream[ch], &dma_init);
#endif
}

// ===========================================================================
// Per-family DMA flag-clear (TC for stream-based, GI for G4 channels)
// ===========================================================================
static void clearCaptureDMAFlag(int ch)
{
#if defined(STM32F4xx) || defined(STM32F7xx)
  switch (cap_dma_stream[ch]) {
    case LL_DMA_STREAM_0: LL_DMA_ClearFlag_TC0(CAP_DMA); break;
    case LL_DMA_STREAM_1: LL_DMA_ClearFlag_TC1(CAP_DMA); break;
    case LL_DMA_STREAM_2: LL_DMA_ClearFlag_TC2(CAP_DMA); break;
    case LL_DMA_STREAM_3: LL_DMA_ClearFlag_TC3(CAP_DMA); break;
    case LL_DMA_STREAM_4: LL_DMA_ClearFlag_TC4(CAP_DMA); break;
    case LL_DMA_STREAM_5: LL_DMA_ClearFlag_TC5(CAP_DMA); break;
    case LL_DMA_STREAM_6: LL_DMA_ClearFlag_TC6(CAP_DMA); break;
    case LL_DMA_STREAM_7: LL_DMA_ClearFlag_TC7(CAP_DMA); break;
  }

#elif defined(STM32G4xx)
  switch (ch) {
    case 0: LL_DMA_ClearFlag_GI1(CAP_DMA); break;
    case 1: LL_DMA_ClearFlag_GI2(CAP_DMA); break;
    case 2: LL_DMA_ClearFlag_GI3(CAP_DMA); break;
    case 3: LL_DMA_ClearFlag_GI4(CAP_DMA); break;
  }

#elif defined(STM32H7xx)
  switch (cap_dma_stream[ch]) {
    case LL_DMA_STREAM_0: LL_DMA_ClearFlag_TC0(CAP_DMA); break;
    case LL_DMA_STREAM_1: LL_DMA_ClearFlag_TC1(CAP_DMA); break;
    case LL_DMA_STREAM_2: LL_DMA_ClearFlag_TC2(CAP_DMA); break;
    case LL_DMA_STREAM_3: LL_DMA_ClearFlag_TC3(CAP_DMA); break;
  }
#endif
}

// ===========================================================================
// Arm all captures — clear buffers, clear flags, enable DMA + timer requests
// ===========================================================================
static void armAllCaptures(void)
{
  for (int ch = 0; ch < NUM_CAP; ch++) {
    for (int i = 0; i < CAPTURE_BUF_SIZE; i++) {
      capture_buf[ch][i] = 0;
    }
  }

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
  SCB_CleanDCache_by_Addr((uint32_t *)capture_buf, sizeof(capture_buf));
#endif

  for (int ch = 0; ch < NUM_CAP; ch++) {
    clearCaptureDMAFlag(ch);
    LL_DMA_SetDataLength(CAP_DMA, cap_dma_stream[ch], CAPTURE_BUF_SIZE);
#if defined(STM32G4xx)
    LL_DMA_EnableChannel(CAP_DMA, cap_dma_stream[ch]);
#else
    LL_DMA_EnableStream(CAP_DMA, cap_dma_stream[ch]);
#endif
  }

  LL_TIM_SetCounter(TIM2, 0);
  enableAllCaptureDMAReqs();
}

static void enableAllCaptureDMAReqs(void)
{
  LL_TIM_EnableDMAReq_CC1(TIM2);
  LL_TIM_EnableDMAReq_CC2(TIM2);
#if NUM_CAP >= 3
  LL_TIM_EnableDMAReq_CC3(TIM2);
#endif
#if NUM_CAP >= 4
  LL_TIM_EnableDMAReq_CC4(TIM2);
#endif
}

static void disableAllCaptureDMAReqs(void)
{
  LL_TIM_DisableDMAReq_CC1(TIM2);
  LL_TIM_DisableDMAReq_CC2(TIM2);
#if NUM_CAP >= 3
  LL_TIM_DisableDMAReq_CC3(TIM2);
#endif
#if NUM_CAP >= 4
  LL_TIM_DisableDMAReq_CC4(TIM2);
#endif
}

// ===========================================================================
// Stop one capture, return edges actually captured
// ===========================================================================
static int stopAndCount(int ch)
{
#if defined(STM32G4xx)
  LL_DMA_DisableChannel(CAP_DMA, cap_dma_stream[ch]);
#else
  LL_DMA_DisableStream(CAP_DMA, cap_dma_stream[ch]);
#endif
  uint32_t remaining = LL_DMA_GetDataLength(CAP_DMA, cap_dma_stream[ch]);
  return CAPTURE_BUF_SIZE - (int)remaining;
}

// ===========================================================================
// Verify one captured packet — lifted from DShot_Burst_Verification:348
// ===========================================================================
static bool verifyPacket(int ch, uint16_t expected_throttle, int num_captured,
                         const char *label)
{
  uint16_t expected = DShot::encodePacket(expected_throttle, false);
  volatile uint32_t *buf = capture_buf[ch];

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
  SCB_InvalidateDCache_by_Addr((uint32_t *)buf, sizeof(capture_buf[0]));
#endif

  uint32_t tim_clk = DShot::getTimerClockFreq(TIM2);
  uint32_t bit0_ticks = (uint32_t)((uint64_t)DShot::BIT_0_DUTY * tim_clk / DShot::DSHOT600);
  uint32_t bit1_ticks = (uint32_t)((uint64_t)DShot::BIT_1_DUTY * tim_clk / DShot::DSHOT600);
  uint32_t threshold = (bit0_ticks + bit1_ticks) / 2;

  int start = 0;
  while (start < num_captured && buf[start] == 0) start++;
  if (start + 32 > num_captured) {
    CI_LOGF("    %s: not enough valid edges (start=%d)\n", label, start);
    return false;
  }

  uint16_t reconstructed = 0;
  int bits_decoded = 0;
  for (int i = 0; i < 32 && (start + i + 1) < num_captured; i += 2) {
    uint32_t rising  = buf[start + i];
    uint32_t falling = buf[start + i + 1];
    if (falling == 0 || rising == 0) break;
    uint32_t high_time = falling - rising;
    reconstructed <<= 1;
    if (high_time > threshold) reconstructed |= 1;
    bits_decoded++;
  }

  if (bits_decoded < 16) {
    CI_LOGF("    %s: only decoded %d bits\n", label, bits_decoded);
    return false;
  }

  if (reconstructed != expected) {
    CI_LOGF("    %s: expected 0x%04X got 0x%04X\n",
            label, expected, reconstructed);
    return false;
  }

  uint32_t bit_period_ticks =
      (uint32_t)((uint64_t)DShot::BIT_PERIOD * tim_clk / DShot::DSHOT600);
  for (int i = 0; i < 15 && (start + 2 * (i + 1)) < num_captured; i++) {
    uint32_t period = buf[start + 2 * (i + 1)] - buf[start + 2 * i];
    if (period == 0) break;
    uint32_t lower = bit_period_ticks * 80 / 100;
    uint32_t upper = bit_period_ticks * 120 / 100;
    if (period < lower || period > upper) {
      CI_LOGF("    %s: timing out at bit %d period=%lu expected ~%lu\n",
              label, i, (unsigned long)period, (unsigned long)bit_period_ticks);
      return false;
    }
  }
  return true;
}
