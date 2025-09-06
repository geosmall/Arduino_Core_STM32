/*
 * BasicRTT.ino
 * 
 * Basic example of using SEGGER RTT (Real Time Transfer) for debugging output.
 * RTT provides real-time bidirectional communication between target and host
 * without requiring UART pins or affecting real-time behavior.
 * 
 * Hardware Required:
 * - STM32 board with SWD/JTAG connection
 * - J-Link debugger or ST-Link (reflashed to J-Link firmware)
 * 
 * Host Software Required:
 * - J-Link Software Pack
 * - JLinkRTTClient or JLinkRTTViewer
 * 
 * Client Selection:
 * - JLinkRTTClient: Command-line tool, Best for HIL testing and CI/CD.
 * - JLinkRTTViewer: GUI app. Best for interactive debug and development.
 * 
 * Usage:
 * 1. Upload this sketch to your STM32 board
 * 2. Connect J-Link debugger to SWD pins
 * 3. Start JLinkRTTClient on host PC
 * 4. Observe real-time output without serial connection
 */

#include "SEGGER_RTT.h"

void setup() {
  // Initialize SEGGER RTT
  SEGGER_RTT_Init();
  
  // Send startup message
  SEGGER_RTT_WriteString(0, "RTT Example Started\n");
  SEGGER_RTT_printf(0, "Board: %s\n", BOARD_NAME);
  SEGGER_RTT_printf(0, "CPU Clock: %lu MHz\n", SystemCoreClock / 1000000);
}

void loop() {
  static unsigned long counter = 0;
  static unsigned long lastTime = 0;
  const unsigned long MAX_ITERATIONS = 10;  // Run for 10 seconds then exit for HIL testing
  
  unsigned long currentTime = millis();
  
  // Send periodic messages every second
  if (currentTime - lastTime >= 1000) {
    SEGGER_RTT_printf(0, "Counter: %lu, Uptime: %lu ms\n", counter++, currentTime);
    lastTime = currentTime;
    
    // Exit after demonstration period for HIL testing
    if (counter >= MAX_ITERATIONS) {
      SEGGER_RTT_WriteString(0, "RTT demonstration complete\n");
      SEGGER_RTT_WriteString(0, "*STOP*\n");  // Exit wildcard for HIL testing
      while(1) { delay(1000); }  // Halt execution
    }
  }
  
  // Check for input from RTT host
  int input = SEGGER_RTT_GetKey();
  if (input >= 0) {
    SEGGER_RTT_printf(0, "Received: '%c' (0x%02X)\n", input, input);
    
    // Echo back the character
    SEGGER_RTT_PutChar(0, input);
    SEGGER_RTT_WriteString(0, "\n");
  }
  
  delay(10);  // Small delay to prevent overwhelming RTT buffer
}