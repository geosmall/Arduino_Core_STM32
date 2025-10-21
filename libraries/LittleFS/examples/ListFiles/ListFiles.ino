// Print a list of all files stored on a flash memory chip

#include <LittleFS.h>
#include <ci_log.h>

void Local_Error_Handler()
{
    asm("BKPT #0\n"); // break into the debugger
}

#if defined(ARDUINO_BLACKPILL_F411CE)
//              MOSI  MISO  SCLK
SPIClass SPIbus(PA7,  PA6,  PA5);
#define CS_PIN PA4
#else
//              MOSI  MISO  SCLK
SPIClass SPIbus(PC12, PC11, PC10);
#define CS_PIN PD2
#endif

LittleFS_SPIFlash myfs;

void setup() {
  bool res;

  Serial.begin(115200);
  while (!Serial) delay(100); // wait until Serial/monitor is opened

  CI_BUILD_INFO();
  CI_LOG("LittleFS ListFiles test starting\n");
  CI_READY_TOKEN();

  CI_LOG("SPI Flash test...\n");

  // ensure the CS pin is pulled HIGH
  pinMode(CS_PIN, OUTPUT); digitalWrite(CS_PIN, HIGH);

  delay(10); // Wait a bit to make sure w25qxx chip is ready

  res = myfs.begin(CS_PIN, SPIbus);
  if (!res) {
    CI_LOG("initialization failed!\n");
    Local_Error_Handler();
  }

  CI_LOGF("Space Used = %lu\n", (unsigned long)myfs.usedSize());
  CI_LOGF("Filesystem Size = %lu\n", (unsigned long)myfs.totalSize());

  printDirectory(myfs);
  CI_LOG("*STOP*\n");
}


void loop() {
}


void printDirectory(FS &fs) {
  CI_LOG("Directory\n---------\n");
  printDirectory(fs.open("/"), 0);
  CI_LOG("\n");
}

void printDirectory(File dir, int numSpaces) {
   while(true) {
     File entry = dir.openNextFile();
     if (! entry) {
       break;
     }
     printSpaces(numSpaces);
     CI_LOG(entry.name());
     if (entry.isDirectory()) {
       CI_LOG("/\n");
       printDirectory(entry, numSpaces+2);
     } else {
       // files have sizes, directories do not
       printSpaces(36 - numSpaces - strlen(entry.name()));
       CI_LOGF("  %lu", (unsigned long)entry.size());
       DateTimeFields datetime;
       if (entry.getModifyTime(datetime)) {
         printSpaces(4);
         printTime(datetime);
       }
       CI_LOG("\n");
     }
     entry.close();
   }
}

void printSpaces(int num) {
  for (int i=0; i < num; i++) {
    CI_LOG(" ");
  }
}

void printTime(const DateTimeFields tm) {
  const char *months[12] = {
    "January","February","March","April","May","June",
    "July","August","September","October","November","December"
  };
  if (tm.hour < 10) CI_LOG("0");
  CI_LOGF("%d", tm.hour);
  CI_LOG(":");
  if (tm.min < 10) CI_LOG("0");
  CI_LOGF("%d", tm.min);
  CI_LOG("  ");
  CI_LOG(tm.mon < 12 ? months[tm.mon] : "???");
  CI_LOG(" ");
  CI_LOGF("%d", tm.mday);
  CI_LOG(", ");
  CI_LOGF("%d", tm.year + 1900);
}
