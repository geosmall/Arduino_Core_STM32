#pragma once
#include "daisy_core.h"
#include "stm32yyxx.h"

#ifdef __cplusplus

namespace daisy
{

constexpr uint32_t FLASH_WRITE_SIZE = 32;   // Flash write size for STM32H7xx
constexpr uint32_t DATA_BLOCK_SIZE = 2048;  // 2K block size

// Static assert that DATA_BLOCK_SIZE is a multiple of FLASH_WRITE_SIZE
static_assert((DATA_BLOCK_SIZE % FLASH_WRITE_SIZE) == 0, "DATA_BLOCK_SIZE must be a multiple of FLASH_WRITE_SIZE");

struct FlashBlockHeader {
    uint32_t magic;
    uint32_t index;
    uint32_t bytes;
    uint32_t crc32;
};

constexpr uint32_t FlashBlockHeaderSize = sizeof(FlashBlockHeader);

struct FlashBlock {
    FlashBlockHeader header;
    uint8_t data[DATA_BLOCK_SIZE - FlashBlockHeaderSize]; // Adjust size to fit within 2K block
};

// Define the size of the data sectin in FlashBlock
constexpr uint32_t FlashBlockDataSize = sizeof(FlashBlock::data);

// Define the flash sector to use for storing configuration data
constexpr uint32_t FLASH_SECTOR_ADDRESS = 0x081E0000; // Address of Sector 7 in Bank 2
constexpr uint32_t FLASH_SECTOR_BANK    = FLASH_BANK_2;
constexpr uint32_t FLASH_SECTOR_NUM     = FLASH_SECTOR_7;

// Define the magic number to identify valid blocks and the number of bytes available for data
constexpr uint32_t MAGIC_NUMBER         = 0xDEADBEEF; // Magic number to identify valid blocks
constexpr uint32_t NUM_BLOCKS           = FLASH_SECTOR_SIZE / sizeof(FlashBlock); // FLASH_SECTOR_SIZE from stm32h7yyxx.h

constexpr uint32_t NumDataBytesHeaderChunk = FLASH_WRITE_SIZE - FlashBlockHeaderSize;

class FlashConfig
{
  public:
    enum class Result
    {
        OK, // No error
        ERR_NO_BLOCK_FOUND, // No block found
        ERR_INVALID_BLOCK_INFO, // Invalid block
        ERR_READ_FAILED, // Read failed
        ERR_WRITE_FAILED, // Write failed    
        ERR_ERASE_FAILED, // Erase failed
        ERR_DATA_TOO_LARGE, // Data too large to fit in block
        ERR_DATA_CRC_MISMATCH, // Data CRC does not match header
        ERR_DATA_INVALID_INPUT, // Invalid data input provided
        ERR_DATA_ALREADY_SAVED, // Data already saved to flash
        ERR // General error occurred
    };

    FlashConfig() {}
    ~FlashConfig() {}

    // Save the config data to flash
    // Returns OK if successful, ERR otherwise
    Result SaveConfigData(const uint8_t* data, uint32_t length);

    // Get the number of bytes stored in latest data block
    // Returns OK if successful, ERR otherwise (i.e. no data available)
    Result GetCurrentConfigDataSize(uint32_t* size);

    // Read the current config data from flash
    // Returns OK if successful, ERR otherwise (i.e. no data available)
    Result ReadCurrentConfigData(uint8_t* data, uint32_t length);

    // Verify CRC-32 of current config data in flash
    // Returns OK if successful, ERR otherwise (i.e. no data available)
    Result VerifyCurrentConfigCRC(void);

  private:
    // Erase the entire flash sector
    Result EraseSector(void);

    // Function to get a pointer to latest data block from flash
    // Search from the last block to the first block
    // Return the pointer if a valid block is found, otherwise NULL
    FlashBlock* GetCurrentDataBlock(void);

    // Function to determine if the data is new and does
    // not match the data in the current block
    bool DataMatchesCurrentDataBlock(const uint8_t* data, uint32_t length);

    // Find index to next available block
    // Returns NUM_BLOCKS if no blocks are available
    uint32_t GetNextUnusedBlockIndex(void);
};

} // namespace daisy

#endif /* __cplusplus */