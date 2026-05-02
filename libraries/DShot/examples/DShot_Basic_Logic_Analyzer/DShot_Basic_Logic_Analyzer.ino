/**
 * DShot_Basic_Logic_Analyzer - 4-motor DShot600 output configured for
 * external decoding with a Saleae Logic 16 + logic-dshot analyzer.
 *
 * Same 4-motor layout as DShot_Basic, but the throttle behaviour is
 * tailored for a decoder-friendly capture rather than a throttle demo:
 *
 *   MODE_FIXED  (default) — every frame encodes throttle=1000, telemetry=0.
 *                            Use this to verify PASS criteria (decoder shows
 *                            same throttle on every frame, CRC valid, bit
 *                            timing within tolerance) — see the pass table
 *                            below.
 *   MODE_SWEEP  — throttle increments 0..2047 and wraps, one step per
 *                            frame. Use this to verify monotonic increment
 *                            and clean wrap in the decoder output.
 *
 * Toggle by editing the DSHOT_LA_SWEEP define at the top of this file;
 * reflash between captures.
 *
 * Targets: NUCLEO-G474RE, NUCLEO-F411RE, NUCLEO-F405RG (Nucleo-64),
 *          NUCLEO-H753ZI (Nucleo-144). Same MCU pins on every board —
 *          the probe wires connect to different silkscreen positions
 *          on Nucleo-144 vs Nucleo-64. Find pins by the raw Pxx
 *          silkscreen label.
 *
 * Motor <-> MCU pin (timer assignment is identical across all targets):
 *   Motor 1: PB4 (TIM3_CH1)
 *   Motor 2: PB5 (TIM3_CH2)
 *   Motor 3: PB6 (TIM4_CH1)
 *   Motor 4: PB7 (TIM4_CH2)
 *   LED ref: PA5 (LED_BUILTIN equivalent on G474RE/F411RE; on H753 the
 *            sketch still drives PA5 via the LED_BUILTIN macro — see
 *            note below for H753 LED specifics)
 *
 * Saleae Logic 16 probe map — Nucleo-64 (G474RE / F411RE / F405RG):
 *   Ch 0  Motor 1   PB4   CN9-5 (D5)
 *   Ch 1  Motor 2   PB5   CN9-6 (D4)
 *   Ch 2  Motor 3   PB6   CN5-3 (D10)
 *   Ch 3  Motor 4   PB7   CN7-21 (Morpho only — not on Arduino header)
 *   Ch 4  LED ref   PA5   CN5-6 (D13) — 5 Hz heartbeat
 *   GND             —     CN6-6 or CN7-8
 *
 *   F405RG note: this is a custom-rework board (F411 PCB with the
 *   F411 chip swapped for F405RG and an 8 MHz crystal added). LD2
 *   (PA5 LED) is dark on this rig because the LED wiring was
 *   disturbed by the rework — but PA5 itself still toggles, so the
 *   Saleae Ch 4 trace shows the heartbeat normally. VCOM is also
 *   broken on this rig; chip-alive verification comes from Saleae
 *   activity on PB4..PB7 + PA5, not from Serial output.
 *
 * Saleae Logic 16 probe map — Nucleo-144 (H753ZI):
 *   Ch 0  Motor 1   PB4   Zio (D25)        — not on Arduino-compatible top
 *   Ch 1  Motor 2   PB5   Arduino (D11)
 *   Ch 2  Motor 3   PB6   Arduino (D1)
 *   Ch 3  Motor 4   PB7   Arduino (D0)     — also drives LED_BLUE; LED
 *                                            flickers during DShot output,
 *                                            harmless.
 *   Ch 4  LED ref   PA5   Arduino (D13)    — 5 Hz heartbeat
 *   GND             —     any GND pin on Arduino or Zio
 *
 * The heartbeat is driven on PA5 directly (not via LED_BUILTIN) so the
 * Saleae reference edges show up at the same MCU pin on every target.
 * On G474RE / F411RE this is the on-board green LED (LD2). On H753ZI
 * PA5 has no on-board LED — the LD1 LED_GREEN on H753 is PB0 — so the
 * heartbeat output is the pin only; watch the Saleae trace.
 *
 * Saleae capture config:
 *   Sample rate   50 MS/s
 *   Trigger       rising edge on Ch 0, 0% pre-trigger
 *   Duration      500 us (MODE_FIXED) / 500 ms (MODE_SWEEP)
 *   Glitch filter off
 *   Add one "DShot" analyzer per motor channel (Ch 0..3), bit rate
 *   DShot600, invert off.
 *
 * PASS criteria (MODE_FIXED):
 *   Decoder throttle   1000 on every frame, every motor channel
 *   Decoder telemetry  0
 *   Decoder CRC        valid on 100% of frames
 *   Bit period         1.667 us +/- 5%
 *   Logic-1 high time  1.250 us +/- 5%  (75% duty)
 *   Logic-0 high time  0.625 us +/- 5%  (37.5% duty)
 *   Frame width        ~26.7 us
 *   Ch 0<->1 skew      <10 ns (same TIM3 burst)
 *   Ch 2<->3 skew      <10 ns (same TIM4 burst)
 *   Ch 0<->2 skew      <2 us  (TIM3 vs TIM4, started sequentially)
 *
 * The jumpers fitted for the DShot_Validation loopback fixture on
 * any rig can stay in place — they add high-Z MCU inputs to the
 * DShot output pins, electrically invisible to DShot600 and to
 * Saleae probe integrity:
 *   G474RE: PB4->PA0, PA8->PA1, PB0->PB10, PB6->PA10
 *   F411RE: PB4->PA0, PA8->PA1, PB0->PB10
 *   F405RG: no DShot_Validation fixture on the breadboard rig
 *   H753ZI: PB4->PA0, PE11->PB3, PB0->PB10, PB6->PB11
 *
 * On F411RE specifically, this sketch closes a runtime-validation gap
 * for tim_up_map[TIM4]: that table entry (DMA1 Stream 6 Ch 2 = TIM4_UP)
 * cannot be exercised by DShot_Validation on F411 because TIM2_CH2
 * input capture also lives on DMA1 Stream 6 (silicon-shared), so the
 * loopback test drops TIM4 burst on F411. This sketch uses external
 * Saleae capture instead — no TIM2 capture, no Stream 6 contention,
 * TIM4 burst output goes through the new tim_up_map and is verified
 * directly via waveform decode.
 */

#include <DShot.h>

// 0 = MODE_FIXED (decoder PASS-criteria capture)
// 1 = MODE_SWEEP (throttle 0..2047 wrap check)
#ifndef DSHOT_LA_SWEEP
  #define DSHOT_LA_SWEEP 0
#endif

#if !defined(ARDUINO_NUCLEO_G474RE) && !defined(ARDUINO_NUCLEO_F411RE) \
    && !defined(ARDUINO_NUCLEO_F405RG) && !defined(ARDUINO_NUCLEO_H753ZI)
  #error "DShot_Basic_Logic_Analyzer: targets NUCLEO-G474RE, NUCLEO-F411RE, NUCLEO-F405RG, or NUCLEO-H753ZI"
#endif

static constexpr uint16_t FIXED_THROTTLE = 1000;

DShotOutput motors;

#if DSHOT_LA_SWEEP
static uint16_t throttle = 0;
#endif

void setup()
{
  Serial.begin(115200);
  Serial.println("DShot_Basic_Logic_Analyzer: initializing");
#if DSHOT_LA_SWEEP
  Serial.println("  mode: SWEEP (0..2047)");
#else
  Serial.print("  mode: FIXED throttle=");
  Serial.println(FIXED_THROTTLE);
#endif

  // Drive PA5 directly (rather than LED_BUILTIN) so the heartbeat
  // appears at the same MCU pin on every target. On G474RE/F411RE
  // this is also the on-board LED (LD2); on H753ZI LED_BUILTIN is
  // PB0, but we want the Saleae reference at PA5 regardless.
  pinMode(PA5, OUTPUT);

  motors.AddMotor(TIM3, PB4, 1, DShot::DSHOT600);
  motors.AddMotor(TIM3, PB5, 2, DShot::DSHOT600);
  motors.AddMotor(TIM4, PB6, 1, DShot::DSHOT600);
  motors.AddMotor(TIM4, PB7, 2, DShot::DSHOT600);

  Serial.print("  motors: ");
  Serial.println(motors.GetNumMotors());
}

void loop()
{
#if DSHOT_LA_SWEEP
  motors.SetAllThrottle(throttle);
  throttle = (throttle + 1) & 0x7FFU;
#else
  motors.SetAllThrottle(FIXED_THROTTLE);
#endif
  motors.Send();

  // 200 us pacing -> ~220 us frame cadence, ~5 kHz. Well under ESC rate
  // limits and matches the frame-cadence PASS criterion above.
  delayMicroseconds(200);

  // 5 Hz stateless heartbeat (edge every 100 ms) — serves as LA Ch 4
  // reference. Fast enough that any reasonable capture window catches
  // at least one edge.
  digitalWrite(PA5, (millis() / 100UL) & 1UL);
}
