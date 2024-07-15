#include <stdio.h>
#include <string.h>
#include "daisy_seed.h"
#include "example-selftest.h"

/* --------------------------------------------------------------------------------------
 *  Example configuration
 * -------------------------------------------------------------------------------------- */

/*
 * Select UART port on which INV_MSG() will be printed.
 */
#define LOG_UART_ID INV_UART_SENSOR_CTRL

/* 
 * Timer used throughout standalone applications 
 */
#define DELAY_TIMER    INV_TIMER3
#define TIMEBASE_TIMER INV_TIMER2

/* 
 * Select communication between SmartMotion and ICM426xx by setting correct SERIF_TYPE 
 */
#define SERIF_TYPE ICM426XX_UI_SPI4
//#define SERIF_TYPE ICM426XX_UI_I2C

/* 
 * Define msg level 
 */
#define MSG_LEVEL INV_MSG_LEVEL_DEBUG

/* --------------------------------------------------------------------------------------
 *  Forward declaration
 * -------------------------------------------------------------------------------------- */

static void SetupMCUHardware(struct inv_icm426xx_serif *icm_serif);
static void check_rc(int rc, const char *msg_context);
void        msg_printer(int level, const char *str, va_list ap);

using namespace daisy;

static DaisySeed hw;

uint8_t sumbuff[1024];

void UsbCallback(uint8_t* buf, uint32_t* len)
{
    for(size_t i = 0; i < *len; i++)
    {
        sumbuff[i] = buf[i];
    }
}


int main(void)
{
    int                       rc = 0;
    struct inv_icm426xx_serif icm426xx_serif;

    /* Initialize MCU hardware */
    SetupMCUHardware(&icm426xx_serif);

    hw.Configure();
    hw.Init();
    hw.usb_handle.Init(UsbHandle::FS_INTERNAL);
    int  tick_cnt = 0;
    bool ledstate = false;
    char buff[512];
    sprintf(buff, "Tick:\t%d\r\n", tick_cnt);
    hw.usb_handle.TransmitInternal((uint8_t*)buff, strlen(buff));
    System::Delay(500);
    hw.usb_handle.SetReceiveCallback(UsbCallback, UsbHandle::FS_INTERNAL);
    while(1)
    {
        System::Delay(500);
        sprintf(buff, "Tick:\t%d\r\n", tick_cnt);
        hw.usb_handle.TransmitInternal((uint8_t*)buff, strlen(buff));
        tick_cnt = (tick_cnt + 1) % 100;
        hw.SetLed(ledstate);
        ledstate = !ledstate;
    }
}

int inv_io_hal_read_reg(struct inv_icm426xx_serif *serif, uint8_t reg, uint8_t *rbuffer,
                        uint32_t rlen)
{
    return 0;
}

int inv_io_hal_write_reg(struct inv_icm426xx_serif *serif, uint8_t reg, const uint8_t *wbuffer,
                         uint32_t wlen)
{
    return 0;
}

/*
 * This function initializes MCU on which this software is running.
 * It configures:
 *   - a UART link used to print some messages
 *   - interrupt priority group
 *   - a microsecond timer requested by Icm426xx driver to compute some delay
 *   - a serial link to communicate from MCU to Icm426xx
 */
static void SetupMCUHardware(struct inv_icm426xx_serif *icm_serif)
{
    INV_MSG(INV_MSG_LEVEL_INFO, "#########################");
    INV_MSG(INV_MSG_LEVEL_INFO, "#   Example Self-Test   #");
    INV_MSG(INV_MSG_LEVEL_INFO, "#########################");

    /* Initialize serial inteface between MCU and Icm426xx */
    icm_serif->context    = 0; /* no need */
    icm_serif->read_reg   = inv_io_hal_read_reg;
    icm_serif->write_reg  = inv_io_hal_write_reg;
    icm_serif->max_read   = 1024 * 32; /* maximum number of bytes allowed per serial read */
    icm_serif->max_write  = 1024 * 32; /* maximum number of bytes allowed per serial write */
    icm_serif->serif_type = SERIF_TYPE;
}

extern "C" void System_delay_us(uint32_t us)
{
    System::DelayUs(us);
}

extern "C" uint64_t System_get_us(void)
{
 return System::GetUs();
}