// LittleFS ChipID Example - Display SPI flash chip information

#include <SPI.h>
#include "LittleFS.h"

// Board configuration — peripheral-aware SPI bus selection
#if defined(ARDUINO_BLACKPILL_F411CE)
#include "targets/BLACKPILL_F411CE.h"
#elif defined(ARDUINO_WEACT_G474CE)
#include "targets/WEACT_G474_HIL007.h"
#else
#include "targets/NUCLEO_F411RE_HIL001.h"
#endif

/** Uncomment to erase W25QXX chip */
// #define ERASE_CHIP

void Local_Error_Handler()
{
    asm("BKPT #0\n"); // break into the debugger
}

SPIClass SPIbus(BoardConfig::storage.instance,
                BoardConfig::storage.mosi_pin,
                BoardConfig::storage.miso_pin,
                BoardConfig::storage.sclk_pin);
#define CS_PIN BoardConfig::storage.cs_pin

LittleFS_SPIFlash myfs;

// the setup routine runs once when you press reset:
void setup()
{
    bool res;

    Serial.begin(115200);
    while (!Serial) delay(100); // wait until Serial/monitor is opened

    Serial.println("LittleFS ChipID test starting");
    Serial.println("SPI Flash test...");

    pinMode(LED_BUILTIN, OUTPUT);

    // ensure the CS pin is pulled HIGH
    pinMode(CS_PIN, OUTPUT); digitalWrite(CS_PIN, HIGH);

    delay(10); // Wait a bit to make sure w25qxx chip is ready

    res = myfs.begin(CS_PIN, SPIbus);
    if (!res) {
        Serial.println("initialization failed!");
        Local_Error_Handler();
    }

    // Get and display chip information
    LFS_W25QXX_info_t info;
    if (myfs.getChipInfo(info)) {
        Serial.println("W25QXX successfully initialized");
        Serial.printf("Manufacturer       = 0x%02x\n", info.manufacturer_id);
        Serial.printf("JEDEC Device       = 0x%04x\n", info.jedec_id);
        Serial.printf("Block size         = 0x%04lx (%lu)\n", info.block_size, info.block_size);
        Serial.printf("Block count        = 0x%04lx (%lu)\n", info.block_count, info.block_count);
        Serial.printf("Sector size        = 0x%04lx (%lu)\n", info.sector_size, info.sector_size);
        Serial.printf("Sectors per block  = 0x%04lx (%lu)\n", info.sectors_in_block, info.sectors_in_block);
        Serial.printf("Page size          = 0x%04lx (%lu)\n", info.page_size, info.page_size);
        Serial.printf("Pages per sector   = 0x%04lx (%lu)\n", info.pages_in_sector, info.pages_in_sector);
        Serial.printf("Total size (in kB) = 0x%04lx (%lu)\n", (info.block_count * info.block_size) / 1024, (info.block_count * info.block_size) / 1024);
    } else {
        Serial.println("Unable to retrieve chip information");
        Local_Error_Handler();
    }

#if defined (ERASE_CHIP)
    Serial.println("Erasing...");
    int ret = myfs.eraseChip();
    if (ret != LFS_ERR_OK) {
        Local_Error_Handler();
    }
    Serial.println("Done erasing chip");
#endif

    Serial.println("*STOP*");
}

// the loop routine runs over and over again forever:
void loop()
{
    delay(100);
}
