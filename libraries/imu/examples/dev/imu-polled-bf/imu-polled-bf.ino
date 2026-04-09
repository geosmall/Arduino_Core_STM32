/*
 * IMU Library - Polled Betaflight Configuration Example
 *
 * Demonstrates polling-based IMU data acquisition with EXACT Betaflight defaults:
 * - FSR: Gyro ±2000 DPS, Accel ±16G (wide range for extreme maneuvers)
 * - ODR: Configurable 8k/4k/2k/1k (both gyro and accel match)
 * - AAF: Both 258 Hz (Betaflight standard anti-aliasing)
 * - UI Filters: Code 15 (low-latency), order left at reset default (2nd-order)
 *
 * This configuration exactly matches the Betaflight ICM-42688-P driver
 * (accgyro_spi_icm426xx.c). Betaflight does NOT explicitly write the UI filter
 * order registers - it relies on the chip's power-on reset default (2nd order).
 * This example mirrors that behavior by skipping order writes.
 *
 * Betaflight relies on robust software filters (gyro LPF, D-term, dynamic notch,
 * RPM) for fine noise control, so hardware filters are kept minimal to reduce delay.
 *
 * BETAFLIGHT ODR SELECTION:
 * Uncomment ONE of the BF_ODR_* defines below to set the output data rate.
 * Betaflight uses 8kHz on capable targets, with fallback to 4k/2k/1k on others.
 *
 * POLLING METHODOLOGY:
 * - Loop rate: 2kHz continuous polling (common flight controller rate)
 * - Higher ODRs (4k/8k) provide oversampling opportunity
 * - Lower ODRs (1k/2k) may match loop rate exactly
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection
 * - Supported: NUCLEO_F411RE, BLACKPILL_F411CE (requires ICM-42688-P IMU)
 * - Pin assignments and SPI frequency from board configuration
 * - No interrupt pin required
 *
 * REFERENCE:
 * - Icm42688p Analysis & Guidance.md - Filter configuration guide (Section 2, 9)
 * - Betaflight driver: src/main/drivers/accgyro/accgyro_spi_icm426xx.c
 */

// ========================================================================
// BETAFLIGHT ODR CONFIGURATION
// Uncomment ONE option to select IMU output data rate
// ========================================================================
#define BF_ODR_8K   // Default for capable targets (8kHz gyro + accel)
// #define BF_ODR_4K   // 4kHz gyro + accel
// #define BF_ODR_2K   // 2kHz gyro + accel
// #define BF_ODR_1K   // 1kHz gyro + accel

#include <IMU.h>
#include <SPI.h>
#include <libPrintf.h>

// printf_() output routing
extern "C" void putchar_(char c) {
    Serial.write(c);
}

// Board configuration (requires ICM-42688-P)
#if defined(ARDUINO_BLACKPILL_F411CE)
#include "targets/BLACKPILL_F411CE.h"
#else
#include "targets/NUCLEO_F411RE_HIL001.h"
#endif

// ============================================================================
// ICM-42688-P Register Definitions (Bank 0 unless noted)
// ============================================================================

// Bank selection
#define REG_BANK_SEL            0x76

// Gyro/Accel configuration (Bank 0)
#define REG_GYRO_CONFIG0        0x4F    // [7:5]=FSR, [3:0]=ODR
#define REG_ACCEL_CONFIG0       0x50    // [7:5]=FSR, [3:0]=ODR
#define REG_GYRO_ACCEL_CONFIG0  0x52    // UI filter bandwidth

// ODR codes (for GYRO_CONFIG0[3:0] and ACCEL_CONFIG0[3:0])
#define ODR_8KHZ                0x03
#define ODR_4KHZ                0x04
#define ODR_2KHZ                0x05
#define ODR_1KHZ                0x06

// UI Filter bandwidth codes (GYRO_ACCEL_CONFIG0)
#define UI_BW_LOW_LATENCY_X2    0x0F    // Extra low latency (Betaflight default)

// AAF register addresses (require bank switching)
// Bank 1: Gyro AAF
#define BANK1_GYRO_CONFIG3      0x0C    // GYRO_CONFIG_STATIC3: delt
#define BANK1_GYRO_CONFIG4      0x0D    // GYRO_CONFIG_STATIC4: deltSqr[7:0]
#define BANK1_GYRO_CONFIG5      0x0E    // GYRO_CONFIG_STATIC5: deltSqr[11:8], bitshift[7:4]

// Bank 2: Accel AAF
#define BANK2_ACCEL_CONFIG2     0x03    // ACCEL_CONFIG_STATIC2: delt << 1
#define BANK2_ACCEL_CONFIG3     0x04    // ACCEL_CONFIG_STATIC3: deltSqr[7:0]
#define BANK2_ACCEL_CONFIG4     0x05    // ACCEL_CONFIG_STATIC4: deltSqr[11:8], bitshift[7:4]

// Bank numbers
#define BANK_0                  0x00
#define BANK_1                  0x01
#define BANK_2                  0x02

// AAF 258 Hz configuration (Betaflight default) - from ICM-42688-P datasheet Table 15
#define AAF_258HZ_DELT          6
#define AAF_258HZ_DELTSQR       36
#define AAF_258HZ_BITSHIFT      10

// Determine ODR based on configuration
#if defined(BF_ODR_8K)
    #define ODR_CODE ODR_8KHZ
    #define ODR_STRING "8kHz"
#elif defined(BF_ODR_4K)
    #define ODR_CODE ODR_4KHZ
    #define ODR_STRING "4kHz"
#elif defined(BF_ODR_2K)
    #define ODR_CODE ODR_2KHZ
    #define ODR_STRING "2kHz"
#elif defined(BF_ODR_1K)
    #define ODR_CODE ODR_1KHZ
    #define ODR_STRING "1kHz"
#else
    #error "No BF_ODR_* defined! Uncomment one of: BF_ODR_8K, BF_ODR_4K, BF_ODR_2K, BF_ODR_1K"
#endif

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin,
                 BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin,
                 BoardConfig::imu.spi.get_ssel_pin());

// Create IMU instance
IMU imu;

// ============================================================================
// Helper Functions for Direct Register Access
// ============================================================================

/**
 * @brief Select register bank (ICM-42688-P has banks 0-4)
 */
void selectBank(uint8_t bank) {
    imu.WriteReg_Ex(REG_BANK_SEL, bank);
    delayMicroseconds(20);
}

/**
 * @brief Configure gyro ODR using direct register access
 */
void setGyroODR(uint8_t odr_code) {
    selectBank(BANK_0);
    uint8_t reg = imu.ReadReg_Ex(REG_GYRO_CONFIG0);
    reg = (reg & 0xF0) | (odr_code & 0x0F);  // Preserve FSR, set ODR
    imu.WriteRegVerify_Ex(REG_GYRO_CONFIG0, reg);
}

/**
 * @brief Configure accel ODR using direct register access
 */
void setAccelODR(uint8_t odr_code) {
    selectBank(BANK_0);
    uint8_t reg = imu.ReadReg_Ex(REG_ACCEL_CONFIG0);
    reg = (reg & 0xF0) | (odr_code & 0x0F);  // Preserve FSR, set ODR
    imu.WriteRegVerify_Ex(REG_ACCEL_CONFIG0, reg);
}

/**
 * @brief Configure UI filter bandwidth
 */
void setUIFilters(uint8_t gyro_bw, uint8_t accel_bw) {
    selectBank(BANK_0);
    uint8_t reg = (accel_bw << 4) | (gyro_bw & 0x0F);
    imu.WriteRegVerify_Ex(REG_GYRO_ACCEL_CONFIG0, reg);
}

/**
 * @brief Configure gyro Anti-Alias Filter (requires Bank 1)
 */
void setGyroAAF(uint8_t delt, uint16_t deltSqr, uint8_t bitshift) {
    selectBank(BANK_1);
    imu.WriteReg_Ex(BANK1_GYRO_CONFIG3, delt);
    imu.WriteReg_Ex(BANK1_GYRO_CONFIG4, deltSqr & 0xFF);
    imu.WriteReg_Ex(BANK1_GYRO_CONFIG5, (deltSqr >> 8) | (bitshift << 4));
    selectBank(BANK_0);
}

/**
 * @brief Configure accel Anti-Alias Filter (requires Bank 2)
 */
void setAccelAAF(uint8_t delt, uint16_t deltSqr, uint8_t bitshift) {
    selectBank(BANK_2);
    imu.WriteReg_Ex(BANK2_ACCEL_CONFIG2, delt << 1);  // Note: shifted by 1
    imu.WriteReg_Ex(BANK2_ACCEL_CONFIG3, deltSqr & 0xFF);
    imu.WriteReg_Ex(BANK2_ACCEL_CONFIG4, (deltSqr >> 8) | (bitshift << 4));
    selectBank(BANK_0);
}

/**
 * @brief Dump current register configuration
 */
void dumpConfiguration() {
    selectBank(BANK_0);
    uint8_t gyro_cfg = imu.ReadReg_Ex(REG_GYRO_CONFIG0);
    uint8_t accel_cfg = imu.ReadReg_Ex(REG_ACCEL_CONFIG0);
    uint8_t ui_cfg = imu.ReadReg_Ex(REG_GYRO_ACCEL_CONFIG0);

    printf_("  GYRO_CONFIG0:       0x%02X (FSR=%d, ODR=%d)\n",
            gyro_cfg, (gyro_cfg >> 5) & 0x07, gyro_cfg & 0x0F);
    printf_("  ACCEL_CONFIG0:      0x%02X (FSR=%d, ODR=%d)\n",
            accel_cfg, (accel_cfg >> 5) & 0x07, accel_cfg & 0x0F);
    printf_("  GYRO_ACCEL_CONFIG0: 0x%02X (Gyro UI=%d, Accel UI=%d)\n",
            ui_cfg, ui_cfg & 0x0F, (ui_cfg >> 4) & 0x0F);

    // Read AAF config from Bank 1
    selectBank(BANK_1);
    uint8_t gyro_aaf3 = imu.ReadReg_Ex(BANK1_GYRO_CONFIG3);
    uint8_t gyro_aaf4 = imu.ReadReg_Ex(BANK1_GYRO_CONFIG4);
    uint8_t gyro_aaf5 = imu.ReadReg_Ex(BANK1_GYRO_CONFIG5);
    printf_("  Gyro AAF:           delt=%d, deltSqr=%d, bitshift=%d\n",
            gyro_aaf3, gyro_aaf4 | ((gyro_aaf5 & 0x0F) << 8), gyro_aaf5 >> 4);

    selectBank(BANK_2);
    uint8_t accel_aaf2 = imu.ReadReg_Ex(BANK2_ACCEL_CONFIG2);
    uint8_t accel_aaf3 = imu.ReadReg_Ex(BANK2_ACCEL_CONFIG3);
    uint8_t accel_aaf4 = imu.ReadReg_Ex(BANK2_ACCEL_CONFIG4);
    printf_("  Accel AAF:          delt=%d, deltSqr=%d, bitshift=%d\n",
            accel_aaf2 >> 1, accel_aaf3 | ((accel_aaf4 & 0x0F) << 8), accel_aaf4 >> 4);

    selectBank(BANK_0);
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println("\n=== IMU Library - Betaflight Configuration ===");

    // Display configuration
    Serial.println("Betaflight ICM-42688-P Configuration:");
    Serial.println("  ODR: " ODR_STRING " (gyro + accel)");
    Serial.println("  FSR: +/-2000 DPS gyro, +/-16G accel");
    Serial.println("  AAF: 258 Hz (both)");
    Serial.println("  UI: Code 15 (low-latency), 2nd-order\n");

    // Display pin configuration
    Serial.println("Pin Configuration (BoardConfig):");
    printf_("  CS: %d, MOSI: %d, MISO: %d, SCLK: %d\n",
            (int)BoardConfig::imu.spi.cs_pin.toPinName(),
            (int)BoardConfig::imu.spi.mosi_pin.toPinName(),
            (int)BoardConfig::imu.spi.miso_pin.toPinName(),
            (int)BoardConfig::imu.spi.sclk_pin.toPinName());
    printf_("  SPI Speed: %lu Hz\n", (unsigned long)BoardConfig::imu.spi.freq_hz);
    Serial.println("  Polling Mode: No interrupt pin required\n");

    // Give IMU time to stabilize
    delay(5);

    // Initialize IMU
    Serial.println("Initializing IMU...");
    if (imu.Init(spi_bus, BoardConfig::imu.spi.cs_pin, BoardConfig::imu.spi.freq_hz) != IMU::Result::OK) {
        Serial.println("ERROR: Failed to initialize IMU!");
        Serial.println("*STOP*");
        while (1) delay(1000);
    }
    Serial.println("IMU initialized successfully");

    // Detect chip type
    IMU::ChipType chip = imu.GetChipType();
    const char* chip_name = "UNKNOWN";
    switch (chip) {
        case IMU::ChipType::ICM42688_P: chip_name = "ICM-42688-P"; break;
        case IMU::ChipType::MPU_6000:   chip_name = "MPU-6000"; break;
        case IMU::ChipType::MPU_9250:   chip_name = "MPU-9250"; break;
        default: break;
    }
    printf_("Detected chip: %s (0x%02X)\n", chip_name, static_cast<uint8_t>(chip));

    // Verify chip is ICM-42688-P (this example uses chip-specific register access)
    if (chip != IMU::ChipType::ICM42688_P) {
        Serial.println("ERROR: This example requires ICM-42688-P!");
        printf_("Detected: %s (0x%02X)\n", chip_name, static_cast<uint8_t>(chip));
        Serial.println("*STOP*");
        while (1) delay(1000);
    }
    Serial.println();

    // Configure IMU using ACRO preset as baseline (8kHz, +/-2000dps, +/-16g)
    Serial.println("Applying ACRO preset as baseline...");
    if (imu.ApplyPreset(IMU::Preset::ACRO) != IMU::Result::OK) {
        Serial.println("ERROR: Failed to apply ACRO preset!");
        Serial.println("*STOP*");
        while (1) delay(1000);
    }
    Serial.println("ACRO preset applied\n");

    // Apply Betaflight-specific configuration via direct register access
    Serial.println("Applying Betaflight configuration...");

    // 1. Set ODR (configurable via #define)
    Serial.println("  Setting ODR to " ODR_STRING "...");
    setGyroODR(ODR_CODE);
    setAccelODR(ODR_CODE);

    // 2. Set AAF to 258 Hz (Betaflight default)
    Serial.println("  Setting AAF to 258 Hz...");
    setGyroAAF(AAF_258HZ_DELT, AAF_258HZ_DELTSQR, AAF_258HZ_BITSHIFT);
    setAccelAAF(AAF_258HZ_DELT, AAF_258HZ_DELTSQR, AAF_258HZ_BITSHIFT);

    // 3. Set UI filters to code 15 (low-latency) - Betaflight default
    Serial.println("  Setting UI filters to low-latency mode...");
    setUIFilters(UI_BW_LOW_LATENCY_X2, UI_BW_LOW_LATENCY_X2);

    // Wait for configuration to stabilize
    delay(10);

    Serial.println("\nBetaflight configuration complete.");
    Serial.println("Final register state:");
    dumpConfiguration();

    Serial.println("\nIMU configured for Betaflight operation");
    Serial.println("  Gyro: +/-2000 DPS, " ODR_STRING " ODR");
    Serial.println("  Accel: +/-16G, " ODR_STRING " ODR");
    Serial.println("  AAF: 258 Hz (both)");
    Serial.println("  UI: Code 15 (low-latency), 2nd-order");
    Serial.println("  Mode: Continuous 2kHz polling loop\n");

    Serial.println("Starting continuous 2kHz polling loop...");
    Serial.println("Will print 20 samples over ~10 seconds\n");
}

void loop() {
    static unsigned long current_time = 0;
    static unsigned long loop_timer = 0;
    static uint32_t sample_count = 0;
    static const uint32_t loop_freq = 2000; // 2kHz loop rate
    static const uint32_t inv_freq = 1000000 / loop_freq; // 500us period

    current_time = micros();

    // Read IMU at 2kHz
    std::array<int16_t, 6> imu_data;
    if (imu.ReadIMU6(imu_data) == 0) {
        sample_count++;

        // Print every 500 samples (~4Hz at 2kHz loop)
        if (sample_count % 500 == 0) {
            printf_("Sample %lu: ", sample_count);
            printf_("Accel[%6d,%6d,%6d] ",
                    imu_data[0], imu_data[1], imu_data[2]);
            printf_("Gyro[%6d,%6d,%6d]\n",
                    imu_data[3], imu_data[4], imu_data[5]);
        }

        // Exit after 10000 samples (~5 seconds)
        if (sample_count >= 10000) {
            Serial.println("\nData collection complete");
            Serial.println("\n=== Test Complete ===");
            Serial.println("*STOP*");
            while(1); // Halt
        }
    }

    // Regulate loop rate to 2kHz
    loop_timer = micros();
    while (inv_freq > (loop_timer - current_time)) {
        loop_timer = micros();
    }
}
