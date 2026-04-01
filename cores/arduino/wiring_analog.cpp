/*
 * wiring_analog.cpp — Analog I/O with Pin type
 *
 * Renamed from .c to .cpp (Pin is a C++ struct).
 * Uses explicit pin.toPinName() at the HAL boundary.
 * No analogInputToPinName / digitalPinToPinName indirection.
 */

#include "Arduino.h"
#include "stm32/PinConfigured.h"
#include "stm32/analog.h"

#if (defined(HAL_DAC_MODULE_ENABLED) && !defined(HAL_DAC_MODULE_ONLY)) ||\
    (defined(HAL_TIM_MODULE_ENABLED) && !defined(HAL_TIM_MODULE_ONLY))
uint32_t g_anOutputPinConfigured[MAX_NB_PORT] = {0};
#endif

#if !defined(ADC_RESOLUTION_16B)
#define MAX_ADC_RESOLUTION 12
#else
#define MAX_ADC_RESOLUTION 16
#endif
#define MAX_PWM_RESOLUTION 16

static int _readResolution = ADC_RESOLUTION;
static int _internalReadResolution =
#if ADC_RESOLUTION > MAX_ADC_RESOLUTION
  MAX_ADC_RESOLUTION
#else

#ifdef ADC_RESOLUTION_12B

#if ADC_RESOLUTION <= 6 && defined(ADC_RESOLUTION_6B)
  6
#elif ADC_RESOLUTION <= 8
  8
#elif ADC_RESOLUTION <= 10
  10
#elif ADC_RESOLUTION <= 12
  12
#elif ADC_RESOLUTION <= 14 && defined(ADC_RESOLUTION_14B)
  14
#elif defined(ADC_RESOLUTION_16B)
  16
#endif
#else /* ADC_RESOLUTION_12B */
  12
#endif /* ADC_RESOLUTION_12B */
#endif /* ADC_RESOLUTION > MAX_ADC_RESOLUTION */
  ;

static int _writeResolution = PWM_RESOLUTION;
static int _internalWriteResolution =
#if PWM_RESOLUTION > MAX_PWM_RESOLUTION
  MAX_PWM_RESOLUTION
#else
  PWM_RESOLUTION
#endif
  ;

static uint32_t _writeFreq = PWM_FREQUENCY;

void analogReadResolution(int res)
{
  if ((res > 0) && (res <= 32)) {
    _readResolution = res;
    _internalReadResolution = _readResolution;
    if (_readResolution > MAX_ADC_RESOLUTION) {
      _internalReadResolution = MAX_ADC_RESOLUTION;
    } else {
#ifdef ADC_RESOLUTION_12B
#ifdef ADC_RESOLUTION_6B
      if (_internalReadResolution <= 6) {
        _internalReadResolution = 6;
      } else
#endif
        if (_internalReadResolution <= 8) {
          _internalReadResolution = 8;
        } else if (_internalReadResolution <= 10) {
          _internalReadResolution = 10;
        } else if (_internalReadResolution <= 12) {
          _internalReadResolution = 12;
        }
#ifdef ADC_RESOLUTION_14B
        else if (_internalReadResolution <= 14) {
          _internalReadResolution = 14;
        }
#endif
#ifdef ADC_RESOLUTION_16B
        else if (_internalReadResolution <= 16) {
          _internalReadResolution = 16;
        }
#endif
#else
      _internalReadResolution = 12;
#endif
    }
  } else {
    Error_Handler();
  }
}

void analogWriteResolution(int res)
{
  if ((res > 0) && (res <= 32)) {
    _writeResolution = res;
    if (_writeResolution > MAX_PWM_RESOLUTION) {
      _internalWriteResolution = MAX_PWM_RESOLUTION;
    } else {
      _internalWriteResolution = _writeResolution;
    }
  } else {
    Error_Handler();
  }
}

void analogWriteFrequency(uint32_t freq)
{
  _writeFreq = freq;
}

static inline uint32_t mapResolution(uint32_t value, uint32_t from, uint32_t to)
{
  if (from != to) {
    if (from > to) {
      value = (value < (uint32_t)(1 << (from - to))) ? 0 : ((value + 1) >> (from - to)) - 1;
    } else {
      if (value != 0) {
        value = ((value + 1) << (to - from)) - 1;
      }
    }
  }
  return value;
}

void analogReference(eAnalogReference ulMode)
{
  UNUSED(ulMode);
}

uint32_t analogRead(Pin pin)
{
  uint32_t value = 0;
#if defined(HAL_ADC_MODULE_ENABLED) && !defined(HAL_ADC_MODULE_ONLY)
  if (pin.IsValid()) {
    PinName p = pin.toPinName();
    value = adc_read_value(p, _internalReadResolution);
    value = mapResolution(value, _internalReadResolution, _readResolution);
  }
#else
  UNUSED(pin);
#endif
  return value;
}

void analogOutputInit(void)
{
}

void analogWrite(Pin pin, uint32_t ulValue)
{
  if (!pin.IsValid()) return;
  PinName p = pin.toPinName();

#if defined(HAL_DAC_MODULE_ENABLED) && !defined(HAL_DAC_MODULE_ONLY)
  uint8_t do_init = 0;
#endif

#if defined(HAL_DAC_MODULE_ENABLED) && !defined(HAL_DAC_MODULE_ONLY)
  if (pin_in_pinmap(p, PinMap_DAC)) {
    if (is_pin_configured(p, g_anOutputPinConfigured) == false) {
      do_init = 1;
      set_pin_configured(p, g_anOutputPinConfigured);
    }
    ulValue = mapResolution(ulValue, _writeResolution, DACC_RESOLUTION);
    dac_write_value(p, ulValue, do_init);
  } else
#endif
#if defined(HAL_TIM_MODULE_ENABLED) && !defined(HAL_TIM_MODULE_ONLY)
    if (pin_in_pinmap(p, PinMap_TIM)) {
      if (is_pin_configured(p, g_anOutputPinConfigured) == false) {
        set_pin_configured(p, g_anOutputPinConfigured);
      }
      ulValue = mapResolution(ulValue, _writeResolution, _internalWriteResolution);
      pwm_start(p, _writeFreq, ulValue, (TimerCompareFormat_t)_internalWriteResolution);
    } else
#endif
    {
      // Digital fallback
      pinMode(pin, OUTPUT);
      ulValue = mapResolution(ulValue, _writeResolution, 8);
      if (ulValue < 128) {
        digitalWrite(pin, LOW);
      } else {
        digitalWrite(pin, HIGH);
      }
    }
}
