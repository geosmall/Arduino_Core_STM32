/*
 * wiring_analog.h — Analog I/O declarations
 *
 * Non-pin functions (resolution, frequency, reference) are declared here
 * with C linkage. Pin-typed analogRead/analogWrite are declared in
 * Arduino.h under #ifdef __cplusplus (same pattern as digital I/O).
 */

#ifndef _WIRING_ANALOG_
#define _WIRING_ANALOG_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _eAnalogReference {
  AR_DEFAULT,
} eAnalogReference;

void analogReference(eAnalogReference ulMode);
void analogReadResolution(int res);
void analogWriteResolution(int res);
void analogWriteFrequency(uint32_t freq);
void analogOutputInit(void);

#ifdef __cplusplus
}
#endif

#endif /* _WIRING_ANALOG_ */
