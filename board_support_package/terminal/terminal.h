

#ifndef TERMINAL_H_
#define TERMINAL_H_

#include	"board_support_package.h"

#define		_TERMINAL_PERMISSION_WRAPPER_

#ifdef		_TERMINAL_PERMISSION_WRAPPER_
	#if BSP_MSB0_PSPM1_BOARD==0
		#define		_TERMINAL_WRAPPER_ADDRESS_		0x0C						// PUBV -10, BC -11, MSB -12, PSPM -13
	#else
		#define		_TERMINAL_WRAPPER_ADDRESS_		0x0D
	#endif
	#define		_TERMINAL_WRAPPER_ALT_ADDRESS_	_TERMINAL_WRAPPER_ADDRESS_
#else

#endif

#define		_TERMINAL_RX_BUFF_SIZE_				1024*2
#define		_TERMINAL_TX_BUFF_SIZE_				1024*8
#define		_TERMINAL_TASK_TICK_	        	1U		    //	mSec
#define   	_TERMINAL_RECEIVE_TIMEOUT_US_		20000U		//	uSec
#define		_TERMINAL_BOOTLOADER_TIMEOUT_		5000U
#define		_TERMINAL_FPFA_MAX_NUM_BLOCK_		6


//	memory identifiers
#define		_ID_FPGA_FLASH			0x00
#define		_ID_APPL_FLASH			0x01
#define		_ID_DEV_NUM				2

/*
 *		eAssist max = 1024 !!!!!
 */
#define	DSPA_CFG_WRITE_PAGE_SIZE		1024
#define	DSPA_CFG_READ_PAGE_SIZE			1024
/*
 *
 */

bool	terminal_init(void);
void	terminal_task(void);
void	terminal_telemetry_handler(void);



#endif /* TERMINAL_H_ */
