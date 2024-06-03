#include "daisy_seed.h"
#include "dev/icm42688p.h"

/*
resource SPI_SCK 1 A05
resource SPI_MISO 1 A06
resource SPI_MOSI 1 D07
resource GYRO_CS 1 C15
*/

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;

// Declare a DaisySeed object called hardware
DaisySeed hardware;

Icm42688p<Icm42688pSpiTransport> imu;
Icm42688p<Icm42688pSpiTransport>::Config imu_cfg;

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
    imu_cfg.transport_config.periph = SpiHandle::Config::Peripheral::SPI_1;
    imu_cfg.transport_config.nss    = Pin(PORTA, 4);
    imu_cfg.transport_config.sclk   = Pin(PORTA, 5);
    imu_cfg.transport_config.miso   = Pin(PORTA, 6);
    imu_cfg.transport_config.mosi   = Pin(PORTA, 7);

    // Initialize the ICM-42688P IMU
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
