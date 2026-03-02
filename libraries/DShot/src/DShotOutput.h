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
  // dma_override: optional pre-resolved DMA assignment (for external allocator).
  //   When non-null, bypasses internal DMA lookup. Default: nullptr (auto-resolve).
  // Returns motor index (0-based) on success, -1 on failure.
  int AddMotor(TIM_TypeDef *timer, uint32_t pin, uint32_t channel,
               DShot::Speed speed = DShot::DSHOT600,
               const DShot::DMAResource *dma_override = nullptr);

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

  // Check if DMA initialization failed (unresolvable stream conflict)
  bool IsInitFailed() const { return _dma_init_failed; }

  // Number of configured motors
  int GetNumMotors() const { return _num_motors; }

private:
  static constexpr int MAX_TIMER_GROUPS = 4;

  struct TimerGroup {
    TIM_TypeDef *timer;
    bool initialized;
#if defined(STM32F4xx) || defined(STM32F7xx)
    bool use_burst;            // true if DMAR burst mode (stream conflict detected)
    DShot::BurstGroup burst;   // burst DMA state (valid only if use_burst)
#endif
  };

  TimerGroup _groups[MAX_TIMER_GROUPS];
  int _num_groups;

  DShot::MotorHW _motors[DShot::MAX_MOTORS];
  int _num_motors;
  DShot::Speed _speed;
  bool _timers_started;
  bool _dma_initialized;
  bool _dma_init_failed;

  // Find or create a timer group, returns group index or -1
  int findOrCreateGroup(TIM_TypeDef *timer);

  // Start all timer counters and enable outputs
  void startTimers();

  // Initialize DMA for all motors (called once, after all motors registered).
  // Claims streams via dma_claim(), detects conflicts, auto-upgrades to
  // DMAR burst on F4/F7 when cross-group conflicts are found.
  // Returns false on unresolvable conflict.
  bool initAllDMA();

  // Convert 1-based channel number to LL_TIM_CHANNEL_CHx constant
  static uint32_t channelToLL(uint32_t channel);

  // Convert 1-based channel number to 0-based index
  static uint8_t channelToIndex(uint32_t channel);
};

#endif // DSHOT_OUTPUT_H
