/*
 * ICM42688P Interrupt-Driven Raw Data Registers Example
 *
 * This example demonstrates interrupt-driven raw sensor data acquisition from
 * the ICM42688P using the InvenSense reference implementation with STM32 Arduino integration.
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection (NUCLEO_F411RE / BLACKPILL_F411CE)
 * - Pin assignments and SPI frequency from board configuration
 * - Interrupt pin configured for data-ready signaling (PC4/EXTI4)
 * - Supports multiple board targets with single codebase
 *
 * CI/HIL INTEGRATION:
 * - RTT output for automated testing
 * - Serial output for Arduino IDE
 * - Build traceability with git SHA and timestamp
 */

#include "icm42688p.h"
#include <ci_log.h>
#include <SPI.h>
#include <cstring>

// Board configuration
#if defined(ARDUINO_BLACKPILL_F411CE)
#include "../../../../targets/BLACKPILL_F411CE.h"
#else
#include "../../../../targets/NUCLEO_F411RE_JHEF411.h"
#endif

#include "inv_main.h"
#include "inv_gpio.h"

// libPrintf integration - automatic aliasing enabled
#include <libPrintf.h>

// BoardConfig integration for dynamic pin and frequency configuration
#define IMU_CS_PIN        BoardConfig::imu.spi.cs_pin
#define IMU_MOSI_PIN      BoardConfig::imu.spi.mosi_pin
#define IMU_MISO_PIN      BoardConfig::imu.spi.miso_pin
#define IMU_SCLK_PIN      BoardConfig::imu.spi.sclk_pin
#define IMU_SPI_SPEED     BoardConfig::imu.spi.freq_hz
#define IMU_INT_PIN       BoardConfig::imu.int_pin
#define INT1_PIN          IMU_INT_PIN

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin,
                 BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin,
                 BoardConfig::imu.spi.get_ssel_pin());

// UART types for InvenSense compatibility
typedef enum {
  INV_UART_0 = 0,
  INV_UART_1,
  INV_UART_MAX
} inv_uart_num_t;

// Function declarations
extern "C" {
  int inv_uart_mngr_puts(inv_uart_num_t uart_num, const char* s, unsigned short l);
}

// Global print buffer
char buf[128];

void setup() {
    // Initialize the Arduino hardware
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial) delay(10);
#endif

    CI_LOG("=== ICM42688P Raw Data Registers ===\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();

    // Display pin configuration from BoardConfig
    CI_LOG("Pin Configuration (BoardConfig):\n");
    printf("  CS: %d, MOSI: %d, MISO: %d, SCLK: %d\n",
           (int)IMU_CS_PIN, (int)IMU_MOSI_PIN,
           (int)IMU_MISO_PIN, (int)IMU_SCLK_PIN);
    printf("  SPI Speed: %lu Hz\n", (unsigned long)IMU_SPI_SPEED);
    if (IMU_INT_PIN != 0) {
        printf("  Interrupt Pin: %d\n", (int)IMU_INT_PIN);
    } else {
        CI_LOG("  Interrupt Pin: None (polling mode)\n");
    }

    // Setup interrupt if pin is configured
    if (IMU_INT_PIN != 0) {
        pinMode(IMU_INT_PIN, INPUT);
        // Note: attachInterrupt will be called by inv_gpio_sensor_irq_init() from within inv_main()
        CI_LOG("✓ Interrupt pin configured\n");
    }

    // Give ICM-42688P some time to stabilize
    delay(5);

    // Call the main function of the Invensense example (PRESERVE THIS!)
    inv_main();
}

void loop() {
    // Do nothing - all work done in inv_main()
}

/* --------------------------------------------------------------------------------------
 *  Extern functions definition - Invensense to UVOS adapters
 * -------------------------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

// libPrintf putchar_ implementation for RTT
void putchar_(char c) {
    char buf[2] = {c, '\0'};
    CI_LOG(buf);
}

/* This variable contains the number of nested calls to disable_irq */
static uint32_t sDisableIntCount = 0;

void inv_disable_irq(void)
{
    if(sDisableIntCount == 0) {
        __disable_irq();
    }
    sDisableIntCount ++;
}

void inv_enable_irq(void)
{
    sDisableIntCount --;
    if(sDisableIntCount == 0) {
        __enable_irq();
    }
}

int inv_uart_mngr_puts(inv_uart_num_t uart_num, const char* s, unsigned short l)
{
    CI_LOG(s);
    return 1;
}

uint64_t inv_timer_get_counter(unsigned timer_num)
{
    return micros();
}

void inv_delay_us(uint32_t us)
{
    delayMicroseconds(us);
}

/* Inv gpio functions - Arduino simplified version */

static void (*gpio_callback)(void *context, unsigned pin_num) = nullptr;
static void* gpio_context = nullptr;

// Arduino interrupt callback
void gpio_common_callback(void)
{
    if (gpio_callback) {
        gpio_callback(gpio_context, INV_GPIO_INT1);
    }
}

// Arduino version - simplified interrupt setup
void inv_gpio_sensor_irq_init(unsigned pin_num,
        void (*interrupt_cb)(void * context, unsigned int_num), void * context)
{
    if(pin_num >= INV_GPIO_MAX)
        return;

    gpio_callback = interrupt_cb;
    gpio_context = context;

    pinMode(INT1_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(INT1_PIN), gpio_common_callback, RISING);
}

#ifdef __cplusplus
}
#endif