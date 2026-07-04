/**
 * ******************************************************************************
 * @file    application.c
 * @brief   Internal Flash Memory Controller and Firmware Verification Driver.
 *          Implements fast word programming and hardware-accelerated CRC check.
 *          All comments in English.
 * ******************************************************************************
 */

#include "application.h"
#include <string.h>

/**
 * @brief  Reads a block of data from internal Flash memory to RAM buffer.
 *         Executed at the maximum speed of the system bus.
 */
void eeprom_memory_read_buffer(uint8_t *dst, const uint8_t *src, uint32_t len) {
    memcpy(dst, src, len);
}

/**
 * @brief  Erases the entire allocated application Flash area (Sectors 5 and 6).
 *         Warning: Erase process can block execution for up to 2-4 seconds.
 *         During erase, code execution from internal Flash is hardware-suspended.
 */
bool eeprom_memory_erase_app(void) {
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError = 0;
    bool result = true;

    /* Configure erase parameters for range of sectors */
    EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3; /* Requires 2.7V - 3.6V for fast erase */

    /* Pass starting sector and total count (Sector 5, 2 sectors total) */
    EraseInitStruct.Sector        = APPL_SECTOR_START;
    EraseInitStruct.NbSectors     = FLASH_CPU_SECTORS_COUNT;

    /* Unlock the Flash register access */
    if (HAL_FLASH_Unlock() != HAL_OK) {
        return false;
    }

    /* Clear sticky write-protection or programming error flags */
    CLEAR_FLAG;

    /* Execute sector erase operation */
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
        result = false;
    }

    /* Re-lock the Flash register access */
    HAL_FLASH_Lock();

    return result;
}

/**
 * @brief  Writes 32-bit words into internal Flash memory (Fast Programming Mode).
 *         Uses FLASH_TYPEPROGRAM_WORD which is 4 times faster than byte programming.
 */
bool eeprom_memory_write_words(const uint32_t *p_data, uint32_t addr, uint32_t words_count) {
    bool result = true;

    /* Guard: Address must be 32-bit word aligned */
    if (addr % 4 != 0) {
        return false;
    }

    if (HAL_FLASH_Unlock() != HAL_OK) {
        return false;
    }

    /* Clear previous error flags */
    CLEAR_FLAG;

    /* Program Flash memory word-by-word (4 bytes per write) */
    for (uint32_t i = 0; i < words_count; ++i) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, (uint64_t)p_data[i]) == HAL_OK) {
            addr += 4; /* Advance write pointer by 4 bytes */
        } else {
            result = false;
            break; /* Halt programming immediately upon first failure */
        }
    }

    HAL_FLASH_Lock();

    return result;
}

/**
 * @brief  Validates the application binary using hardware CRC32.
 */
bool check_application(void) {
    fw_header_t *fw_info = NULL;
    uint32_t *search_ptr = (uint32_t *)APPLICATION_ADDRESS;

    /* 1. Search for the magic word (scan the first 1024 bytes / 256 words) */
    for (int i = 0; i < 256; i++) {
        if (search_ptr[i] == 0x55AA55AAU) {
            fw_info = (fw_header_t *)&search_ptr[i];
            break;
        }
    }

    /* 2. Basic safety checks for missing or corrupted firmware */
    if (fw_info == NULL || fw_info->fw_size == 0xFFFFFFFFU || fw_info->fw_size == 0) {
        return false;
    }

    /* 3. Prepare data offsets for CRC32 calculation */
    uint32_t *payload_ptr = (uint32_t *)(fw_info + 1);
    uint32_t header_offset_bytes = (uint32_t)payload_ptr - APPLICATION_ADDRESS;

    /* Safety guard: if size is corrupt (smaller than the header itself), prevent HardFault */
    if (fw_info->fw_size <= header_offset_bytes) {
        return false;
    }

    uint32_t payload_bytes = fw_info->fw_size - header_offset_bytes;
    uint32_t payload_words = payload_bytes / 4; /* Convert byte size into 32-bit words */

    /* 4. Calculate hardware-accelerated CRC32 */
    uint32_t calc_crc = HAL_CRC_Calculate(&hcrc, payload_ptr, payload_words);

    /* 5. Return comparison result */
    return (calc_crc == fw_info->crc32);
}
