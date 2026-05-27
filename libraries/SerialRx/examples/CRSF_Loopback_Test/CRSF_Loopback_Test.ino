/*
 * SerialRx CRSF Loopback Test
 *
 * Hardware-in-the-Loop validation test for CRSFParser using loopback
 * between two USART peripherals on the same MCU.
 *
 * HARDWARE SETUP:
 *   Jumper wire: PA11 (USART6 TX) -> PA10 (USART1 RX)
 *
 * TEST PHASES:
 *   Phase 1: RC channel parsing - verify frame reception and channel values
 *   Phase 2: Good LQ - link stats LQ=100, verify no failsafe flag
 *   Phase 3: LQ=0 failsafe - link stats LQ=0, verify error_flags set
 *   Phase 4: LQ recovery - link stats LQ=80, verify failsafe clears
 *   Phase 5: Bad CRC rejection - corrupted frame, verify frames_failed
 *
 * EXIT CRITERIA:
 *   - "*TEST_PASS*" - All validations successful
 *   - "*TEST_FAIL*" - Validation failure
 *
 * OUTPUT:
 *   - ~6-second test duration
 *   - Compatible with Arduino IDE Serial Monitor and CI automation
 *
 * Build:
 *   arduino-cli compile --fqbn STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE \
 *     Arduino_Core_STM32/libraries/SerialRx/examples/CRSF_Loopback_Test
 */

#include <SerialRx.h>

// Hardware configuration
// TX: USART6 for CRSF frame generation (TX-only — pass NC_PIN for RX).
HardwareSerial SerialTx(USART6, NC_PIN, PA11);

// RX: USART1 for SerialRx library
HardwareSerial SerialRC(USART1, PA10, PA9);

// SerialRx instance
SerialRx rc;

// CRSF protocol constants (local copies for frame generation)
static constexpr uint8_t SYNC = 0xC8;
static constexpr uint8_t TYPE_RC   = 0x16;
static constexpr uint8_t TYPE_LINK = 0x14;

// Test state
bool test_failed = false;
uint32_t frames_sent = 0;

// ---------------------------------------------------------------------------
// CRC8-DVB-S2 (for frame generation — must match parser implementation)
// ---------------------------------------------------------------------------
static uint8_t crc8(uint8_t crc, uint8_t a) {
    crc ^= a;
    for (uint8_t i = 0; i < 8; i++) {
        crc = (crc & 0x80) ? (crc << 1) ^ 0xD5 : (crc << 1);
    }
    return crc;
}

static uint8_t crc8_buf(const uint8_t* buf, uint8_t len) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        crc = crc8(crc, buf[i]);
    }
    return crc;
}

// ---------------------------------------------------------------------------
// Pack 16 channels (11-bit each) into 22 bytes
// ---------------------------------------------------------------------------
static void packChannels(const uint16_t* ch, uint8_t* packed) {
    memset(packed, 0, 22);
    uint16_t bit_pos = 0;
    for (uint8_t i = 0; i < 16; i++) {
        uint16_t val = ch[i] & 0x07FF;
        uint16_t byte_idx = bit_pos / 8;
        uint8_t bit_off = bit_pos % 8;

        packed[byte_idx]     |= (uint8_t)(val << bit_off);
        packed[byte_idx + 1] |= (uint8_t)(val >> (8 - bit_off));
        if (bit_off > 5) {
            packed[byte_idx + 2] |= (uint8_t)(val >> (16 - bit_off));
        }
        bit_pos += 11;
    }
}

// ---------------------------------------------------------------------------
// Send a CRSF RC Channels frame (type 0x16)
// Frame: [sync] [length=24] [type] [22 payload] [CRC]
// ---------------------------------------------------------------------------
static void sendRCFrame(const uint16_t* channels) {
    uint8_t frame[26];
    frame[0] = SYNC;
    frame[1] = 24;   // length: type(1) + payload(22) + crc(1)
    frame[2] = TYPE_RC;
    packChannels(channels, &frame[3]);
    frame[25] = crc8_buf(&frame[2], 23);  // CRC over type + payload

    SerialTx.write(frame, 26);
    frames_sent++;
}

// ---------------------------------------------------------------------------
// Send a CRSF Link Statistics frame (type 0x14)
// Frame: [sync] [length=12] [type] [10 payload] [CRC]
// ---------------------------------------------------------------------------
static void sendLinkStatsFrame(uint8_t uplink_lq) {
    uint8_t frame[14];
    frame[0] = SYNC;
    frame[1] = 12;   // length: type(1) + payload(10) + crc(1)
    frame[2] = TYPE_LINK;
    // Payload: rssi1, rssi2, lq, snr, antenna, rfMode, txPower, dRSSI, dLQ, dSNR
    frame[3]  = 80;          // uplinkRSSIAnt1
    frame[4]  = 85;          // uplinkRSSIAnt2
    frame[5]  = uplink_lq;   // uplinkLQ — the field under test
    frame[6]  = 10;          // uplinkSNR
    frame[7]  = 0;           // activeAntenna
    frame[8]  = 2;           // rfMode
    frame[9]  = 3;           // uplinkTXPower
    frame[10] = 90;          // downlinkRSSI
    frame[11] = 100;         // downlinkLQ
    frame[12] = 8;           // downlinkSNR
    frame[13] = crc8_buf(&frame[2], 11);  // CRC over type + payload

    SerialTx.write(frame, 14);
    frames_sent++;
}

// ---------------------------------------------------------------------------
// Send a CRSF RC Channels frame with intentionally bad CRC
// ---------------------------------------------------------------------------
static void sendBadCRCFrame(const uint16_t* channels) {
    uint8_t frame[26];
    frame[0] = SYNC;
    frame[1] = 24;
    frame[2] = TYPE_RC;
    packChannels(channels, &frame[3]);
    frame[25] = crc8_buf(&frame[2], 23) ^ 0xFF;  // Corrupt CRC

    SerialTx.write(frame, 26);
    frames_sent++;
}

// ---------------------------------------------------------------------------
// Drain and process all pending frames, return last message received
// ---------------------------------------------------------------------------
static bool drainFrames(RCMessage* last_msg, uint32_t wait_ms) {
    bool got_msg = false;
    uint32_t start = millis();
    while (millis() - start < wait_ms) {
        rc.update();
        if (rc.available()) {
            rc.getMessage(last_msg);
            got_msg = true;
        }
    }
    return got_msg;
}

// ---------------------------------------------------------------------------
// Test helpers
// ---------------------------------------------------------------------------
static void fail(const char* msg) {
    Serial.print("  FAIL: ");
    Serial.println(msg);
    test_failed = true;
}

static void pass(const char* msg) {
    Serial.print("  PASS: ");
    Serial.println(msg);
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println("\n=== SerialRx CRSF Loopback Test ===");
    Serial.println("\nHardware Setup:");
    Serial.println("  Jumper: PA11 (USART6 TX) -> PA10 (USART1 RX)");
    Serial.println("  TX: USART6 @ 420000 baud (CRSF generator)");
    Serial.println("  RX: USART1 @ 420000 baud (SerialRx library)\n");

    // Initialize transmitter (USART6)
    SerialTx.begin(420000);

    // Initialize receiver (USART1 via SerialRx)
    SerialRx::Config config;
    config.serial = &SerialRC;
    config.rx_protocol = SerialRx::CRSF;
    config.baudrate = 420000;
    config.timeout_ms = 100;
    config.invert_rx = false;

    if (!rc.begin(config)) {
        Serial.println("ERROR: SerialRx initialization failed!");
        Serial.println("*TEST_FAIL*");
        Serial.println("*STOP*");
        while (1);
    }

    Serial.println("Test starting...\n");
    delay(50);  // Let UART settle

    // ==================================================================
    // Phase 1: RC Channel Parsing (3 seconds, ~150 Hz)
    // ==================================================================
    Serial.println("Phase 1: RC Channel Parsing");

    uint16_t test_channels[16];
    uint32_t phase_start = millis();
    uint32_t last_tx = 0;
    uint32_t phase1_rx_count = 0;

    // Set known channel values: 1000 + (i * 60) for channels 0-13
    for (uint8_t i = 0; i < 16; i++) {
        test_channels[i] = 1000 + (i * 60);
    }

    while (millis() - phase_start < 3000) {
        uint32_t now = millis();
        if (now - last_tx >= 7) {  // ~143 Hz
            sendRCFrame(test_channels);
            last_tx = now;
        }

        rc.update();
        if (rc.available()) {
            RCMessage msg;
            if (rc.getMessage(&msg)) {
                phase1_rx_count++;

                // Verify first 4 channels on first received frame
                if (phase1_rx_count == 1) {
                    bool ch_ok = true;
                    for (uint8_t i = 0; i < 4; i++) {
                        uint16_t expected = 1000 + (i * 60);
                        if (msg.channels[i] != expected) {
                            Serial.print("  FAIL: Ch");
                            Serial.print(i);
                            Serial.print(" expected ");
                            Serial.print(expected);
                            Serial.print(" got ");
                            Serial.println(msg.channels[i]);
                            ch_ok = false;
                            test_failed = true;
                        }
                    }
                    if (ch_ok) {
                        pass("Channel values match expected (Ch0-3)");
                    }

                    // Verify channels 4-13
                    bool aux_ok = true;
                    for (uint8_t i = 4; i < 14; i++) {
                        uint16_t expected = 1000 + (i * 60);
                        if (msg.channels[i] != expected) {
                            aux_ok = false;
                            test_failed = true;
                        }
                    }
                    if (aux_ok) {
                        pass("Channel values match expected (Ch4-13)");
                    } else {
                        fail("Channel mismatch in Ch4-13");
                    }

                    // Verify no failsafe flag
                    if (msg.error_flags == 0) {
                        pass("No failsafe flag on normal frame");
                    } else {
                        fail("Unexpected error_flags on normal frame");
                    }
                }
            }
        }
    }

    Serial.print("  Phase 1 frames received: ");
    Serial.println(phase1_rx_count);

    if (phase1_rx_count < 300) {
        fail("Too few frames received in Phase 1 (<300)");
    } else {
        pass("Frame count OK (>=300)");
    }

    // ==================================================================
    // Phase 2: Link Stats with Good LQ
    // ==================================================================
    Serial.println("\nPhase 2: Link Stats with Good LQ");

    sendLinkStatsFrame(100);  // LQ = 100%
    delay(10);  // Let it transmit

    // Send RC frame after link stats
    sendRCFrame(test_channels);

    RCMessage msg;
    if (drainFrames(&msg, 50)) {
        if (msg.error_flags == 0) {
            pass("No failsafe with LQ=100");
        } else {
            fail("Unexpected failsafe flag with LQ=100");
        }
    } else {
        fail("No frame received after link stats");
    }

    // ==================================================================
    // Phase 3: Link Stats with LQ=0 (Failsafe)
    // ==================================================================
    Serial.println("\nPhase 3: LQ=0 Failsafe Detection");

    sendLinkStatsFrame(0);  // LQ = 0 — RF link lost
    delay(10);

    // Send RC frame — should have failsafe flag
    sendRCFrame(test_channels);

    if (drainFrames(&msg, 50)) {
        if (msg.error_flags & 0x02) {
            pass("Failsafe flag SET with LQ=0");
        } else {
            Serial.print("  error_flags=0x");
            Serial.println(msg.error_flags, HEX);
            fail("Failsafe flag NOT set with LQ=0");
        }
    } else {
        fail("No frame received after LQ=0 link stats");
    }

    // ==================================================================
    // Phase 4: LQ Recovery
    // ==================================================================
    Serial.println("\nPhase 4: LQ Recovery");

    sendLinkStatsFrame(80);  // LQ = 80% — link recovered
    delay(10);

    // Send RC frame — failsafe should clear
    sendRCFrame(test_channels);

    if (drainFrames(&msg, 50)) {
        if (msg.error_flags == 0) {
            pass("Failsafe flag CLEARED after LQ recovery");
        } else {
            fail("Failsafe flag still set after LQ recovery");
        }
    } else {
        fail("No frame received after LQ recovery");
    }

    // ==================================================================
    // Phase 5: Bad CRC Rejection
    // ==================================================================
    Serial.println("\nPhase 5: Bad CRC Rejection");

    uint32_t failed_before = rc.getFramesFailed();

    sendBadCRCFrame(test_channels);
    delay(10);

    // Drain — the bad frame should NOT produce a message
    rc.update();
    bool got_bad = rc.available();

    // Also send a good frame to ensure parser recovered
    sendRCFrame(test_channels);

    if (drainFrames(&msg, 50)) {
        uint32_t failed_after = rc.getFramesFailed();
        if (failed_after > failed_before) {
            pass("Bad CRC frame rejected (frames_failed incremented)");
        } else {
            fail("frames_failed did not increment on bad CRC");
        }

        if (!got_bad) {
            pass("No message queued from bad CRC frame");
        } else {
            fail("Message queued from bad CRC frame");
        }

        // Verify parser recovered — good frame after bad should parse fine
        pass("Parser recovered after bad CRC");
    } else {
        fail("Parser did not recover after bad CRC");
    }

    // ==================================================================
    // Final Results
    // ==================================================================
    uint32_t total_received = rc.getFramesReceived();
    uint32_t total_failed = rc.getFramesFailed();
    float loss = rc.getFrameLossPercent();

    Serial.println("\n=== Test Complete ===");
    Serial.print("Frames Sent: "); Serial.println(frames_sent);
    Serial.print("Frames Received: "); Serial.println(total_received);
    Serial.print("Frames Failed: "); Serial.println(total_failed);
    Serial.print("Loss Rate: "); Serial.print(loss, 2); Serial.println("%");

    if (!test_failed) {
        Serial.println("\n*TEST_PASS*");
    } else {
        Serial.println("\n*TEST_FAIL*");
    }

    Serial.println("*STOP*");
    while (1);
}

void loop() {
    // All test logic in setup() for deterministic sequencing
}
