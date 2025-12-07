//Arduino/Teensy Flight Controller - dRehmFlight
//Author: Nicholas Rehm
//Project Start: 1/6/2020
//Last Updated: 7/29/2022
//Version: Beta 1.3 (original Teensy)
//
//STM32 Port: BETA 1.4 - GPS support with optional UART DMA
//Target: STM32F4/H7 (NUCLEO_F411RE, NOXE V3, MATEK H743)

//========================================================================================================================//

//This file contains all necessary functions and code used for radio communication to avoid cluttering the main code
//STM32: Using SerialRx library for IBus/SBUS protocols only

#include <SerialRx.h>

// SerialRx library objects
HardwareSerial SerialRC(BoardConfig::rc_receiver.rx_pin, BoardConfig::rc_receiver.tx_pin);
SerialRx rx;

// Raw channel data - populated by SerialRx adapter
unsigned long channel_1_raw, channel_2_raw, channel_3_raw, channel_4_raw, channel_5_raw, channel_6_raw;

void radioSetup() {
  // Initialize SerialRx library
  SerialRx::Config config;
  config.serial = &SerialRC;

  #if defined USE_IBUS_RX
    config.rx_protocol = SerialRx::IBUS;
    config.baudrate = 115200;
  #elif defined USE_SBUS_RX
    config.rx_protocol = SerialRx::SBUS;
    config.baudrate = 100000;
  #else
    #error No serial RX protocol defined (USE_IBUS_RX or USE_SBUS_RX)
  #endif

  config.timeout_ms = BoardConfig::rc_receiver.timeout_ms;
  config.idle_threshold_us = BoardConfig::rc_receiver.idle_threshold_us;

  rx.begin(config);
  CI_LOG("Radio RX initialized\n");
}

void updateRadioChannels() {
  // Adapter: SerialRx → dRehmFlight channel_X_raw variables
  rx.update();

  if (rx.available()) {
    RCMessage msg;
    if (rx.getMessage(&msg)) {
      // Map SerialRx channels to dRehmFlight PWM variables
      channel_1_raw = msg.channels[0];  // Throttle
      channel_2_raw = msg.channels[1];  // Aileron
      channel_3_raw = msg.channels[2];  // Elevator
      channel_4_raw = msg.channels[3];  // Rudder
      channel_5_raw = msg.channels[4];  // Gear (throttle cut)
      channel_6_raw = msg.channels[5];  // Aux1
    }
  }
}

unsigned long getRadioPWM(int ch_num) {
  //DESCRIPTION: Get current radio commands from SerialRx adapter
  unsigned long returnPWM = 0;

  if (ch_num == 1) {
    returnPWM = channel_1_raw;
  }
  else if (ch_num == 2) {
    returnPWM = channel_2_raw;
  }
  else if (ch_num == 3) {
    returnPWM = channel_3_raw;
  }
  else if (ch_num == 4) {
    returnPWM = channel_4_raw;
  }
  else if (ch_num == 5) {
    returnPWM = channel_5_raw;
  }
  else if (ch_num == 6) {
    returnPWM = channel_6_raw;
  }

  return returnPWM;
}
