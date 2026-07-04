/**
 * ******************************************************************************
 * @file    w25q128.c
 * @brief   W25Q128 SPI Flash Driver Implementation with Corrected Timers.
 *          Supports conditional compilation of RTOS Mutex protection.
 *          All comments in English.
 * ******************************************************************************
 */

#include "w25q128.h"
#include "pin_mgmt.h"

/* Global Instance Instantiation (Externed in w25q128.h) */
W25Q128_Handle_t bsp_flash;

/* Static SPI Mutex pointer if Mutex compiles in */
#if (BSP_W25Q128_USE_MUTEX == 1)
static osMutexId_t static_spi_mutex = NULL;
#endif

/* Private functions prototypes */
static void W25Q128_CS_Select(W25Q128_Handle_t *flash);
static void W25Q128_CS_Deselect(W25Q128_Handle_t *flash);
static osStatus_t W25Q128_TransmitReceive(W25Q128_Handle_t *flash, uint8_t *tx_data, uint8_t *rx_data, uint16_t size);
static osStatus_t W25Q128_Transmit(W25Q128_Handle_t *flash, uint8_t *data, uint16_t size);

/* Mutex Locking Helpers - completely compiled out if BSP_W25Q128_USE_MUTEX = 0 */
#if (BSP_W25Q128_USE_MUTEX == 1)
static osStatus_t W25Q128_LockMutex(W25Q128_Handle_t *flash, uint32_t timeout) {
    if (flash == NULL || flash->spi_mutex == NULL) {
        return osErrorParameter;
    }
    return osMutexAcquire(flash->spi_mutex, timeout);
}

static osStatus_t W25Q128_UnlockMutex(W25Q128_Handle_t *flash) {
    if (flash == NULL || flash->spi_mutex == NULL) {
        return osErrorParameter;
    }
    return osMutexRelease(flash->spi_mutex);
}
#else
/* Optimized Macro-stubs with zero warning & zero runtime overhead */
#define W25Q128_LockMutex(flash, timeout)     (osOK)
#define W25Q128_UnlockMutex(flash)            ((void)0) /* Cast to void prevents "statement with no effect" warning */
#endif

/* Chip Select Operations (Using fast Pin Macros) */
static void W25Q128_CS_Select(W25Q128_Handle_t *flash) {
    if (flash->use_pin_mgmt) {
        PIN_Reset_F(&pin_spi_cso);  /* Direct port register write */
    } else {
        HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
    }
}

static void W25Q128_CS_Deselect(W25Q128_Handle_t *flash) {
    if (flash->use_pin_mgmt) {
        PIN_Set_F(&pin_spi_cso);    /* Direct port register write */
    } else {
        HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
    }
}

/* Transmit/Receive Operations (With conditional Mutex Protection) */
static osStatus_t W25Q128_TransmitReceive(W25Q128_Handle_t *flash, uint8_t *tx_data, uint8_t *rx_data, uint16_t size) {
    osStatus_t status = W25Q128_LockMutex(flash, osWaitForever);
    if (status != osOK) {
        return status;
    }

    W25Q128_CS_Select(flash);
    HAL_StatusTypeDef hal_status = HAL_SPI_TransmitReceive(flash->hspi, tx_data, rx_data, size, HAL_MAX_DELAY);
    W25Q128_CS_Deselect(flash);

    W25Q128_UnlockMutex(flash);

    return (hal_status == HAL_OK) ? osOK : osError;
}

static osStatus_t W25Q128_Transmit(W25Q128_Handle_t *flash, uint8_t *data, uint16_t size) {
    osStatus_t status = W25Q128_LockMutex(flash, osWaitForever);
    if (status != osOK) {
        return status;
    }

    W25Q128_CS_Select(flash);
    HAL_StatusTypeDef hal_status = HAL_SPI_Transmit(flash->hspi, data, size, HAL_MAX_DELAY);
    W25Q128_CS_Deselect(flash);

    W25Q128_UnlockMutex(flash);

    return (hal_status == HAL_OK) ? osOK : osError;
}

/* Read JEDEC ID */
osStatus_t W25Q128_ReadJEDECID(W25Q128_Handle_t *flash) {
    if (flash == NULL) {
        return osErrorParameter;
    }

    uint8_t tx_data[4] = {W25Q128_CMD_READ_JEDEC_ID, 0x00, 0x00, 0x00};
    uint8_t rx_data[4];

    osStatus_t status = W25Q128_TransmitReceive(flash, tx_data, rx_data, 4);
    if (status != osOK) {
        flash->jedec_valid = 0;
        return status;
    }

    flash->manufacturer_id = rx_data[1];
    flash->memory_type = rx_data[2];
    flash->capacity_id = rx_data[3];
    flash->jedec_valid = 1;

    return osOK;
}

osStatus_t W25Q128_GetJEDECID(W25Q128_Handle_t *flash, uint8_t *manufacturer, uint8_t *memory_type, uint8_t *capacity) {
    if (flash == NULL) {
        return osErrorParameter;
    }

    if (!flash->jedec_valid) {
        return osErrorResource;
    }

    if (manufacturer) *manufacturer = flash->manufacturer_id;
    if (memory_type) *memory_type = flash->memory_type;
    if (capacity) *capacity = flash->capacity_id;

    return osOK;
}

uint8_t W25Q128_IsJEDECValid(W25Q128_Handle_t *flash) {
    return (flash != NULL) ? flash->jedec_valid : 0;
}

/* Read Unique ID */
osStatus_t W25Q128_ReadUID(W25Q128_Handle_t *flash) {
    if (flash == NULL) {
        return osErrorParameter;
    }

    uint8_t tx_data[5] = {W25Q128_CMD_READ_UID, 0x00, 0x00, 0x00, 0x00};
    uint8_t rx_data[13];

    osStatus_t status = W25Q128_TransmitReceive(flash, tx_data, rx_data, 13);
    if (status != osOK) {
        flash->uid_valid = 0;
        return status;
    }

    memcpy(flash->unique_id, &rx_data[5], W25Q128_UID_SIZE);
    flash->uid_valid = 1;

    return osOK;
}

osStatus_t W25Q128_GetUID(W25Q128_Handle_t *flash, uint8_t *uid) {
    if (flash == NULL || uid == NULL) {
        return osErrorParameter;
    }

    if (!flash->uid_valid) {
        return osErrorResource;
    }

    memcpy(uid, flash->unique_id, W25Q128_UID_SIZE);
    return osOK;
}

const uint8_t* W25Q128_GetUIDPtr(W25Q128_Handle_t *flash) {
    return (flash != NULL) ? flash->unique_id : NULL;
}

uint8_t W25Q128_IsUIDValid(W25Q128_Handle_t *flash) {
    return (flash != NULL) ? flash->uid_valid : 0;
}

/* Flash Initialization */
osStatus_t W25Q128_Init_Ex(W25Q128_Handle_t *flash, SPI_HandleTypeDef *hspi,
                         GPIO_TypeDef *cs_port, uint16_t cs_pin,
                         uint8_t use_pin_mgmt) {
    if (flash == NULL || hspi == NULL) {
        return osErrorParameter;
    }

    memset(flash, 0, sizeof(W25Q128_Handle_t));

    flash->hspi = hspi;
    flash->cs_port = cs_port;
    flash->cs_pin = cs_pin;
    flash->use_pin_mgmt = use_pin_mgmt;
    flash->capacity = W25Q128_FLASH_SIZE;

    flash->jedec_valid = 0;
    flash->uid_valid = 0;

    /* Acquire SPI bus pins via Pin Management safely using standard OS types */
    if (use_pin_mgmt) {
        osStatus_t pin_status = SPI_Bus_Acquire_For_STM32();
        if (pin_status != osOK) {
            return pin_status;
        }
    }

    /* Compile Mutex if requested */
#if (BSP_W25Q128_USE_MUTEX == 1)
    if (static_spi_mutex == NULL) {
        const osMutexAttr_t mutex_attr = W25Q128_MUTEX_ATTR;
        static_spi_mutex = osMutexNew(&mutex_attr);
    }

    if (static_spi_mutex == NULL) {
        if (use_pin_mgmt) {
          /*  SPI_Bus_Release_To_FPGA();	*/
        }
        return osErrorResource;
    }
    flash->spi_mutex = static_spi_mutex;
#endif

    osStatus_t status = W25Q128_ReadJEDECID(flash);
    if (status != osOK) {
        if (use_pin_mgmt) {
          /*  SPI_Bus_Release_To_FPGA(); */
        }
        return status;
    }

    if (flash->manufacturer_id != W25Q128_MANUFACTURER_ID ||
        flash->memory_type != W25Q128_MEMORY_TYPE ||
        flash->capacity_id != W25Q128_CAPACITY) {
        if (use_pin_mgmt) {
          /*  SPI_Bus_Release_To_FPGA(); */
        }
        flash->jedec_valid = 0;
        return osError;
    }

    flash->initialized = 1;
    return osOK;
}

osStatus_t W25Q128_Init(W25Q128_Handle_t *flash, SPI_HandleTypeDef *hspi,
                       GPIO_TypeDef *cs_port, uint16_t cs_pin) {
    return W25Q128_Init_Ex(flash, hspi, cs_port, cs_pin, 0);
}

osStatus_t W25Q128_Init_With_PinMgmt(W25Q128_Handle_t *flash, SPI_HandleTypeDef *hspi) {
    return W25Q128_Init_Ex(flash, hspi, 
                          SP1_FPGA_CSO_GPIO_Port, SP1_FPGA_CSO_Pin,
                          1);
}

osStatus_t W25Q128_DeInit(W25Q128_Handle_t *flash) {
    if (flash == NULL || !flash->initialized) {
        return osErrorParameter;
    }

    if (flash->use_pin_mgmt) {
      /*  SPI_Bus_Release_To_FPGA(); */
    }

#if (BSP_W25Q128_USE_MUTEX == 1)
    if (flash->spi_mutex != NULL) {
        osMutexDelete(flash->spi_mutex);
    }
#endif

    memset(flash, 0, sizeof(W25Q128_Handle_t));
    return osOK;
}

/* WaitForReady */
osStatus_t W25Q128_WaitForReady(W25Q128_Handle_t *flash, uint32_t timeout) {
    if (flash == NULL || !flash->initialized) {
        return osErrorParameter;
    }

    uint32_t start_tick = HAL_GetTick();
    uint8_t status;

    do {
        osStatus_t result = W25Q128_ReadStatusRegister(flash, 1, &status);
        if (result != osOK) {
            return result;
        }

        if (!(status & W25Q128_STATUS_BUSY)) {
            return osOK;
        }

        osDelay(1);

    } while ((HAL_GetTick() - start_tick) < timeout);

    return osErrorTimeout;
}

osStatus_t W25Q128_WriteEnable(W25Q128_Handle_t *flash) {
    if (flash == NULL || !flash->initialized) {
        return osErrorParameter;
    }

    uint8_t cmd = W25Q128_CMD_WRITE_ENABLE;
    return W25Q128_Transmit(flash, &cmd, 1);
}

osStatus_t W25Q128_WriteDisable(W25Q128_Handle_t *flash) {
    if (flash == NULL || !flash->initialized) {
        return osErrorParameter;
    }

    uint8_t cmd = W25Q128_CMD_WRITE_DISABLE;
    return W25Q128_Transmit(flash, &cmd, 1);
}

osStatus_t W25Q128_ReadStatusRegister(W25Q128_Handle_t *flash, uint8_t reg_num, uint8_t *status) {
    if (flash == NULL || !flash->initialized || status == NULL) {
        return osErrorParameter;
    }

    uint8_t cmd;
    switch (reg_num) {
        case 1: cmd = W25Q128_CMD_READ_STATUS1; break;
        case 2: cmd = W25Q128_CMD_READ_STATUS2; break;
        case 3: cmd = W25Q128_CMD_READ_STATUS3; break;
        default: return osErrorParameter;
    }

    uint8_t tx_data[2] = {cmd, 0x00};
    uint8_t rx_data[2];

    osStatus_t result = W25Q128_TransmitReceive(flash, tx_data, rx_data, 2);
    if (result == osOK) {
        *status = rx_data[1];
    }

    return result;
}

osStatus_t W25Q128_WriteStatusRegister(W25Q128_Handle_t *flash, uint8_t reg_num, uint8_t status) {
    return osError;
}

/* Read Data with Lock wrapping */
osStatus_t W25Q128_ReadData(W25Q128_Handle_t *flash, uint32_t address, uint8_t *data, uint32_t size) {
    if (flash == NULL || !flash->initialized || data == NULL || size == 0) {
        return osErrorParameter;
    }

    if (address + size > flash->capacity) {
        return osErrorParameter;
    }

    uint8_t tx_buffer[4] = {
        W25Q128_CMD_READ_DATA,
        (uint8_t)((address >> 16) & 0xFF),
        (uint8_t)((address >> 8) & 0xFF),
        (uint8_t)(address & 0xFF)
    };

    osStatus_t status = W25Q128_LockMutex(flash, osWaitForever);
    if (status != osOK) {
        return status;
    }

    W25Q128_CS_Select(flash);
    HAL_SPI_Transmit(flash->hspi, tx_buffer, 4, HAL_MAX_DELAY);
    HAL_StatusTypeDef hal_status = HAL_SPI_Receive(flash->hspi, data, size, HAL_MAX_DELAY);
    W25Q128_CS_Deselect(flash);

    W25Q128_UnlockMutex(flash);

    return (hal_status == HAL_OK) ? osOK : osError;
}

/* Fast Read with Lock wrapping */
osStatus_t W25Q128_FastRead(W25Q128_Handle_t *flash, uint32_t address, uint8_t *data, uint32_t size) {
    if (flash == NULL || !flash->initialized || data == NULL || size == 0) {
        return osErrorParameter;
    }

    if (address + size > flash->capacity) {
        return osErrorParameter;
    }

    uint8_t tx_buffer[5] = {
        W25Q128_CMD_FAST_READ,
        (uint8_t)((address >> 16) & 0xFF),
        (uint8_t)((address >> 8) & 0xFF),
        (uint8_t)(address & 0xFF),
        0x00
    };

    osStatus_t status = W25Q128_LockMutex(flash, osWaitForever);
    if (status != osOK) {
        return status;
    }

    W25Q128_CS_Select(flash);
    HAL_SPI_Transmit(flash->hspi, tx_buffer, 5, HAL_MAX_DELAY);
    HAL_StatusTypeDef hal_status = HAL_SPI_Receive(flash->hspi, data, size, HAL_MAX_DELAY);
    W25Q128_CS_Deselect(flash);

    W25Q128_UnlockMutex(flash);

    return (hal_status == HAL_OK) ? osOK : osError;
}

/* Page Program with Lock wrapping (Using correct Page Timeout) */
osStatus_t W25Q128_PageProgram(W25Q128_Handle_t *flash, uint32_t address, const uint8_t *data, uint32_t size) {
    if (flash == NULL || !flash->initialized || data == NULL || size == 0) {
        return osErrorParameter;
    }

    if (address + size > flash->capacity || size > W25Q128_PAGE_SIZE) {
        return osErrorParameter;
    }

    osStatus_t status = W25Q128_WaitForReady(flash, W25Q128_TIMEOUT_PAGE_MS);
    if (status != osOK) {
        return status;
    }

    status = W25Q128_WriteEnable(flash);
    if (status != osOK) {
        return status;
    }

    uint8_t tx_buffer[4] = {
        W25Q128_CMD_PAGE_PROGRAM,
        (uint8_t)((address >> 16) & 0xFF),
        (uint8_t)((address >> 8) & 0xFF),
        (uint8_t)(address & 0xFF)
    };

    status = W25Q128_LockMutex(flash, osWaitForever);
    if (status != osOK) {
        return status;
    }

    W25Q128_CS_Select(flash);
    HAL_SPI_Transmit(flash->hspi, tx_buffer, 4, HAL_MAX_DELAY);
    HAL_StatusTypeDef hal_status = HAL_SPI_Transmit(flash->hspi, (uint8_t*)data, size, HAL_MAX_DELAY);
    W25Q128_CS_Deselect(flash);

    W25Q128_UnlockMutex(flash);

    if (hal_status != HAL_OK) {
        return osError;
    }

    return W25Q128_WaitForReady(flash, W25Q128_TIMEOUT_PAGE_MS);
}

/* Sector Erase (Using correct Sector Timeout) */
osStatus_t W25Q128_SectorErase(W25Q128_Handle_t *flash, uint32_t address) {
    if (flash == NULL || !flash->initialized) {
        return osErrorParameter;
    }

    if (address >= flash->capacity) {
        return osErrorParameter;
    }

    address = address & ~(W25Q128_SECTOR_SIZE - 1);

    osStatus_t status = W25Q128_WaitForReady(flash, W25Q128_TIMEOUT_SECTOR_MS);
    if (status != osOK) {
        return status;
    }

    status = W25Q128_WriteEnable(flash);
    if (status != osOK) {
        return status;
    }

    uint8_t tx_buffer[4] = {
        W25Q128_CMD_SECTOR_ERASE,
        (uint8_t)((address >> 16) & 0xFF),
        (uint8_t)((address >> 8) & 0xFF),
        (uint8_t)(address & 0xFF)
    };

    status = W25Q128_Transmit(flash, tx_buffer, 4);
    if (status != osOK) {
        return status;
    }

    return W25Q128_WaitForReady(flash, W25Q128_TIMEOUT_SECTOR_MS);
}

/* Block Erase (Using correct Block Timeout) */
osStatus_t W25Q128_BlockErase(W25Q128_Handle_t *flash, uint32_t address) {
    if (flash == NULL || !flash->initialized) {
        return osErrorParameter;
    }

    if (address >= flash->capacity) {
        return osErrorParameter;
    }

    address = address & ~(W25Q128_BLOCK_SIZE - 1);

    osStatus_t status = W25Q128_WaitForReady(flash, W25Q128_TIMEOUT_BLOCK_MS);
    if (status != osOK) {
        return status;
    }

    status = W25Q128_WriteEnable(flash);
    if (status != osOK) {
        return status;
    }

    uint8_t tx_buffer[4] = {
        W25Q128_CMD_BLOCK_ERASE,
        (uint8_t)((address >> 16) & 0xFF),
        (uint8_t)((address >> 8) & 0xFF),
        (uint8_t)(address & 0xFF)
    };

    status = W25Q128_Transmit(flash, tx_buffer, 4);
    if (status != osOK) {
        return status;
    }

    return W25Q128_WaitForReady(flash, W25Q128_TIMEOUT_BLOCK_MS);
}

/* Chip Erase (Using correct Chip Timeout) */
osStatus_t W25Q128_ChipErase(W25Q128_Handle_t *flash) {
    if (flash == NULL || !flash->initialized) {
        return osErrorParameter;
    }

    osStatus_t status = W25Q128_WaitForReady(flash, W25Q128_TIMEOUT_BLOCK_MS); /* Pre-erase check */
    if (status != osOK) {
        return status;
    }

    status = W25Q128_WriteEnable(flash);
    if (status != osOK) {
        return status;
    }

    uint8_t cmd = W25Q128_CMD_CHIP_ERASE;
    status = W25Q128_Transmit(flash, &cmd, 1);
    if (status != osOK) {
        return status;
    }

    return W25Q128_WaitForReady(flash, W25Q128_TIMEOUT_CHIP_ERASE_S * 1000);
}

/* Advanced Operations */
osStatus_t W25Q128_ReadSector(W25Q128_Handle_t *flash, uint32_t sector, uint8_t *data) {
    if (flash == NULL || !flash->initialized || data == NULL) {
        return osErrorParameter;
    }

    uint32_t address = sector * W25Q128_SECTOR_SIZE;
    if (address >= flash->capacity) {
        return osErrorParameter;
    }

    return W25Q128_ReadData(flash, address, data, W25Q128_SECTOR_SIZE);
}

osStatus_t W25Q128_WriteSector(W25Q128_Handle_t *flash, uint32_t sector, const uint8_t *data) {
    if (flash == NULL || !flash->initialized || data == NULL) {
        return osErrorParameter;
    }

    uint32_t address = sector * W25Q128_SECTOR_SIZE;
    if (address >= flash->capacity) {
        return osErrorParameter;
    }

    osStatus_t status = W25Q128_SectorErase(flash, address);
    if (status != osOK) {
        return status;
    }

    for (uint32_t i = 0; i < W25Q128_SECTOR_SIZE; i += W25Q128_PAGE_SIZE) {
        status = W25Q128_PageProgram(flash, address + i, data + i,
                                   (W25Q128_SECTOR_SIZE - i) < W25Q128_PAGE_SIZE ?
                                   (W25Q128_SECTOR_SIZE - i) : W25Q128_PAGE_SIZE);
        if (status != osOK) {
            return status;
        }
    }

    return osOK;
}

osStatus_t W25Q128_EraseSector(W25Q128_Handle_t *flash, uint32_t sector) {
    if (flash == NULL || !flash->initialized) {
        return osErrorParameter;
    }

    uint32_t address = sector * W25Q128_SECTOR_SIZE;
    if (address >= flash->capacity) {
        return osErrorParameter;
    }

    return W25Q128_SectorErase(flash, address);
}

osStatus_t W25Q128_EraseBlock(W25Q128_Handle_t *flash, uint32_t block) {
    if (flash == NULL || !flash->initialized) {
        return osErrorParameter;
    }

    uint32_t address = block * W25Q128_BLOCK_SIZE;
    if (address >= flash->capacity) {
        return osErrorParameter;
    }

    return W25Q128_BlockErase(flash, address);
}
