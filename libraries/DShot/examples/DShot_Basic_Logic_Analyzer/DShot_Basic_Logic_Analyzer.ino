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
 * Target: NUCLEO-G474RE (the probe map below is specific to that board's
 *         Arduino + Morpho connector layout).
 *
 * Motor <-> MCU pin <-> connector:
 *   Motor 1: PB4 (TIM3_CH1) — CN9-5  (D5)
 *   Motor 2: PB5 (TIM3_CH2) — CN9-6  (D4)
 *   Motor 3: PB6 (TIM4_CH1) — CN5-3  (D10)
 *   Motor 4: PB7 (TIM4_CH2) — CN7-21 (Morpho only — not on Arduino header)
 *
 * Saleae Logic 16 probe map (5 signals + GND):
 *   Ch 0  Motor 1   PB4   CN9-5 (D5)
 *   Ch 1  Motor 2   PB5   CN9-6 (D4)
 *   Ch 2  Motor 3   PB6   CN5-3 (D10)
 *   Ch 3  Motor 4   PB7   CN7-21 (Morpho)
 *   Ch 4  LED ref   PA5   CN5-6 (D13) — 5 Hz heartbeat (edge every 100 ms)
 *   GND             —     CN6-6 or CN7-8
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
 * The jumpers fitted for software loopback validation on this rig
 * (PB4->PA0, PA8->PA1, PB0->PB10, PB6->PA10) can stay in place. They
 * add a high-Z MCU input to PB4/PB6 which is electrically invisible to
 * DShot600 and does not affect Saleae probe signal integrity.
 *
 * See DSHOT_SALEAE_TEST.md in the workspace root for the full procedure
 * and troubleshooting guide.
 */

#include <DShot.h>

// 0 = MODE_FIXED (decoder PASS-criteria capture)
// 1 = MODE_SWEEP (throttle 0..2047 wrap check)
#ifndef DSHOT_LA_SWEEP
  #define DSHOT_LA_SWEEP 1
#endif

#if !defined(ARDUINO_NUCLEO_G474RE)
  #error "DShot_Basic_Logic_Analyzer: probe map/pins are NUCLEO-G474RE specific"
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

  pinMode(LED_BUILTIN, OUTPUT);

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
  digitalWrite(LED_BUILTIN, (millis() / 100UL) & 1UL);
}
