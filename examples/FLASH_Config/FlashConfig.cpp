#include "FlashConfig.h"
#include "stm32h7xx_hal.h"
#include <string.h>

// CRC-32 calculation function
static uint32_t CalculateCRC32(uint8_t* data, uint32_t length)
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
    // if (length > sizeof(((FlashBlock*) 0)->data))
    if (length > FlashBlockDataSize) return Result::ERR;

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

    // FlashBlock newBlock;
    // newBlock.magic = MAGIC_NUMBER;
    // newBlock.index = nextBlockIndex;
    // std::memcpy(newBlock.data, data, length);
    // newBlock.crc32 = CalculateCRC32(newBlock.data, sizeof(newBlock.data));

    // Unlock the Flash to enable the flash control register access
    HAL_FLASH_Unlock();

    // Clear any pending effor flags
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK2);

    uint32_t address = FLASH_SECTOR_ADDRESS + nextBlockIndex * sizeof(FlashBlock);
    uint32_t remaining = length;    // Count of remaining bytes to be written
    uint32_t offset = 0;            // Offset within the data buffer

    while (remaining > 0) {
        uint32_t chunkSize = (remaining >= 32) ? 32 : remaining;
        uint8_t buffer[32] = {0xFF}; // Initialize buffer with all 0xFF

        // void *memcpy(void* _Dst, const void* _Src, size_t _Size)
        memcpy(buffer, data + offset, chunkSize);

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, reinterpret_cast<uint32_t>(buffer)) != HAL_OK) {
            HAL_FLASH_Lock();
            return Result::ERR;
        }

        // Move to the next 32-byte chunk by incrementing the address and offset
        // and decrementing the remaining bytes by the chunk size
        address += 32;
        offset += chunkSize;
        remaining -= chunkSize;
    }

    // Lock the Flash to disable the flash control register access
    HAL_FLASH_Lock();

    return Result::OK;
}

FlashConfig::Result FlashConfig::GetCurrentConfigDataSize(uint32_t* size)
{
    FlashBlock* block = GetLatestDataBlock();
    if (block == NULL) return Result::ERR;

    // put the size of the data block into the size pointer
    *size = block->bytes;
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
    if (block->crc32 != CalculateCRC32(block->data, block->bytes))
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
        if (block->magic == MAGIC_NUMBER)
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
        if (block->magic != MAGIC_NUMBER)
        {
            return i;
        }
    }
    return NUM_BLOCKS; // No available block found
}

FlashConfig::Result FlashConfig::WriteNextDataBlock(uint8_t* Data, uint32_t Length)
{
    if (Length > sizeof(((FlashBlock*) 0)->data))
    {
        return Result::ERR; // Data too large
    }

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

    FlashBlock newBlock;
    newBlock.magic = MAGIC_NUMBER;
    newBlock.index = nextBlockIndex;
    memcpy(newBlock.data, Data, Length);
    newBlock.crc32 = CalculateCRC32(newBlock.data, sizeof(newBlock.data));

    uint32_t address = FLASH_SECTOR_ADDRESS + nextBlockIndex * sizeof(FlashBlock);

    // Unlock the Flash to enable the flash control register access
    HAL_FLASH_Unlock();

    HAL_StatusTypeDef status = HAL_OK;
    uint32_t* blockPtr = (uint32_t*) &newBlock;
    for (uint32_t i = 0; i < sizeof(FlashBlock) / 4; i += 8)
    {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address + i * 4, (uint64_t) blockPtr[i]);
        if (status != HAL_OK)
        {
            break;
        }
    }

    // Lock the Flash to disable the flash control register access
    HAL_FLASH_Lock();

    return (status == HAL_OK) ? Result::OK : Result::ERR;
}

} // namespace daisy
