/*  Storage Glue functions for the minIni library, using Generic Storage Abstraction
 *
 *  Integrates minIni with unified Storage.h interface supporting both:
 *  - LittleFS (SPI flash storage)
 *  - SDFS (SD card storage via SPI)
 *
 *  Storage backend automatically selected based on BoardConfig configuration.
 *
 *  This file is in the public domain. It is distributed without
 *  warranties or conditions of any kind, either express or implied.
 */

#pragma once

#include <stdio.h>
#include <Arduino.h>
#include <Storage.h>
#include <BoardStorage.h>
#include <libPrintf.h>  // Embedded printf with float support (provides sprintf_)
// libPrintf aliases standard names via macros (e.g., #define vsnprintf vsnprintf_).
// These conflict with C++ std:: variants (std::vsnprintf in std::to_string).
// StorageGlue only needs sprintf_() directly, so undo the global aliases.
#undef printf
#undef sprintf
#undef vsprintf
#undef snprintf
#undef vsnprintf
#undef vprintf

// Temporarily disable debug output to isolate issue
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)

// Map minIni file types to our Generic Storage abstraction
#define INI_FILETYPE                    File

// Storage instance access
static Storage* getStorageInstance() {
    if (!BoardStorage::isInitialized()) {
        return nullptr;  // Storage not initialized
    }
    return &BOARD_STORAGE;
}

static int ini_openread(const char* filename, INI_FILETYPE *file)
{
    Storage* storage = getStorageInstance();
    if (!storage) return 0;  // Storage not initialized
    *file = storage->open(filename, FILE_READ);
    return file->operator bool();  // Check if file opened successfully
}

static int ini_openwrite(const char* filename, INI_FILETYPE *file)
{
    Storage* storage = getStorageInstance();
    if (!storage) return 0;  // Storage not initialized
    *file = storage->open(filename, FILE_WRITE);
    return file->operator bool();  // Check if file opened successfully
}

// #define ini_openrewrite(filename,file)   not defined

static int ini_close(INI_FILETYPE *file)
{
    file->close();
    return 1;
}

static int ini_read(char *buffer, int size, INI_FILETYPE *file)
{
    int i;

    for(i = 0; i < size-1; i++){
        if(file->available() > 0){
            char c = file->read();
            buffer[i] = c;
            if(c == '\n'){
                i++;
                break;
            }
        }else{
            break;
        }
    }
    buffer[i] = 0;

    return i;
}

static int ini_write(char *buffer, INI_FILETYPE *file)
{
    int size = strlen(buffer);
    return (file->write(buffer, size) > 0);
}

static int ini_rename(const char *source, const char *dest)
{
    Storage* storage = getStorageInstance();
    if (!storage) return 0;  // Storage not initialized

    File srcFile = storage->open(source, FILE_READ);
    if(!srcFile) return 0;

    File dstFile = storage->open(dest, FILE_WRITE);
    if(!dstFile) {
        srcFile.close();
        return 0;
    }

    const int BUFF_SIZE = 512;
    char buffer[BUFF_SIZE];
    int size;
    while((size = srcFile.available()) > 0){
        if(size > BUFF_SIZE) size = BUFF_SIZE;
        srcFile.read(buffer, size);
        dstFile.write(buffer, size);
    }

    srcFile.close();
    dstFile.close();

    return storage->remove(source);
}

static int ini_remove_impl(const char* filename) {
    Storage* storage = getStorageInstance();
    if (!storage) return 0;  // Storage not initialized
    return storage->remove(filename);
}

#define ini_remove(filename)            ini_remove_impl(filename)

#define INI_FILEPOS                     long int
#define ini_tell(file,pos)              ( *(pos) = (file)->position() )
#define ini_seek(file,pos)              ( (file)->seek(*(pos)) )

/* for floating-point support, define additional types and functions */
#define INI_REAL                        float
#define ini_ftoa(string,value)          sprintf_((string),"%f",(value))  // Use libPrintf directly
#define ini_atof(string)                (INI_REAL)strtod((string),NULL)