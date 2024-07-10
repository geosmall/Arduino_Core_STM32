#ifndef ICM42688_REGISTERS_H_
#define ICM42688_REGISTERS_H_

#include <cstdint>

namespace ICM42688reg {

  // Accesible from all user banks
  static constexpr uint8_t REG_BANK_SEL = 0x76;


  // User Bank 0
  static constexpr uint8_t UB0_REG_DEVICE_CONFIG = 0x11;
  // break
  static constexpr uint8_t UB0_REG_DRIVE_CONFIG = 0x13;
  static constexpr uint8_t UB0_REG_INT_CONFIG = 0x14;
    #define INT1_MODE_PULSED (0 << 2)
    #define INT1_MODE_LATCHED (1 << 2)
    #define INT1_DRIVE_CIRCUIT_OD (0 << 1)
    #define INT1_DRIVE_CIRCUIT_PP (1 << 1)
    #define INT1_POLARITY_ACTIVE_LOW (0 << 0)
    #define INT1_POLARITY_ACTIVE_HIGH (1 << 0)
  // break
  static constexpr uint8_t UB0_REG_FIFO_CONFIG = 0x16;
  // break
  static constexpr uint8_t UB0_REG_TEMP_DATA1 = 0x1D;
  static constexpr uint8_t UB0_REG_TEMP_DATA0 = 0x1E;
  static constexpr uint8_t UB0_REG_ACCEL_DATA_X1 = 0x1F;
  static constexpr uint8_t UB0_REG_ACCEL_DATA_X0 = 0x20;
  static constexpr uint8_t UB0_REG_ACCEL_DATA_Y1 = 0x21;
  static constexpr uint8_t UB0_REG_ACCEL_DATA_Y0 = 0x22;
  static constexpr uint8_t UB0_REG_ACCEL_DATA_Z1 = 0x23;
  static constexpr uint8_t UB0_REG_ACCEL_DATA_Z0 = 0x24;
  static constexpr uint8_t UB0_REG_GYRO_DATA_X1 = 0x25;
  static constexpr uint8_t UB0_REG_GYRO_DATA_X0 = 0x26;
  static constexpr uint8_t UB0_REG_GYRO_DATA_Y1 = 0x27;
  static constexpr uint8_t UB0_REG_GYRO_DATA_Y0 = 0x28;
  static constexpr uint8_t UB0_REG_GYRO_DATA_Z1 = 0x29;
  static constexpr uint8_t UB0_REG_GYRO_DATA_Z0 = 0x2A;
  static constexpr uint8_t UB0_REG_TMST_FSYNCH = 0x2B;
  static constexpr uint8_t UB0_REG_TMST_FSYNCL = 0x2C;
  static constexpr uint8_t UB0_REG_INT_STATUS = 0x2D;
  static constexpr uint8_t UB0_REG_FIFO_COUNTH = 0x2E;
  static constexpr uint8_t UB0_REG_FIFO_COUNTL = 0x2F;
  static constexpr uint8_t UB0_REG_FIFO_DATA = 0x30;
  static constexpr uint8_t UB0_REG_APEX_DATA0 = 0x31;
  static constexpr uint8_t UB0_REG_APEX_DATA1 = 0x32;
  static constexpr uint8_t UB0_REG_APEX_DATA2 = 0x33;
  static constexpr uint8_t UB0_REG_APEX_DATA3 = 0x34;
  static constexpr uint8_t UB0_REG_APEX_DATA4 = 0x35;
  static constexpr uint8_t UB0_REG_APEX_DATA5 = 0x36;
  static constexpr uint8_t UB0_REG_INT_STATUS2 = 0x37;
  static constexpr uint8_t UB0_REG_INT_STATUS3 = 0x38;
  // break
  static constexpr uint8_t UB0_REG_SIGNAL_PATH_RESET = 0x4B;
  static constexpr uint8_t UB0_REG_INTF_CONFIG0 = 0x4C;
  static constexpr uint8_t UB0_REG_INTF_CONFIG1 = 0x4D;
    #define INTF_CONFIG1_AFSR_MASK 0xC0
    #define INTF_CONFIG1_AFSR_DISABLE 0x40
  static constexpr uint8_t UB0_REG_PWR_MGMT0 = 0x4E;
    #define PWR_MGMT0_ACCEL_MODE_LN (3 << 0)
    #define PWR_MGMT0_GYRO_MODE_LN (3 << 2)
    #define PWR_MGMT0_GYRO_ACCEL_MODE_OFF ((0 << 0) | (0 << 2))
    #define PWR_MGMT0_TEMP_DISABLE_OFF (0 << 5)
    #define PWR_MGMT0_TEMP_DISABLE_ON (1 << 5)
  static constexpr uint8_t UB0_REG_GYRO_CONFIG0 = 0x4F;
  static constexpr uint8_t UB0_REG_ACCEL_CONFIG0 = 0x50;
  static constexpr uint8_t UB0_REG_GYRO_CONFIG1 = 0x51;
  static constexpr uint8_t UB0_REG_GYRO_ACCEL_CONFIG0 = 0x52;
    #define ACCEL_UI_FILT_BW_LOW_LATENCY (15 << 4) 
    #define GYRO_UI_FILT_BW_LOW_LATENCY (15 << 0)
  static constexpr uint8_t UB0_REG_ACCEFL_CONFIG1 = 0x53;
  static constexpr uint8_t UB0_REG_TMST_CONFIG = 0x54;
  // break
  static constexpr uint8_t UB0_REG_APEX_CONFIG0 = 0x56;
  static constexpr uint8_t UB0_REG_SMD_CONFIG = 0x57;
  // break
  static constexpr uint8_t UB0_REG_FIFO_CONFIG1 = 0x5F;
  static constexpr uint8_t UB0_REG_FIFO_CONFIG2 = 0x60;
  static constexpr uint8_t UB0_REG_FIFO_CONFIG3 = 0x61;
  static constexpr uint8_t UB0_REG_FSYNC_CONFIG = 0x62;
  static constexpr uint8_t UB0_REG_INT_CONFIG0 = 0x63;
    #define UI_DRDY_INT_CLEAR_MASK             ((1 << 5) | (1 << 4))
    #define UI_DRDY_INT_CLEAR_ON_SBR           ((0 << 5) | (0 << 4))
    #define UI_DRDY_INT_CLEAR_ON_SBR_DUPLICATE ((0 << 5) | (1 << 4)) // duplicate settings in datasheet, Rev 1.8.
    #define UI_DRDY_INT_CLEAR_ON_F1BR          ((1 << 5) | (0 << 4))
    #define UI_DRDY_INT_CLEAR_ON_SBR_AND_F1BR  ((1 << 5) | (1 << 4))
  static constexpr uint8_t UB0_REG_INT_CONFIG1 = 0x64;
    #define INT_CONFIG1_MASK          ((1 << 6) | (1 << 5) | (1 << 4))
    #define INT_ASYNC_RESET_BIT       4
    #define INT_TDEASSERT_DISABLE_BIT 5
    #define INT_TDEASSERT_ENABLED     (0 << INT_TDEASSERT_DISABLE_BIT)
    #define INT_TDEASSERT_DISABLED    (1 << INT_TDEASSERT_DISABLE_BIT)
    #define INT_TPULSE_DURATION_BIT   6
    #define INT_TPULSE_DURATION_100   (0 << INT_TPULSE_DURATION_BIT)
    #define INT_TPULSE_DURATION_8     (1 << INT_TPULSE_DURATION_BIT)
  static constexpr uint8_t UB0_REG_INT_SOURCE0 = 0x65;
    #define UI_DRDY_INT1_MASK         (1 << 3)
    #define UI_DRDY_INT1_EN_DISABLED  (0 << 3)
    #define UI_DRDY_INT1_EN_ENABLED   (1 << 3)
  static constexpr uint8_t UB0_REG_INT_SOURCE1 = 0x66;
  // break
  static constexpr uint8_t UB0_REG_INT_SOURCE3 = 0x68;
  static constexpr uint8_t UB0_REG_INT_SOURCE4 = 0x69;
  // break
  static constexpr uint8_t UB0_REG_FIFO_LOST_PKT0 = 0x6C;
  static constexpr uint8_t UB0_REG_FIFO_LOST_PKT1 = 0x6D;
  // break
  static constexpr uint8_t UB0_REG_SELF_TEST_CONFIG = 0x70;
  // break
  static constexpr uint8_t UB0_REG_WHO_AM_I = 0x75;


  // User Bank 1
  static constexpr uint8_t UB1_REG_SENSOR_CONFIG0 = 0x03;
  // break
  static constexpr uint8_t UB1_REG_GYRO_CONFIG_STATIC2 = 0x0B;
  static constexpr uint8_t UB1_REG_GYRO_CONFIG_STATIC3 = 0x0C;
  static constexpr uint8_t UB1_REG_GYRO_CONFIG_STATIC4 = 0x0D;
  static constexpr uint8_t UB1_REG_GYRO_CONFIG_STATIC5 = 0x0E;
  static constexpr uint8_t UB1_REG_GYRO_CONFIG_STATIC6 = 0x0F;
  static constexpr uint8_t UB1_REG_GYRO_CONFIG_STATIC7 = 0x10;
  static constexpr uint8_t UB1_REG_GYRO_CONFIG_STATIC8 = 0x11;
  static constexpr uint8_t UB1_REG_GYRO_CONFIG_STATIC9 = 0x12;
  static constexpr uint8_t UB1_REG_GYRO_CONFIG_STATIC10 = 0x13;
  // break
  static constexpr uint8_t UB1_REG_XG_ST_DATA = 0x5F;
  static constexpr uint8_t UB1_REG_YG_ST_DATA = 0x60;
  static constexpr uint8_t UB1_REG_ZG_ST_DATA = 0x61;
  static constexpr uint8_t UB1_REG_TMSTVAL0 = 0x62;
  static constexpr uint8_t UB1_REG_TMSTVAL1 = 0x63;
  static constexpr uint8_t UB1_REG_TMSTVAL2 = 0x64;
  // break
  static constexpr uint8_t UB1_REG_INTF_CONFIG4 = 0x7A;
  static constexpr uint8_t UB1_REG_INTF_CONFIG5 = 0x7B;
  static constexpr uint8_t UB1_REG_INTF_CONFIG6 = 0x7C;


  // User Bank 2
  static constexpr uint8_t UB2_REG_ACCEL_CONFIG_STATIC2 = 0x03;
  static constexpr uint8_t UB2_REG_ACCEL_CONFIG_STATIC3 = 0x04;
  static constexpr uint8_t UB2_REG_ACCEL_CONFIG_STATIC4 = 0x05;
  // break
  static constexpr uint8_t UB2_REG_XA_ST_DATA = 0x3B;
  static constexpr uint8_t UB2_REG_YA_ST_DATA = 0x3C;
  static constexpr uint8_t UB2_REG_ZA_ST_DATA = 0x3D;


  // User Bank 4
  static constexpr uint8_t UB4_REG_APEX_CONFIG1 = 0x40;
  static constexpr uint8_t UB4_REG_APEX_CONFIG2 = 0x41;
  static constexpr uint8_t UB4_REG_APEX_CONFIG3 = 0x42;
  static constexpr uint8_t UB4_REG_APEX_CONFIG4 = 0x43;
  static constexpr uint8_t UB4_REG_APEX_CONFIG5 = 0x44;
  static constexpr uint8_t UB4_REG_APEX_CONFIG6 = 0x45;
  static constexpr uint8_t UB4_REG_APEX_CONFIG7 = 0x46;
  static constexpr uint8_t UB4_REG_APEX_CONFIG8 = 0x47;
  static constexpr uint8_t UB4_REG_APEX_CONFIG9 = 0x48;
  // break
  static constexpr uint8_t UB4_REG_ACCEL_WOM_X_THR = 0x4A;
  static constexpr uint8_t UB4_REG_ACCEL_WOM_Y_THR = 0x4B;
  static constexpr uint8_t UB4_REG_ACCEL_WOM_Z_THR = 0x4C;
  static constexpr uint8_t UB4_REG_INT_SOURCE6 = 0x4D;
  static constexpr uint8_t UB4_REG_INT_SOURCE7 = 0x4E;
  static constexpr uint8_t UB4_REG_INT_SOURCE8 = 0x4F;
  static constexpr uint8_t UB4_REG_INT_SOURCE9 = 0x50;
  static constexpr uint8_t UB4_REG_INT_SOURCE10 = 0x51;
  // break
  static constexpr uint8_t UB4_REG_OFFSET_USER0 = 0x77;
  static constexpr uint8_t UB4_REG_OFFSET_USER1 = 0x78;
  static constexpr uint8_t UB4_REG_OFFSET_USER2 = 0x79;
  static constexpr uint8_t UB4_REG_OFFSET_USER3 = 0x7A;
  static constexpr uint8_t UB4_REG_OFFSET_USER4 = 0x7B;
  static constexpr uint8_t UB4_REG_OFFSET_USER5 = 0x7C;
  static constexpr uint8_t UB4_REG_OFFSET_USER6 = 0x7D;
  static constexpr uint8_t UB4_REG_OFFSET_USER7 = 0x7E;
  static constexpr uint8_t UB4_REG_OFFSET_USER8 = 0x7F;

} // ns ICM42688reg

#endif // ICM42688_REGISTERS_H_
