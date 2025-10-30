#ifndef __ICM_42688_P_H__
#define __ICM_42688_P_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "Invn/Drivers/Icm426xx/Icm426xxDriver_HL.h"
#include "Invn/Drivers/Icm426xx/Icm426xxSelfTest.h"

#ifdef __cplusplus
}
#endif

// ============================================================================
// ICM-42688-P Filter Configuration (Arduino Extension)
// ============================================================================

/**
 * @brief AAF (Anti-Alias Filter) bandwidth selection
 *
 * Enum for selecting AAF 3dB bandwidth from datasheet presets.
 * The ICM-42688-P uses a configurable 2nd-order IIR anti-alias filter
 * before decimation.
 *
 * Source: ICM-42688-P Datasheet Rev 1.8 (DS-000347), Section 5.3
 *
 * Usage guidelines:
 * - AAF frequency should not exceed ~0.45 × ODR to prevent aliasing
 * - Gyro default: ICM42688P_AAF_258HZ (Betaflight standard, MPU-6000 DLPF 260 equivalent)
 * - Accel default: ICM42688P_AAF_170HZ (good vibration rejection)
 * - Tuning range: ICM42688P_AAF_126HZ (tight) to ICM42688P_AAF_303HZ (loose)
 */
typedef enum {
    ICM42688P_AAF_42HZ = 0,
    ICM42688P_AAF_84HZ,
    ICM42688P_AAF_126HZ,
    ICM42688P_AAF_170HZ,      // Accel default
    ICM42688P_AAF_213HZ,
    ICM42688P_AAF_258HZ,      // Gyro default (Betaflight standard)
    ICM42688P_AAF_303HZ,
    ICM42688P_AAF_348HZ,
    ICM42688P_AAF_394HZ,
    ICM42688P_AAF_441HZ,
    ICM42688P_AAF_488HZ,
    ICM42688P_AAF_536HZ,
    ICM42688P_AAF_585HZ,
    ICM42688P_AAF_634HZ,
    ICM42688P_AAF_684HZ,
    ICM42688P_AAF_734HZ,
    ICM42688P_AAF_785HZ,
    ICM42688P_AAF_837HZ,
    ICM42688P_AAF_890HZ,
    ICM42688P_AAF_943HZ,
    ICM42688P_AAF_997HZ,
    ICM42688P_AAF_1051HZ
} icm42688p_aaf_bandwidth_t;

/**
 * @brief AAF preset configuration (internal)
 *
 * Maps enum to register values. Same preset applies to both
 * gyro (Bank 1) and accel (Bank 2) filters.
 */
typedef struct {
    uint16_t freq_hz;    // Target 3dB bandwidth (Hz)
    uint8_t delt;        // DELT parameter (6-bit)
    uint16_t deltsqr;    // DELTSQR parameter (12-bit)
    uint8_t bitshift;    // BITSHIFT parameter (4-bit)
} icm42688p_aaf_preset_t;

/**
 * @brief AAF preset lookup table
 *
 * Source: ICM-42688-P Datasheet Rev 1.8 (DS-000347)
 *         Section 5.3 "ANTI-ALIAS FILTER"
 *
 * Range: 42-1051 Hz (22 presets)
 * - Same table applies to both gyro (Bank 1) and accel (Bank 2)
 * - AAF frequency should not exceed ~0.45 × ODR to prevent aliasing
 *
 * Typical ODR limits:
 * - 2 kHz ODR → max AAF ~900 Hz
 * - 4 kHz ODR → max AAF ~1800 Hz
 * - 8 kHz ODR → max AAF ~3600 Hz
 *
 * Common usage:
 * - Gyro: 258 Hz (Betaflight default, matches MPU-6000 DLPF 260 Hz)
 * - Accel: 170 Hz (good vibration rejection for level mode)
 * - Tuning range: 126-303 Hz (tight to loose)
 */
static const icm42688p_aaf_preset_t ICM42688P_AAF_PRESETS[] = {
    {  42,  1,   1, 15},
    {  84,  2,   4, 13},
    { 126,  3,   9, 12},
    { 170,  4,  16, 11},  // Accel default
    { 213,  5,  25, 10},
    { 258,  6,  36, 10},  // Gyro default (Betaflight standard)
    { 303,  7,  49,  9},
    { 348,  8,  64,  9},
    { 394,  9,  81,  9},
    { 441, 10, 100,  8},
    { 488, 11, 122,  8},
    { 536, 12, 144,  8},
    { 585, 13, 170,  8},
    { 634, 14, 196,  7},
    { 684, 15, 224,  7},
    { 734, 16, 256,  7},
    { 785, 17, 288,  7},
    { 837, 18, 324,  7},
    { 890, 19, 360,  6},
    { 943, 20, 400,  6},
    { 997, 21, 440,  6},
    {1051, 22, 488,  6}
};

#define ICM42688P_AAF_PRESET_COUNT (sizeof(ICM42688P_AAF_PRESETS) / sizeof(ICM42688P_AAF_PRESETS[0]))

/**
 * @brief Configure gyroscope AAF (Anti-Alias Filter)
 *
 * @param s Pointer to inv_icm426xx driver instance
 * @param bandwidth Desired AAF bandwidth from icm42688p_aaf_bandwidth_t enum
 * @return 0 on success, negative error code on failure
 *
 * Configures the gyro AAF by:
 * 1. Looking up preset from ICM42688P_AAF_PRESETS table
 * 2. Switching to Bank 1 (gyro AAF registers)
 * 3. Writing DELT, DELTSQR, BITSHIFT parameters
 * 4. Returning to Bank 0
 *
 * Register sequence:
 * - GYRO_CONFIG_STATIC3 (0x0C): DELT[5:0]
 * - GYRO_CONFIG_STATIC4 (0x0D): DELTSQR[7:0]
 * - GYRO_CONFIG_STATIC5 (0x0E): BITSHIFT[7:4] | DELTSQR[11:8]
 */
static inline int icm42688p_set_gyro_aaf(struct inv_icm426xx *s, icm42688p_aaf_bandwidth_t bandwidth) {
    const icm42688p_aaf_preset_t *preset = &ICM42688P_AAF_PRESETS[bandwidth];
    int rc = 0;

    // Switch to Bank 1 (gyro AAF registers)
    rc |= inv_icm426xx_set_reg_bank(s, 1);

    // Write DELT (6-bit value in GYRO_CONFIG_STATIC3)
    uint8_t val = preset->delt & 0x3F;
    rc |= inv_icm426xx_write_reg(s, MPUREG_GYRO_CONFIG_STATIC3_B1, 1, &val);

    // Write DELTSQR low byte (GYRO_CONFIG_STATIC4)
    val = preset->deltsqr & 0xFF;
    rc |= inv_icm426xx_write_reg(s, MPUREG_GYRO_CONFIG_STATIC4_B1, 1, &val);

    // Write BITSHIFT (bits 7:4) and DELTSQR high nibble (bits 3:0) in GYRO_CONFIG_STATIC5
    val = ((preset->bitshift & 0x0F) << 4) | ((preset->deltsqr >> 8) & 0x0F);
    rc |= inv_icm426xx_write_reg(s, MPUREG_GYRO_CONFIG_STATIC5_B1, 1, &val);

    // Return to Bank 0
    rc |= inv_icm426xx_set_reg_bank(s, 0);

    return rc;
}

/**
 * @brief Configure accelerometer AAF (Anti-Alias Filter)
 *
 * @param s Pointer to inv_icm426xx driver instance
 * @param bandwidth Desired AAF bandwidth from icm42688p_aaf_bandwidth_t enum
 * @return 0 on success, negative error code on failure
 *
 * Configures the accel AAF by:
 * 1. Looking up preset from ICM42688P_AAF_PRESETS table
 * 2. Switching to Bank 2 (accel AAF registers)
 * 3. Writing DELT, DELTSQR, BITSHIFT parameters
 * 4. Returning to Bank 0
 *
 * Register sequence:
 * - ACCEL_CONFIG_STATIC2 (0x03): DELT[6:1] | AAF_DIS[0]
 * - ACCEL_CONFIG_STATIC3 (0x04): DELTSQR[7:0]
 * - ACCEL_CONFIG_STATIC4 (0x05): BITSHIFT[7:4] | DELTSQR[11:8]
 */
static inline int icm42688p_set_accel_aaf(struct inv_icm426xx *s, icm42688p_aaf_bandwidth_t bandwidth) {
    const icm42688p_aaf_preset_t *preset = &ICM42688P_AAF_PRESETS[bandwidth];
    int rc = 0;

    // Switch to Bank 2 (accel AAF registers)
    rc |= inv_icm426xx_set_reg_bank(s, 2);

    // Write DELT (bits 6:1) in ACCEL_CONFIG_STATIC2, bit 0 = AAF_DIS (0 = enabled)
    uint8_t val = ((preset->delt & 0x3F) << 1) | 0x00;  // AAF enabled
    rc |= inv_icm426xx_write_reg(s, MPUREG_ACCEL_CONFIG_STATIC2_B2, 1, &val);

    // Write DELTSQR low byte (ACCEL_CONFIG_STATIC3)
    val = preset->deltsqr & 0xFF;
    rc |= inv_icm426xx_write_reg(s, MPUREG_ACCEL_CONFIG_STATIC3_B2, 1, &val);

    // Write BITSHIFT (bits 7:4) and DELTSQR high nibble (bits 3:0) in ACCEL_CONFIG_STATIC4
    val = ((preset->bitshift & 0x0F) << 4) | ((preset->deltsqr >> 8) & 0x0F);
    rc |= inv_icm426xx_write_reg(s, MPUREG_ACCEL_CONFIG_STATIC4_B2, 1, &val);

    // Return to Bank 0
    rc |= inv_icm426xx_set_reg_bank(s, 0);

    return rc;
}

/**
 * @brief Verify AAF (Anti-Alias Filter) configuration
 *
 * @param s Pointer to inv_icm426xx driver instance
 * @param gyro_bandwidth Expected gyro AAF bandwidth enum
 * @param accel_bandwidth Expected accel AAF bandwidth enum
 * @return 0 if configuration matches, -1 if mismatch or read error
 *
 * Reads back AAF registers from both Bank 1 (gyro) and Bank 2 (accel)
 * and verifies they match the expected preset values.
 *
 * This provides hardware verification that the AAF configuration was
 * actually applied to the ICM-42688-P registers.
 */
static inline int icm42688p_verify_aaf(struct inv_icm426xx *s,
                                       icm42688p_aaf_bandwidth_t gyro_bandwidth,
                                       icm42688p_aaf_bandwidth_t accel_bandwidth) {
    const icm42688p_aaf_preset_t *gyro_preset = &ICM42688P_AAF_PRESETS[gyro_bandwidth];
    const icm42688p_aaf_preset_t *accel_preset = &ICM42688P_AAF_PRESETS[accel_bandwidth];
    int rc = 0;
    uint8_t val;

    // Verify Gyro AAF (Bank 1)
    rc |= inv_icm426xx_set_reg_bank(s, 1);

    // Read DELT
    rc |= inv_icm426xx_read_reg(s, MPUREG_GYRO_CONFIG_STATIC3_B1, 1, &val);
    if ((val & 0x3F) != (gyro_preset->delt & 0x3F)) return -1;

    // Read DELTSQR low byte
    rc |= inv_icm426xx_read_reg(s, MPUREG_GYRO_CONFIG_STATIC4_B1, 1, &val);
    if (val != (gyro_preset->deltsqr & 0xFF)) return -1;

    // Read BITSHIFT and DELTSQR high nibble
    rc |= inv_icm426xx_read_reg(s, MPUREG_GYRO_CONFIG_STATIC5_B1, 1, &val);
    uint8_t expected = ((gyro_preset->bitshift & 0x0F) << 4) | ((gyro_preset->deltsqr >> 8) & 0x0F);
    if (val != expected) return -1;

    // Verify Accel AAF (Bank 2)
    rc |= inv_icm426xx_set_reg_bank(s, 2);

    // Read DELT (bits 6:1)
    rc |= inv_icm426xx_read_reg(s, MPUREG_ACCEL_CONFIG_STATIC2_B2, 1, &val);
    if (((val >> 1) & 0x3F) != (accel_preset->delt & 0x3F)) return -1;

    // Read DELTSQR low byte
    rc |= inv_icm426xx_read_reg(s, MPUREG_ACCEL_CONFIG_STATIC3_B2, 1, &val);
    if (val != (accel_preset->deltsqr & 0xFF)) return -1;

    // Read BITSHIFT and DELTSQR high nibble
    rc |= inv_icm426xx_read_reg(s, MPUREG_ACCEL_CONFIG_STATIC4_B2, 1, &val);
    expected = ((accel_preset->bitshift & 0x0F) << 4) | ((accel_preset->deltsqr >> 8) & 0x0F);
    if (val != expected) return -1;

    // Return to Bank 0
    rc |= inv_icm426xx_set_reg_bank(s, 0);

    return rc;
}

/**
 * @brief Verify UI (User Interface) filter configuration
 *
 * @param s Pointer to inv_icm426xx driver instance
 * @return 0 if configuration is 1st-order wide mode, -1 if mismatch or read error
 *
 * Reads back UI filter registers from Bank 0 and verifies:
 * - GYRO_CONFIG1: UI_FILT_ORD[3:2] = 00 (1st order)
 * - ACCEL_CONFIG1: UI_FILT_ORD[4:3] = 00 (1st order)
 * - GYRO_ACCEL_CONFIG0: Both BW codes = 0 (ODR/2)
 *
 * This provides hardware verification that the UI filter configuration
 * was actually applied to the ICM-42688-P registers.
 */
static inline int icm42688p_verify_ui_filters_wide(struct inv_icm426xx *s) {
    int rc = 0;
    uint8_t val;

    // Switch to Bank 0
    rc |= inv_icm426xx_set_reg_bank(s, 0);

    // Verify GYRO_CONFIG1: UI_FILT_ORD[3:2] = 00
    rc |= inv_icm426xx_read_reg(s, MPUREG_GYRO_CONFIG1, 1, &val);
    if ((val & 0x0C) != 0x00) return -1;  // Bits [3:2] should be 00

    // Verify ACCEL_CONFIG1: UI_FILT_ORD[4:3] = 00
    rc |= inv_icm426xx_read_reg(s, MPUREG_ACCEL_CONFIG1, 1, &val);
    if ((val & 0x18) != 0x00) return -1;  // Bits [4:3] should be 00

    // Verify GYRO_ACCEL_CONFIG0: Both BW codes = 0
    rc |= inv_icm426xx_read_reg(s, MPUREG_ACCEL_GYRO_CONFIG0, 1, &val);
    if (val != 0x00) return -1;  // All bits should be 0

    return rc;
}

/**
 * @brief Configure UI (User Interface) filters to "wide" 1st-order mode
 *
 * @param s Pointer to inv_icm426xx driver instance
 * @return 0 on success, negative error code on failure
 *
 * Sets both gyro and accel UI filters to:
 * - Filter order: 1st order (minimal phase lag)
 * - Bandwidth: ODR/2 (wide, AAF dominates filtering)
 *
 * This configuration allows the AAF to be the dominant filter corner,
 * matching MPU-6000 DLPF 260 "wide" feel while maintaining proper
 * anti-aliasing protection.
 *
 * Register configuration:
 * - GYRO_CONFIG1 (0x51): UI_FILT_ORD = 00 (1st order)
 * - ACCEL_CONFIG1 (0x53): UI_FILT_ORD = 00 (1st order)
 * - GYRO_ACCEL_CONFIG0 (0x52): GYRO_UI_FILT_BW = 0 (ODR/2), ACCEL_UI_FILT_BW = 0 (ODR/2)
 *
 * Reference: ICM-42688-P Datasheet Rev 1.8, Section 5.3
 */
static inline int icm42688p_set_ui_filters_wide(struct inv_icm426xx *s) {
    int rc = 0;
    uint8_t val;

    // Switch to Bank 0 (UI filter registers are in Bank 0)
    rc |= inv_icm426xx_set_reg_bank(s, 0);

    // GYRO_CONFIG1 (0x51): Set UI_FILT_ORD[3:2] = 00 (1st order)
    // Read-modify-write to preserve other bits
    rc |= inv_icm426xx_read_reg(s, MPUREG_GYRO_CONFIG1, 1, &val);
    val &= ~0x0C;  // Clear bits [3:2]
    val |= 0x00;   // Set to 00 (1st order)
    rc |= inv_icm426xx_write_reg(s, MPUREG_GYRO_CONFIG1, 1, &val);

    // ACCEL_CONFIG1 (0x53): Set UI_FILT_ORD[4:3] = 00 (1st order)
    rc |= inv_icm426xx_read_reg(s, MPUREG_ACCEL_CONFIG1, 1, &val);
    val &= ~0x18;  // Clear bits [4:3]
    val |= 0x00;   // Set to 00 (1st order)
    rc |= inv_icm426xx_write_reg(s, MPUREG_ACCEL_CONFIG1, 1, &val);

    // ACCEL_GYRO_CONFIG0 (0x52): Set both BW codes to 0 (ODR/2)
    // GYRO_UI_FILT_BW[3:0] = 0000, ACCEL_UI_FILT_BW[7:4] = 0000
    val = 0x00;
    rc |= inv_icm426xx_write_reg(s, MPUREG_ACCEL_GYRO_CONFIG0, 1, &val);

    // Bank 0 is the default bank, no need to explicitly return
    // (all subsequent operations will be in Bank 0)

    return rc;
}

#endif /* __ICM_42688_P_H__ */