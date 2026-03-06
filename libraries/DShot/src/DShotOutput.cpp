#include "DShotOutput.h"
#include "dma.h"
#include "core_debug.h"

DShotOutput::DShotOutput()
  : _num_groups(0), _num_motors(0), _speed(DShot::DSHOT600),
    _timers_started(false), _dma_initialized(false), _dma_init_failed(false)
{
  for (int i = 0; i < MAX_TIMER_GROUPS; i++) {
    _groups[i].timer = nullptr;
    _groups[i].initialized = false;
    _groups[i].use_burst = false;
  }
  // Zero-init all motor structs
  for (int i = 0; i < DShot::MAX_MOTORS; i++) {
    _motors[i] = {};
  }
}

uint32_t DShotOutput::channelToLL(uint32_t channel)
{
  switch (channel) {
    case 1: return LL_TIM_CHANNEL_CH1;
    case 2: return LL_TIM_CHANNEL_CH2;
    case 3: return LL_TIM_CHANNEL_CH3;
    case 4: return LL_TIM_CHANNEL_CH4;
    default: return LL_TIM_CHANNEL_CH1;
  }
}

uint8_t DShotOutput::channelToIndex(uint32_t channel)
{
  if (channel >= 1 && channel <= 4) return (uint8_t)(channel - 1);
  return 0;
}

int DShotOutput::findOrCreateGroup(TIM_TypeDef *timer)
{
  for (int i = 0; i < _num_groups; i++) {
    if (_groups[i].timer == timer) return i;
  }
  if (_num_groups < MAX_TIMER_GROUPS) {
    _groups[_num_groups].timer = timer;
    _groups[_num_groups].initialized = false;
    return _num_groups++;
  }
  return -1;
}

int DShotOutput::AddMotor(TIM_TypeDef *timer, uint32_t pin, uint32_t channel,
                          DShot::Speed speed,
                          const DShot::DMAResource *dma_override)
{
  if (_num_motors >= DShot::MAX_MOTORS) return -1;
  if (channel < 1 || channel > 4) return -1;

  _speed = speed;

  // Find or create timer group
  int group = findOrCreateGroup(timer);
  if (group < 0) return -1;

  // Initialize timer once per group
  if (!_groups[group].initialized) {
    DShot::initTimer(timer, channelToLL(channel), speed);
    _groups[group].initialized = true;
  }

  int idx = _num_motors;
  DShot::MotorHW *motor = &_motors[idx];

  motor->timer = timer;
  motor->ll_channel = channelToLL(channel);
  motor->channel_index = channelToIndex(channel);
  motor->throttle = 0;
  motor->telemetry = false;

  // Apply DMA override or mark for auto-resolution in initAllDMA()
  if (dma_override) {
    motor->dma = dma_override->dma;
    motor->dma_stream = dma_override->stream;
#if defined(STM32F4xx) || defined(STM32F7xx)
    motor->dma_channel_sel = dma_override->channel_sel;
#endif
    motor->dma_resolved = true;
  } else {
    motor->dma_resolved = false;
  }

  // Zero the DMA buffer
  for (int i = 0; i < DShot::DMA_BUF_SIZE; i++) {
    motor->dma_buffer[i] = 0;
  }

  // Configure GPIO for timer output
  DShot::initGPIO(pin, timer, motor->ll_channel);

  // Initialize output compare for this channel
  // (initTimer sets up the time base; we add OC per channel here)
  LL_TIM_OC_InitTypeDef oc_init;
  LL_TIM_OC_StructInit(&oc_init);
  oc_init.OCMode = LL_TIM_OCMODE_PWM1;
  oc_init.OCState = LL_TIM_OCSTATE_ENABLE;
  oc_init.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
  oc_init.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  oc_init.CompareValue = 0;
  LL_TIM_OC_Init(timer, motor->ll_channel, &oc_init);
  LL_TIM_OC_EnablePreload(timer, motor->ll_channel);
  LL_TIM_CC_EnableChannel(timer, motor->ll_channel);

  // DMA resolution and init deferred to initAllDMA() (called from Send/startTimers).
  // This allows detecting stream conflicts across all motors on a timer group
  // before choosing per-channel vs DMAR burst mode on F4/F7.

  _num_motors++;
  return idx;
}

void DShotOutput::startTimers()
{
  for (int i = 0; i < _num_groups; i++) {
    if (_groups[i].initialized) {
      LL_TIM_EnableAllOutputs(_groups[i].timer);
      LL_TIM_EnableCounter(_groups[i].timer);
    }
  }
}

bool DShotOutput::shouldUseBurst(int g) const
{
  int group_motor_count = 0;

#if defined(STM32F4xx) || defined(STM32F7xx)
  // F4/F7: conflict-triggered — check for stream collisions
  uint32_t group_streams[4];

  for (int m = 0; m < _num_motors; m++) {
    if (_motors[m].timer != _groups[g].timer) continue;

    // Cross-group or UART conflict: stream already claimed?
    if (dma_is_claimed(_motors[m].dma, _motors[m].dma_stream)) {
      return true;
    }
    // Internal conflict: two motors in this group resolved to same stream?
    for (int s = 0; s < group_motor_count; s++) {
      if (group_streams[s] == _motors[m].dma_stream) {
        return true;
      }
    }
    if (group_motor_count < 4) {
      group_streams[group_motor_count] = _motors[m].dma_stream;
    }
    group_motor_count++;
  }
  return false;
#else
  // G4/H7: burst for stream conservation when 2+ motors share a timer
  for (int m = 0; m < _num_motors; m++) {
    if (_motors[m].timer != _groups[g].timer) continue;
    group_motor_count++;
  }
  return group_motor_count >= 2;
#endif
}

bool DShotOutput::initAllDMA()
{
  for (int g = 0; g < _num_groups; g++) {
    // --- Phase A: Resolve DMA for all motors in this group ---
    for (int m = 0; m < _num_motors; m++) {
      if (_motors[m].timer != _groups[g].timer) continue;
      if (_motors[m].dma_resolved) continue;  // override: already set by AddMotor
      if (!DShot::resolveDMA(&_motors[m])) {
        core_debug("DShot: no DMA available for TIM%d_CH%d\n",
                   0, _motors[m].channel_index + 1);
        return false;
      }
    }

    // --- Phase B: Decide burst vs per-channel ---
    bool need_burst = shouldUseBurst(g);

    // --- Phase C: Claim + Init ---
    if (need_burst) {
      // DMAR burst mode for this timer group
      _groups[g].use_burst = true;
      _groups[g].burst.timer = _groups[g].timer;

      // Determine burst length (highest channel + 1) and trigger channel (lowest)
      uint8_t max_ch = 0, min_ch = 3;
      for (int m = 0; m < _num_motors; m++) {
        if (_motors[m].timer != _groups[g].timer) continue;
        if (_motors[m].channel_index > max_ch) max_ch = _motors[m].channel_index;
        if (_motors[m].channel_index < min_ch) min_ch = _motors[m].channel_index;
      }
      _groups[g].burst.burst_length = max_ch + 1;
      _groups[g].burst.trigger_ch_index = min_ch;

      // Zero the burst buffer (unused channel slots stay zero)
      for (int i = 0; i < DShot::DMA_BUF_SIZE * 4; i++) {
        _groups[g].burst.burst_buffer[i] = 0;
      }

      // Resolve burst DMA (trigger channel's stream) and claim it
      if (!DShot::resolveDMABurst(&_groups[g].burst)) {
        core_debug("DShot: burst DMA resolve failed for timer group %d\n", g);
        return false;
      }
      if (dma_claim(_groups[g].burst.dma, _groups[g].burst.dma_stream) != 0) {
        core_debug("DShot: DMA%d_Stream%lu conflict — burst trigger stream already claimed\n",
                   (_groups[g].burst.dma == DMA1) ? 1 : 2,
                   (unsigned long)_groups[g].burst.dma_stream);
        return false;
      }
      DShot::initDMABurst(&_groups[g].burst);
    } else {
      // Per-channel DMA (no conflicts on F4/F7, or single motor on G4/H7)
      for (int m = 0; m < _num_motors; m++) {
        if (_motors[m].timer != _groups[g].timer) continue;
        if (dma_claim(_motors[m].dma, _motors[m].dma_stream) != 0) {
          core_debug("DShot: DMA%d_Stream%lu conflict — TIM_CH%d stream already claimed\n",
                     (_motors[m].dma == DMA1) ? 1 : 2,
                     (unsigned long)_motors[m].dma_stream,
                     _motors[m].channel_index + 1);
          return false;
        }
        DShot::initDMA(&_motors[m]);
      }
    }
  }
  _dma_initialized = true;
  return true;
}

void DShotOutput::SetThrottle(int motor_idx, uint16_t throttle, bool telemetry)
{
  if (motor_idx < 0 || motor_idx >= _num_motors) return;
  if (throttle > 2047) throttle = 2047;

  _motors[motor_idx].throttle = throttle;
  _motors[motor_idx].telemetry = telemetry;
}

void DShotOutput::SetAllThrottle(uint16_t throttle, bool telemetry)
{
  for (int i = 0; i < _num_motors; i++) {
    SetThrottle(i, throttle, telemetry);
  }
}

void DShotOutput::Send()
{
  // Start timers on first Send() — enables counter and MOE for advanced timers.
  // This handles the AddMotor() path where Init()/startTimers() isn't called.
  if (!_timers_started) {
    startTimers();
    _timers_started = true;
  }

  // Initialize DMA on first Send() — deferred from AddMotor() to allow
  // stream conflict detection across all motors on each timer group.
  // By this point, UART DMA listen (and other consumers) have already
  // claimed their streams, so the allocator can see and avoid them.
  if (!_dma_initialized) {
    if (!initAllDMA()) {
      _dma_init_failed = true;
    }
  }
  if (_dma_init_failed) return;

  // --- Cleanup from previous transfer ---
  for (int g = 0; g < _num_groups; g++) {
    if (_groups[g].use_burst) {
      DShot::cleanupPreviousBurstTransfer(&_groups[g].burst);
      continue;
    }
    for (int m = 0; m < _num_motors; m++) {
      if (_motors[m].timer == _groups[g].timer) {
        DShot::cleanupPreviousTransfer(&_motors[m]);
      }
    }
  }

  // --- Encode packets and fill DMA buffers ---
  for (int m = 0; m < _num_motors; m++) {
    uint16_t packet = DShot::encodePacket(_motors[m].throttle, _motors[m].telemetry);

    // Check if this motor's group uses burst mode
    bool filled = false;
    for (int g = 0; g < _num_groups; g++) {
      if (_groups[g].timer == _motors[m].timer && _groups[g].use_burst) {
        DShot::fillDmaBurstBuffer(
            _groups[g].burst.burst_buffer,
            _motors[m].channel_index,
            _groups[g].burst.burst_length,
            packet);
        filled = true;
        break;
      }
    }
    if (!filled) {
      DShot::fillDmaBuffer(_motors[m].dma_buffer, packet);
    }
  }

  // --- Reset timer counters and trigger DMA ---
  for (int g = 0; g < _num_groups; g++) {
    LL_TIM_SetCounter(_groups[g].timer, 0);

    if (_groups[g].use_burst) {
      DShot::triggerDMABurst(&_groups[g].burst);
      continue;
    }
    for (int m = 0; m < _num_motors; m++) {
      if (_motors[m].timer == _groups[g].timer) {
        DShot::triggerDMA(&_motors[m]);
      }
    }
  }
}

void DShotOutput::Disarm()
{
  SetAllThrottle(0, false);
  Send();
}

bool DShotOutput::IsTransferComplete() const
{
  for (int g = 0; g < _num_groups; g++) {
    if (_groups[g].use_burst) {
      if (!DShot::burstTransferComplete(&_groups[g].burst)) return false;
      continue;
    }
    // Per-channel: check each motor on this group
    for (int m = 0; m < _num_motors; m++) {
      if (_motors[m].timer == _groups[g].timer) {
#if defined(STM32G4xx)
        if (LL_DMA_IsEnabledChannel(_motors[m].dma, _motors[m].dma_stream))
          return false;
#else
        if (LL_DMA_IsEnabledStream(_motors[m].dma, _motors[m].dma_stream))
          return false;
#endif
      }
    }
  }
  return true;
}
