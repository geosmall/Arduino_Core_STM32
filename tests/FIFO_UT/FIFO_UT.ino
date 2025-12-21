/**
 * FIFO_UT.ino - Unit tests for FIFO.h lock-free ring buffer
 *
 * Tests the FIFO utility imported from UVOS_Duino with bug fixes:
 * - Copy constructor fix (proper base class initialization)
 * - PutWithOverwrite race condition fix (write data before advancing tail)
 *
 * Run with: ./system/ci/aflash.sh tests/FIFO_UT --use-rtt --build-id
 */

#include <aunit_hil.h>
#include "util/FIFO.h"

using namespace uvos;

//=============================================================================
// Basic Operations Tests
//=============================================================================

test(fifo_empty_on_creation) {
  FIFO<uint8_t, 8> fifo;
  assertTrue(fifo.IsEmpty());
  assertFalse(fifo.IsFull());
  assertEqual(fifo.GetNumElements(), (size_t)0);
  assertEqual(fifo.GetCapacity(), (size_t)8);
}

test(fifo_put_and_get_single) {
  FIFO<uint8_t, 8> fifo;

  assertTrue(fifo.PutIfNotFull(42));
  assertFalse(fifo.IsEmpty());
  assertEqual(fifo.GetNumElements(), (size_t)1);

  uint8_t val;
  assertTrue(fifo.Get(val));
  assertEqual(val, 42);
  assertTrue(fifo.IsEmpty());
}

test(fifo_put_and_get_multiple) {
  FIFO<uint8_t, 8> fifo;

  // Put 5 elements
  for (uint8_t i = 0; i < 5; i++) {
    assertTrue(fifo.PutIfNotFull(i * 10));
  }
  assertEqual(fifo.GetNumElements(), (size_t)5);

  // Get them back in FIFO order
  for (uint8_t i = 0; i < 5; i++) {
    uint8_t val;
    assertTrue(fifo.Get(val));
    assertEqual(val, i * 10);
  }
  assertTrue(fifo.IsEmpty());
}

test(fifo_peek_does_not_remove) {
  FIFO<uint8_t, 8> fifo;

  fifo.PutIfNotFull(99);

  uint8_t val;
  assertTrue(fifo.Peek(val));
  assertEqual(val, 99);
  assertEqual(fifo.GetNumElements(), (size_t)1);  // Still there

  assertTrue(fifo.Get(val));
  assertEqual(val, 99);
  assertTrue(fifo.IsEmpty());
}

//=============================================================================
// Full Buffer Tests
//=============================================================================

test(fifo_full_detection) {
  FIFO<uint8_t, 4> fifo;  // Capacity 4

  // Fill it up
  for (uint8_t i = 0; i < 4; i++) {
    assertTrue(fifo.PutIfNotFull(i));
  }

  assertTrue(fifo.IsFull());
  assertEqual(fifo.GetNumElements(), (size_t)4);

  // Should reject additional element
  assertFalse(fifo.PutIfNotFull(99));
}

test(fifo_put_with_overwrite_when_full) {
  FIFO<uint8_t, 4> fifo;

  // Fill with 0, 1, 2, 3
  for (uint8_t i = 0; i < 4; i++) {
    fifo.PutIfNotFull(i);
  }
  assertTrue(fifo.IsFull());

  // Overwrite oldest (0) with 99
  assertTrue(fifo.PutWithOverwrite(99));
  assertEqual(fifo.GetNumElements(), (size_t)4);  // Still full

  // First out should be 1 (0 was overwritten)
  uint8_t val;
  assertTrue(fifo.Get(val));
  assertEqual(val, 1);

  // Continue reading: 2, 3, 99
  assertTrue(fifo.Get(val)); assertEqual(val, 2);
  assertTrue(fifo.Get(val)); assertEqual(val, 3);
  assertTrue(fifo.Get(val)); assertEqual(val, 99);
  assertTrue(fifo.IsEmpty());
}

test(fifo_multiple_overwrites) {
  FIFO<uint8_t, 3> fifo;  // Capacity 3

  // Fill with 0, 1, 2
  for (uint8_t i = 0; i < 3; i++) {
    fifo.PutIfNotFull(i);
  }

  // Overwrite 3 times: should end up with 3, 4, 5
  fifo.PutWithOverwrite(3);  // Overwrites 0 -> 1, 2, 3
  fifo.PutWithOverwrite(4);  // Overwrites 1 -> 2, 3, 4
  fifo.PutWithOverwrite(5);  // Overwrites 2 -> 3, 4, 5

  uint8_t val;
  assertTrue(fifo.Get(val)); assertEqual(val, 3);
  assertTrue(fifo.Get(val)); assertEqual(val, 4);
  assertTrue(fifo.Get(val)); assertEqual(val, 5);
  assertTrue(fifo.IsEmpty());
}

//=============================================================================
// Wraparound Tests
//=============================================================================

test(fifo_wraparound) {
  FIFO<uint8_t, 4> fifo;

  // Fill and drain a few times to force wraparound
  for (int cycle = 0; cycle < 3; cycle++) {
    // Add 3 elements
    for (uint8_t i = 0; i < 3; i++) {
      assertTrue(fifo.PutIfNotFull(cycle * 10 + i));
    }

    // Remove 3 elements
    for (uint8_t i = 0; i < 3; i++) {
      uint8_t val;
      assertTrue(fifo.Get(val));
      assertEqual(val, cycle * 10 + i);
    }
  }
  assertTrue(fifo.IsEmpty());
}

//=============================================================================
// Copy Constructor Test (Bug Fix Validation)
// Note: Same-capacity copy uses implicitly deleted copy constructor (std::atomic is non-copyable)
// The template copy constructor only works for DIFFERENT capacities
//=============================================================================

test(fifo_copy_to_different_capacity) {
  FIFO<uint8_t, 4> small;
  small.PutIfNotFull(1);
  small.PutIfNotFull(2);
  small.PutIfNotFull(3);

  // Copy to larger FIFO - this tests the fixed template copy constructor
  FIFO<uint8_t, 16> large(small);

  assertEqual(large.GetNumElements(), (size_t)3);
  assertEqual(large.GetCapacity(), (size_t)16);

  uint8_t val;
  assertTrue(large.Get(val)); assertEqual(val, 1);
  assertTrue(large.Get(val)); assertEqual(val, 2);
  assertTrue(large.Get(val)); assertEqual(val, 3);
  assertTrue(large.IsEmpty());

  // Original should still have its elements
  assertEqual(small.GetNumElements(), (size_t)3);
}

test(fifo_assignment_operator) {
  FIFO<uint8_t, 8> original;
  original.PutIfNotFull(10);
  original.PutIfNotFull(20);
  original.PutIfNotFull(30);

  // Use assignment operator (different capacity)
  FIFO<uint8_t, 16> copy;
  copy = original;

  assertEqual(copy.GetNumElements(), (size_t)3);

  uint8_t val;
  assertTrue(copy.Get(val)); assertEqual(val, 10);
  assertTrue(copy.Get(val)); assertEqual(val, 20);
  assertTrue(copy.Get(val)); assertEqual(val, 30);
}

//=============================================================================
// Clear Test
//=============================================================================

test(fifo_clear) {
  FIFO<uint8_t, 8> fifo;

  fifo.PutIfNotFull(1);
  fifo.PutIfNotFull(2);
  fifo.PutIfNotFull(3);
  assertEqual(fifo.GetNumElements(), (size_t)3);

  fifo.Clear();

  assertTrue(fifo.IsEmpty());
  assertEqual(fifo.GetNumElements(), (size_t)0);
}

//=============================================================================
// Edge Cases
//=============================================================================

test(fifo_get_from_empty) {
  FIFO<uint8_t, 4> fifo;

  uint8_t val = 0xFF;
  assertFalse(fifo.Get(val));
  assertEqual(val, 0xFF);  // Unchanged
}

test(fifo_peek_from_empty) {
  FIFO<uint8_t, 4> fifo;

  uint8_t val = 0xFF;
  assertFalse(fifo.Peek(val));
  assertEqual(val, 0xFF);  // Unchanged
}

test(fifo_capacity_one) {
  FIFO<uint8_t, 1> fifo;  // Minimum capacity

  assertEqual(fifo.GetCapacity(), (size_t)1);

  assertTrue(fifo.PutIfNotFull(42));
  assertTrue(fifo.IsFull());

  uint8_t val;
  assertTrue(fifo.Get(val));
  assertEqual(val, 42);
  assertTrue(fifo.IsEmpty());
}

//=============================================================================
// Different Types Test
//=============================================================================

test(fifo_with_uint32) {
  FIFO<uint32_t, 4> fifo;

  fifo.PutIfNotFull(0x12345678);
  fifo.PutIfNotFull(0xDEADBEEF);

  uint32_t val;
  assertTrue(fifo.Get(val));
  assertEqual(val, 0x12345678UL);
  assertTrue(fifo.Get(val));
  assertEqual(val, 0xDEADBEEFUL);
}

//=============================================================================
// Test Runner
//=============================================================================

void setup() {
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#else
  SEGGER_RTT_Init();
#endif

  HIL_TEST_SETUP();
  HIL_TEST_TIMEOUT(30);
}

void loop() {
  HIL_TEST_RUN();
}
