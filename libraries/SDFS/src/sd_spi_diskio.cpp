#include "sd_spi_diskio.h"
#include "SDFSConfig.h"
#include "fatfs/ff.h"
#include "fatfs/diskio.h"


// SPI SD card interface globals
static uint8_t spi_cs_pin = 0;
static SPIClass *spi_port = nullptr;

static bool spi_initialized = false;
static bool is_sdhc_card = false;  // Track if card uses block addressing (SDHC) or byte addressing (SD)
static uint32_t card_sector_count = 0;  // Dynamic card capacity in sectors
static uint16_t actual_sector_size = SDFS_SECTOR_SIZE;  // Runtime detected sector size
// static bool spi_session_active = false;  // Track persistent SPI session

// SPI speed configuration (now configurable via SDFSConfig.h)
// #define SPI_SPEED_INIT_HZ       2000000   // 2MHz for initialization
// #define SPI_SPEED_OPERATION_HZ  4000000   // 4MHz for normal operation

// Current speed mode selection
static uint32_t current_spi_speed = SDFS_SPI_INIT_SPEED_HZ;
static bool spi_speed_fast_mode = false;

// SD card commands
#define CMD0    (0x40 + 0)     // GO_IDLE_STATE
#define CMD1    (0x40 + 1)     // SEND_OP_COND (MMC)
#define CMD8    (0x40 + 8)     // SEND_IF_COND
#define CMD9    (0x40 + 9)     // SEND_CSD
#define CMD10   (0x40 + 10)    // SEND_CID
#define CMD12   (0x40 + 12)    // STOP_TRANSMISSION
#define CMD16   (0x40 + 16)    // SET_BLOCKLEN
#define CMD17   (0x40 + 17)    // READ_SINGLE_BLOCK
#define CMD18   (0x40 + 18)    // read_MULTIPLE_BLOCK
#define CMD23   (0x40 + 23)    // SET_BLOCK_COUNT (MMC)
#define CMD24   (0x40 + 24)    // WRITE_BLOCK
#define CMD25   (0x40 + 25)    // WRITE_MULTIPLE_BLOCK
#define CMD41   (0x40 + 41)    // SEND_OP_COND (SDC)
#define CMD55   (0x40 + 55)    // APP_CMD
#define CMD58   (0x40 + 58)    // READ_OCR

// SD card responses
#define R1_READY_STATE          0x00
#define R1_IDLE_STATE           0x01
#define R1_ILLEGAL_COMMAND      0x04

// SD card data tokens
#define TOKEN_SINGLE_MULTI_BLOCK_READ   0xFE
#define TOKEN_SINGLE_BLOCK_WRITE        0xFE
#define TOKEN_MULTI_BLOCK_WRITE         0xFC
#define TOKEN_STOP_MULTI_BLOCK_WRITE    0xFD

// Forward declarations (CubeMX-style HAL approach)
static uint8_t xchg_spi(uint8_t data);
static void rcvr_spi_multi(uint8_t *buffer, uint16_t length);
static void xmit_spi_multi(const uint8_t *buffer, uint16_t length);
static int wait_ready(uint32_t timeout_ms);
static void despiselect(void);
static int spiselect(void);
static uint8_t send_cmd(uint8_t cmd, uint32_t arg);
static bool sd_read_block(uint8_t *buffer, uint32_t block_num);
static bool sd_write_block(const uint8_t *buffer, uint32_t block_num);
static void set_spi_speed_slow(void);
static void set_spi_speed_fast(void);

// SPI communication functions
static uint8_t xchg_spi(uint8_t data)
{
    // Use Arduino SPI transfer like working implementations
    return spi_port->transfer(data);
}

static void rcvr_spi_multi(uint8_t *buffer, uint16_t length)
{
    // Pre-fill buffer with 0xFF for SD card protocol
    memset(buffer, 0xFF, length);
    spi_port->transfer(buffer, length);
}

static void xmit_spi_multi(const uint8_t *buffer, uint16_t length)
{
    // Bidirectional transfer with temporary RX buffer
    uint8_t rxBuf[4096];  // Max possible sector size
    if (length > 4096) return; // Safety check
    spi_port->transfer(const_cast<uint8_t*>(buffer), rxBuf, length);
}

// Speed control functions
static void set_spi_speed_slow(void)
{
    current_spi_speed = SDFS_SPI_INIT_SPEED_HZ;
    spi_speed_fast_mode = false;
}

static void set_spi_speed_fast(void)
{
    current_spi_speed = SDFS_SPI_MAX_SPEED_HZ;
    spi_speed_fast_mode = true;
}

// Wait for SD card ready
static int wait_ready(uint32_t timeout_ms)
{
    uint8_t data;
    uint32_t start_time = millis();
    
    do {
        data = xchg_spi(0xFF);
        if (data == 0xFF) return 1;  // Ready
        delay(1);
    } while ((millis() - start_time) < timeout_ms);
    
    return 0;  // Timeout
}

// Chip select management
static void despiselect(void)
{
    digitalWrite(spi_cs_pin, HIGH);  // Set CS# high
    xchg_spi(0xFF);  // Dummy clock
}

static int spiselect(void)
{
    digitalWrite(spi_cs_pin, LOW);   // Set CS# low
    xchg_spi(0xFF);  // Dummy clock
    if (wait_ready(SDFS_CMD_TIMEOUT_MS)) return 1;
    
    despiselect();
    return 0;  // Timeout
}

// SD command transmission
static uint8_t send_cmd(uint8_t cmd, uint32_t arg)
{
    uint8_t n, res;
    
    // Handle ACMD commands
    if (cmd & 0x80) {
        // ACMD - send CMD55 first
        cmd &= 0x7F;
        res = send_cmd(55, 0);  // CMD55
        if (res > 1) return res;
    }
    
    // Chip select handling - all commands need CS LOW except CMD12
    if (cmd != 12) {  // Not CMD12 (STOP_TRANSMISSION)
        despiselect();
        if (!spiselect()) return 0xFF;
    }
    
    // Send command packet (6 bytes)
    xchg_spi(0x40 | cmd);  // Start + command index
    xchg_spi((uint8_t)(arg >> 24));  // Argument[31..24]
    xchg_spi((uint8_t)(arg >> 16));  // Argument[23..16]
    xchg_spi((uint8_t)(arg >> 8));   // Argument[15..8]
    xchg_spi((uint8_t)arg);          // Argument[7..0]
    
    // CRC values
    n = 0x01;  // Dummy CRC + Stop
    if (cmd == 0) n = 0x95;  // CRC for CMD0
    if (cmd == 8) n = 0x87;  // CRC for CMD8
    xchg_spi(n);
    
    // Receive command response
    if (cmd == 12) xchg_spi(0xFF);  // Skip stuff byte for CMD12
    
    // Wait for a valid response in timeout of 10 attempts
    n = 10;
    do {
        res = xchg_spi(0xFF);
    } while ((res & 0x80) && --n);
    
    return res;  // Return with the response value
}


static bool sd_read_block(uint8_t *buffer, uint32_t block_num)
{
    uint8_t token;
    uint32_t start_time;


    if (!spi_initialized || !buffer) {
        return false;
    }
    
    // Arduino SPI transaction (following SdFat pattern)
    spi_port->beginTransaction(SPISettings(current_spi_speed, MSBFIRST, SPI_MODE0));
    
    // Convert block number to address based on card type
    uint32_t address;
    if (is_sdhc_card) {
        address = block_num;  // SDHC uses block addressing
    } else {
        address = block_num << 9;  // SD uses byte addressing (block * 512)
    }
    
    // Send READ_SINGLE_BLOCK command (CMD17)
    if (send_cmd(17, address) != 0) {
        spi_port->endTransaction();
        return false;
    }
    
    // Wait for data packet (following SdFat timing)
    start_time = millis();
    do {
        token = xchg_spi(0xFF);
        if (token == 0xFE) break;  // Data token received
        if (token != 0xFF) break;  // Unexpected token
    } while ((millis() - start_time) < SDFS_DATA_TIMEOUT_MS);
    
    if (token != 0xFE) {
        despiselect();
        spi_port->endTransaction();
        return false;  // Data token timeout or error
    }
    
    // Receive data block using SdFat pattern (bidirectional transfer)
    rcvr_spi_multi(buffer, actual_sector_size);
    
    // Discard CRC
    xchg_spi(0xFF);
    xchg_spi(0xFF);
    
    despiselect();
    spi_port->endTransaction();
    return true;
}

static bool sd_write_block(const uint8_t *buffer, uint32_t block_num)
{
    uint8_t token;
    
    if (!spi_initialized || !buffer) return false;
    
    // Arduino SPI transaction (following SdFat pattern)
    spi_port->beginTransaction(SPISettings(current_spi_speed, MSBFIRST, SPI_MODE0));
    
    // Convert block number to address based on card type
    uint32_t address;
    if (is_sdhc_card) {
        address = block_num;  // SDHC uses block addressing
    } else {
        address = block_num << 9;  // SD uses byte addressing (block * 512)
    }
    
    // Send WRITE_SINGLE_BLOCK command (CMD24)
    if (send_cmd(24, address) != 0) {
        spi_port->endTransaction();
        return false;
    }
    
    // Send data token
    xchg_spi(0xFE);
    
    // Send data block using SdFat pattern (bidirectional transfer)
    xmit_spi_multi(buffer, actual_sector_size);
    
    // Send dummy CRC
    xchg_spi(0xFF);
    xchg_spi(0xFF);
    
    // Wait for data response
    token = xchg_spi(0xFF) & 0x1F;
    if (token != 0x05) {
        despiselect();
        spi_port->endTransaction();
        return false;
    }
    
    // Wait for write completion
    if (!wait_ready(SDFS_DATA_TIMEOUT_MS)) {
        despiselect();
        spi_port->endTransaction();
        return false;
    }
    
    despiselect();
    spi_port->endTransaction();
    return true;
}

// Read CSD register to determine card capacity
static bool sd_read_csd(void)
{
    uint8_t csd[16];
    uint32_t start_time;
    uint8_t token;

    spi_port->beginTransaction(SPISettings(current_spi_speed, MSBFIRST, SPI_MODE0));

    if (send_cmd(9, 0) != 0) {  // CMD9: SEND_CSD
        spi_port->endTransaction();
        return false;
    }

    // Wait for data token
    start_time = millis();
    do {
        token = xchg_spi(0xFF);
        if (token == 0xFE) break;  // Data token received
    } while ((millis() - start_time) < SDFS_BUSY_TIMEOUT_MS);

    if (token != 0xFE) {
        spi_port->endTransaction();
        return false;
    }

    // Read CSD register (16 bytes)
    rcvr_spi_multi(csd, 16);

    // Discard CRC
    xchg_spi(0xFF);
    xchg_spi(0xFF);

    despiselect();
    spi_port->endTransaction();

    // Parse CSD to get capacity
    if (is_sdhc_card) {
        // CSD Version 2.0 (SDHC/SDXC)
        uint32_t c_size = ((uint32_t)(csd[7] & 0x3F) << 16) |
                          ((uint32_t)csd[8] << 8) |
                          csd[9];
        card_sector_count = (c_size + 1) * 1024;
    } else {
        // CSD Version 1.0 (SD)
        uint32_t c_size = ((uint32_t)(csd[6] & 0x03) << 10) |
                          ((uint32_t)csd[7] << 2) |
                          ((csd[8] & 0xC0) >> 6);
        uint8_t c_size_mult = ((csd[9] & 0x03) << 1) | ((csd[10] & 0x80) >> 7);
        uint8_t read_bl_len = csd[5] & 0x0F;

        card_sector_count = (c_size + 1) * (1 << (c_size_mult + 2)) * (1 << read_bl_len) / actual_sector_size;
    }


    return true;
}

// SD card initialization
bool sd_spi_initialize(uint8_t cs_pin, SPIClass *spi)
{
    uint8_t n, cmd, ty, ocr[4];
    uint32_t start_time;
    
    if (!spi) return false;
    
    spi_cs_pin = cs_pin;
    spi_port = spi;
    
    // Initialize CS pin
    pinMode(spi_cs_pin, OUTPUT);
    digitalWrite(spi_cs_pin, HIGH);
    
    // Initialize SPI
    spi_port->begin();
    
    // Start with conservative speed for initialization
    current_spi_speed = SDFS_SPI_INIT_SPEED_HZ;
    spi_speed_fast_mode = false;
    
    // Arduino SPI transaction for initialization
    spi_port->beginTransaction(SPISettings(current_spi_speed, MSBFIRST, SPI_MODE0));
    
    delay(10);  // Power up delay
    
    // Send 80 dummy clocks with CS high (SD specification)
    for (n = 10; n; n--) xchg_spi(0xFF);
    
    ty = 0;
    if (send_cmd(0, 0) == 1) {  // GO_IDLE_STATE
        start_time = millis();
        if (send_cmd(8, 0x1AA) == 1) {  // SEND_IF_COND
            // SDv2+ card
            for (n = 0; n < 4; n++) ocr[n] = xchg_spi(0xFF);  // Get R7 response
            if (ocr[2] == 0x01 && ocr[3] == 0xAA) {  // Voltage range check
                // Wait for initialization complete (ACMD41 with HCS bit)
                while ((millis() - start_time) < SDFS_INIT_TIMEOUT_MS && send_cmd(41 | 0x80, 1UL << 30) != 0);
                if ((millis() - start_time) < SDFS_INIT_TIMEOUT_MS && send_cmd(58, 0) == 0) {  // Check CCS bit
                    for (n = 0; n < 4; n++) ocr[n] = xchg_spi(0xFF);
                    ty = (ocr[0] & 0x40) ? 12 : 4;  // SDv2 (HC or SC)
                }
            }
        } else {  // SDv1 or MMCv3
            if (send_cmd(41 | 0x80, 0) <= 1) {
                ty = 2; cmd = 41 | 0x80;  // SDv1
            } else {
                ty = 1; cmd = 1;  // MMCv3
            }
            while ((millis() - start_time) < SDFS_INIT_TIMEOUT_MS && send_cmd(cmd, 0) != 0);
            if ((millis() - start_time) >= SDFS_INIT_TIMEOUT_MS || send_cmd(16, actual_sector_size) != 0) {  // Set block length
                ty = 0;
            }
        }
    }
    
    despiselect();
    spi_port->endTransaction();
    
    if (ty) {  // Initialization succeeded
        // Set card type flags
        is_sdhc_card = (ty & 8) ? true : false;  // SDHC/SDXC
        
        // Switch to faster speed for normal operation
        current_spi_speed = SDFS_SPI_MAX_SPEED_HZ;
        spi_speed_fast_mode = true;

        // Read card capacity from CSD register
        if (!sd_read_csd()) {
            card_sector_count = 0x10000; // Default fallback capacity if CSD read fails
        }

        spi_initialized = true;
        return true;
    }

    return false;  // Initialization failed
}

uint8_t sd_spi_get_cs_pin(void)
{
    return spi_cs_pin;
}

SPIClass* sd_spi_get_port(void)
{
    return spi_port;
}

// FatFs disk I/O functions
extern "C" {

DSTATUS disk_initialize(BYTE pdrv)
{
    if (pdrv != 0) return STA_NOINIT;
    return spi_initialized ? 0 : STA_NOINIT;
}

DSTATUS disk_status(BYTE pdrv)
{
    if (pdrv != 0) return STA_NOINIT;
    return spi_initialized ? 0 : STA_NOINIT;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
    if (pdrv != 0 || !spi_initialized) {
        return RES_PARERR;
    }


    for (UINT i = 0; i < count; i++) {
        if (!sd_read_block(buff + (i * actual_sector_size), sector + i)) {
            return RES_ERROR;
        }
    }

    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
    if (pdrv != 0 || !spi_initialized) return RES_PARERR;
    
    for (UINT i = 0; i < count; i++) {
        if (!sd_write_block(buff + (i * actual_sector_size), sector + i)) {
            return RES_ERROR;
        }
    }
    
    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
    if (pdrv != 0 || !spi_initialized) return RES_PARERR;
    
    switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;
            
        case GET_SECTOR_COUNT:
            // Use dynamically determined card capacity
            *(DWORD*)buff = card_sector_count;
            return RES_OK;
            
        case GET_SECTOR_SIZE:
            *(WORD*)buff = actual_sector_size;
            return RES_OK;
            
        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 1;
            return RES_OK;
            
        default:
            return RES_PARERR;
    }
}

DWORD get_fattime(void)
{
    // Return current time in FAT format
    // For now, return a fixed time (2024-01-01 12:00:00)
    return ((2024 - 1980) << 25) | (1 << 21) | (1 << 16) | (12 << 11) | (0 << 5) | (0 >> 1);
}

} // extern "C"

// SPI speed configuration functions
void sd_spi_set_speed(uint32_t speed_hz)
{
    current_spi_speed = speed_hz;
}

uint32_t sd_spi_get_speed(void)
{
    return current_spi_speed;
}

// Runtime sector size functions
uint16_t sd_spi_get_sector_size(void)
{
    return actual_sector_size;
}

bool sd_spi_set_sector_size(uint16_t size)
{
    // Validate sector size (must be power of 2, between 512 and 4096)
    if (size < 512 || size > 4096 || (size & (size - 1)) != 0) {
        return false;
    }
    actual_sector_size = size;
    return true;
}