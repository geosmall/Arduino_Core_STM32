/**
 * PWMOutputBank.cpp - Hardware timer-based PWM library for STM32 Arduino
 */

#include "PWMOutputBank.h"

PWMOutputBank::PWMOutputBank()
  : _timer(nullptr),
    _timer_instance(nullptr),
    _frequency_hz(0),
    _period_us(0),
    _initialized(false)
{
  // Initialize channel configs
  for (int i = 0; i < 4; i++) {
    _channels[i].active = false;
    _channels[i].current_us = 0;
  }
}

PWMOutputBank::~PWMOutputBank()
{
  if (_timer) {
    delete _timer;
    _timer = nullptr;
  }
}

uint32_t PWMOutputBank::GetTimerClockFreq()
{
  uint32_t timer_clock = 0;

  // Determine which APB bus the timer is on
  if (_timer_instance == TIM1
#ifdef TIM8
      || _timer_instance == TIM8
#endif
#ifdef TIM9
      || _timer_instance == TIM9
#endif
#ifdef TIM10
      || _timer_instance == TIM10
#endif
#ifdef TIM11
      || _timer_instance == TIM11
#endif
#ifdef TIM15
      || _timer_instance == TIM15
#endif
#ifdef TIM16
      || _timer_instance == TIM16
#endif
#ifdef TIM17
      || _timer_instance == TIM17
#endif
     ) {
    // APB2 timers
    timer_clock = HAL_RCC_GetPCLK2Freq();

    // RCC clock tree: if APB2 prescaler != 1, timer clock is 2x PCLK2
#if defined(STM32H7xx)
    // H7 series: D2CFGR register with D2PPRE2 bits
    if ((RCC->D2CFGR & RCC_D2CFGR_D2PPRE2) != 0) {
      timer_clock *= 2;
    }
#else
    // F4/F7/G4/etc: CFGR register with PPRE2 bits
    if ((RCC->CFGR & RCC_CFGR_PPRE2) != 0) {
      timer_clock *= 2;
    }
#endif
  } else {
    // APB1 timers (TIM2, TIM3, TIM4, TIM5, etc.)
    timer_clock = HAL_RCC_GetPCLK1Freq();

    // RCC clock tree: if APB1 prescaler != 1, timer clock is 2x PCLK1
#if defined(STM32H7xx)
    // H7 series: D2CFGR register with D2PPRE1 bits
    if ((RCC->D2CFGR & RCC_D2CFGR_D2PPRE1) != 0) {
      timer_clock *= 2;
    }
#else
    // F4/F7/G4/etc: CFGR register with PPRE1 bits
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != 0) {
      timer_clock *= 2;
    }
#endif
  }

  return timer_clock;
}

uint32_t PWMOutputBank::Calculate1MHzPrescaler()
{
  uint32_t timer_clock = GetTimerClockFreq();
  uint32_t target_tick_freq = 1000000; // 1 MHz

  // PSC = (timer_clock / target_freq) - 1
  uint32_t prescaler = (timer_clock / target_tick_freq) - 1;

  return prescaler;
}

bool PWMOutputBank::Init(TIM_TypeDef *timer, uint32_t frequency_hz)
{
  if (_initialized) {
    return false; // Already initialized
  }

  _timer_instance = timer;
  _frequency_hz = frequency_hz;

  // Calculate period in microseconds
  _period_us = 1000000 / frequency_hz;

  // Create HardwareTimer instance
  _timer = new HardwareTimer(_timer_instance);
  if (!_timer) {
    return false;
  }

  // Configure timer for 1 MHz tick rate
  uint32_t prescaler = Calculate1MHzPrescaler();
  _timer->setPrescaleFactor(prescaler);

  // Set overflow (period) in microseconds using MICROSEC_FORMAT
  _timer->setOverflow(_period_us, MICROSEC_FORMAT);

  _initialized = true;
  return true;
}

bool PWMOutputBank::AttachChannel(uint32_t channel, uint32_t pin,
                                  uint32_t min_us, uint32_t max_us)
{
  if (!_initialized) {
    return false;
  }

  if (channel < 1 || channel > 4) {
    return false; // Invalid channel
  }

  uint32_t ch_index = channel - 1;

  // Store channel configuration
  _channels[ch_index].pin = pin;
  _channels[ch_index].channel = channel;
  _channels[ch_index].min_us = min_us;
  _channels[ch_index].max_us = max_us;
  _channels[ch_index].current_us = min_us; // Start at minimum
  _channels[ch_index].active = true;

  // Configure timer channel for PWM output
  _timer->setMode(channel, TIMER_OUTPUT_COMPARE_PWM1, pin);

  // Set initial pulse width using MICROSEC_COMPARE_FORMAT
  _timer->setCaptureCompare(channel, min_us, MICROSEC_COMPARE_FORMAT);

  // Enable PWM output on this channel (CCxE bit per AN4013 Section 2.5)
  _timer->resumeChannel(channel);

  return true;
}

void PWMOutputBank::SetPulseWidth(uint32_t channel, uint32_t pulse_us)
{
  if (!_initialized || channel < 1 || channel > 4) {
    return;
  }

  uint32_t ch_index = channel - 1;

  if (!_channels[ch_index].active) {
    return;
  }

  // Clamp to min/max range
  if (pulse_us < _channels[ch_index].min_us) {
    pulse_us = _channels[ch_index].min_us;
  }
  if (pulse_us > _channels[ch_index].max_us) {
    pulse_us = _channels[ch_index].max_us;
  }

  _channels[ch_index].current_us = pulse_us;

  // Update timer compare value using MICROSEC_COMPARE_FORMAT
  _timer->setCaptureCompare(channel, pulse_us, MICROSEC_COMPARE_FORMAT);
}

void PWMOutputBank::Write(uint32_t channel, int value)
{
  if (!_initialized || channel < 1 || channel > 4) {
    return;
  }

  uint32_t ch_index = channel - 1;

  if (!_channels[ch_index].active) {
    return;
  }

  uint32_t pulse_us;

  // Handle both degree (0-180) and microsecond (544-2400) formats
  if (value >= 0 && value <= 180) {
    // Convert degrees to microseconds (Arduino Servo compatible)
    // Map 0-180 degrees to min_us - max_us range
    pulse_us = map(value, 0, 180,
                   _channels[ch_index].min_us,
                   _channels[ch_index].max_us);
  } else if (value >= 544 && value <= 2400) {
    // Direct microsecond value (Arduino Servo compatible range)
    pulse_us = value;
  } else {
    return; // Invalid value
  }

  SetPulseWidth(channel, pulse_us);
}

void PWMOutputBank::Start()
{
  if (!_initialized) {
    return;
  }

  _timer->resume();
}

void PWMOutputBank::Stop()
{
  if (!_initialized) {
    return;
  }

  _timer->pause();
}

uint32_t PWMOutputBank::GetPulseWidth(uint32_t channel)
{
  if (!_initialized || channel < 1 || channel > 4) {
    return 0;
  }

  uint32_t ch_index = channel - 1;

  if (!_channels[ch_index].active) {
    return 0;
  }

  return _channels[ch_index].current_us;
}
