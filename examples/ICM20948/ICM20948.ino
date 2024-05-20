#include "daisy_seed.h"
#include "per/icm20948.h"

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;

// Declare a DaisySeed object called hardware
DaisySeed hardware;

// Icm20948Spi imu;
Icm20948<Icm20948SpiTransport> imu;
Icm20948<Icm20948SpiTransport>::Config imu_cfg;

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

    // Configure the ICM-20948 IMU SPI interface
    imu_cfg.transport_config.periph = SpiHandle::Config::Peripheral::SPI_1;
    imu_cfg.transport_config.sclk   = Pin(PORTG, 11);
    imu_cfg.transport_config.miso   = Pin(PORTB, 4);
    imu_cfg.transport_config.mosi   = Pin(PORTB, 5);
    imu_cfg.transport_config.nss    = Pin(PORTG, 10);

    // Initialize the ICM-20948 IMU
    imu.Init(imu_cfg);

    // imu.Init(hardware.seed.spi4);

    // Loop forever
    for(;;)
    {
        // Set the onboard LED
        hardware.SetLed(led_state);

        // Toggle the LED state for the next time around.
        led_state = !led_state;

        // Wait 500ms
        System::Delay(500);
    }
}
