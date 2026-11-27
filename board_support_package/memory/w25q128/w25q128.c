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
static uint8_t W25Q128_IsSupportedID(W25Q128_Handle_t *flash);

/* Supported 16 MByte command-set compatible SPI flash parts {id, marketing name}.
 * Read/program/erase/status commands are identical on all listed parts.
 * CAUTION: Read Unique ID (0x4B) is NOT uniform: GD/Puya/ISSI need 8 dummy bytes
 * (driver sends 4 -> UID is garbage), Macronix has no UID command (reads 0xFF).
 * The UID is informational only - never use it for authentication or decisions.
 * Spansion S25FLxxx has no 0x20 4KB sector erase: it uses 0xD7
 * (handled in W25Q128_SectorErase by manufacturer id). */
typedef struct {
    uint8_t     id[3];
    const char *name;
} W25Q128_IdEntry_t;

static const W25Q128_IdEntry_t jedec_whitelist[] = {
    { {0xEF, 0x40, 0x18}, "W25Q128"    }, /* Winbond    */
    { {0xC8, 0x40, 0x18}, "GD25Q128E"  }, /* GigaDevice */
    { {0x85, 0x40, 0x18}, "PY25Q128"   }, /* Puya       */
    { {0x9D, 0x60, 0x18}, "IS25LP128"  }, /* ISSI       */
    { {0xC2, 0x20, 0x18}, "MX25L128xx" }, /* Macronix   */
    { {0x01, 0x60, 0x18}, "S25FL128"   }, /* Spansion/AMD (Cypress) */
};

/* Mutex Locking Helpers - completely compiled out if BSP_W25Q128_USE_MUTEX = 0 */
#if (BSP_W25Q128_USE_MUTEX == 1)
/**
 * @brief  Acquire the shared SPI bus mutex.
 * @param  flash  Flash handle carrying the mutex id.
 * @param  timeout Lock acquisition timeout, ms (osWaitForever allowed).
 * @return osOK on success, osErrorParameter on bad handle, timeout error otherwise.
 */
static osStatus_t W25Q128_LockMutex(W25Q128_Handle_t *flash, uint32_t timeout) {
    if (flash == NULL || flash->spi_mutex == NULL) {
        return osErrorParameter;
    }
    return osMutexAcquire(flash->spi_mutex, timeout);
}

/**
 * @brief  Release the shared SPI bus mutex.
 * @param  flash Flash handle carrying the mutex id.
 * @return osOK on success, osErrorParameter on bad handle.
 */
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

/**
 * @brief  Assert Chip Select (drive low) to start a SPI flash access.
 * @param  flash Flash handle with bound CS pin descriptor.
 * @note   Direct port register write, no bounds check: callers are internal.
 */
static void W25Q128_CS_Select(W25Q128_Handle_t *flash) {
    PIN_Reset_F(flash->cs);  /* Direct port register write */
}

/**
 * @brief  Deassert Chip Select (drive high) to end a SPI flash access.
 * @param  flash Flash handle with bound CS pin descriptor.
 */
static void W25Q128_CS_Deselect(W25Q128_Handle_t *flash) {
    PIN_Set_F(flash->cs);    /* Direct port register write */
}

/**
 * @brief  Mutex-protected full-duplex SPI transfer with CS framing.
 *         Sequence: lock -> CS low -> HAL exchange -> CS high -> unlock.
 * @param  flash   Flash handle (SPI + CS + mutex).
 * @param  tx_data Transmit buffer.
 * @param  rx_data Receive buffer.
 * @param  size    Transfer length, bytes.
 * @return osOK on success, mutex error or osError on SPI failure/timeout.
 */
static osStatus_t W25Q128_TransmitReceive(W25Q128_Handle_t *flash, uint8_t *tx_data, uint8_t *rx_data, uint16_t size) {
    osStatus_t status = W25Q128_LockMutex(flash, osWaitForever);
    if (status != osOK) {
        return status;
    }

    W25Q128_CS_Select(flash);
    HAL_StatusTypeDef hal_status = HAL_SPI_TransmitReceive(flash->hspi, tx_data, rx_data, size, W25Q128_TIMEOUT_SPI_MS);
    W25Q128_CS_Deselect(flash);

    W25Q128_UnlockMutex(flash);

    return (hal_status == HAL_OK) ? osOK : osError;
}

/**
 * @brief  Mutex-protected SPI write-only transfer with CS framing.
 *         Sequence: lock -> CS low -> HAL transmit -> CS high -> unlock.
 * @param  flash Flash handle (SPI + CS + mutex).
 * @param  data  Transmit buffer.
 * @param  size  Transfer length, bytes.
 * @return osOK on success, mutex error or osError on SPI failure/timeout.
 */
static osStatus_t W25Q128_Transmit(W25Q128_Handle_t *flash, uint8_t *data, uint16_t size) {
    osStatus_t status = W25Q128_LockMutex(flash, osWaitForever);
    if (status != osOK) {
        return status;
    }

    W25Q128_CS_Select(flash);
    HAL_StatusTypeDef hal_status = HAL_SPI_Transmit(flash->hspi, data, size, W25Q128_TIMEOUT_SPI_MS);
    W25Q128_CS_Deselect(flash);

    W25Q128_UnlockMutex(flash);

    return (hal_status == HAL_OK) ? osOK : osError;
}

/**
 * @brief  Read the 3-byte JEDEC identification (command 0x9F).
 * @param  flash Flash handle; result cached in manufacturer_id/memory_type/
 *               capacity_id, jedec_valid set on success / cleared on SPI error.
 * @return osOK on success, osErrorParameter on NULL handle, other on SPI error.
 */
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

/**
 * @brief  Return the previously cached JEDEC ID bytes (any output may be NULL).
 * @param  flash       Flash handle with valid cache.
 * @param  manufacturer Receives manufacturer id.
 * @param  memory_type  Receives memory type id.
 * @param  capacity     Receives capacity id.
 * @return osOK, osErrorParameter on NULL handle, osErrorResource if cache invalid.
 */
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

/**
 * @brief  Check whether the cached JEDEC ID is present and valid.
 * @param  flash Flash handle.
 * @return 1 if valid, 0 otherwise (also 0 for NULL handle).
 */
uint8_t W25Q128_IsJEDECValid(W25Q128_Handle_t *flash) {
    return (flash != NULL) ? flash->jedec_valid : 0;
}

/**
 * @brief  Get the human-readable part name resolved from the JEDEC whitelist.
 * @param  flash Flash handle.
 * @return Part name ("W25Q128", "GD25Q128E", ...), "unknown" if ID was read
 *         but not whitelisted, "none" if never resolved (or NULL handle).
 */
const char* W25Q128_GetName(W25Q128_Handle_t *flash) {
    if (flash == NULL || flash->name == NULL) {
        return "none";
    }
    return flash->name;
}

/**
 * @brief  Match the cached JEDEC ID against jedec_whitelist and store the
 *         matching marketing name in the handle.
 * @param  flash Flash handle with valid JEDEC cache.
 * @return 1 if the part is supported, 0 otherwise (name set to "unknown").
 */
static uint8_t W25Q128_IsSupportedID(W25Q128_Handle_t *flash) {
    for (uint32_t i = 0; i < sizeof(jedec_whitelist) / sizeof(jedec_whitelist[0]); i++) {
        if (flash->manufacturer_id == jedec_whitelist[i].id[0] &&
            flash->memory_type     == jedec_whitelist[i].id[1] &&
            flash->capacity_id     == jedec_whitelist[i].id[2]) {
            flash->name = jedec_whitelist[i].name;
            return 1;
        }
    }
    flash->name = "unknown";
    return 0;
}

/**
 * @brief  Read the 8-byte Unique ID (command 0x4B + 4 dummy bytes).
 * @param  flash Flash handle; result cached in unique_id, uid_valid updated.
 * @return osOK on success.
 * @note   See the whitelist comment: UID validity differs per vendor, so the
 *         value is informational only (terminal display), never for decisions.
 */
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

/**
 * @brief  Copy the cached Unique ID into a caller buffer (8 bytes).
 * @param  flash Flash handle with valid UID cache.
 * @param  uid   Destination buffer, at least W25Q128_UID_SIZE bytes.
 * @return osOK, osErrorParameter on NULL args, osErrorResource if cache invalid.
 */
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

/**
 * @brief  Get a read-only pointer to the cached Unique ID inside the handle.
 * @param  flash Flash handle.
 * @return Pointer to 8-byte UID storage, NULL for NULL handle. Validity is
 *         checked separately with W25Q128_IsUIDValid().
 */
const uint8_t* W25Q128_GetUIDPtr(W25Q128_Handle_t *flash) {
    return (flash != NULL) ? flash->unique_id : NULL;
}

/**
 * @brief  Check whether the cached Unique ID was read successfully.
 * @param  flash Flash handle.
 * @return 1 if valid, 0 otherwise (also 0 for NULL handle).
 */
uint8_t W25Q128_IsUIDValid(W25Q128_Handle_t *flash) {
    return (flash != NULL) ? flash->uid_valid : 0;
}

/**
 * @brief  Initialize the flash handle: bind SPI/CS, create the bus mutex,
 *         read the JEDEC ID and accept the part only if it is whitelisted.
 * @param  flash Flash handle to initialize (fully zeroed first).
 * @param  hspi  Initialized HAL SPI handle (CPOL/CPHA mode 0).
 * @param  cs    Chip select pin descriptor (driven by pin_management).
 * @return osOK on success; osErrorParameter on NULL args; osErrorResource if
 *         mutex creation failed; osError if the JEDEC ID is not supported
 *         (jedec_valid is then cleared); SPI error otherwise.
 */
osStatus_t W25Q128_Init(W25Q128_Handle_t *flash, SPI_HandleTypeDef *hspi,
                       const Pin_Descriptor_t *cs) {
    if (flash == NULL || hspi == NULL || cs == NULL) {
        return osErrorParameter;
    }

    memset(flash, 0, sizeof(W25Q128_Handle_t));

    flash->hspi = hspi;
    flash->cs = cs;
    flash->capacity = W25Q128_FLASH_SIZE;

    flash->jedec_valid = 0;
    flash->uid_valid = 0;

#if (BSP_W25Q128_USE_MUTEX == 1)
    if (static_spi_mutex == NULL) {
        const osMutexAttr_t mutex_attr = W25Q128_MUTEX_ATTR;
        static_spi_mutex = osMutexNew(&mutex_attr);
    }

    if (static_spi_mutex == NULL) {
        return osErrorResource;
    }
    flash->spi_mutex = static_spi_mutex;
#endif

    osStatus_t status = W25Q128_ReadJEDECID(flash);
    if (status != osOK) {
        return status;
    }

    if (!W25Q128_IsSupportedID(flash)) {
        flash->jedec_valid = 0;
        return osError;
    }

    flash->initialized = 1;
    return osOK;
}

/**
 * @brief  De-initialize the flash handle: delete the shared bus mutex and
 *         zero all handle state (make before jumping to the application).
 * @param  flash Flash handle (must be initialized).
 * @return osOK, or osErrorParameter if handle is NULL / not initialized.
 */
osStatus_t W25Q128_DeInit(W25Q128_Handle_t *flash) {
    if (flash == NULL || !flash->initialized) {
        return osErrorParameter;
    }

#if (BSP_W25Q128_USE_MUTEX == 1)
    if (static_spi_mutex != NULL) {
        osMutexDelete(static_spi_mutex);
        static_spi_mutex = NULL;
    }
    flash->spi_mutex = NULL;
#endif

    memset(flash, 0, sizeof(W25Q128_Handle_t));
    return osOK;
}

/**
 * @brief  Poll status register 1 until the BUSY bit clears, 1 ms per poll.
 * @param  flash   Flash handle (must be initialized).
 * @param  timeout Maximum wait, ms (pick from W25Q128_TIMEOUT_* per operation).
 * @return osOK when ready, osErrorParameter on bad handle, SPI error or
 *         osErrorTimeout if the device stays busy.
 */
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

/**
 * @brief  Send Write Enable (0x06): sets the WEL bit required by program/erase.
 * @param  flash Flash handle (must be initialized).
 * @return osOK, osErrorParameter on bad handle, or SPI error.
 */
osStatus_t W25Q128_WriteEnable(W25Q128_Handle_t *flash) {
    if (flash == NULL || !flash->initialized) {
        return osErrorParameter;
    }

    uint8_t cmd = W25Q128_CMD_WRITE_ENABLE;
    return W25Q128_Transmit(flash, &cmd, 1);
}

/**
 * @brief  Send Write Disable (0x04): clears the WEL bit.
 * @param  flash Flash handle (must be initialized).
 * @return osOK, osErrorParameter on bad handle, or SPI error.
 */
osStatus_t W25Q128_WriteDisable(W25Q128_Handle_t *flash) {
    if (flash == NULL || !flash->initialized) {
        return osErrorParameter;
    }

    uint8_t cmd = W25Q128_CMD_WRITE_DISABLE;
    return W25Q128_Transmit(flash, &cmd, 1);
}

/**
 * @brief  Read one status register (0x05/0x35/0x15).
 * @param  flash   Flash handle (must be initialized).
 * @param  reg_num Register number: 1, 2 or 3.
 * @param  status  Destination byte.
 * @return osOK, osErrorParameter on bad handle/register, or SPI error.
 */
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

/**
 * @brief  Status register write - intentionally NOT implemented.
 * @return always osError.
 * @note   Status bits 0 control block protection (BP/SRWD) and quad modes;
 *         the firmware never programs protection, and a wrong write can
 *         permanently lock the array, so the operation is kept unavailable.
 */
osStatus_t W25Q128_WriteStatusRegister(W25Q128_Handle_t *flash, uint8_t reg_num, uint8_t status) {
    return osError;
}

/**
 * @brief  Read an arbitrary data block (Read Data command 0x03).
 * @param  flash   Flash handle (must be initialized).
 * @param  address Start address, must satisfy address+size <= capacity.
 * @param  data    Destination buffer.
 * @param  size    Length, bytes (CS is held low for the whole transfer).
 * @return osOK, osErrorParameter on bad args, mutex error or osError on SPI fault.
 */
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
    HAL_StatusTypeDef hal_status = HAL_SPI_Transmit(flash->hspi, tx_buffer, 4, W25Q128_TIMEOUT_SPI_MS);
    if (hal_status == HAL_OK) {
        hal_status = HAL_SPI_Receive(flash->hspi, data, size, W25Q128_TIMEOUT_SPI_MS);
    }
    W25Q128_CS_Deselect(flash);

    W25Q128_UnlockMutex(flash);

    return (hal_status == HAL_OK) ? osOK : osError;
}

/**
 * @brief  Read with Fast Read command (0x0B) + 1 dummy byte.
 * @param  flash   Flash handle (must be initialized).
 * @param  address Start address, must satisfy address+size <= capacity.
 * @param  data    Destination buffer.
 * @param  size    Length, bytes.
 * @return osOK, osErrorParameter on bad args, mutex error or osError on SPI fault.
 * @note   Preferred at SPI clock above the plain-read limit of the part;
 *         otherwise identical to W25Q128_ReadData().
 */
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
    HAL_StatusTypeDef hal_status = HAL_SPI_Transmit(flash->hspi, tx_buffer, 5, W25Q128_TIMEOUT_SPI_MS);
    if (hal_status == HAL_OK) {
        hal_status = HAL_SPI_Receive(flash->hspi, data, size, W25Q128_TIMEOUT_SPI_MS);
    }
    W25Q128_CS_Deselect(flash);

    W25Q128_UnlockMutex(flash);

    return (hal_status == HAL_OK) ? osOK : osError;
}

/**
 * @brief  Program one flash page (Page Program 0x02), max 256 bytes,
 *         never crossing a page boundary.
 * @param  flash   Flash handle (must be initialized).
 * @param  address Start address within the page.
 * @param  data    Source buffer.
 * @param  size    Length, 1..256, with (address%256)+size <= 256.
 * @return osOK on success (device confirmed not-busy), parameter/mutex/SPI error
 *         or osErrorTimeout if the program operation did not finish in time.
 * @note   Only clears bits (1->0); the region must be pre-erased. Larger writes
 *         must be split by the caller (see W25Q128_WriteSector()).
 */
osStatus_t W25Q128_PageProgram(W25Q128_Handle_t *flash, uint32_t address, const uint8_t *data, uint32_t size) {
    if (flash == NULL || !flash->initialized || data == NULL || size == 0) {
        return osErrorParameter;
    }

    if (address + size > flash->capacity || size > W25Q128_PAGE_SIZE ||
        (address & 0xFFU) + size > W25Q128_PAGE_SIZE) {
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
    HAL_StatusTypeDef hal_status = HAL_SPI_Transmit(flash->hspi, tx_buffer, 4, W25Q128_TIMEOUT_SPI_MS);
    if (hal_status == HAL_OK) {
        hal_status = HAL_SPI_Transmit(flash->hspi, (uint8_t*)data, size, W25Q128_TIMEOUT_SPI_MS);
    }
    W25Q128_CS_Deselect(flash);

    W25Q128_UnlockMutex(flash);

    if (hal_status != HAL_OK) {
        return osError;
    }

    return W25Q128_WaitForReady(flash, W25Q128_TIMEOUT_PAGE_MS);
}

/**
 * @brief  Erase the 4 KB sector containing the address (command 0x20).
 * @param  flash   Flash handle (must be initialized).
 * @param  address Any address inside the sector (aligned down automatically).
 * @return osOK on success (sector reads 0xFF afterwards), parameter/mutex/SPI
 *         error or osErrorTimeout if the erase did not finish in time.
 */
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

    uint8_t erase_cmd = (flash->manufacturer_id == W25Q128_MF_SPANSION)
                        ? W25Q128_CMD_SECTOR_ERASE_SPANSION
                        : W25Q128_CMD_SECTOR_ERASE;

    uint8_t tx_buffer[4] = {
        erase_cmd,
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

/**
 * @brief  Erase the 64 KB block containing the address (command 0xD8).
 * @param  flash   Flash handle (must be initialized).
 * @param  address Any address inside the block (aligned down automatically).
 * @return osOK on success, parameter/mutex/SPI error or osErrorTimeout.
 */
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

/**
 * @brief  Erase the whole 16 MB array (command 0xC7). Destructive, long
 *         operation (up to ~200 s, the calling task blocks, 1 ms polling).
 * @param  flash Flash handle (must be initialized).
 * @return osOK on success, parameter/mutex/SPI error or osErrorTimeout.
 */
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

/**
 * @brief  Read one whole 4 KB sector by its index.
 * @param  flash  Flash handle (must be initialized).
 * @param  sector Sector index (address = sector * 4 KB).
 * @param  data   Destination buffer, at least W25Q128_SECTOR_SIZE bytes.
 * @return osOK, osErrorParameter on bad args/out of range, or W25Q128_ReadData()
 *         result.
 */
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

/**
 * @brief  Erase-then-program one 4 KB sector by its index (read-modify-write
 *         unit of this driver).
 * @param  flash  Flash handle (must be initialized).
 * @param  sector Sector index (address = sector * 4 KB).
 * @param  data   Source buffer, full W25Q128_SECTOR_SIZE bytes.
 * @return osOK on success; on error the sector is left erased/partially
 *         programmed - the caller must retry or mark the block bad.
 * @note   Previous sector content is destroyed by the erase even on later
 *         failure; pass a complete image of the sector.
 */
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

/**
 * @brief  Erase one 4 KB sector by its index.
 * @param  flash  Flash handle (must be initialized).
 * @param  sector Sector index (address = sector * 4 KB).
 * @return osOK, osErrorParameter on bad args/out of range, or
 *         W25Q128_SectorErase() result.
 */
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

/**
 * @brief  Erase one 64 KB block by its index.
 * @param  flash Flash handle (must be initialized).
 * @param  block Block index (address = block * 64 KB).
 * @return osOK, osErrorParameter on bad args/out of range, or
 *         W25Q128_BlockErase() result.
 */
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
