#include "FlashConfig.h"
#include "stm32h7xx_hal.h"
#include "sys/system.h"
#include <array>
#include <cstring>
#include <cstdint>

#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
static bool isICacheEnabled()
{
    return (SCB->CCR & SCB_CCR_IC_Msk) != 0;
}
#else
#error "expected __ICACHE_PRESENT to be defined as 1 in FlashConfig.cpp"
#endif

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
static bool isDCacheEnabled()
{
    return (SCB->CCR & SCB_CCR_DC_Msk) != 0;
}
#else
#error "expected __DCACHE_PRESENT to be defined as 1 in FlashConfig.cpp"
#endif

static constexpr uint32_t limit(uint32_t value, uint32_t max)
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
            crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320 : (crc >> 1);
        }
    }
    return crc ^ 0xFFFFFFFF;
}

namespace daisy
{

FlashConfig::Result FlashConfig::SaveConfigData(const uint8_t* data, uint32_t length)
{
    if (!data || length == 0) return Result::ERR_DATA_INVALID_INPUT;

    if (length > FlashBlockDataSize) return Result::ERR_DATA_TOO_LARGE;

    if (DataMatchesCurrentDataBlock(data, length)) return Result::ERR_DATA_ALREADY_SAVED;

    std::array<uint8_t, FLASH_WRITE_SIZE> buffer{};
    buffer.fill(0xFF);

    uint32_t nextBlockIndex = GetNextUnusedBlockIndex();
    if (nextBlockIndex == NUM_BLOCKS)
    {
        if (EraseSector() != Result::OK) return Result::ERR_ERASE_FAILED;
        nextBlockIndex = 0;
    }

    uint32_t address = FLASH_SECTOR_ADDRESS + nextBlockIndex * sizeof(FlashBlock);

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR);

    bool icache_saved_status = isICacheEnabled();
    SCB_DisableICache();

    bool dcache_saved_status = isDCacheEnabled();
    SCB_DisableDCache();

    int32_t bytes_remaining = length;
    uint32_t offset_ptr = 0;

    FlashBlockHeader header{MAGIC_NUMBER, nextBlockIndex, length, CalculateCRC32(data, length)};
    std::memcpy(buffer.data(), &header, FlashBlockHeaderSize);

    uint32_t bytes_max = FLASH_WRITE_SIZE - FlashBlockHeaderSize;
    bool first_chunk = true;

    while (bytes_remaining > 0)
    {
        uint32_t bytes_to_write = limit(bytes_remaining, bytes_max);

        if (first_chunk)
        {
            std::memcpy(buffer.data() + FlashBlockHeaderSize, data, bytes_to_write);
            first_chunk = false;
            bytes_max = FLASH_WRITE_SIZE;
        }
        else
        {
            std::memcpy(buffer.data(), data + offset_ptr, bytes_to_write);
        }

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, reinterpret_cast<uint32_t>(buffer.data())) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return Result::ERR_WRITE_FAILED;
        }

        // Move to the next chunk by incrementing the address and offset_ptr
        // and decrement bytes_remaining bytes by the written number of bytes
        address += FLASH_WRITE_SIZE;
        offset_ptr += bytes_to_write;
        bytes_remaining -= bytes_to_write;

        buffer.fill(0xFF);
    }

    // Lock the Flash to disable the flash control register access
    HAL_FLASH_Lock();

    // After programming, the caches can be restored to previous state
    if (icache_saved_status) SCB_EnableICache();
    if (dcache_saved_status) SCB_EnableDCache();

    return Result::OK;
}

FlashConfig::Result FlashConfig::GetCurrentConfigDataSize(uint32_t* size)
{
    FlashBlock* block = GetCurrentDataBlock();
    if (!block) return Result::ERR_NO_BLOCK_FOUND;

    // put the size of the data block into the size pointer
    *size = block->header.bytes;
    return Result::OK;
}

FlashConfig::Result FlashConfig::GetCurrentConfigIndex(uint32_t* index)
{
    FlashBlock* block = GetCurrentDataBlock();
    if (!block) return Result::ERR_NO_BLOCK_FOUND;

    // put the index of the data block into the index pointer
    *index = block->header.index;
    return Result::OK;
}

FlashConfig::Result FlashConfig::ReadCurrentConfigData(uint8_t* data, uint32_t length)
{
    FlashBlock* block = GetCurrentDataBlock();
    if (!block) return Result::ERR_NO_BLOCK_FOUND;

    std::memcpy(data, block->data, length);
    return Result::OK;
}

FlashConfig::Result FlashConfig::VerifyCurrentConfigCRC(void)
{
    FlashBlock* block = GetCurrentDataBlock();
    if (!block) return Result::ERR_NO_BLOCK_FOUND;

    if (block->header.magic != MAGIC_NUMBER) return Result::ERR_INVALID_BLOCK_INFO;

    if (block->header.crc32 != CalculateCRC32(block->data, block->header.bytes))
    {
        return Result::ERR_DATA_CRC_MISMATCH;
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
    if (HAL_FLASHEx_Erase(&eraseInitStruct, &SectorError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return Result::ERR_ERASE_FAILED;
    }

    // Lock the Flash to disable the flash control register access
    HAL_FLASH_Lock();
    return Result::OK;
}

// Function to get a pointer to latest data block from flash
// Search from the last block to the first block
// Return the pointer if a valid block is found, otherwise NULL
FlashBlock* FlashConfig::GetCurrentDataBlock(void)
{
    for (int32_t i = NUM_BLOCKS - 1; i >= 0; i--)
    {
        auto block = reinterpret_cast<FlashBlock*>(FLASH_SECTOR_ADDRESS + i * sizeof(FlashBlock));
        if (block->header.magic == MAGIC_NUMBER)
        {
            return block;
        }
    }
    return nullptr;
}

// Function to determine if the data matches the data in the
// current block, return true if the data matches, otherwise false
bool FlashConfig::DataMatchesCurrentDataBlock(const uint8_t* data, uint32_t length)
{
    FlashBlock* block = GetCurrentDataBlock();
    if (!block) return false;

    // Check if the length of the data is same and the CRC32 matches
    if (block->header.bytes == length && block->header.crc32 == CalculateCRC32(data, length))
    {
        // Check if the data matches
        if (std::memcmp(data, block->data, length) == 0)
        {
            return true;
        }
    }

    return false;
}

// Function to get the next unused block index
// Return the index of the next unused block,
// or NUM_BLOCKS if no blocks are available
uint32_t FlashConfig::GetNextUnusedBlockIndex(void)
{
    for (uint32_t i = 0; i < NUM_BLOCKS; i++)
    {
        auto block = reinterpret_cast<FlashBlock*>(FLASH_SECTOR_ADDRESS + i * sizeof(FlashBlock));
        if (block->header.magic != MAGIC_NUMBER)
        {
            return i;
        }
    }
    return NUM_BLOCKS;
}

} // namespace daisy
