#include "daisy_seed.h"
#include "dev/icm42688p.h"
#include "stm32h7xx_hal.h"

static void Error_Handler()
{
    asm("bkpt 255");
    while (1)
    {
    }
}

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;

// Uncomment to use software driven NSS 
// #define USE_SOFT_NSS
#define DESIRED_SPI_FREQ 1000000

// Declare a DaisySeed object called hardware
DaisySeed hardware;

// Handle we'll use to interact with IMU SPI
SpiHandle spi_handle;

// Structure to configure the IMU SPI instance
SpiHandle::Config spi_conf;

// ICM42688P imu
ICM42688 imu;

int main(void)
{
    // Declare a variable to store the state we want to set for the LED.
    bool led_state;
    led_state = true;

    // Configure and Initialize the Daisy Seed
    // These are separate to allow reconfiguration of any of the internal
    // components before initialization.
    hardware.Configure();
    hardware.Init();

    // Configure the ICM-42688P IMU SPI interface
    spi_conf.periph = SpiHandle::Config::Peripheral::SPI_1;
    spi_conf.mode = SpiHandle::Config::Mode::MASTER;
    spi_conf.direction = SpiHandle::Config::Direction::TWO_LINES;
    spi_conf.clock_polarity = SpiHandle::Config::ClockPolarity::HIGH;
    spi_conf.clock_phase = SpiHandle::Config::ClockPhase::TWO_EDGE;
#ifdef USE_SOFT_NSS
    spi_conf.nss = SpiHandle::Config::NSS::SOFT;
#else
    spi_conf.nss = SpiHandle::Config::NSS::HARD_OUTPUT;
#endif // USE_SOFT_NSS
    spi_conf.pin_config.nss = Pin(PORTA, 4);
    spi_conf.pin_config.sclk = Pin(PORTA, 5);
    spi_conf.pin_config.miso = Pin(PORTA, 6);
    spi_conf.pin_config.mosi = Pin(PORTA, 7);

    // spi_conf.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_32;
    spi_handle.GetBaudHz(spi_conf.periph, DESIRED_SPI_FREQ, spi_conf.baud_prescaler);

    // Initialize the IMU SPI instance
    spi_handle.Init(spi_conf);

    // Initialize the ICM-42688P IMU
    if (imu.Init(spi_handle) != ICM42688::Result::OK)
    {
        Error_Handler();
    }

    // Loop forever
    for (;;)
    {
        // Set the onboard LED
        hardware.SetLed(led_state);

        // Toggle the LED state for the next time around.
        led_state = !led_state;

        // Wait 500ms
        System::Delay(500);
    }
}
