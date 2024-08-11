#include "daisy_seed.h"
#include <string.h>

#define NUM_TEST_DATA_BYTES 36

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;

// Declare a DaisySeed object called hw
DaisySeed hw;

/*
    enum class Result
    {
        OK, // No error
        ERR_NO_BLOCK_FOUND,     // No block found
        ERR_INVALID_BLOCK_INFO, // Invalid block
        ERR_READ_FAILED,        // Read failed
        ERR_WRITE_FAILED,       // Write failed    
        ERR_ERASE_FAILED,       // Erase failed
        ERR_DATA_TOO_LARGE,     // Data too large to fit in block
        ERR_DATA_CRC_MISMATCH,  // Data CRC does not match header
        ERR_DATA_INVALID_INPUT, // Invalid data input provided
        ERR_DATA_ALREADY_SAVED, // Data already saved to flash
        ERR                     // General error occurred
    };
Function to convert enum class Result to string
*/
static const char *ResultToString(InternalFlash::Result result)
{
    switch (result)
    {
    case InternalFlash::Result::OK:
        return "OK";
    case InternalFlash::Result::ERR_NO_BLOCK_FOUND:
        return "ERR_NO_BLOCK_FOUND";
    case InternalFlash::Result::ERR_INVALID_BLOCK_INFO:
        return "ERR_INVALID_BLOCK_INFO";
    case InternalFlash::Result::ERR_READ_FAILED:
        return "ERR_READ_FAILED";
    case InternalFlash::Result::ERR_WRITE_FAILED:
        return "ERR_WRITE_FAILED";
    case InternalFlash::Result::ERR_ERASE_FAILED:
        return "ERR_ERASE_FAILED";
    case InternalFlash::Result::ERR_DATA_TOO_LARGE:
        return "ERR_DATA_TOO_LARGE";
    case InternalFlash::Result::ERR_DATA_CRC_MISMATCH:
        return "ERR_DATA_CRC_MISMATCH";
    case InternalFlash::Result::ERR_DATA_INVALID_INPUT:
        return "ERR_DATA_INVALID_INPUT";
    case InternalFlash::Result::ERR_DATA_ALREADY_SAVED:
        return "ERR_DATA_ALREADY_SAVED";
    case InternalFlash::Result::ERR:
        return "ERR";
    default:
        return "Unknown";
    }
}

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

    hw.StartLog(
        true); /* true == wait for PC: will block until a terminal is connected */

    InternalFlash InternalFlashConfig;

    static_assert((NUM_TEST_DATA_BYTES <= daisy::FlashBlockDataSize),
                  "Data to save is too large");

    uint8_t dataToSave[NUM_TEST_DATA_BYTES] = {0};
    uint8_t dataRead[NUM_TEST_DATA_BYTES];

    // fill date to write with 8 A5's
    memset(dataToSave, 0xCC, 8);

    // Save the configuration data
    InternalFlash::Result result = InternalFlashConfig.SaveData(dataToSave, sizeof(dataToSave));
    if (result == InternalFlash::Result::OK)
    {
        hw.PrintLine("Data saved successfully.");
    }
    else
    {
        // Print the error message
        hw.PrintLine("Failed to save data. Error message: %s", ResultToString(result));
    }

    // Get the size of the current configuration data
    uint32_t dataSize;
    result = InternalFlashConfig.GetCurrentDataSize(&dataSize);
    if (result == InternalFlash::Result::OK)
    {
        hw.PrintLine("Current config data size: %d bytes.\n", dataSize);  
    }
    else
    {
        hw.PrintLine("Failed to get current config data size. Error message: %s", ResultToString(result));
    }

    // Get the size of the current configuration data
    uint32_t index;
    result = InternalFlashConfig.GetCurrentIndex(&index);
    if (result == InternalFlash::Result::OK)
    {
        hw.PrintLine("Current config index: %d.\n", index);  
    }
    else
    {
        hw.PrintLine("Failed to get current config data size. Error code: %d", result);
    }

    // Read the current configuration data
    result = InternalFlashConfig.ReadCurrentData(dataRead, sizeof(dataRead));
    if (result == InternalFlash::Result::OK)
    {
        hw.PrintLine("Data read successfully: ");
        for (uint32_t i = 0; i < dataSize; ++i)
        {
            // Print i and dataRead separated by ": "
            // %02X prints data in 2 char hex, leading zeros
            hw.PrintLine("%d: %02X", i, dataRead[i]);
        }
        hw.PrintLine("");
    }
    else
    {
        hw.PrintLine("Failed to read data. Error message: %s", ResultToString(result));
    }

    // Verify that the data header CRC of the current config matches the written data
    // using InternalFlash method VerifyCurrentCRC()
    result = InternalFlashConfig.VerifyCurrentCRC();
    if (result == InternalFlash::Result::OK)
    {
        hw.PrintLine("Data CRC verified successfully.");
    }
    else
    {
        hw.PrintLine("Failed to verify data CRC. Error message: %s", ResultToString(result));
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