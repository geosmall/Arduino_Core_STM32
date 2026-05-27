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

// Some variables for later use
uint64_t fTot, totSize1;

// To use SPI flash we need to create a instance of the library telling it to use SPI flash.
LittleFS_SPIFlash myfs;

// Specifies that the file, file1 and file3 are File types, same as you would do for creating files
// on a SD Card
File file, file1, file2;

void setup()
{
  Serial.begin(115200);
  while (!Serial) delay(100); // wait until Serial/monitor is opened

  Serial.println("LittleFS Usage test starting");

  // ensure the CS pin is pulled HIGH
  pinMode(CS_PIN, OUTPUT); digitalWrite(CS_PIN, HIGH);

  delay(10); // Wait a bit to make sure w25qxx chip is ready

  Serial.print("Initializing LittleFS ...");

  // see if the Flash is present and can be initialized:
  // Note:  SPI is default so if you are using SPI and not SPI for instance
  //        you can just specify myfs.begin(chipSelect).
  if (!myfs.begin(CS_PIN, SPIbus)) {
    Serial.println("Error starting SPI FLASH");
    while (1) {
      // Error, so don't do anything more - stay stuck here
    }
  }
  Serial.println(myfs.getMediaName());

  myfs.format();
  Serial.println("LittleFS initialized.");


  // To get the current space used and Filesystem size
  Serial.println("\n---------------");
  uint64_t usedSize = myfs.usedSize();
  uint64_t totalSize = myfs.totalSize();
  Serial.print("Bytes Used: ");
  Serial.print((unsigned long)usedSize);
  Serial.print(", Bytes Total: ");
  Serial.println((unsigned long)totalSize);

  // Now lets create a file and write some data.  Note: basically the same usage for
  // creating and writing to a file using SD library.
  Serial.println("\n---------------");
  Serial.println("Now lets create a file with some data in it");
  Serial.println("---------------");
  char someData[128];
  memset( someData, 'z', 128 );
  file = myfs.open("bigfile.txt", FILE_WRITE);
  file.write(someData, sizeof(someData));

  for (uint16_t j = 0; j < 100; j++)
    file.write(someData, sizeof(someData));
  file.close();

  // We can also get the size of the file just created.  Note we have to open and
  // thes close the file unless we do file size before we close it in the previous step
  file = myfs.open("bigfile.txt", FILE_WRITE);
  Serial.printf("File Size of bigfile.txt (bytes): %u\n", file.size());
  file.close();

  // Now that we initialized the FS and created a file lets print the directory.
  // Note:  Since we are going to be doing print directory and getting disk usuage
  // lets make it a function which can be copied and used in your own sketches.
  listFiles();

  // Now lets rename the file
  Serial.println("\n---------------");
  Serial.println("Rename bigfile to file10");
  myfs.rename("bigfile.txt", "file10.txt");
  listFiles();

  // To delete the file
  Serial.println("\n---------------");
  Serial.println("Delete file10.txt");
  myfs.remove("file10.txt");
  listFiles();

  Serial.println("\n---------------");
  Serial.println("Create a directory and a subfile");
  myfs.mkdir("structureData1");

  file = myfs.open("structureData1/temp_test.txt", FILE_WRITE);
  file.println("SOME DATA TO TEST");
  file.close();
  listFiles();

  Serial.println("\n---------------");
  Serial.println("Rename directory");
  myfs.rename("structureData1", "structuredData");
  listFiles();

  Serial.println("\n---------------");
  Serial.println("Lets remove them now...");
  //Note have to remove directories files first
  myfs.remove("structuredData/temp_test.txt");
  myfs.rmdir("structuredData");
  listFiles();

  Serial.println("\n---------------");
  Serial.println("Now lets create a file and read the data back...");

  // LittleFS also supports truncate function similar to SDFat. As shown in this
  // example, you can truncate files.
  //
  Serial.println("Writing to datalog.bin using LittleFS functions");
  file1 = myfs.open("datalog.bin", FILE_WRITE);
  unsigned int len = file1.size();
  Serial.print("datalog.bin started with ");
  Serial.print(len);
  Serial.println(" bytes");
  if (len > 0) {
    // reduce the file to zero if it already had data
    file1.truncate();
  }
  file1.print("Just some test data written to the file (by SdFat functions)");
  file1.write((uint8_t) 0);
  file1.close();

  // You can also use regular SD type functions, even to access the same file.  Just
  // remember to close the file before opening as a regular SD File.
  //
  Serial.println("Reading to datalog.bin using LittleFS functions");
  file2 = myfs.open("datalog.bin");
  if (file2) {
    char mybuffer[100];
    int index = 0;
    while (file2.available()) {
      char c = file2.read();
      mybuffer[index] = c;
      if (c == 0) break;  // end of string
      index = index + 1;
      if (index == 99) break; // buffer full
    }
    mybuffer[index] = 0;
    Serial.print("  Read from file: ");
    Serial.println(mybuffer);
  } else {
    Serial.println("unable to open datalog.bin :(");
  }
  file2.close();

  Serial.println("\nBasic Usage Example Finished");
  Serial.println("*STOP*");
}

void loop() {}

void listFiles()
{
  Serial.println("---------------");
  printDirectory(myfs);
  Serial.print("Bytes Used: ");
  Serial.print((unsigned long)myfs.usedSize());
  Serial.print(", Bytes Total: ");
  Serial.println((unsigned long)myfs.totalSize());
}

void printDirectory(FS &fs) {
  Serial.println("Directory\n---------");
  printDirectory(fs.open("/"), 0);
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
       Serial.print("  ");
       Serial.printf("%u\n", entry.size());
     }
     entry.close();
   }
}

void printSpaces(int num) {
  for (int i=0; i < num; i++) {
    Serial.print(" ");
  }
}
