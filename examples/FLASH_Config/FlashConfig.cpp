#include "FlashConfig.h"
#include "sys/system.h"
#include "stm32h7xx_hal.h"
#include <string.h>

static FORCE_INLINE uint32_t limit(uint32_t value, uint32_t max)
{
    return (value > max) ? max : value;
}

// CRC-32 calculation function
static uint32_t CalculateCRC32(const uint8_t* data, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFF;
    for (uint32_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (uint32_t j = 0; j < 8; j++)
        {
            if (crc & 1)
            {
                crc = (crc >> 1) ^ 0xEDB88320;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFF;
}

namespace daisy
{

FlashConfig::Result FlashConfig::SaveConfigData(const uint8_t* data, uint32_t length)
{
    // Check input paramaters
    if (data == nullptr || length == 0) return Result::ERR;

    // Check if the data is too large to fit in block
    if (length > FlashBlockDataSize) return Result::ERR;

    // Define buffer with alignment (TODO: Verify need for source data buffer alignment)
    // alignas(FLASH_WRITE_SIZE) uint8_t buffer[FLASH_WRITE_SIZE] = {0};
    uint8_t buffer[FLASH_WRITE_SIZE] = {0};

    uint32_t nextBlockIndex = GetNextUnusedBlockIndex();
    if (nextBlockIndex == NUM_BLOCKS)
    {
        // No available block, erase the sector
        if (EraseSector() != Result::OK)
        {
            return Result::ERR;
        }
        nextBlockIndex = 0;
    }

    // Unlock the Flash to enable the flash control register access
    HAL_FLASH_Unlock();

    // Clear pending flags (if any)
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR |
                           FLASH_FLAG_WRPERR);

    SCB_DisableICache();
    SCB_DisableDCache();

    uint32_t address = FLASH_SECTOR_ADDRESS + nextBlockIndex * sizeof(FlashBlock);

    uint32_t bytes_remaining = length; // Count of bytes_remaining bytes to be written
    uint32_t offset_ptr = 0;           // Offset within 'data' buffer

    // Initialize buffer with all 0xFF
    memset(buffer, 0xFF, FLASH_WRITE_SIZE);

    // Populate the header fields in buffer
    FlashBlockHeader header;
    header.magic = MAGIC_NUMBER;
    header.index = nextBlockIndex;
    header.bytes = length;
    header.crc32 = CalculateCRC32(data, length);

    // Put header in buffer - memcpy(Destination, Source, Size)
    memcpy(buffer, &header, FlashBlockHeaderSize);

    // Max bytes to write on first chunk FLASH_WRITE_SIZE minus header size
    uint32_t bytes_max = FLASH_WRITE_SIZE - FlashBlockHeaderSize;

    bool first_chunk = true;

    while (bytes_remaining > 0)
    {
        uint32_t bytes_to_write = limit(bytes_remaining, bytes_max);

        if (first_chunk)
        {
            memcpy(buffer + FlashBlockHeaderSize, data, bytes_to_write);
            first_chunk = false;
            bytes_max = FLASH_WRITE_SIZE;
        }
        else
        {
            memcpy(buffer, data + offset_ptr, bytes_to_write);
        }

        // NOTE: writes FLASHWORDs which are FLASH_WRITE_SIZE bytes in size (defined in header file)
        // HAL_FLASH_Program(uint32_t TypeProgram, uint32_t FlashAddress, uint32_t DataAddress)
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, reinterpret_cast<uint32_t>(buffer)) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return Result::ERR;
        }

        // Move to the next chunk by incrementing the address and offset_ptr
        // and decrement bytes_remaining bytes by the written number of bytes
        address += FLASH_WRITE_SIZE;
        offset_ptr += bytes_to_write;
        bytes_remaining -= bytes_to_write;

        // Reset buffer with all 0xFF
        memset(buffer, 0xFF, FLASH_WRITE_SIZE);
    }

    // Lock the Flash to disable the flash control register access
    HAL_FLASH_Lock();

    // After programming, the caches can be restored to previous state
    if (System::UseDcache())
    {
        SCB_EnableICache();
    }
    SCB_EnableDCache();

    return Result::OK;
}

FlashConfig::Result FlashConfig::GetCurrentConfigDataSize(uint32_t* size)
{
    FlashBlock* block = GetLatestDataBlock();
    if (block == NULL) return Result::ERR;

    // put the size of the data block into the size pointer
    *size = block->header.bytes;
    return Result::OK;
}

FlashConfig::Result FlashConfig::ReadCurrentConfigData(uint8_t* data, uint32_t length)
{
    FlashBlock* block = GetLatestDataBlock();
    if (block == NULL) return Result::ERR;

    memcpy(data, block->data, length);
    return Result::OK;
}

FlashConfig::Result FlashConfig::VerifyBlockCRC32(FlashBlock* block)
{
    if (block->header.crc32 != CalculateCRC32(block->data, block->header.bytes))
    {
        return Result::ERR;
    }
    return Result::OK;
}

FlashConfig::Result FlashConfig::EraseSector(void)
{
    FLASH_EraseInitTypeDef eraseInitStruct;
    uint32_t SectorError;

    // Unlock the Flash to enable the flash control register access
    HAL_FLASH_Unlock();

    // Fill EraseInit structure
    eraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInitStruct.Banks = FLASH_SECTOR_BANK;
    eraseInitStruct.Sector = FLASH_SECTOR_NUM;
    eraseInitStruct.NbSectors = 1;
    eraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    // Erase the specified flash sector
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&eraseInitStruct, &SectorError);

    // Lock the Flash to disable the flash control register access
    HAL_FLASH_Lock();

    return (status == HAL_OK) ? Result::OK : Result::ERR;
}

FlashBlock* FlashConfig::GetLatestDataBlock(void)
{
    for (int32_t i = NUM_BLOCKS - 1; i >= 0; i--)
    {
        FlashBlock* block = (FlashBlock*) (FLASH_SECTOR_ADDRESS + i * sizeof(FlashBlock));
        if (block->header.magic == MAGIC_NUMBER)
        {
            return block;
        }
    }
    return NULL;
}

uint32_t FlashConfig::GetNextUnusedBlockIndex(void)
{
    for (uint32_t i = 0; i < NUM_BLOCKS; i++)
    {
        FlashBlock* block = (FlashBlock*) (FLASH_SECTOR_ADDRESS + i * sizeof(FlashBlock));
        if (block->header.magic != MAGIC_NUMBER)
        {
            return i;
        }
    }
    return NUM_BLOCKS; // No available block found
}

} // namespace daisy
