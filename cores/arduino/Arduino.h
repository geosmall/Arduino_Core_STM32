/*
  Arduino.h - Main include file for the Arduino SDK
  Copyright (c) 2005-2013 Arduino Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef Arduino_h
#define Arduino_h

#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)
#endif
#if GCC_VERSION < 60300
  #error "GCC version 6.3 or higher is required"
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "stm32_def.h"
#include "wiring_constants.h"
#include "wiring_time.h"
#include "stm32/digital_io.h"

#ifdef __cplusplus
#include "Pin.h"
#include "Tone.h"
#include "WSerial.h"

// Pin-typed digital I/O
void pinMode(Pin pin, uint32_t mode);
void digitalWrite(Pin pin, uint32_t val);
int  digitalRead(Pin pin);
void digitalToggle(Pin pin);
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern void initVariant() __attribute__((weak));
extern void setup(void);
extern void loop(void);
void yield(void);
#ifdef __cplusplus
}
#endif

#include "pins_arduino.h"

#endif // Arduino_h
