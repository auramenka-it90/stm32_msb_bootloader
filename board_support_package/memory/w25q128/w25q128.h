/**
 * ******************************************************************************
 * @file    w25q128.h
 * @brief   W25Q128 SPI Flash Driver Header with Datasheet-aligned Timers.
 *          All comments in English.
 * ******************************************************************************
 */

#ifndef MEMORY_W25Q128_W25Q128_H_
#define MEMORY_W25Q128_W25Q128_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "board_support_package.h" /* Central config containing BSP_W25Q128_USE_MUTEX */

/* Memory sizes */
#define W25Q128_FLASH_SIZE         0x1000000    /* 16 MB */
#define W25Q128_SECTOR_SIZE        0x1000       /* 4 KB */
#define W25Q128_BLOCK_SIZE         0x10000      /* 64 KB */
#define W25Q128_PAGE_SIZE          0x100        /* 256 B */

/* W25Q128 commands */
#define W25Q128_CMD_WRITE_ENABLE   0x06
#define W25Q128_CMD_WRITE_DISABLE  0x04
#define W25Q128_CMD_READ_DATA      0x03
#define W25Q128_CMD_FAST_READ      0x0B
#define W25Q128_CMD_PAGE_PROGRAM   0x02
#define W25Q128_CMD_SECTOR_ERASE   0x20
#define W25Q128_CMD_BLOCK_ERASE    0xD8
#define W25Q128_CMD_CHIP_ERASE     0xC7
#define W25Q128_CMD_READ_STATUS1   0x05
#define W25Q128_CMD_READ_STATUS2   0x35
#define W25Q128_CMD_READ_STATUS3   0x15
#define W25Q128_CMD_WRITE_STATUS1  0x01
#define W25Q128_CMD_WRITE_STATUS2  0x31
#define W25Q128_CMD_WRITE_STATUS3  0x11
#define W25Q128_CMD_READ_JEDEC_ID  0x9F
#define W25Q128_CMD_READ_UID       0x4B
#define W25Q128_CMD_POWER_DOWN     0xB9
#define W25Q128_CMD_RELEASE_PD     0xAB

/* Status registers */
#define W25Q128_STATUS_BUSY        0x01
#define W25Q128_STATUS_WEL         0x02

/* JEDEC ID */
#define W25Q128_MANUFACTURER_ID    0xEF
#define W25Q128_MEMORY_TYPE        0x40
#define W25Q128_CAPACITY           0x18

/* ========================================================================= */
/*  DATASHEET-BASED TIMEOUTS (With safety margin for aging silicon)          */
/* ========================================================================= */
#define W25Q128_TIMEOUT_PAGE_MS         50    /* Max page program is 3ms (safety: 50ms) */
#define W25Q128_TIMEOUT_SECTOR_MS       1000  /* Max 4KB sector erase is 400ms (safety: 1000ms) */
#define W25Q128_TIMEOUT_BLOCK_MS        3000  /* Max 64KB block erase is 2000ms (safety: 3000ms) */
#define W25Q128_TIMEOUT_CHIP_ERASE_S    200   /* Max chip erase is 200 seconds */

/* Mutex attributes */
#define W25Q128_MUTEX_ATTR { \
    .name = "W25Q128_Mutex", \
    .attr_bits = osMutexRecursive | osMutexPrioInherit, \
    .cb_mem = NULL, \
    .cb_size = 0 \
}

/* UID size */
#define W25Q128_UID_SIZE           8

/* Flash management handle structure */
typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    uint8_t use_pin_mgmt;     /* Flag to enable hardware pin management */

    uint8_t initialized;
    uint32_t capacity;

#if (BSP_W25Q128_USE_MUTEX == 1)
    osMutexId_t spi_mutex;    /* Compiled only if Mutexes are enabled */
#endif

    /* ID information stored in handle */
    uint8_t manufacturer_id;
    uint8_t memory_type;
    uint8_t capacity_id;
    uint8_t unique_id[W25Q128_UID_SIZE];
    uint8_t jedec_valid;
    uint8_t uid_valid;
} W25Q128_Handle_t;

/* Memory operation structure */
typedef struct {
    uint32_t address;
    uint8_t *data;
    uint32_t size;
    osStatus_t status;
} W25Q128_Operation_t;

/* ========================================================================= */
/*  GLOBAL CONFIGURATION HANDLES (Extern declaration)                       */
/* ========================================================================= */
extern W25Q128_Handle_t bsp_flash; /* Global Flash Handle */

/* Public functions */
osStatus_t W25Q128_Init(W25Q128_Handle_t *flash, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);
osStatus_t W25Q128_Init_Ex(W25Q128_Handle_t *flash, SPI_HandleTypeDef *hspi,
                          GPIO_TypeDef *cs_port, uint16_t cs_pin,
                          uint8_t use_pin_mgmt);
osStatus_t W25Q128_Init_With_PinMgmt(W25Q128_Handle_t *flash, SPI_HandleTypeDef *hspi);
osStatus_t W25Q128_DeInit(W25Q128_Handle_t *flash);

/* Basic operations */
osStatus_t W25Q128_ReadData(W25Q128_Handle_t *flash, uint32_t address, uint8_t *data, uint32_t size);
osStatus_t W25Q128_FastRead(W25Q128_Handle_t *flash, uint32_t address, uint8_t *data, uint32_t size);
osStatus_t W25Q128_PageProgram(W25Q128_Handle_t *flash, uint32_t address, const uint8_t *data, uint32_t size);
osStatus_t W25Q128_SectorErase(W25Q128_Handle_t *flash, uint32_t address);
osStatus_t W25Q128_BlockErase(W25Q128_Handle_t *flash, uint32_t address);
osStatus_t W25Q128_ChipErase(W25Q128_Handle_t *flash);

/* ID functions */
osStatus_t W25Q128_ReadJEDECID(W25Q128_Handle_t *flash);
osStatus_t W25Q128_GetJEDECID(W25Q128_Handle_t *flash, uint8_t *manufacturer, uint8_t *memory_type, uint8_t *capacity);
uint8_t W25Q128_IsJEDECValid(W25Q128_Handle_t *flash);

osStatus_t W25Q128_ReadUID(W25Q128_Handle_t *flash);
osStatus_t W25Q128_GetUID(W25Q128_Handle_t *flash, uint8_t *uid);
const uint8_t* W25Q128_GetUIDPtr(W25Q128_Handle_t *flash);
uint8_t W25Q128_IsUIDValid(W25Q128_Handle_t *flash);

/* Utility functions */
osStatus_t W25Q128_ReadStatusRegister(W25Q128_Handle_t *flash, uint8_t reg_num, uint8_t *status);
osStatus_t W25Q128_WriteStatusRegister(W25Q128_Handle_t *flash, uint8_t reg_num, uint8_t status);
osStatus_t W25Q128_WaitForReady(W25Q128_Handle_t *flash, uint32_t timeout);
osStatus_t W25Q128_WriteEnable(W25Q128_Handle_t *flash);
osStatus_t W25Q128_WriteDisable(W25Q128_Handle_t *flash);

/* Advanced functions */
osStatus_t W25Q128_ReadSector(W25Q128_Handle_t *flash, uint32_t sector, uint8_t *data);
osStatus_t W25Q128_WriteSector(W25Q128_Handle_t *flash, uint32_t sector, const uint8_t *data);
osStatus_t W25Q128_EraseSector(W25Q128_Handle_t *flash, uint32_t sector);
osStatus_t W25Q128_EraseBlock(W25Q128_Handle_t *flash, uint32_t block);

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_W25Q128_W25Q128_H_ */
