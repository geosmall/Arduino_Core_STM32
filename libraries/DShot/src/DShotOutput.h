#ifndef DSHOT_OUTPUT_H
#define DSHOT_OUTPUT_H

#include <Arduino.h>
#include "DShot_packet.h"
#include "DShot_ll.h"

class DShotOutput {
public:
  DShotOutput();

  // Add a motor with explicit timer/pin/channel assignment.
  // channel: 1-4 (timer channel number, same as BoardConfig convention).
  // Returns motor index (0-based) on success, -1 on failure.
  int AddMotor(TIM_TypeDef *timer, uint32_t pin, uint32_t channel,
               DShot::Speed speed = DShot::DSHOT600);

  // Initialize from a BoardConfig::Motor::motors array.
  // Calls AddMotor() for each entry, then starts all timers.
  template<typename MotorArray>
  bool Init(const MotorArray &motors, int count,
            DShot::Speed speed = DShot::DSHOT600)
  {
    if (count > DShot::MAX_MOTORS) return false;

    for (int i = 0; i < count; i++) {
      if (AddMotor(motors[i].timer, motors[i].pin, motors[i].channel, speed) < 0) {
        return false;
      }
    }

    startTimers();
    return true;
  }

  // Set throttle for one motor (0-2047). Not sent until Send() is called.
  void SetThrottle(int motor_idx, uint16_t throttle, bool telemetry = false);

  // Set throttle for all motors
  void SetAllThrottle(uint16_t throttle, bool telemetry = false);

  // Encode packets, fill DMA buffers, and trigger all DMA transfers
  void Send();

  // Send throttle 0 to all motors
  void Disarm();

  // Check if all DMA transfers from the last Send() have completed
  bool IsTransferComplete() const;

  // Number of configured motors
  int GetNumMotors() const { return _num_motors; }

private:
  static constexpr int MAX_TIMER_GROUPS = 4;

  struct TimerGroup {
    TIM_TypeDef *timer;
    bool initialized;
  };

  TimerGroup _groups[MAX_TIMER_GROUPS];
  int _num_groups;

  DShot::MotorHW _motors[DShot::MAX_MOTORS];
  int _num_motors;
  DShot::Speed _speed;
  bool _timers_started;

  // Find or create a timer group, returns group index or -1
  int findOrCreateGroup(TIM_TypeDef *timer);

  // Start all timer counters and enable outputs
  void startTimers();

  // Convert 1-based channel number to LL_TIM_CHANNEL_CHx constant
  static uint32_t channelToLL(uint32_t channel);

  // Convert 1-based channel number to 0-based index
  static uint8_t channelToIndex(uint32_t channel);
};

#endif // DSHOT_OUTPUT_H
