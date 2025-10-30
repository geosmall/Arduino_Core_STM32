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
 * @brief Set UI (User Interface) filters with configurable bandwidth code and filter order
 *
 * Configures the post-AAF low-pass filters in the signal path. These filters operate
 * after the Anti-Aliasing Filter (AAF) and before data reaches the FIFO/registers.
 *
 * @param s Pointer to device structure
 * @param ui_bw_code Filter bandwidth code (0-15):
 *                   - 0: ODR/2 (widest, lowest delay)
 *                   - 1-14: Progressively narrower bandwidths
 *                   - 15: Low-latency path (trivial decimation, Betaflight default)
 * @param gyro_order Gyro filter order (1-3):
 *                   - 1: 1st order
 *                   - 2: 2nd order (Betaflight default)
 *                   - 3: 3rd order
 * @param accel_order Accel filter order (1-3): Same as gyro_order
 *
 * @return 0 on success, negative on error
 *
 * Reference: ICM-42688-P Datasheet Rev 1.8, Section 5.3
 *           Betaflight_Filtering.md Section 2 (UI filter registers)
 */
static inline int icm42688p_set_ui_filters(struct inv_icm426xx *s, uint8_t ui_bw_code, uint8_t gyro_order, uint8_t accel_order) {
    int rc = 0;
    uint8_t val;

    // Validate inputs
    if (ui_bw_code > 15 || gyro_order < 1 || gyro_order > 3 || accel_order < 1 || accel_order > 3) {
        return -1;
    }

    // Convert order (1/2/3) to register encoding (0/1/2)
    uint8_t gyro_ord_bits = (gyro_order - 1) << 2;   // bits [3:2]
    uint8_t accel_ord_bits = (accel_order - 1) << 3; // bits [4:3]

    // Switch to Bank 0 (UI filter registers are in Bank 0)
    rc |= inv_icm426xx_set_reg_bank(s, 0);

    // GYRO_CONFIG1 (0x51): Set UI_FILT_ORD[3:2]
    rc |= inv_icm426xx_read_reg(s, MPUREG_GYRO_CONFIG1, 1, &val);
    val &= ~0x0C;  // Clear bits [3:2]
    val |= gyro_ord_bits;
    rc |= inv_icm426xx_write_reg(s, MPUREG_GYRO_CONFIG1, 1, &val);

    // ACCEL_CONFIG1 (0x53): Set UI_FILT_ORD[4:3]
    rc |= inv_icm426xx_read_reg(s, MPUREG_ACCEL_CONFIG1, 1, &val);
    val &= ~0x18;  // Clear bits [4:3]
    val |= accel_ord_bits;
    rc |= inv_icm426xx_write_reg(s, MPUREG_ACCEL_CONFIG1, 1, &val);

    // ACCEL_GYRO_CONFIG0 (0x52): Set bandwidth codes
    // GYRO_UI_FILT_BW[3:0] = ui_bw_code, ACCEL_UI_FILT_BW[7:4] = ui_bw_code
    val = (ui_bw_code << 4) | ui_bw_code;
    rc |= inv_icm426xx_write_reg(s, MPUREG_ACCEL_GYRO_CONFIG0, 1, &val);

    return rc;
}

/**
 * @brief Set UI filters to Betaflight defaults (code 15, 2nd-order both)
 *
 * Convenience wrapper that configures UI filters to match Betaflight driver defaults:
 * - BW Code 15: Low-latency path (trivial decimation, minimal delay)
 * - 2nd order: Balance between noise rejection and phase lag
 *
 * Betaflight relies on software filters (gyro LPF, D-term, dynamic notch) for fine
 * noise control, so hardware UI filters are kept minimal to reduce delay.
 *
 * @param s Pointer to device structure
 * @return 0 on success, negative on error
 *
 * Reference: Betaflight_Filtering.md Section 2, 5
 */
static inline int icm42688p_set_ui_filters_betaflight(struct inv_icm426xx *s) {
    return icm42688p_set_ui_filters(s, 15, 2, 2);
}

/**
 * @brief Verify UI filter configuration by reading back registers
 *
 * Reads GYRO_CONFIG1, ACCEL_CONFIG1, and ACCEL_GYRO_CONFIG0 to verify
 * the UI filter settings match expected values.
 *
 * @param s Pointer to device structure
 * @param expected_bw_code Expected bandwidth code (0-15)
 * @param expected_gyro_order Expected gyro filter order (1-3)
 * @param expected_accel_order Expected accel filter order (1-3)
 * @return 0 if verified, -1 on mismatch or error
 *
 * Reference: ICM-42688-P Datasheet Rev 1.8, Section 5.3
 */
static inline int icm42688p_verify_ui_filters(struct inv_icm426xx *s, uint8_t expected_bw_code, uint8_t expected_gyro_order, uint8_t expected_accel_order) {
    int rc = 0;
    uint8_t gyro_cfg1, accel_cfg1, bw_cfg;

    // Switch to Bank 0
    rc |= inv_icm426xx_set_reg_bank(s, 0);

    // Read registers
    rc |= inv_icm426xx_read_reg(s, MPUREG_GYRO_CONFIG1, 1, &gyro_cfg1);
    rc |= inv_icm426xx_read_reg(s, MPUREG_ACCEL_CONFIG1, 1, &accel_cfg1);
    rc |= inv_icm426xx_read_reg(s, MPUREG_ACCEL_GYRO_CONFIG0, 1, &bw_cfg);

    if (rc != 0) {
        return -1; // Read error
    }

    // Extract and verify gyro order (bits [3:2])
    uint8_t gyro_order = ((gyro_cfg1 & 0x0C) >> 2) + 1; // Convert 0/1/2 to 1/2/3
    if (gyro_order != expected_gyro_order) {
        return -1;
    }

    // Extract and verify accel order (bits [4:3])
    uint8_t accel_order = ((accel_cfg1 & 0x18) >> 3) + 1; // Convert 0/1/2 to 1/2/3
    if (accel_order != expected_accel_order) {
        return -1;
    }

    // Extract and verify bandwidth codes
    uint8_t gyro_bw = bw_cfg & 0x0F;         // bits [3:0]
    uint8_t accel_bw = (bw_cfg >> 4) & 0x0F; // bits [7:4]
    if (gyro_bw != expected_bw_code || accel_bw != expected_bw_code) {
        return -1;
    }

    return 0; // All verified
}

#endif /* __ICM_42688_P_H__ */