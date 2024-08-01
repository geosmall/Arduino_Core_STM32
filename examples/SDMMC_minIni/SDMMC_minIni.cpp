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


int Callback(const char *section, const char *key, const char *value, void *userdata)
{
    (void)userdata; /* this parameter is not used in this example */
    hw.PrintLine("    [%s]\t%s=%s\n", section, key, value);
    return 1;
}

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

    hw.PrintLine("Test begin...[");
    hw.PrintLine("");

    /** mount the filesystem to the root directory 
     *  fsi.GetSDPath can be used when mounting multiple filesystems on different media
     */
    if(f_mount(&fs, "/", 0) == FR_OK)
    {
        res = f_open(&file, "test.ini", (FA_READ + FA_OPEN_EXISTING));
        if (res == FR_OK)
        {
            while (f_gets(str, sizearray(str), &file))
            {
                hw.PrintLine(str);
            }
            f_close(&file);
        }

        hw.PrintLine("");

        /* string reading */
        n = ini_gets("first", "string", "dummy", str, sizearray(str), inifile);
        assert(n==4 && strcmp(str,"noot")==0);
        n = ini_gets("second", "string", "dummy", str, sizearray(str), inifile);
        assert(n==4 && strcmp(str,"mies")==0);
        n = ini_gets("first", "undefined", "dummy", str, sizearray(str), inifile);
        assert(n==5 && strcmp(str,"dummy")==0);
        /* ----- */
        hw.PrintLine("1. String reading tests passed\n");

        /* value reading */
        n = ini_getl("first", "val", -1, inifile);
        assert(n==1);
        n = ini_getl("second", "val", -1, inifile);
        assert(n==2);
        n = ini_getl("first", "undefined", -1, inifile);
        assert(n==-1);
        /* ----- */
        hw.PrintLine("2. Value reading tests passed\n");

        /* string writing */
        n = ini_puts("first", "alt", "flagged as \"correct\"", inifile);
        assert(n==1);
        n = ini_gets("first", "alt", "dummy", str, sizearray(str), inifile);
        assert(n==20 && strcmp(str,"flagged as \"correct\"")==0);
        /* ----- */
        n = ini_puts("second", "alt", "correct", inifile);
        assert(n==1);
        n = ini_gets("second", "alt", "dummy", str, sizearray(str), inifile);
        assert(n==7 && strcmp(str,"correct")==0);
        /* ----- */
        n = ini_puts("third", "test", "correct", inifile);
        assert(n==1);
        n = ini_gets("third", "test", "dummy", str, sizearray(str), inifile);
        assert(n==7 && strcmp(str,"correct")==0);
        /* ----- */
        n = ini_puts("second", "alt", "overwrite", inifile);
        assert(n==1);
        n = ini_gets("second", "alt", "dummy", str, sizearray(str), inifile);
        assert(n==9 && strcmp(str,"overwrite")==0);
        /* ----- */
        n = ini_puts("second", "alt", "123456789", inifile);
        assert(n==1);
        n = ini_gets("second", "alt", "dummy", str, sizearray(str), inifile);
        assert(n==9 && strcmp(str,"123456789")==0);
        /* ----- */
        hw.PrintLine("3. String writing tests passed\n");

        /* section/key enumeration */
        hw.PrintLine("4. Section/key enumeration, file structure follows\n");
        for (s = 0; ini_getsection(s, section, sizearray(section), inifile) > 0; s++)
        {
            hw.PrintLine("    [%s]\n", section);
            for (k = 0; ini_getkey(section, k, str, sizearray(str), inifile) > 0; k++)
            {
                hw.PrintLine("\t%s\n", str);
            }
        }

        /* section/key presence check */
        assert(ini_hassection("first", inifile));
        assert(!ini_hassection("fourth", inifile));
        assert(ini_haskey("first", "val", inifile));
        assert(!ini_haskey("first", "test", inifile));
        hw.PrintLine("5. checking presence of sections and keys passed\n");

        /* browsing through the file */
        hw.PrintLine("6. browse through all settings, file field list follows\n");
        ini_browse(Callback, NULL, inifile);

        /* string deletion */
        n = ini_puts("first", "alt", NULL, inifile);
        assert(n==1);
        n = ini_puts("second", "alt", NULL, inifile);
        assert(n==1);
        n = ini_puts("third", NULL, NULL, inifile);
        assert(n==1);
        /* ----- */
        hw.PrintLine("7. String deletion tests passed\n");
    }

    hw.PrintLine("");
    hw.PrintLine("]...Test End");

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
