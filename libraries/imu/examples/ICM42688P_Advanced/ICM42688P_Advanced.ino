/*
 * IMU Library - ICM-42688-P Advanced Configuration Example
 *
 * Demonstrates ICM-42688-P specific advanced features using the _Ex APIs:
 * - Direct register access (ReadReg_Ex, WriteReg_Ex, WriteRegVerify_Ex)
 * - Custom ODR configuration (1kHz, 2kHz, 4kHz, 8kHz)
 * - Anti-Alias Filter (AAF) configuration
 * - UI Filter bandwidth configuration
 * - FSR configuration (SetGyroFSR_Ex, SetAccelFSR_Ex)
 *
 * NOTE: This example is ICM-42688-P SPECIFIC. It will detect and reject
 * other IMU chips. Use the generic examples for MPU-6000/9250/ICM-206xx.
 *
 * REGISTER MAP REFERENCE (ICM-42688-P Datasheet):
 * - Bank 0: 0x4E PWR_MGMT0, 0x4F GYRO_CONFIG0, 0x50 ACCEL_CONFIG0
 *           0x52 GYRO_ACCEL_CONFIG0 (UI filters)
 * - Bank 1: 0x0C-0x0E GYRO_CONFIG_STATIC3-5 (Gyro AAF)
 * - Bank 2: 0x03-0x05 ACCEL_CONFIG_STATIC2-4 (Accel AAF)
 *
 * HARDWARE CONFIGURATION:
 * - Target: NUCLEO_F411RE with JHEF411 breakout board (ICM-42688-P)
 * - Uses NUCLEO_F411RE_JHEF411.h BoardConfig for pin assignments
 * - Will reject other IMU chips at runtime
 *
 * CI/HIL INTEGRATION:
 * - RTT output for automated testing
 * - Serial output for Arduino IDE
 * - Deterministic exit with "*STOP*" wildcard
 */

#include <IMU.h>
#include <ci_log.h>
#include <SPI.h>
#include <libPrintf.h>

// CI_PRINTF requires putchar_() for libPrintf output routing
extern "C" void putchar_(char c) {
#ifdef USE_RTT
    SEGGER_RTT_PutChar(0, c);
#else
    Serial.write(c);
#endif
}

// Board configuration - ICM-42688-P specific (NUCLEO_F411RE with JHEF411 breakout)
// This example requires ICM-42688-P hardware. Other IMU chips will be rejected.
#include "../../../../targets/NUCLEO_F411RE_JHEF411.h"

// ============================================================================
// ICM-42688-P Register Definitions (Bank 0 unless noted)
// ============================================================================

// Bank selection
#define REG_BANK_SEL            0x76

// Power management
#define REG_PWR_MGMT0           0x4E
#define PWR_GYRO_LN             0x0C    // Gyro Low Noise mode
#define PWR_ACCEL_LN            0x03    // Accel Low Noise mode

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
// Gyro: [3:0], Accel: [7:4]
#define UI_BW_ODR_DIV_2         0x00    // Widest bandwidth
#define UI_BW_ODR_DIV_4         0x01
#define UI_BW_ODR_DIV_5         0x02
#define UI_BW_ODR_DIV_8         0x03
#define UI_BW_ODR_DIV_10        0x04
#define UI_BW_ODR_DIV_16        0x05
#define UI_BW_ODR_DIV_20        0x06
#define UI_BW_ODR_DIV_40        0x07
#define UI_BW_LOW_LATENCY       0x0E    // Low latency mode (BW = ODR/2 or ODR/4)
#define UI_BW_LOW_LATENCY_X2    0x0F    // Extra low latency (recommended default)

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

// ============================================================================
// AAF Configuration Lookup Table (from ICM-42688-P datasheet Table 15)
// ============================================================================

struct AAFConfigEx {
    uint16_t bw_hz;     // 3dB bandwidth
    uint8_t  delt;      // DELT value
    uint16_t deltSqr;   // DELTSQR value
    uint8_t  bitshift;  // BITSHIFT value
};

// Common AAF configurations (ICM-42688-P)
const AAFConfigEx AAF_42HZ   = {42,   1,    1,  15};
const AAFConfigEx AAF_84HZ   = {84,   2,    4,  13};
const AAFConfigEx AAF_170HZ  = {170,  4,   16,  11};  // Recommended for accel
const AAFConfigEx AAF_258HZ  = {258,  6,   36,  10};  // Recommended for gyro
const AAFConfigEx AAF_536HZ  = {536, 12,  144,   8};
const AAFConfigEx AAF_997HZ  = {997, 21,  440,   6};  // Highest BW

// ============================================================================
// Global Objects
// ============================================================================

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin,
                 BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin,
                 BoardConfig::imu.spi.get_ssel_pin());

// Create IMU instance
IMU imu;

// ============================================================================
// Helper Functions
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
 * @param odr_code ODR code (ODR_1KHZ, ODR_2KHZ, ODR_4KHZ, ODR_8KHZ)
 */
void setGyroODR(uint8_t odr_code) {
    selectBank(BANK_0);
    uint8_t reg = imu.ReadReg_Ex(REG_GYRO_CONFIG0);
    reg = (reg & 0xF0) | (odr_code & 0x0F);  // Preserve FSR, set ODR
    imu.WriteRegVerify_Ex(REG_GYRO_CONFIG0, reg);
}

/**
 * @brief Configure accel ODR using direct register access
 * @param odr_code ODR code (ODR_1KHZ, ODR_2KHZ, ODR_4KHZ, ODR_8KHZ)
 */
void setAccelODR(uint8_t odr_code) {
    selectBank(BANK_0);
    uint8_t reg = imu.ReadReg_Ex(REG_ACCEL_CONFIG0);
    reg = (reg & 0xF0) | (odr_code & 0x0F);  // Preserve FSR, set ODR
    imu.WriteRegVerify_Ex(REG_ACCEL_CONFIG0, reg);
}

/**
 * @brief Configure UI filter bandwidth
 * @param gyro_bw Gyro UI filter code (0-7 or 0x0E-0x0F for low latency)
 * @param accel_bw Accel UI filter code
 */
void setUIFilters(uint8_t gyro_bw, uint8_t accel_bw) {
    selectBank(BANK_0);
    uint8_t reg = (accel_bw << 4) | (gyro_bw & 0x0F);
    imu.WriteRegVerify_Ex(REG_GYRO_ACCEL_CONFIG0, reg);
}

/**
 * @brief Configure gyro Anti-Alias Filter (requires Bank 1)
 * @param delt DELT value from datasheet
 * @param deltSqr DELTSQR value from datasheet
 * @param bitshift BITSHIFT value from datasheet
 */
void setGyroAAF(uint8_t delt, uint16_t deltSqr, uint8_t bitshift) {
    selectBank(BANK_1);
    imu.WriteReg_Ex(BANK1_GYRO_CONFIG3, delt);
    imu.WriteReg_Ex(BANK1_GYRO_CONFIG4, deltSqr & 0xFF);
    imu.WriteReg_Ex(BANK1_GYRO_CONFIG5, (deltSqr >> 8) | (bitshift << 4));
    selectBank(BANK_0);  // Return to Bank 0
}

/**
 * @brief Configure accel Anti-Alias Filter (requires Bank 2)
 * @param delt DELT value from datasheet
 * @param deltSqr DELTSQR value from datasheet
 * @param bitshift BITSHIFT value from datasheet
 */
void setAccelAAF(uint8_t delt, uint16_t deltSqr, uint8_t bitshift) {
    selectBank(BANK_2);
    imu.WriteReg_Ex(BANK2_ACCEL_CONFIG2, delt << 1);  // Note: shifted by 1
    imu.WriteReg_Ex(BANK2_ACCEL_CONFIG3, deltSqr & 0xFF);
    imu.WriteReg_Ex(BANK2_ACCEL_CONFIG4, (deltSqr >> 8) | (bitshift << 4));
    selectBank(BANK_0);  // Return to Bank 0
}

/**
 * @brief Read and display current configuration
 */
void dumpConfiguration() {
    selectBank(BANK_0);

    uint8_t pwr = imu.ReadReg_Ex(REG_PWR_MGMT0);
    uint8_t gyro_cfg = imu.ReadReg_Ex(REG_GYRO_CONFIG0);
    uint8_t accel_cfg = imu.ReadReg_Ex(REG_ACCEL_CONFIG0);
    uint8_t ui_cfg = imu.ReadReg_Ex(REG_GYRO_ACCEL_CONFIG0);

    CI_PRINTF("  PWR_MGMT0:          0x%02X\n", pwr);
    CI_PRINTF("  GYRO_CONFIG0:       0x%02X (FSR=%d, ODR=%d)\n",
           gyro_cfg, (gyro_cfg >> 5) & 0x07, gyro_cfg & 0x0F);
    CI_PRINTF("  ACCEL_CONFIG0:      0x%02X (FSR=%d, ODR=%d)\n",
           accel_cfg, (accel_cfg >> 5) & 0x07, accel_cfg & 0x0F);
    CI_PRINTF("  GYRO_ACCEL_CONFIG0: 0x%02X (Gyro UI=%d, Accel UI=%d)\n",
           ui_cfg, ui_cfg & 0x0F, (ui_cfg >> 4) & 0x0F);

    // Read AAF config from Bank 1
    selectBank(BANK_1);
    uint8_t gyro_aaf3 = imu.ReadReg_Ex(BANK1_GYRO_CONFIG3);
    uint8_t gyro_aaf4 = imu.ReadReg_Ex(BANK1_GYRO_CONFIG4);
    uint8_t gyro_aaf5 = imu.ReadReg_Ex(BANK1_GYRO_CONFIG5);
    CI_PRINTF("  Gyro AAF:           delt=%d, deltSqr=%d, bitshift=%d\n",
           gyro_aaf3, gyro_aaf4 | ((gyro_aaf5 & 0x0F) << 8), gyro_aaf5 >> 4);

    selectBank(BANK_2);
    uint8_t accel_aaf2 = imu.ReadReg_Ex(BANK2_ACCEL_CONFIG2);
    uint8_t accel_aaf3 = imu.ReadReg_Ex(BANK2_ACCEL_CONFIG3);
    uint8_t accel_aaf4 = imu.ReadReg_Ex(BANK2_ACCEL_CONFIG4);
    CI_PRINTF("  Accel AAF:          delt=%d, deltSqr=%d, bitshift=%d\n",
           accel_aaf2 >> 1, accel_aaf3 | ((accel_aaf4 & 0x0F) << 8), accel_aaf4 >> 4);

    selectBank(BANK_0);
}

/**
 * @brief Get ODR in Hz from code
 */
uint16_t odrCodeToHz(uint8_t code) {
    switch (code & 0x0F) {
        case ODR_8KHZ: return 8000;
        case ODR_4KHZ: return 4000;
        case ODR_2KHZ: return 2000;
        case ODR_1KHZ: return 1000;
        default: return 0;
    }
}

// ============================================================================
// Main Program
// ============================================================================

void setup() {
    // Initialize communication (Serial or RTT)
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial) delay(10);
#endif

    CI_LOG("\n=== IMU Library - ICM-42688-P Advanced Configuration ===\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();

    // Display pin configuration
    CI_LOG("Pin Configuration (BoardConfig):\n");
    CI_PRINTF("  CS: %d, MOSI: %d, MISO: %d, SCLK: %d\n",
           (int)BoardConfig::imu.spi.cs_pin,
           (int)BoardConfig::imu.spi.mosi_pin,
           (int)BoardConfig::imu.spi.miso_pin,
           (int)BoardConfig::imu.spi.sclk_pin);
    CI_PRINTF("  SPI Speed: %lu Hz\n\n", (unsigned long)BoardConfig::imu.spi.freq_hz);

    // Give IMU time to stabilize
    delay(5);

    // Initialize IMU
    CI_LOG("Initializing IMU...\n");
    if (imu.Init(spi_bus, BoardConfig::imu.spi.cs_pin, BoardConfig::imu.spi.freq_hz) != IMU::Result::OK) {
        CI_LOG("ERROR: Failed to initialize IMU!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    // Check chip type - this example is ICM-42688-P specific
    IMU::ChipType chip = imu.GetChipType();
    if (chip != IMU::ChipType::ICM42688_P) {
        CI_PRINTF("ERROR: This example requires ICM-42688-P (detected 0x%02X)\n",
               static_cast<uint8_t>(chip));
        CI_LOG("Use generic examples for other IMU chips.\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }
    CI_LOG("Detected chip: ICM-42688-P (0x47)\n\n");

    // ========================================================================
    // STEP 1: Show default configuration after Init()
    // ========================================================================
    CI_LOG("--- Step 1: Default Configuration ---\n");
    dumpConfiguration();

    // ========================================================================
    // STEP 2: Apply SAFE preset as baseline
    // ========================================================================
    CI_LOG("\n--- Step 2: Apply SAFE Preset (baseline) ---\n");
    if (imu.ApplyPreset(IMU::Preset::SAFE) != IMU::Result::OK) {
        CI_LOG("ERROR: Failed to apply SAFE preset!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }
    CI_LOG("SAFE preset applied and verified\n");
    dumpConfiguration();

    // Collect baseline samples
    CI_LOG("\nCollecting 10 samples with SAFE preset...\n");
    for (int i = 0; i < 10; i++) {
        std::array<int16_t, 6> data;
        if (imu.ReadIMU6(data) == 0) {
            if (i % 2 == 0) {
                CI_PRINTF("  [%d] Accel[%6d,%6d,%6d] Gyro[%6d,%6d,%6d]\n",
                       i, data[0], data[1], data[2], data[3], data[4], data[5]);
            }
        }
        delay(10);
    }

    // ========================================================================
    // STEP 3: Custom configuration using _Ex APIs
    // ========================================================================
    CI_LOG("\n--- Step 3: Custom Configuration via _Ex APIs ---\n");
    CI_LOG("Configuring: 8kHz ODR, 997Hz AAF, low-latency UI filters\n\n");

    // Set ODR to 8kHz (maximum rate)
    CI_LOG("Setting Gyro ODR to 8kHz...\n");
    setGyroODR(ODR_8KHZ);

    CI_LOG("Setting Accel ODR to 8kHz...\n");
    setAccelODR(ODR_8KHZ);

    // Set AAF to widest bandwidth (997 Hz)
    CI_LOG("Setting Gyro AAF to 997 Hz...\n");
    setGyroAAF(AAF_997HZ.delt, AAF_997HZ.deltSqr, AAF_997HZ.bitshift);

    CI_LOG("Setting Accel AAF to 997 Hz...\n");
    setAccelAAF(AAF_997HZ.delt, AAF_997HZ.deltSqr, AAF_997HZ.bitshift);

    // Set UI filters to low latency mode
    CI_LOG("Setting UI filters to low-latency mode...\n");
    setUIFilters(UI_BW_LOW_LATENCY_X2, UI_BW_LOW_LATENCY_X2);

    // Wait for filters to stabilize
    delay(50);

    CI_LOG("\nCustom configuration applied:\n");
    dumpConfiguration();

    // Collect samples with custom config
    CI_LOG("\nCollecting 10 samples with custom 8kHz config...\n");
    for (int i = 0; i < 10; i++) {
        std::array<int16_t, 6> data;
        if (imu.ReadIMU6(data) == 0) {
            if (i % 2 == 0) {
                CI_PRINTF("  [%d] Accel[%6d,%6d,%6d] Gyro[%6d,%6d,%6d]\n",
                       i, data[0], data[1], data[2], data[3], data[4], data[5]);
            }
        }
        delay(1);  // 1ms delay for 8kHz sampling
    }

    // ========================================================================
    // STEP 4: Demonstrate FSR change using SetGyroFSR_Ex
    // ========================================================================
    CI_LOG("\n--- Step 4: FSR Configuration via SetGyroFSR_Ex ---\n");

    CI_LOG("Setting Gyro FSR to +/-250 DPS (highest resolution)...\n");
    if (imu.SetGyroFSR_Ex(GyroFSR::DPS_250) != IMU::Result::OK) {
        CI_LOG("ERROR: Failed to set gyro FSR!\n");
    }

    CI_LOG("Setting Accel FSR to +/-2G (highest resolution)...\n");
    if (imu.SetAccelFSR_Ex(AccelFSR::G_2) != IMU::Result::OK) {
        CI_LOG("ERROR: Failed to set accel FSR!\n");
    }

    CI_PRINTF("Gyro sensitivity: %.1f LSB/dps\n", imu.GetGyroSensitivity());
    CI_PRINTF("Accel sensitivity: %.1f LSB/g\n", imu.GetAccelSensitivity());

    CI_LOG("\nConfiguration after FSR change:\n");
    dumpConfiguration();

    // Collect samples with high-resolution FSR
    CI_LOG("\nCollecting 10 samples with +/-250 DPS / +/-2G FSR...\n");
    for (int i = 0; i < 10; i++) {
        std::array<int16_t, 6> data;
        if (imu.ReadIMU6(data) == 0) {
            if (i % 2 == 0) {
                // Convert to physical units with new sensitivity
                float ax = data[0] / imu.GetAccelSensitivity();
                float ay = data[1] / imu.GetAccelSensitivity();
                float az = data[2] / imu.GetAccelSensitivity();
                float gx = data[3] / imu.GetGyroSensitivity();
                float gy = data[4] / imu.GetGyroSensitivity();
                float gz = data[5] / imu.GetGyroSensitivity();
                CI_PRINTF("  [%d] Accel(g): %6.3f,%6.3f,%6.3f | Gyro(dps): %6.2f,%6.2f,%6.2f\n",
                       i, ax, ay, az, gx, gy, gz);
            }
        }
        delay(1);
    }

    // ========================================================================
    // STEP 5: Return to BALANCED preset
    // ========================================================================
    CI_LOG("\n--- Step 5: Return to BALANCED Preset ---\n");
    if (imu.ApplyPreset(IMU::Preset::BALANCED) != IMU::Result::OK) {
        CI_LOG("ERROR: Failed to apply BALANCED preset!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }
    CI_LOG("BALANCED preset applied and verified\n");
    dumpConfiguration();

    CI_LOG("\n=== ICM-42688-P Advanced Configuration Complete ===\n");
    CI_LOG("*STOP*\n");
}

void loop() {
    // Nothing to do
}
