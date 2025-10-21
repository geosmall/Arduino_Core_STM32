#include "uvos_brd.h"
#include <cassert>

// Use the uvos namespace to prevent having to type
// uvos:: before all libuvos functions
using namespace uvos;

// Declare a UVOSboard object called hardware
UVOSboard hardware;

// Helper function to convert a duty cycle to a pulse width
static inline uint32_t duty_cycle_to_pulse(uint32_t period, uint32_t duty_cycle)
{
    return ((period * duty_cycle) / 100);
}

int main(void)
{
    // Declare a variable to store the state we want to set for the LED.
    bool led_state;
    led_state = true;

    // Configure and Initialize the UVOS board
    // These are separate to allow reconfiguration of any of the internal
    // components before initialization.
    hardware.Configure();
    hardware.Init();

    uint32_t timer_freq_hz = (System::GetPClk1Freq() * 2);
    uint32_t target_tick_freq_hz = 1'000'000; // 1 MHz
    uint32_t period_uSec = 1000; // 1 ms period

    // Calculate prescaler
    uint32_t prescale_val = (timer_freq_hz / target_tick_freq_hz) - 1;

    uvos::TimerHandle timer1;
    uvos::TimerHandle::Config timer_cfg;
    timer_cfg.periph = uvos::TimerHandle::Config::Peripheral::TIM_3;
    timer_cfg.prescaler = prescale_val; // Set the prescaler 1 MHz
    timer_cfg.period = (period_uSec - 1); // Set the period to 1 ms

    // Matek H743 S1 and S2 on TIM3
    uvos::TimerHandle::PWMChannelConfig pwm_ch1[2] = {
        {TIM_CHANNEL_3, Pin(PORTB, 0), TIM_OCPOLARITY_HIGH, GPIO_AF2_TIM3},
        {TIM_CHANNEL_4, Pin(PORTB, 1), TIM_OCPOLARITY_HIGH, GPIO_AF2_TIM3}
    };

    pwm_ch1[0].pulse = duty_cycle_to_pulse(timer_cfg.period, 50); // 50% duty cycle
    pwm_ch1[1].pulse = duty_cycle_to_pulse(timer_cfg.period, 37); // 37% duty cycle

    timer1.InitPWM(timer_cfg, pwm_ch1, ARRAYLEN(pwm_ch1));

    timer1.StartPWM();

    uvos::TimerHandle timer2;
    timer_cfg.periph = uvos::TimerHandle::Config::Peripheral::TIM_5;

    // Matek H743 S3-S6 on TIM5
    uvos::TimerHandle::PWMChannelConfig pwm_ch2[4] = {
        {TIM_CHANNEL_1, Pin(PORTA, 0), TIM_OCPOLARITY_HIGH, GPIO_AF2_TIM5},
        {TIM_CHANNEL_2, Pin(PORTA, 1), TIM_OCPOLARITY_HIGH, GPIO_AF2_TIM5},
        {TIM_CHANNEL_3, Pin(PORTA, 2), TIM_OCPOLARITY_HIGH, GPIO_AF2_TIM5},
        {TIM_CHANNEL_4, Pin(PORTA, 3), TIM_OCPOLARITY_HIGH, GPIO_AF2_TIM5}
    };

    pwm_ch2[0].pulse = duty_cycle_to_pulse(timer_cfg.period, 85); // 85% duty cycle
    pwm_ch2[1].pulse = duty_cycle_to_pulse(timer_cfg.period, 75); // 75% duty cycle
    pwm_ch2[2].pulse = duty_cycle_to_pulse(timer_cfg.period, 65); // 65% duty cycle
    pwm_ch2[3].pulse = duty_cycle_to_pulse(timer_cfg.period, 55); // 55% duty cycle

    timer2.InitPWM(timer_cfg, pwm_ch2, ARRAYLEN(pwm_ch2));

    timer2.StartPWM();

    // Loop forever
    for(;;)
    {
        // Set the onboard LED
        hardware.SetLed(led_state);

        // Toggle the LED state for the next time around.
        led_state = !led_state;

        // Wait 500ms
        System::Delay(500);
    }
}
