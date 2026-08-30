#ifndef TERMINAL_H_
#define TERMINAL_H_

#include "board_support_package.h"

#define _TERMINAL_PERMISSION_WRAPPER_

#ifdef _TERMINAL_PERMISSION_WRAPPER_
	#if BSP_MSB0_PSPM1_BOARD==0
		#define _TERMINAL_WRAPPER_ADDRESS_      0x0C // PUBV -10, BC -11, MSB -12, PSPM -13
	#else
		#define _TERMINAL_WRAPPER_ADDRESS_      0x0D
	#endif
	#define _TERMINAL_WRAPPER_ALT_ADDRESS_      _TERMINAL_WRAPPER_ADDRESS_
#endif

#define _TERMINAL_RX_BUFF_SIZE_                 (1024 * 2)
#define _TERMINAL_TX_BUFF_SIZE_                 (1024 * 8)
#define _TERMINAL_TASK_TICK_                    1U          // ms
#define _TERMINAL_RECEIVE_TIMEOUT_US_           20000U      // us
#define _TERMINAL_BOOTLOADER_TIMEOUT_           5000U       // ms
#define _TERMINAL_FPFA_MAX_NUM_BLOCK_           6           // 6 blocks * 64KB = 384KB per FPGA image

// Memory Device Identifiers
#define _ID_FPGA_FLASH_GOLD                     0x00U       // Device 0: Gold Image (Offset 0x00000000)
#define _ID_FPGA_FLASH_1                        0x01U       // Device 1: User Image 1 (Offset 0x00100000 = 1 MB)
#define _ID_APPL_FLASH                          0x02U       // Device 2: STM32 Internal Application Flash
#define _ID_DEV_NUM                             3U

// Flash Byte Start Addresses for Spartan-6 MultiBoot
#define _FPGA_FLASH_GOLD_ADR_                   0x00000000U // 0 MB Offset
#define _FPGA_FLASH_1_ADR_                      0x00100000U // 1 MB Offset (Matches multiboot_icap.v)

#define DSPA_CFG_WRITE_PAGE_SIZE                1024U
#define DSPA_CFG_READ_PAGE_SIZE                 1024U

bool terminal_init(void);
void terminal_task(void);
void terminal_telemetry_handler(void);

#endif /* TERMINAL_H_ */
