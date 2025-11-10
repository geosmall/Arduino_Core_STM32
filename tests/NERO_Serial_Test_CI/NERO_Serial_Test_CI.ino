/*
  ASCII table - CI/HIL Version

  Prints out byte values in all possible formats:
  - as raw binary values
  - as ASCII-encoded decimal, hex, octal, and binary values

  For more on ASCII, see http://www.asciitable.com and http://en.wikipedia.org/wiki/ASCII

  The circuit: No external hardware needed.

  created 2006
  by Nicholas Zambetti <http://www.zambetti.com>
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/ASCIITable

  Modified for NERO F7 board CI/HIL testing
*/

#include <ci_log.h>

//                      RX    TX
// HardwareSerial Serial(PA10, PA9);

void setup() {
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#else
  // Small delay for RTT to initialize
  delay(100);
#endif

  CI_LOG("ASCII Table ~ Character Map\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();
}

// first visible ASCIIcharacter '!' is number 33:
int thisByte = 33;
// you can also write ASCII characters in single quotes.
// for example, '!' is the same as 33, so you could also use this:
// int thisByte = '!';

void loop() {
  // Print character, decimal, hex, and binary representation
  // Note: %o (octal) removed - SEGGER_RTT_printf doesn't support it
  CI_LOGF("%c, dec: %d, hex: %x, bin: ", thisByte, thisByte, thisByte);

  // Print binary representation (manual conversion since printf doesn't support %b)
  for (int bit = 7; bit >= 0; bit--) {
    CI_LOGF("%d", (thisByte >> bit) & 1);
  }
  CI_LOG("\n");

  // if printed last visible character '~' or 126, stop:
  if (thisByte == 126) {    // you could also use if (thisByte == '~') {
    CI_LOG("*STOP*\n");
    while (true) {
      continue;
    }
  }
  // go on to the next character
  thisByte++;

  // Small delay to prevent overwhelming RTT buffer
  delay(10);
}
