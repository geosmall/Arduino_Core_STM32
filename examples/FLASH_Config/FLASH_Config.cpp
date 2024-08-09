#include "daisy_seed.h"
#include <string.h>
#include "FlashConfig.h"

#define NUM_TEST_DATA_BYTES 36

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;

// Declare a DaisySeed object called hw
DaisySeed hw;

int main(void)
{
    // Declare a variable to store the state we want to set for the LED.
    bool led_state;
    led_state = true;

    // Configure and Initialize the Daisy Seed
    // These are separate to allow reconfiguration of any of the internal
    // components before initialization.
    hw.Configure();
    hw.Init();

    // hw.StartLog(
    //     true); /* true == wait for PC: will block until a terminal is connected */

    FlashConfig flashConfig;

    static_assert((NUM_TEST_DATA_BYTES <= daisy::FlashBlockDataSize),
                  "Data to save is too large");

    uint8_t dataToSave[NUM_TEST_DATA_BYTES] = {0};
    uint8_t dataRead[NUM_TEST_DATA_BYTES];

    // fill date to write with 8 A5's
    memset(dataToSave, 0x5A, 8);

    // Save the configuration data
    FlashConfig::Result result = flashConfig.SaveConfigData(dataToSave, sizeof(dataToSave));
    if (result == FlashConfig::Result::OK)
    {
        hw.PrintLine("Data saved successfully.");
    }
    else
    {
        // hw.PrintLine("Failed to save data.");
        // Print the error message and the error code
        hw.PrintLine("Failed to save data. Error code: %d", result);
    }

    // Get the size of the current configuration data
    uint32_t dataSize;
    result = flashConfig.GetCurrentConfigDataSize(&dataSize);
    if (result == FlashConfig::Result::OK)
    {
        hw.PrintLine("Current config data size: %d bytes.\n", dataSize);  
    }
    else
    {
        hw.PrintLine("Failed to get current config data size. Error code: %d", result);
    }

    // Read the current configuration data
    result = flashConfig.ReadCurrentConfigData(dataRead, sizeof(dataRead));
    if (result == FlashConfig::Result::OK)
    {
        hw.PrintLine("Data read successfully: ");
        for (uint32_t i = 0; i < dataSize; ++i)
        {
            // %02X prints data in 2 char hex, leading zeros if needed
            hw.PrintLine("%02X  ", dataRead[i]);
        }
        hw.PrintLine("");
    }
    else
    {
        hw.PrintLine("Failed to read data. Error code: %d", result);
    }

    // Verify that the data header CRC of the current config matches the written data
    // using FlashConfig method VerifyCurrentConfigCRC()
    result = flashConfig.VerifyCurrentConfigCRC();
    if (result == FlashConfig::Result::OK)
    {
        hw.PrintLine("Data CRC verified successfully.");
    }
    else
    {
        hw.PrintLine("Failed to verify data CRC. Error code: %d", result);
    }

    // Loop forever
    for(;;)
    {
        // Set the onboard LED
        hw.SetLed(led_state);

        // Toggle the LED state for the next time around.
        led_state = !led_state;

        // Wait 500ms
        System::Delay(500);
    }
}