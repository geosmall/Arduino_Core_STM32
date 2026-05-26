// Print a list of all files stored on a flash memory chip

#include <SPI.h>
#include <LittleFS.h>

// Board configuration — peripheral-aware SPI bus selection
#if defined(ARDUINO_BLACKPILL_F411CE)
#include "targets/BLACKPILL_F411CE.h"
#elif defined(ARDUINO_WEACT_G474CE)
#include "targets/WEACT_G474_HIL007.h"
#else
#include "targets/NUCLEO_F411RE_HIL001.h"
#endif

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

void setup() {
  bool res;

  Serial.begin(115200);
  while (!Serial) delay(100); // wait until Serial/monitor is opened

  Serial.println("LittleFS ListFiles test starting");
  Serial.println("SPI Flash test...");

  // ensure the CS pin is pulled HIGH
  pinMode(CS_PIN, OUTPUT); digitalWrite(CS_PIN, HIGH);

  delay(10); // Wait a bit to make sure w25qxx chip is ready

  res = myfs.begin(CS_PIN, SPIbus);
  if (!res) {
    Serial.println("initialization failed!");
    Local_Error_Handler();
  }

  Serial.printf("Space Used = %lu\n", (unsigned long)myfs.usedSize());
  Serial.printf("Filesystem Size = %lu\n", (unsigned long)myfs.totalSize());

  printDirectory(myfs);
  Serial.println("*STOP*");
}


void loop() {
}


void printDirectory(FS &fs) {
  Serial.println("Directory\n---------");
  printDirectory(fs.open("/"), 0);
  Serial.println();
}

void printDirectory(File dir, int numSpaces) {
   while(true) {
     File entry = dir.openNextFile();
     if (! entry) {
       break;
     }
     printSpaces(numSpaces);
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numSpaces+2);
     } else {
       // files have sizes, directories do not
       printSpaces(36 - numSpaces - strlen(entry.name()));
       Serial.printf("  %lu", (unsigned long)entry.size());
       DateTimeFields datetime;
       if (entry.getModifyTime(datetime)) {
         printSpaces(4);
         printTime(datetime);
       }
       Serial.println();
     }
     entry.close();
   }
}

void printSpaces(int num) {
  for (int i=0; i < num; i++) {
    Serial.print(" ");
  }
}

void printTime(const DateTimeFields tm) {
  const char *months[12] = {
    "January","February","March","April","May","June",
    "July","August","September","October","November","December"
  };
  if (tm.hour < 10) Serial.print("0");
  Serial.printf("%d", tm.hour);
  Serial.print(":");
  if (tm.min < 10) Serial.print("0");
  Serial.printf("%d", tm.min);
  Serial.print("  ");
  Serial.print(tm.mon < 12 ? months[tm.mon] : "???");
  Serial.print(" ");
  Serial.printf("%d", tm.mday);
  Serial.print(", ");
  Serial.printf("%d", tm.year + 1900);
}
