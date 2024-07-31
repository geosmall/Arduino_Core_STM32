/** Example of reading and writing an Ini file to an SD Card 
 *  If the SD Card is present, and writing is successful the onboard LED will blink rapidly.
 *  Otherwise, the LED will blink once every two seconds.
 */
#include "daisy_seed.h"

#include "fatfs.h"

#include "minIni.h"

#include <string.h>
#include <assert.h>

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

/** Global Hardware access */
DaisySeed hw;

/** SDMMC Configuration */
SdmmcHandler sdmmc;
/** FatFS Interface for libDaisy */
FatFSInterface fsi;
/** Global File object */
FIL file;

#define sizearray(a)  (sizeof(a) / sizeof((a)[0]))

const char inifile[] = "test.ini";

int main(void)
{
    /** Initialize our hardware */
    hw.Init();
    hw.StartLog(true); /* Wait until terminal is connected */

    /** Initialize the SDMMC Hardware 
     *  For this example we'll use:
     *  Medium (25MHz), 4-bit, w/out power save settings
     */
    SdmmcHandler::Config sd_cfg;
    sd_cfg.speed = SdmmcHandler::Speed::STANDARD;
    sdmmc.Init(sd_cfg);

    /** Setup our interface to the FatFS middleware */
    FatFSInterface::Config fsi_config;
    fsi_config.media = FatFSInterface::Config::MEDIA_SD;
    fsi.Init(fsi_config);

    /** Get the reference to the FATFS Filesystem for use in mounting the hardware. */
    FATFS& fs = fsi.GetSDFileSystem();

    /** default to a known error 
     *  by the end of the next if-statement it should be FR_OK
     */
    FRESULT res = FR_NO_FILESYSTEM;

    char str[64];
    long n;
    int s, k;
    char section[50];

    /** mount the filesystem to the root directory 
     *  fsi.GetSDPath can be used when mounting multiple filesystems on different media
     */
    if(f_mount(&fs, "/", 0) == FR_OK)
    {
        // if(f_open(&file, "helloworld.txt", (FA_CREATE_ALWAYS | FA_WRITE))
        //    == FR_OK)
        // {
        //     FixedCapStr<20> str = "Hello World!";
        //     UINT            bytes_written;
        //     res = f_write(&file, str.Cstr(), str.Size(), &bytes_written);
        //     f_close(&file);
        // }
        res = f_open(&file, "test.ini", (FA_READ + FA_OPEN_EXISTING));
        if (res == FR_OK)
        {
            while (f_gets(str, sizearray(str), &file))
            {
                hw.PrintLine(str);
            }
            f_close(&file);
        }
    }

    n = ini_gets("first", "string", "dummy", str, sizearray(str), "test.ini");
    assert(n==4 && strcmp(str,"noot")==0);

    /** Infinite Loop */
    while(1)
    {
        /** Very basic blink to indicate success or failure */
        uint32_t blink_rate;
        if(res == FR_OK)
            blink_rate = 125;
        else
            blink_rate = 1000;
        System::Delay(blink_rate);
        hw.SetLed(true);
        System::Delay(blink_rate);
        hw.SetLed(false);
    }
}
