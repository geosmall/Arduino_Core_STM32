#include <SPI.h>
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

  CI_BUILD_INFO();
  CI_LOG("LittleFS Usage test starting\n");
  CI_READY_TOKEN();

  // ensure the CS pin is pulled HIGH
  pinMode(CS_PIN, OUTPUT); digitalWrite(CS_PIN, HIGH);

  delay(10); // Wait a bit to make sure w25qxx chip is ready

  CI_LOG("Initializing LittleFS ...");

  // see if the Flash is present and can be initialized:
  // Note:  SPI is default so if you are using SPI and not SPI for instance
  //        you can just specify myfs.begin(chipSelect). 
  if (!myfs.begin(CS_PIN, SPIbus)) {
    CI_LOGF("Error starting %s\n", "SPI FLASH");
    while (1) {
      // Error, so don't do anything more - stay stuck here
    }
  }
  CI_LOG(myfs.getMediaName());
  CI_LOG("\n");

  myfs.format();
  CI_LOG("LittleFS initialized.\n");
  
  
  // To get the current space used and Filesystem size
  CI_LOG("\n---------------\n");
  uint64_t usedSize = myfs.usedSize();
  uint64_t totalSize = myfs.totalSize();
  CI_LOG("Bytes Used: ");
  printU64(usedSize);
  CI_LOG(", Bytes Total: ");
  printU64(totalSize);
  CI_LOG("\n");

  // Now lets create a file and write some data.  Note: basically the same usage for
  // creating and writing to a file using SD library.
  CI_LOG("\n---------------\n");
  CI_LOG("Now lets create a file with some data in it\n");
  CI_LOG("---------------\n");
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
  CI_LOGF("File Size of bigfile.txt (bytes): %u\n", file.size());
  file.close();

  // Now that we initialized the FS and created a file lets print the directory.
  // Note:  Since we are going to be doing print directory and getting disk usuage
  // lets make it a function which can be copied and used in your own sketches.
  listFiles();
  
  // Now lets rename the file
  CI_LOG("\n---------------\n");
  CI_LOG("Rename bigfile to file10\n");
  myfs.rename("bigfile.txt", "file10.txt");
  listFiles();

  // To delete the file
  CI_LOG("\n---------------\n");
  CI_LOG("Delete file10.txt\n");
  myfs.remove("file10.txt");
  listFiles();

  CI_LOG("\n---------------\n");
  CI_LOG("Create a directory and a subfile\n");
  myfs.mkdir("structureData1");

  file = myfs.open("structureData1/temp_test.txt", FILE_WRITE);
  file.println("SOME DATA TO TEST");
  file.close();
  listFiles();

  CI_LOG("\n---------------\n");
  CI_LOG("Rename directory\n");
  myfs.rename("structureData1", "structuredData");
  listFiles();

  CI_LOG("\n---------------\n");
  CI_LOG("Lets remove them now...\n");
  //Note have to remove directories files first
  myfs.remove("structuredData/temp_test.txt");
  myfs.rmdir("structuredData");
  listFiles();

  CI_LOG("\n---------------\n");
  CI_LOG("Now lets create a file and read the data back...\n");
  
  // LittleFS also supports truncate function similar to SDFat. As shown in this
  // example, you can truncate files.
  //
  CI_LOG("\n");
  CI_LOG("Writing to datalog.bin using LittleFS functions\n");
  file1 = myfs.open("datalog.bin", FILE_WRITE);
  unsigned int len = file1.size();
  CI_LOG("datalog.bin started with ");
  CI_LOGF("%u", len);
  CI_LOG(" bytes\n");
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
  CI_LOG("\n");
  CI_LOG("Reading to datalog.bin using LittleFS functions\n");
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
    CI_LOG("  Read from file: ");
    CI_LOG(mybuffer);
    CI_LOG("\n");
  } else {
    CI_LOG("unable to open datalog.bin :(\n");
  }
  file2.close();

  CI_LOG("\nBasic Usage Example Finished\n");
  CI_LOG("*STOP*\n");
}

void loop() {}

void listFiles()
{
  CI_LOG("---------------\n");
  printDirectory(myfs);
  CI_LOG("Bytes Used: ");
  printU64(myfs.usedSize());
  CI_LOG(", Bytes Total: ");
  printU64(myfs.totalSize());
  CI_LOG("\n");
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
       CI_LOG("  ");
       CI_LOGF("%u\n", entry.size());
     }
     entry.close();
   }
}

void printSpaces(int num) {
  for (int i=0; i < num; i++) {
    CI_LOG(" ");
  }
}

