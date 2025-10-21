#include "SDFS.h"
#include "SDFSConfig.h"
#include "sd_spi_diskio.h"
#include <string.h>


// Global clock instance
SDFS_clock_class SDFSClock;

// ============================================================================
// SDFSFile Implementation
// ============================================================================

SDFSFile::SDFSFile(FATFS *fsin, FIL *filin, const char *name)
{
    if (!fsin || !filin || !name) {
        fs = nullptr;
        file = nullptr;
        dir = nullptr;
        fullpath[0] = '\0';
        return;
    }
    fs = fsin;
    file = filin;
    dir = nullptr;
    strlcpy(fullpath, name, sizeof(fullpath));
}

SDFSFile::SDFSFile(FATFS *fsin, DIR *dirin, const char *name)
{
    if (!fsin || !dirin || !name) {
        fs = nullptr;
        file = nullptr;
        dir = nullptr;
        fullpath[0] = '\0';
        return;
    }
    fs = fsin;
    dir = dirin;
    file = nullptr;
    strlcpy(fullpath, name, sizeof(fullpath));
}

SDFSFile::~SDFSFile()
{
    close();
}

size_t SDFSFile::read(void *buf, size_t nbyte)
{
    if (!file || !buf || nbyte == 0) return 0;
    
    UINT bytesRead;
    FRESULT fr = f_read(file, buf, nbyte, &bytesRead);
    return (fr == FR_OK) ? bytesRead : 0;
}

size_t SDFSFile::write(const void *buf, size_t size)
{
    if (!file || !buf || size == 0) return 0;
    
    UINT bytesWritten;
    FRESULT fr = f_write(file, buf, size, &bytesWritten);
    return (fr == FR_OK) ? bytesWritten : 0;
}

int SDFSFile::available()
{
    if (!file) return 0;
    
    FSIZE_t fileSize = f_size(file);
    FSIZE_t currentPos = f_tell(file);
    return (int)(fileSize - currentPos);
}

int SDFSFile::peek()
{
    return -1; // Not implemented for now
}

void SDFSFile::flush()
{
    if (file) f_sync(file);
}

bool SDFSFile::truncate(uint64_t size)
{
    if (!file) return false;
    
    // FatFs doesn't have direct truncate, so we use seek + truncate at current position
    FRESULT fr = f_lseek(file, size);
    if (fr != FR_OK) return false;
    
    fr = f_truncate(file);
    return (fr == FR_OK);
}

bool SDFSFile::seek(uint64_t pos, int mode)
{
    if (!file) return false;
    
    FSIZE_t newPos;
    if (mode == SeekSet) {
        newPos = pos;
    } else if (mode == SeekCur) {
        newPos = f_tell(file) + pos;
    } else if (mode == SeekEnd) {
        newPos = f_size(file) + pos;
    } else {
        return false;
    }
    
    FRESULT fr = f_lseek(file, newPos);
    return (fr == FR_OK);
}

uint64_t SDFSFile::position()
{
    if (!file) return 0;
    return f_tell(file);
}

uint64_t SDFSFile::size()
{
    if (!file) return 0;
    return f_size(file);
}

void SDFSFile::close()
{
    if (file) {
        f_close(file);
        free(file);
        file = nullptr;
    }
    if (dir) {
        f_closedir(dir);
        free(dir);
        dir = nullptr;
    }
}

bool SDFSFile::isOpen()
{
    return file || dir;
}

const char* SDFSFile::name()
{
    const char *p = strrchr(fullpath, '/');
    if (p) return p + 1;
    return fullpath;
}

bool SDFSFile::isDirectory()
{
    return dir != nullptr;
}

File SDFSFile::openNextFile(uint8_t mode)
{
    if (!dir) return File();
    
    FILINFO fno;
    FRESULT fr;
    
    // Clear the FILINFO structure before use
    memset(&fno, 0, sizeof(FILINFO));
    
    do {
        fr = f_readdir(dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0) return File(); // End of directory or error
    } while (fno.fname[0] == '.' && (fno.fname[1] == 0 || (fno.fname[1] == '.' && fno.fname[2] == 0)));
    
    // Build full path
    char pathname[128];
    strlcpy(pathname, fullpath, sizeof(pathname));
    size_t len = strlen(pathname);
    if (len > 0 && pathname[len - 1] != '/' && len < sizeof(pathname) - 2) {
        pathname[len++] = '/';
        pathname[len] = 0;
    }
    strlcpy(pathname + len, fno.fname, sizeof(pathname) - len);
    
    if (fno.fattrib & AM_DIR) {
        // Directory
        DIR *d = (DIR *)malloc(sizeof(DIR));
        if (!d) return File();
        if (f_opendir(d, pathname) == FR_OK) {
            return File(new SDFSFile(fs, d, pathname));
        }
        free(d);
    } else {
        // Regular file
        FIL *f = (FIL *)malloc(sizeof(FIL));
        if (!f) return File();
        if (f_open(f, pathname, FA_READ) == FR_OK) {
            return File(new SDFSFile(fs, f, pathname));
        }
        free(f);
    }
    
    return File();
}

void SDFSFile::rewindDirectory()
{
    if (dir) f_rewinddir(dir);
}

bool SDFSFile::getCreateTime(DateTimeFields &tm)
{
    // FatFs doesn't store creation time separately, use modify time
    return getModifyTime(tm);
}

bool SDFSFile::getModifyTime(DateTimeFields &tm)
{
    if (!fs) return false;
    
    FILINFO fno;
    FRESULT fr = f_stat(fullpath, &fno);
    if (fr != FR_OK) return false;
    
    uint32_t unixTime = fatTimeToUnix(fno.fdate, fno.ftime);
    if (unixTime == 0) return false;
    
    breakTime(unixTime, tm);
    return true;
}

bool SDFSFile::setCreateTime(const DateTimeFields &tm)
{
    // FatFs doesn't support setting creation time
    return false;
}

bool SDFSFile::setModifyTime(const DateTimeFields &tm)
{
    // FatFs doesn't support setting modify time after file creation
    return false;
}

uint32_t SDFSFile::fatTimeToUnix(WORD fdate, WORD ftime)
{
    if (fdate == 0 && ftime == 0) return 0;
    
    DateTimeFields dt;
    dt.year = ((fdate >> 9) & 0x7F) + 1980 - 1970; // Convert to years since 1970
    dt.mon = (fdate >> 5) & 0x0F;
    dt.mday = fdate & 0x1F;
    dt.hour = (ftime >> 11) & 0x1F;
    dt.min = (ftime >> 5) & 0x3F;
    dt.sec = (ftime & 0x1F) * 2;
    
    return makeTime(dt);
}

void SDFSFile::unixToFatTime(uint32_t unixTime, WORD *fdate, WORD *ftime)
{
    DateTimeFields dt;
    breakTime(unixTime, dt);
    
    *fdate = ((dt.year + 1970 - 1980) << 9) | (dt.mon << 5) | dt.mday;
    *ftime = (dt.hour << 11) | (dt.min << 5) | (dt.sec / 2);
}

// ============================================================================
// SDFS Implementation
// ============================================================================

SDFS::~SDFS()
{
    if (mounted) {
        unmountFilesystem();
    }
}

File SDFS::open(const char *filepath, uint8_t mode)
{
    if (!filepath || strlen(filepath) == 0) return File();
    if (!mounted) {
        last_error = SDFS_NOT_MOUNTED;
        return File();
    }
    
    if (mode == FILE_READ) {
        // Special case for root directory - FatFs f_stat("/", ...) may not work
        if (strcmp(filepath, "/") == 0) {
            DIR *dir = (DIR *)malloc(sizeof(DIR));
            if (!dir) return File();
            if (f_opendir(dir, filepath) == FR_OK) {
                return File(new SDFSFile(&fatfs, dir, filepath));
            }
            free(dir);
            return File();
        }

        FILINFO fno;
        FRESULT fr = f_stat(filepath, &fno);
        if (fr != FR_OK) {
            return File();
        }

        if (fno.fattrib & AM_DIR) {
            // Directory
            DIR *dir = (DIR *)malloc(sizeof(DIR));
            if (!dir) return File();
            if (f_opendir(dir, filepath) == FR_OK) {
                return File(new SDFSFile(&fatfs, dir, filepath));
            }
            free(dir);
        } else {
            // Regular file
            FIL *file = (FIL *)malloc(sizeof(FIL));
            if (!file) return File();
            if (f_open(file, filepath, FA_READ) == FR_OK) {
                return File(new SDFSFile(&fatfs, file, filepath));
            }
            free(file);
        }
    } else {
        // Write modes
        FIL *file = (FIL *)malloc(sizeof(FIL));
        if (!file) return File();
        
        BYTE openMode = FA_READ | FA_WRITE | FA_OPEN_ALWAYS;
        FRESULT fr = f_open(file, filepath, openMode);
        if (fr == FR_OK) {
            if (mode == FILE_WRITE) {
                // Append mode - seek to end
                f_lseek(file, f_size(file));
            }
            // else FILE_WRITE_BEGIN - start at beginning (default)
            return File(new SDFSFile(&fatfs, file, filepath));
        }
        free(file);
    }
    
    return File();
}

bool SDFS::exists(const char *filepath)
{
    if (!filepath || strlen(filepath) == 0) return false;
    if (!mounted) {
        last_error = SDFS_NOT_MOUNTED;
        return false;
    }

    // Special case for root directory
    if (strcmp(filepath, "/") == 0) {
        return mounted;  // If we're mounted, root directory exists
    }

    FILINFO fno;
    FRESULT fr = f_stat(filepath, &fno);
    return (fr == FR_OK);
}

bool SDFS::mkdir(const char *filepath)
{
    if (!filepath || strlen(filepath) == 0) return false;
    if (!mounted) {
        last_error = SDFS_NOT_MOUNTED;
        return false;
    }
    
    FRESULT fr = f_mkdir(filepath);
    return (fr == FR_OK);
}

bool SDFS::rename(const char *oldfilepath, const char *newfilepath)
{
    if (!oldfilepath || !newfilepath || 
        strlen(oldfilepath) == 0 || strlen(newfilepath) == 0) return false;
    if (!mounted) return false;
    
    FRESULT fr = f_rename(oldfilepath, newfilepath);
    return (fr == FR_OK);
}

bool SDFS::remove(const char *filepath)
{
    if (!filepath || strlen(filepath) == 0) return false;
    if (!mounted) return false;
    
    FRESULT fr = f_unlink(filepath);
    return (fr == FR_OK);
}

bool SDFS::rmdir(const char *filepath)
{
    return remove(filepath); // FatFs f_unlink works for both files and directories
}

uint64_t SDFS::usedSize()
{
    if (!mounted) return 0;
    
    FATFS *fs;
    DWORD freeClusters;
    FRESULT fr = f_getfree(drive_path, &freeClusters, &fs);
    if (fr != FR_OK) return totalSize();
    
    DWORD totalClusters = (fs->n_fatent - 2);
    DWORD usedClusters = totalClusters - freeClusters;
    return (uint64_t)usedClusters * fs->csize * 512; // 512 bytes per sector
}

uint64_t SDFS::totalSize()
{
    if (!mounted) return 0;
    
    FATFS *fs;
    DWORD freeClusters;
    FRESULT fr = f_getfree(drive_path, &freeClusters, &fs);
    if (fr != FR_OK) return 0;
    
    DWORD totalClusters = (fs->n_fatent - 2);
    return (uint64_t)totalClusters * fs->csize * 512;
}

bool SDFS::format()
{
    if (!configured) return false;
    
    unmountFilesystem();
    
    // Format the drive
    static uint8_t work_area[512]; // Work area buffer
    MKFS_PARM opt = {FM_ANY, 0, 0, 0, 0}; // Format options
    FRESULT fr = f_mkfs(drive_path, &opt, work_area, sizeof(work_area));
    if (fr != FR_OK) {
        return false;
    }
    
    // Remount after format
    return (mountFilesystem() == FR_OK);
}

bool SDFS::mediaPresent()
{
    return mounted;
}

const char* SDFS::name()
{
    return "SDFS";
}

FRESULT SDFS::mountFilesystem()
{
    if (mounted) return FR_OK;
    
    FRESULT fr = f_mount(&fatfs, drive_path, 1); // 1 = mount immediately
    if (fr == FR_OK) {
        mounted = true;
    }
    return fr;
}

void SDFS::unmountFilesystem()
{
    if (mounted) {
        f_mount(nullptr, drive_path, 0); // Unmount
        mounted = false;
    }
}

const char* SDFS::errorToString(SDFSERR err)
{
    switch (err) {
        case SDFS_OK: return "No error";
        case SDFS_ALREADY_MOUNTED: return "Already mounted";
        case SDFS_CARD_INIT_FAILED: return "SD card initialization failed";
        case SDFS_MOUNT_FAILED: return "Filesystem mount failed";
        case SDFS_NOT_MOUNTED: return "Not mounted";
        case SDFS_INTERNAL_ERROR: return "Internal error";
        default: return "Unknown error";
    }
}

const char* SDFS::fresultToString(FRESULT fr)
{
    switch (fr) {
        case FR_OK: return "OK";
        case FR_DISK_ERR: return "Disk Error";
        case FR_INT_ERR: return "Internal Error";
        case FR_NOT_READY: return "Not Ready";
        case FR_NO_FILE: return "No File";
        case FR_NO_PATH: return "No Path";
        case FR_INVALID_NAME: return "Invalid Name";
        case FR_DENIED: return "Denied";
        case FR_EXIST: return "Exists";
        case FR_INVALID_OBJECT: return "Invalid Object";
        case FR_WRITE_PROTECTED: return "Write Protected";
        case FR_INVALID_DRIVE: return "Invalid Drive";
        case FR_NOT_ENABLED: return "Not Enabled";
        case FR_NO_FILESYSTEM: return "No Filesystem";
        case FR_MKFS_ABORTED: return "Format Aborted";
        case FR_TIMEOUT: return "Timeout";
        case FR_LOCKED: return "Locked";
        case FR_NOT_ENOUGH_CORE: return "Not Enough Memory";
        case FR_TOO_MANY_OPEN_FILES: return "Too Many Open Files";
        case FR_INVALID_PARAMETER: return "Invalid Parameter";
        default: return "Unknown Error";
    }
}

// ============================================================================
// SDFS_SPI Implementation  
// ============================================================================

bool SDFS_SPI::begin(uint8_t cspin, SPIClass &spiport)
{
    // Guard rail: Prevent multiple begin() calls
    if (mounted) {
        last_error = SDFS_ALREADY_MOUNTED;
        return true; // Already successfully mounted
    }

    pin = cspin;
    port = &spiport;

    // Initialize custom SPI disk I/O
    if (!initializeSDCard()) {
        last_error = SDFS_CARD_INIT_FAILED;
        #ifdef DEBUG
        Serial.printf("SDFS Error: %s\n", errorToString(last_error));
        #endif
        return false;
    }

    configured = true;

    // Attempt to mount the filesystem
    FRESULT fr = mountFilesystem();
    if (fr != FR_OK) {
        last_error = SDFS_MOUNT_FAILED;
        #ifdef DEBUG
        Serial.printf("SDFS Error: %s (FatFs: %s)\n", errorToString(last_error), fresultToString(fr));
        #endif
        return false;
    }

    last_error = SDFS_OK;
    return true;
}

const char* SDFS_SPI::getMediaName()
{
    return "SD Card (SPI)";
}

void SDFS_SPI::setSPISpeed(uint32_t speed_hz)
{
    sd_spi_set_speed(speed_hz);
}

uint32_t SDFS_SPI::getSPISpeed()
{
    return sd_spi_get_speed();
}

bool SDFS_SPI::initializeSDCard()
{
    // Initialize the SPI SD card interface
    // This will be implemented in the custom disk I/O file
    return sd_spi_initialize(pin, port);
}