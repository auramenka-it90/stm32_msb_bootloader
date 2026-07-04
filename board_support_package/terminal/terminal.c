/**
 * ******************************************************************************
 * @file    terminal.c
 * @brief   DSPAssist Terminal Communication and Flash Programming Interface.
 *          Adapted to support both FPGA W25Q128 Flash and STM32F411 Internal Flash.
 *          All comments in English.
 * ******************************************************************************
 */

#include	"dspa.h"
#include	"dspa_defs.h"
#include 	"dspa_sigdefs.h"

#include	"board_support_package.h"
#include 	"terminal.h"
#include	"std_com.h"
#include	"wrapper.h"
#include 	"version_control.h"
#include 	"w25q128.h"
#include 	"pin_mgmt.h"
#include	"terminal_signals.h"
#include    "application.h"  /* Internal Flash Controller APIs */

/* Main terminal UART handle structure */
Uart_ctrl_t	uctrl;

/* DSPAssist engine configuration structures */
static	PROG_Config_t	PROG_Config;
static	SYS_Config_t  	SYS_Config;
static 	bool reset_request = false;

static	void led_blink(bool fl_stay_here);

static 	bool fl_stay_here = false;
static 	int	handle;

/* Terminal communication functions */
static int	Send(du8 *const pbuf, const unsigned int num);
static int	InBufUsed(void);
static int	GetData(du8 *const pbuf, const unsigned int num);
static void	FlushInBuffer(void);

/* Main device functions */
static dboolean	sys_SaveSettings(void);
static void	sys_Reset(void);
static char*	sys_SelfTest(void);

/* Main programming functions */
static du32		prg_ReadId(const du8 dev_num);
static dboolean prg_Read(const du8 dev_num, du8 *const pusData,	const du32 ulStartAddress);
static dboolean prg_Erase(const du8 dev_num);
static dboolean prg_Write(const du8 dev_num, du8 *const pusData, const du32 ulStartAddress);
static void		prg_Reset(const du8 dev_num);
static dboolean prg_Finish(const du8 dev_num);

static	char* uint32_to_string(uint32_t n, char* buffer, size_t buffer_size);

/* Memory Devices Descriptor Array - extended to support Internal CPU Flash */
Dev_descriptor_t dev_descr_arr[_ID_DEV_NUM] =
{
    [0] = {
        /* Device 0: FPGA W25Q128 External SPI Flash */
        "DD4-W25Q128JV/M_ID=",
        W25Q128_PAGE_SIZE,
        W25Q128_PAGE_SIZE,
        _TERMINAL_FPFA_MAX_NUM_BLOCK_ * W25Q128_BLOCK_SIZE,
        W25Q128_TIMEOUT_CHIP_ERASE_S,
        W25Q128_TIMEOUT_CHIP_ERASE_S,
        dfalse,
        0,
        0
    },
    [_ID_APPL_FLASH] = {
        /* Device 1: STM32F411RET6 Internal Application Flash */
        "DD1-STM32F411RET6/ID=",
        FLASH_CPU_PAGE_SIZE,     /* 1024 bytes write block size */
        FLASH_CPU_PAGE_SIZE,     /* 1024 bytes read block size */
        FLASH_CPU_SIZE,          /* Total Application space (256 KB) */
        2,                       /* Write block timeout in seconds */
        6,                       /* Sector erase timeout in seconds */
        dfalse,                  /* Auto-release bus (not applicable for internal flash) */
        FLASH_CPU_START_ADDRESS, /* Physical Start Address (0x08020000) */
        FLASH_CPU_START_ADDRESS
    }
};

/* Telemetry descriptor */
TEL_Descriptor_t TEL_dscr = {
    100/*us*/* (_TERMINAL_TASK_TICK_ * 1000),	/* Period (lsb = 0.01 us) */
    TEL_MAX_SIGNALS, 		/* Maximum number of signals permitted in telemetry */
    0,  					/* Frame size (not supported) */
    TEL_ATTR_SUPPORT_BUFFER	/* Attributes */
};

/**
 * @brief  Initializes the DSPAssist terminal engine and binds the memory drivers.
 */
bool terminal_init(void) {

	PROG_Config.descriptor = dev_descr_arr;
	PROG_Config.descriptor_size = _ID_DEV_NUM;

	PROG_Config.FuncPtr_ReadId = &prg_ReadId;
	PROG_Config.FuncPtr_Read = &prg_Read;
	PROG_Config.FuncPtr_Erase = &prg_Erase;
	PROG_Config.FuncPtr_Write = &prg_Write;
	PROG_Config.FuncPtr_Reset = &prg_Reset;
	PROG_Config.Func_Ptr_Finish = &prg_Finish;

	SYS_Config.FuncPtr_SaveSettings = &sys_SaveSettings;
	SYS_Config.FuncPtr_SelfTest = &sys_SelfTest;
	SYS_Config.FuncPtr_SysReset = &sys_Reset;

	SYS_Config.system_info = device_info_create();

	SYS_Init(&SYS_Config);
	PROG_Init(&PROG_Config);

	TEL_Set_descriptor(&TEL_dscr);
	dspa_dispatcher_init(Send, GetData, InBufUsed, FlushInBuffer);
	handle = init_terminal_signals();

	/* UART hardware initialization */
	if (!STLINK_Is_Connected()) {
		/* STM32 -> 221V34F26 external connector */
		uctrl.huart = &huart1;
		uctrl.dma_handle_rx = &hdma_usart1_rx;
		uctrl.dma_handle_tx = &hdma_usart1_tx;
	} else {
        /* STM32 -> ST-Link Virtual COM Port */
		uctrl.huart = &huart6;
		uctrl.dma_handle_rx = &hdma_usart6_rx;
		uctrl.dma_handle_tx = &hdma_usart6_tx;
	}

	uctrl.rx_buf_size = _TERMINAL_RX_BUFF_SIZE_;
	uctrl.tx_buf_size = _TERMINAL_TX_BUFF_SIZE_;
	com_init(&uctrl);

#ifdef	_TERMINAL_PERMISSION_WRAPPER_
	wrp_init(&uctrl, _TERMINAL_WRAPPER_ADDRESS_, _TERMINAL_WRAPPER_ALT_ADDRESS_, _TERMINAL_RECEIVE_TIMEOUT_US_);
#endif

	com_start_receive(&uctrl);
	return true;
}

void terminal_task(void) {
    /* Initialize the absolute tick counter for precise periodic execution */
    static uint32_t tick_periodic = 0;
    if (tick_periodic == 0) {
        tick_periodic = osKernelGetTickCount();
    }
    /* Increment absolute target tick count by the defined task period */
    tick_periodic += _TERMINAL_TASK_TICK_;

    /* 1. Execute background telemetry and dispatcher handlers */
    terminal_telemetry_handler();
    dspa_dispatcher(_TERMINAL_TASK_TICK_ * 1000);

#ifdef  _TERMINAL_PERMISSION_WRAPPER_
    com_check_timeout(&uctrl, _TERMINAL_TASK_TICK_ * 1000);
#endif

    /* 2. Process system reset requests with a small safety delay */
    if (reset_request) {
        osDelay(100);
        bsp_system_reset();
    }

    /* 3. Handle Bootloader Timeout and Conditional Jump */
    static uint32_t tick_start = 0;
    if (tick_start == 0) {
        tick_start = osKernelGetTickCount();
    }

    /* Calculate dynamic timeout based on reset cause and fast jump configuration */
    uint32_t boot_timeout = _TERMINAL_BOOTLOADER_TIMEOUT_;

#if (BSP_PERMISSION_FAST_JUMP == 1)
    /* If this is a cold boot (Hard Reset), bypass the delay and jump immediately */
    if (!is_soft_reset_detected) {
        boot_timeout = 0;
    }
#endif

    if (((osKernelGetTickCount() - tick_start) >= boot_timeout) && !fl_stay_here) {
#if (BSP_PERMISSION_JUMP_APPL_ == 1)
        fl_stay_here = true;
        bool can_jump = false;

#if (BSP_CHACK_APPL_CRC == 1)
        /* High-reliability boot: check firmware signature and validate hardware CRC32 */
        if (check_application() == true) {
            can_jump = true;
        }
#else
        /* Fast/debug boot: bypass CRC check but verify if MSP (Stack Pointer) is valid */
        /* This prevents CPU HardFault if jumping into empty/unprogrammed memory */
        uint32_t msp_val = *(__IO uint32_t*)APPLICATION_ADDRESS;
        if ((msp_val & 0xFFFE0000U) == 0x20000000U) {
            can_jump = true;
        }
#endif

        /* Execute jump to the main application if verification passed successfully */
        if (can_jump) {
            jump_main_application(APPLICATION_ADDRESS);
        }
#endif
    }

    /* 4. Update status LED blink state */
    led_blink(fl_stay_here);

    /* 5. Precise periodic delay: blocks the thread until absolute target tick is reached */
    osDelayUntil(tick_periodic);
}

/**
 * @brief  Updates and saves current telemetry samples.
 */
void terminal_telemetry_handler(void) {
	TEL_Sample_Update(handle);
	TEL_Sample_Save();
}

/* ========================================================================= */
/*  TERMINAL LOW LEVEL COMMUNICATION FUNCTIONS                               */
/* ========================================================================= */

static int Send(unsigned char *const buf, const unsigned int num) {
	com_send(&uctrl, buf, num);
	fl_stay_here = true;
	return num;
}

static int GetData(unsigned char *const buf, const unsigned int num) {
	unsigned int num_i = num;
	if (num_i == 0) {
		num_i = com_inbuf_used(&uctrl);
	}
	if (num_i == 0) {
		return 0;
	}
	com_inbuf_fetch(&uctrl, buf, num_i);
	return num_i;
}

static int InBufUsed(void) {
	return com_inbuf_used(&uctrl);
}

static void FlushInBuffer(void) {
	com_flush_in_buffer(&uctrl);
}

/* ========================================================================= */
/*  SYSTEM DIAGNOSTICS & MANAGEMENT API                                      */
/* ========================================================================= */

static dboolean sys_SaveSettings(void) {
	bool result = dtrue;
	return result;
}

static void sys_Reset(void) {
    reset_request = true;
}

static char* sys_SelfTest(void) {
	static char result[64];
	char str[16];
	uint32_to_string(test_hardware_result, str, sizeof(str));
	snprintf(result, sizeof(result), "Self-diagnosis: Device status(0=ok): %s", str);
	return result;
}

/* ========================================================================= */
/*  MAIN HARDWARE PROGRAMMING INTERFACE                                      */
/* ========================================================================= */

/**
 * @brief  Reads device identification code.
 */
static du32 prg_ReadId(const du8 dev_num) {
	uint32_t t = 0;

	switch (dev_num) {
        case _ID_FPGA_FLASH: {
            t = (uint32_t)bsp_flash.manufacturer_id;
            break;
        }
        case _ID_APPL_FLASH: {
            t = HAL_GetDEVID(); /* Read internal MCU device ID signature */
            break;
        }
        default: break;
	}

	return t;
}

/**
 * @brief  Reads a block of data from the specified device.
 */
static dboolean prg_Read(const du8 dev_num, du8 *const pusData, const du32 ulStartAddress) {
	dboolean result = dfalse;

	switch (dev_num) {
        case _ID_FPGA_FLASH: {
            result = (W25Q128_ReadData(&bsp_flash, ulStartAddress, pusData, W25Q128_PAGE_SIZE) == osOK);
            break;
        }
        case _ID_APPL_FLASH: {
            /* Read from internal Flash memory using fast memcpy wrapper */
            eeprom_memory_read_buffer(pusData, (const uint8_t*)ulStartAddress, dev_descr_arr[_ID_APPL_FLASH].cnt_rd_block);
            result = dtrue;
            break;
        }
        default: break;
	}
	return result;
}

/**
 * @brief  Erases the entire target memory space.
 */
static dboolean prg_Erase(const du8 dev_num) {
	dboolean result = dfalse;

	switch (dev_num) {
	case _ID_FPGA_FLASH: {
		for (uint32_t i = 0; i < _TERMINAL_FPFA_MAX_NUM_BLOCK_; i++) {
			if (W25Q128_BlockErase(&bsp_flash, i * W25Q128_BLOCK_SIZE)!= osOK) {
				return dfalse;
			}
		}
		result = dtrue;
		break;
	}
	case _ID_APPL_FLASH: {
		/* Erase internal Flash sectors allocated for Application (Sectors 5 and 6) */
		result = eeprom_memory_erase_app();
		break;
	}
	default:
		break;
	}
	return result;
}

/**
 * @brief  Writes a page/block of data to the target address.
 */
static dboolean prg_Write(const du8 dev_num, du8 *const pusData, const du32 ulStartAddress) {
	dboolean result = dfalse;

	switch (dev_num) {
        case _ID_FPGA_FLASH: {
            result = (W25Q128_PageProgram(&bsp_flash, ulStartAddress, pusData, W25Q128_PAGE_SIZE) == osOK);
            break;
        }
        case _ID_APPL_FLASH: {
            uint32_t bytes_to_write = dev_descr_arr[_ID_APPL_FLASH].cnt_wr_block;
            uint32_t words_to_write = bytes_to_write >> 2; /* Convert bytes into 32-bit words */
            result = eeprom_memory_write_words((uint32_t*)pusData, ulStartAddress, words_to_write);
            break;
        }
        default: break;
	}
	return result;
}

/**
 * @brief  Performs physical reset of the programmed component if supported.
 */
static void prg_Reset(const du8 dev_num) {
	switch (dev_num) {
        case _ID_FPGA_FLASH: {
        	/*
            SPI_Bus_Release_To_FPGA();
            FPGA_Reset_OS(10);
            */
            break;
        }
        case _ID_APPL_FLASH: {
            /* No specific hardware reset sequence needed for internal MCU Flash */
            break;
        }
        default: break;
	}
}

/**
 * @brief  Finalizes the programming transaction.
 */
static dboolean prg_Finish(const du8 dev_num) {
	dboolean result = dfalse;
	switch (dev_num) {
        case _ID_FPGA_FLASH: {
          /*
            SPI_Bus_Release_To_FPGA();
            FPGA_Reset_OS(10);
          */
            result = dtrue;
            break;
        }
        case _ID_APPL_FLASH: {
            result = dtrue;
            break;
        }
        default: break;
	}
	return result;
}

/* ========================================================================= */
/*  UTILITY FUNCTIONS                                                        */
/* ========================================================================= */

/**
 * @brief  Blinks the green status LED.
 */
static void led_blink(bool f) {
#define		_LED_PERIOD_	(1500 * _TERMINAL_TASK_TICK_)

	static uint32_t tick_old = 0;

	uint32_t now = osKernelGetTickCount();
	uint32_t delta = now - tick_old;
	uint32_t time_on = (f) ? (_LED_PERIOD_ / 2) : (_LED_PERIOD_ / 30);

	if (delta < time_on) {
		PIN_Set(&pin_led_green);
	} else {
		PIN_Reset(&pin_led_green);
	}

	if (delta >= _LED_PERIOD_)
		tick_old = now;
}

/**
 * @brief  Helper utility to convert uint32_t to string.
 */
static char* uint32_to_string(uint32_t n, char *buffer, size_t buffer_size) {
    if (buffer_size < 11) return NULL;

    if (n == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return buffer;
    }

    char temp[11];
    int i = 0;

    while (n > 0) {
        temp[i++] = '0' + (n % 10);
        n /= 10;
    }

    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';

    return buffer;
}
