#include "DShotOutput.h"

DShotOutput::DShotOutput()
  : _num_groups(0), _num_motors(0), _speed(DShot::DSHOT600)
{
  for (int i = 0; i < MAX_TIMER_GROUPS; i++) {
    _groups[i].timer = nullptr;
    _groups[i].initialized = false;
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
                          DShot::Speed speed)
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

  // Resolve and configure DMA
  if (!DShot::resolveDMA(motor)) return -1;
  DShot::initDMA(motor);

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
  // Cleanup from previous transfer: disable timer DMA requests, clear flags.
  // Normal-mode DMA auto-disables the stream on completion, but the timer
  // DMA request may still be enabled and flags must be cleared before re-arm.
  for (int i = 0; i < _num_motors; i++) {
    DShot::cleanupPreviousTransfer(&_motors[i]);
  }

  // Encode packets and fill DMA buffers
  for (int i = 0; i < _num_motors; i++) {
    uint16_t packet = DShot::encodePacket(_motors[i].throttle, _motors[i].telemetry);
    DShot::fillDmaBuffer(_motors[i].dma_buffer, packet);
  }

  // Reset timer counters per group for synchronized output
  for (int i = 0; i < _num_groups; i++) {
    LL_TIM_SetCounter(_groups[i].timer, 0);
  }

  // Trigger all DMA transfers
  for (int i = 0; i < _num_motors; i++) {
    DShot::triggerDMA(&_motors[i]);
  }
}

void DShotOutput::Disarm()
{
  SetAllThrottle(0, false);
  Send();
}

bool DShotOutput::IsTransferComplete() const
{
  return DShot::allTransfersComplete(_motors, _num_motors);
}
