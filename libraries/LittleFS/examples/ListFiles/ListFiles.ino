// Print a list of all files stored on a flash memory chip

#include <LittleFS.h>

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
