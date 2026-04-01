/*
 * wiring_digital.cpp — Digital I/O with Pin type
 *
 * Renamed from .c to .cpp (Pin is a C++ struct).
 * Uses explicit pin.toPinName() at the HAL boundary.
 */

#include "Arduino.h"
#include "stm32/PinConfigured.h"
#include "stm32/analog.h"

// Defined in wiring_analog.cpp, shared for pinMode DAC/PWM teardown
extern uint32_t g_anOutputPinConfigured[MAX_NB_PORT];

void pinMode(Pin pin, uint32_t ulMode)
{
  if (!pin.IsValid()) return;
  PinName p = pin.toPinName();

  // If the pin supports PWM or DAC output, turn it off
#if (defined(HAL_DAC_MODULE_ENABLED) && !defined(HAL_DAC_MODULE_ONLY)) ||\
    (defined(HAL_TIM_MODULE_ENABLED) && !defined(HAL_TIM_MODULE_ONLY))
  if (is_pin_configured(p, g_anOutputPinConfigured)) {
#if defined(HAL_DAC_MODULE_ENABLED) && !defined(HAL_DAC_MODULE_ONLY)
    if (pin_in_pinmap(p, PinMap_DAC)) {
      dac_stop(p);
    } else
#endif
#if defined(HAL_TIM_MODULE_ENABLED) && !defined(HAL_TIM_MODULE_ONLY)
      if (pin_in_pinmap(p, PinMap_TIM)) {
        pwm_stop(p);
      }
#endif
    {
      reset_pin_configured(p, g_anOutputPinConfigured);
    }
  }
#endif
  switch (ulMode) {
    case INPUT:
      pin_function(p, STM_PIN_DATA(STM_MODE_INPUT, GPIO_NOPULL, 0));
      break;
    case INPUT_PULLUP:
      pin_function(p, STM_PIN_DATA(STM_MODE_INPUT, GPIO_PULLUP, 0));
      break;
    case INPUT_PULLDOWN:
      pin_function(p, STM_PIN_DATA(STM_MODE_INPUT, GPIO_PULLDOWN, 0));
      break;
    case INPUT_ANALOG:
      pin_function(p, STM_PIN_DATA(STM_MODE_ANALOG, GPIO_NOPULL, 0));
      break;
    case OUTPUT:
      pin_function(p, STM_PIN_DATA(STM_MODE_OUTPUT_PP, GPIO_NOPULL, 0));
      break;
    case OUTPUT_OPEN_DRAIN:
      pin_function(p, STM_PIN_DATA(STM_MODE_OUTPUT_OD, GPIO_NOPULL, 0));
      break;
    default:
      Error_Handler();
      break;
  }
}

void digitalWrite(Pin pin, uint32_t ulVal)
{
  digitalWriteFast(pin.toPinName(), ulVal);
}

int digitalRead(Pin pin)
{
  return digitalReadFast(pin.toPinName());
}

void digitalToggle(Pin pin)
{
  digitalToggleFast(pin.toPinName());
}
