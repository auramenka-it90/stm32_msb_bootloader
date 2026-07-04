/**
 * ******************************************************************************
 * @file    application.h
 * @brief   Internal Flash Memory Controller and Firmware Verification Driver.
 *          Targeted for STM32F411RET6TR with Application starting at Sector 5.
 *          All comments in English.
 * ******************************************************************************
 */

#ifndef MEMORY_APPLICATION_H_
#define MEMORY_APPLICATION_H_

#include "board_support_package.h"

/* Hardware CRC peripheral handle (defined in main.c / CubeMX) */
extern CRC_HandleTypeDef hcrc;

/* Application metadata header structure */
typedef struct {
    uint32_t magic;      /* Metadata magic identifier (must be 0x55AA55AA) */
    uint32_t version;    /* Firmware version */
    uint32_t fw_size;    /* Total size of binary image (header + payload) in bytes */
    uint32_t crc32;      /* Target CRC32 of the payload (excluding header) */
} fw_header_t;

/* ========================================================================= */
/*  INTERNAL FLASH ADDRESS & MEMORY MAP                                      */
/* ========================================================================= */

/* Address of Sector 5 (Start of user Application for STM32F411RE) */
#define APPLICATION_ADDRESS         0x08020000U

/* 128 kB (Size of one sector: Sectors 5 and 6 on STM32F411) */
#define FLASH_CPU_SECTOR_SIZE       (128U * 1024U)
#define FLASH_CPU_START_ADDRESS     APPLICATION_ADDRESS 
#define FLASH_CPU_ADR               FLASH_CPU_START_ADDRESS

/* --- APPLICATION ALLOCATION PARAMETERS --- */
/* Allocate 2 sectors (Sector 5 and Sector 6) = 256 KB of Flash memory */
#define FLASH_CPU_SECTORS_COUNT     2U

/* Block chunk exchange size for DSPA (1024 bytes) */
#define FLASH_CPU_PAGE_SIZE         1024U

/* Total allocated memory space (262144 bytes) */
#define FLASH_CPU_SIZE              (FLASH_CPU_SECTOR_SIZE * FLASH_CPU_SECTORS_COUNT) 

/* Calculated upper limit address: 0x08020000 + 256KB - 1 = 0x0805FFFF */
#define FLASH_CPU_ADR_HIGH          (FLASH_CPU_START_ADDRESS + FLASH_CPU_SIZE - 1U)

/* Hardware-specific flash status flag clear macro */
#define CLEAR_FLAG                  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP   | \
                                                           FLASH_FLAG_OPERR | \
                                                           FLASH_FLAG_WRPERR| \
                                                           FLASH_FLAG_PGAERR| \
                                                           FLASH_FLAG_PGPERR| \
                                                           FLASH_FLAG_PGSERR)

/* Erase driver boundaries (Sectors 5 and 6) */
#define APPL_SECTOR_START           FLASH_SECTOR_5
#define APPL_SECTOR_END             FLASH_SECTOR_6

/* ========================================================================= */
/*  PUBLIC FUNCTIONS                                                         */
/* ========================================================================= */

/**
 * @brief  Reads a block of data from internal Flash memory to RAM buffer.
 *         Executed at the maximum speed of the system bus.
 * @param  dst: Pointer to target RAM buffer
 * @param  src: Pointer to source address in internal Flash
 * @param  len: Size of block in bytes
 */
void eeprom_memory_read_buffer(uint8_t *dst, const uint8_t *src, uint32_t len);

/**
 * @brief  Erases the entire allocated application Flash area (Sectors 5 and 6).
 *         Warning: Erase process can block execution for up to 2-4 seconds.
 *         During erase, code execution from internal Flash is hardware-suspended.
 * @retval true on successful erase, false on error
 */
bool eeprom_memory_erase_app(void);

/**
 * @brief  Writes 32-bit words into internal Flash memory (Fast Programming Mode).
 *         Uses FLASH_TYPEPROGRAM_WORD which is 4 times faster than byte programming.
 *         IMPORTANT:
 *         1. Target address must be 32-bit aligned.
 *         2. The memory area must be erased prior to programming (contain 0xFFFFFFFF).
 * @param  p_data: Pointer to data buffer
 * @param  addr: Target address in internal Flash (must be 32-bit aligned)
 * @param  words_count: Number of 32-bit words to write
 * @retval true on success, false on write failure
 */
bool eeprom_memory_write_words(const uint32_t *p_data, uint32_t addr, uint32_t words_count);

/**
 * @brief  Validates the application binary using hardware CRC32.
 * @retval true if application is intact and ready to run, false if corrupted
 */
bool check_application(void);

#endif /* MEMORY_APPLICATION_H_ */
