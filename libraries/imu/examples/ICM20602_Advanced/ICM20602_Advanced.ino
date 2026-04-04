/*
 * IMU Library - ICM-20602 Advanced Configuration Example
 *
 * Demonstrates ICM-20602 specific advanced features using the _Ex APIs:
 * - Direct register access (ReadReg_Ex, WriteReg_Ex, WriteRegVerify_Ex)
 * - DLPF configuration (Digital Low Pass Filter)
 * - Sample rate divider configuration
 * - FSR configuration (SetGyroFSR_Ex, SetAccelFSR_Ex)
 *
 * NOTE: This example is ICM-206xx SPECIFIC (ICM-20601/20602/20689).
 * It will detect and reject other IMU chips.
 * Use the generic examples for ICM-42688-P/MPU-6000/MPU-9250.
 *
 * REGISTER MAP REFERENCE (ICM-20602 Datasheet):
 * - 0x19 SMPLRT_DIV: Sample rate divider
 * - 0x1A CONFIG: DLPF configuration (gyro)
 * - 0x1B GYRO_CONFIG: Gyro FSR [4:3]
 * - 0x1C ACCEL_CONFIG: Accel FSR [4:3]
 * - 0x1D ACCEL_CONFIG2: Accel DLPF configuration
 * - 0x6B PWR_MGMT_1: Power management
 *
 * HARDWARE CONFIGURATION:
 * - Target: NERO F7 flight controller (STM32F722, ICM-20602)
 * - Uses BKMN-NERO.h BoardConfig for pin assignments
 * - Will reject other IMU chips at runtime
 *
 */

#include <IMU.h>
#include <SPI.h>

// Board configuration - ICM-20602 specific (NERO F7 flight controller)
// This example requires ICM-206xx hardware. Other IMU chips will be rejected.
#include "targets/BKMN-NERO.h"

// ============================================================================
// ICM-20602 Register Definitions (MPU-6500 class register map)
// ============================================================================

// Power management
#define REG_PWR_MGMT_1          0x6B
#define PWR_CLKSEL_PLL          0x01    // PLL clock source
#define PWR_DEVICE_RESET        0x80    // Device reset bit

// Configuration registers
#define REG_SMPLRT_DIV          0x19    // Sample rate divider: Rate = Internal / (1 + DIV)
#define REG_CONFIG              0x1A    // DLPF configuration (gyro)
#define REG_GYRO_CONFIG         0x1B    // [4:3] = FSR, [2:0] = FCHOICE_B
#define REG_ACCEL_CONFIG        0x1C    // [4:3] = FSR
#define REG_ACCEL_CONFIG2       0x1D    // [3] = FCHOICE_B, [2:0] = A_DLPF_CFG

// Interrupt registers
#define REG_INT_PIN_CFG         0x37
#define REG_INT_ENABLE          0x38
#define REG_INT_STATUS          0x3A

// WHO_AM_I
#define REG_WHO_AM_I            0x75

// ============================================================================
// DLPF Configuration (Digital Low Pass Filter)
// ============================================================================
// ICM-20602 Gyro DLPF (CONFIG register, DLPF_CFG bits [2:0])
// When FCHOICE_B = 00, DLPF is enabled
//
// DLPF_CFG | Gyro BW (Hz) | Gyro Delay (ms) | Rate (kHz)
// ---------|--------------|-----------------|------------
//    0     |     250      |      0.97       |    8
//    1     |     176      |      1.9        |    1
//    2     |      92      |      3.9        |    1
//    3     |      41      |      5.9        |    1
//    4     |      20      |      9.9        |    1
//    5     |      10      |     17.85       |    1
//    6     |       5      |     33.48       |    1
//    7     |    3281      |      0.17       |    8   (DLPF bypass)
//
// IMPORTANT: When DLPF_CFG = 0 or 7, internal rate is 8kHz and SMPLRT_DIV is bypassed
// When DLPF_CFG = 1-6, internal rate is 1kHz and SMPLRT_DIV is active

#define DLPF_250HZ              0x00    // 250 Hz, 8 kHz internal rate (divider bypassed)
#define DLPF_176HZ              0x01    // 176 Hz, 1 kHz internal rate
#define DLPF_92HZ               0x02    // 92 Hz, 1 kHz internal rate
#define DLPF_41HZ               0x03    // 41 Hz, 1 kHz internal rate
#define DLPF_20HZ               0x04    // 20 Hz, 1 kHz internal rate
#define DLPF_10HZ               0x05    // 10 Hz, 1 kHz internal rate
#define DLPF_5HZ                0x06    // 5 Hz, 1 kHz internal rate
#define DLPF_BYPASS             0x07    // 3281 Hz, 8 kHz internal rate (DLPF bypass)

// Accel DLPF (ACCEL_CONFIG2 register, A_DLPF_CFG bits [2:0])
// When ACCEL_FCHOICE_B = 0, DLPF is enabled
//
// A_DLPF_CFG | Accel BW (Hz) | Accel Delay (ms) | Rate (kHz)
// -----------|---------------|------------------|------------
//     0      |     218.1     |      1.88        |    1
//     1      |     218.1     |      1.88        |    1
//     2      |      99.0     |      2.88        |    1
//     3      |      44.8     |      4.88        |    1
//     4      |      21.2     |      8.87        |    1
//     5      |      10.2     |     16.83        |    1
//     6      |       5.1     |     32.48        |    1
//     7      |     420.0     |      1.38        |    1

#define ACCEL_DLPF_218HZ        0x00    // 218 Hz
#define ACCEL_DLPF_99HZ         0x02    // 99 Hz
#define ACCEL_DLPF_45HZ         0x03    // 45 Hz
#define ACCEL_DLPF_21HZ         0x04    // 21 Hz
#define ACCEL_DLPF_10HZ         0x05    // 10 Hz
#define ACCEL_DLPF_5HZ          0x06    // 5 Hz
#define ACCEL_DLPF_420HZ        0x07    // 420 Hz (widest bandwidth)

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
 * @brief Configure gyro DLPF using direct register access
 * @param dlpf_cfg DLPF configuration (0-7)
 *
 * NOTE: DLPF_CFG affects sample rate!
 * - DLPF_CFG=0 or 7: 8kHz internal rate, SMPLRT_DIV bypassed
 * - DLPF_CFG=1-6: 1kHz internal rate, SMPLRT_DIV active
 */
void setGyroDLPF(uint8_t dlpf_cfg) {
    imu.WriteRegVerify_Ex(REG_CONFIG, dlpf_cfg & 0x07);
}

/**
 * @brief Configure accel DLPF using direct register access
 * @param dlpf_cfg DLPF configuration (0-7)
 */
void setAccelDLPF(uint8_t dlpf_cfg) {
    // Read current register to preserve FCHOICE_B bit
    uint8_t reg = imu.ReadReg_Ex(REG_ACCEL_CONFIG2);
    reg = (reg & 0xF8) | (dlpf_cfg & 0x07);  // Preserve upper bits, set DLPF
    imu.WriteRegVerify_Ex(REG_ACCEL_CONFIG2, reg);
}

/**
 * @brief Configure sample rate divider
 * @param divider Sample rate divider (0-255)
 *
 * Output Rate = Internal_Rate / (1 + divider)
 * NOTE: Only effective when gyro DLPF_CFG = 1-6 (1kHz internal rate)
 * When DLPF_CFG = 0 or 7, output rate is always 8kHz
 */
void setSampleRateDivider(uint8_t divider) {
    imu.WriteRegVerify_Ex(REG_SMPLRT_DIV, divider);
}

/**
 * @brief Read and display current configuration
 */
void dumpConfiguration() {
    uint8_t pwr = imu.ReadReg_Ex(REG_PWR_MGMT_1);
    uint8_t smplrt = imu.ReadReg_Ex(REG_SMPLRT_DIV);
    uint8_t config = imu.ReadReg_Ex(REG_CONFIG);
    uint8_t gyro_cfg = imu.ReadReg_Ex(REG_GYRO_CONFIG);
    uint8_t accel_cfg = imu.ReadReg_Ex(REG_ACCEL_CONFIG);
    uint8_t accel_cfg2 = imu.ReadReg_Ex(REG_ACCEL_CONFIG2);

    CI_PRINTF("  PWR_MGMT_1:     0x%02X\n", pwr);
    CI_PRINTF("  SMPLRT_DIV:     0x%02X (%d)\n", smplrt, smplrt);
    CI_PRINTF("  CONFIG:         0x%02X (DLPF_CFG=%d)\n", config, config & 0x07);
    CI_PRINTF("  GYRO_CONFIG:    0x%02X (FSR=%d)\n", gyro_cfg, (gyro_cfg >> 3) & 0x03);
    CI_PRINTF("  ACCEL_CONFIG:   0x%02X (FSR=%d)\n", accel_cfg, (accel_cfg >> 3) & 0x03);
    CI_PRINTF("  ACCEL_CONFIG2:  0x%02X (A_DLPF=%d)\n", accel_cfg2, accel_cfg2 & 0x07);

    // Calculate effective sample rate
    uint8_t dlpf = config & 0x07;
    uint16_t internal_rate = (dlpf == 0 || dlpf == 7) ? 8000 : 1000;
    uint16_t output_rate = (dlpf == 0 || dlpf == 7) ? 8000 : internal_rate / (1 + smplrt);
    CI_PRINTF("  Effective rate: %d Hz (internal: %d Hz)\n", output_rate, internal_rate);
}

/**
 * @brief Get DLPF bandwidth description
 */
const char* getDLPFBandwidth(uint8_t dlpf_cfg) {
    switch (dlpf_cfg & 0x07) {
        case 0: return "250 Hz";
        case 1: return "176 Hz";
        case 2: return "92 Hz";
        case 3: return "41 Hz";
        case 4: return "20 Hz";
        case 5: return "10 Hz";
        case 6: return "5 Hz";
        case 7: return "3281 Hz (bypass)";
        default: return "Unknown";
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

    CI_LOG("\n=== IMU Library - ICM-20602 Advanced Configuration ===\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();

    // Display pin configuration
    CI_LOG("Pin Configuration (BoardConfig):\n");
    CI_PRINTF("  CS: %d, MOSI: %d, MISO: %d, SCLK: %d\n",
           (int)BoardConfig::imu.spi.cs_pin.toPinName(),
           (int)BoardConfig::imu.spi.mosi_pin.toPinName(),
           (int)BoardConfig::imu.spi.miso_pin.toPinName(),
           (int)BoardConfig::imu.spi.sclk_pin.toPinName());
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

    // Check chip type - this example is ICM-206xx specific
    IMU::ChipType chip = imu.GetChipType();
    bool is_icm206xx = (chip == IMU::ChipType::ICM20602 ||
                        chip == IMU::ChipType::ICM20601 ||
                        chip == IMU::ChipType::ICM20689);

    if (!is_icm206xx) {
        CI_PRINTF("ERROR: This example requires ICM-206xx (detected 0x%02X)\n",
               static_cast<uint8_t>(chip));
        CI_LOG("Use ICM42688P_Advanced for ICM-42688-P.\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    // Display detected chip
    const char* chip_name = "ICM-206xx";
    switch (chip) {
        case IMU::ChipType::ICM20601: chip_name = "ICM-20601"; break;
        case IMU::ChipType::ICM20602: chip_name = "ICM-20602"; break;
        case IMU::ChipType::ICM20689: chip_name = "ICM-20689"; break;
        default: break;
    }
    CI_PRINTF("Detected chip: %s (0x%02X)\n\n", chip_name, static_cast<uint8_t>(chip));

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
    CI_LOG("SAFE preset applied (DLPF=2, ~92Hz bandwidth)\n");
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
    // STEP 3: Custom DLPF configuration using _Ex APIs
    // ========================================================================
    CI_LOG("\n--- Step 3: Custom DLPF Configuration via _Ex APIs ---\n");
    CI_LOG("Configuring: Gyro 41Hz, Accel 45Hz, 500Hz output rate\n\n");

    // Set gyro DLPF to 41 Hz (tight filtering, good for noisy environments)
    CI_LOG("Setting Gyro DLPF to 41 Hz (DLPF_CFG=3)...\n");
    setGyroDLPF(DLPF_41HZ);

    // Set accel DLPF to 45 Hz
    CI_LOG("Setting Accel DLPF to 45 Hz (A_DLPF_CFG=3)...\n");
    setAccelDLPF(ACCEL_DLPF_45HZ);

    // Set sample rate divider for 500 Hz output (1000 / (1+1) = 500 Hz)
    CI_LOG("Setting SMPLRT_DIV=1 for 500 Hz output rate...\n");
    setSampleRateDivider(1);

    // Wait for filters to stabilize
    delay(50);

    CI_LOG("\nCustom configuration applied:\n");
    dumpConfiguration();

    // Collect samples with custom config
    CI_LOG("\nCollecting 10 samples with custom 500Hz/41Hz config...\n");
    for (int i = 0; i < 10; i++) {
        std::array<int16_t, 6> data;
        if (imu.ReadIMU6(data) == 0) {
            if (i % 2 == 0) {
                CI_PRINTF("  [%d] Accel[%6d,%6d,%6d] Gyro[%6d,%6d,%6d]\n",
                       i, data[0], data[1], data[2], data[3], data[4], data[5]);
            }
        }
        delay(2);  // 2ms delay for 500Hz sampling
    }

    // ========================================================================
    // STEP 4: High-rate configuration (8kHz, DLPF bypass)
    // ========================================================================
    CI_LOG("\n--- Step 4: High-Rate Configuration (8kHz) ---\n");
    CI_LOG("Configuring: DLPF bypass for 8kHz operation\n\n");

    // Set gyro DLPF to bypass (8kHz internal rate)
    CI_LOG("Setting Gyro DLPF to bypass (DLPF_CFG=0, 250Hz BW)...\n");
    setGyroDLPF(DLPF_250HZ);

    // Set accel DLPF to widest bandwidth
    CI_LOG("Setting Accel DLPF to 420 Hz...\n");
    setAccelDLPF(ACCEL_DLPF_420HZ);

    // Note: SMPLRT_DIV is bypassed when DLPF_CFG=0, output rate is 8kHz
    delay(50);

    CI_LOG("\nHigh-rate configuration applied:\n");
    dumpConfiguration();

    // Collect samples at 8kHz
    CI_LOG("\nCollecting 10 samples at 8kHz rate...\n");
    for (int i = 0; i < 10; i++) {
        std::array<int16_t, 6> data;
        if (imu.ReadIMU6(data) == 0) {
            if (i % 2 == 0) {
                CI_PRINTF("  [%d] Accel[%6d,%6d,%6d] Gyro[%6d,%6d,%6d]\n",
                       i, data[0], data[1], data[2], data[3], data[4], data[5]);
            }
        }
        delayMicroseconds(125);  // 125us for 8kHz sampling
    }

    // ========================================================================
    // STEP 5: Demonstrate FSR change using SetGyroFSR_Ex
    // ========================================================================
    CI_LOG("\n--- Step 5: FSR Configuration via SetGyroFSR_Ex ---\n");

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
    // STEP 6: Return to BALANCED preset
    // ========================================================================
    CI_LOG("\n--- Step 6: Return to BALANCED Preset ---\n");
    if (imu.ApplyPreset(IMU::Preset::BALANCED) != IMU::Result::OK) {
        CI_LOG("ERROR: Failed to apply BALANCED preset!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }
    CI_LOG("BALANCED preset applied (DLPF=0, 8kHz operation)\n");
    dumpConfiguration();

    CI_LOG("\n=== ICM-20602 Advanced Configuration Complete ===\n");
    CI_LOG("*STOP*\n");
}

void loop() {
    // Nothing to do
}
