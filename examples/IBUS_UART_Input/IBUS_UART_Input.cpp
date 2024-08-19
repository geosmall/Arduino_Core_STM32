/** Example of setting reading MIDI Input via UART
 *  
 * 
 *  This can be used with any 5-pin DIN or TRS connector that has been wired up
 *  to one of the UART Rx pins on Daisy.
 *  This will use D14 as the UART 1 Rx pin
 * 
 *  This example will also log incoming messages to the serial port for general MIDI troubleshooting
 */
#include "daisy_seed.h"

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

/** Global Hardware access */
DaisySeed         hw;
IBusRxHandler     ibus_rx;

int main(void)
{
    /** Initialize our hardware */
    hw.Init();

    System::Delay(50);

    hw.StartLog();

    IBusRxHandler::Config ser_rx_config;
    ibus_rx.Init(ser_rx_config);

    ibus_rx.StartReceive();

    // uint32_t now = System::GetNow();

    /** Infinite Loop */
    while(1)
    {
        // now = System::GetNow();

        /** Process Serial Rx in the background */
        ibus_rx.Listen();

        // hw.PrintLine("---");

        // Wait 500ms
        System::Delay(20);
    }
}