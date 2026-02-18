/*
 * BlackPill F411CE custom target (reverse-engineered from madflight config)
 *
 * This is NOT an official Betaflight target. It defines a generic BlackPill
 * F411CE pin mapping for use with breadboard-attached peripherals:
 *   - MPU-9250 IMU on SPI2 (PB12-PB15)
 *   - W25Q64 SPI flash on SPI1 (PA4-PA7)
 *   - BMP280 barometer on I2C1 (PB8/PB9)
 */

#define FC_TARGET_MCU  STM32F411

#define BOARD_NAME      BLACKPILL_F411CE
#define MANUFACTURER_ID WACT

#define USE_ACC
#define USE_ACC_SPI_MPU9250
#define USE_GYRO
#define USE_GYRO_SPI_MPU9250
#define USE_MAG
#define USE_MAG_AK8963
#define USE_BARO
#define USE_BARO_BMP280
#define USE_FLASH
#define USE_FLASH_W25Q64

// SPI1 - Flash storage
#define SPI1_SCK_PIN   PA5
#define SPI1_SDI_PIN   PA6
#define SPI1_SDO_PIN   PA7

// SPI2 - IMU (MPU-9250)
#define SPI2_SCK_PIN   PB13
#define SPI2_SDI_PIN   PB14
#define SPI2_SDO_PIN   PB15

// I2C1 - Baro/Mag
#define I2C1_SCL_PIN   PB8
#define I2C1_SDA_PIN   PB9

// UART1
#define UART1_TX_PIN   PA9
#define UART1_RX_PIN   PA10

// UART2
#define UART2_TX_PIN   PA2
#define UART2_RX_PIN   PA3

// Motors
#define MOTOR1_PIN     PB4
#define MOTOR2_PIN     PB5
#define MOTOR3_PIN     PB0
#define MOTOR4_PIN     PB1
#define MOTOR5_PIN     PB6
#define MOTOR6_PIN     PB7

// Servos
#define SERVO1_PIN     PA2
#define SERVO2_PIN     PA15
#define SERVO3_PIN     PB3

// ADC
#define ADC_VBAT_PIN   PA1
#define ADC_CURR_PIN   PA0

// LED
#define LED0_PIN       PC13

// Beeper
#define BEEPER_PIN     PC14

// LED Strip
#define LED_STRIP_PIN  PA8

// Gyro
#define GYRO_1_CS_PIN    PB12
#define GYRO_1_EXTI_PIN  PB2
#define GYRO_1_SPI_INSTANCE SPI2
#define GYRO_1_ALIGN     CW0_DEG

// Flash
#define FLASH_CS_PIN     PA4
#define FLASH_SPI_INSTANCE SPI1

// Settings
#define DEFAULT_CURRENT_METER_SCALE 170
#define DEFAULT_BLACKBOX_DEVICE     BLACKBOX_DEVICE_FLASH

// Timer pin map (occurrence-based, matching PeripheralPins.c order)
// Pin   Occ  Timer result (from F411CE PeripheralPins.c)
// PA2    1   TIM2_CH3  (AF1)
// PA15   1   TIM2_CH1  (AF1)
// PB3    1   TIM2_CH2  (AF1)
// PB4    1   TIM3_CH1  (AF2)
// PB5    1   TIM3_CH2  (AF2)
// PB0    2   TIM3_CH3  (AF2)
// PB1    2   TIM3_CH4  (AF2)
// PB6    1   TIM4_CH1  (AF2)
// PB7    1   TIM4_CH2  (AF2)
// PA8    1   TIM1_CH1  (AF1)
#define TIMER_PIN_MAPPING \
    TIMER_PIN_MAP( 0, PA2 , 1, -1) \
    TIMER_PIN_MAP( 1, PA15, 1, -1) \
    TIMER_PIN_MAP( 2, PB3 , 1, -1) \
    TIMER_PIN_MAP( 3, PB4 , 1,  0) \
    TIMER_PIN_MAP( 4, PB5 , 1,  0) \
    TIMER_PIN_MAP( 5, PB0 , 2,  0) \
    TIMER_PIN_MAP( 6, PB1 , 2,  0) \
    TIMER_PIN_MAP( 7, PB6 , 1,  0) \
    TIMER_PIN_MAP( 8, PB7 , 1,  0) \
    TIMER_PIN_MAP( 9, PA8 , 1,  1)
