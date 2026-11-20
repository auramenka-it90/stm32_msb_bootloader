#include <stdio.h>

#include "dspa.h"
#include "dspa_defs.h"
#include "dspa_sigdefs.h"

#include "terminal_signals.h"

static char *sDEV = "";
static char flash_desc[64];

SIGNALS_BEGIN(DSPA_SIGNALS_NAME)

#if BSP_MSB0_PSPM1_BOARD==0
	_STRING_R_  ("Mode switching board(MSB)-BOOTLOADER-", sDEV, NULL),
#else
	_STRING_R_  ("Primary signal processing module(PSPM)", sDEV, NULL),
#endif
		_U32_R_	  ("Test hardware(0-0k)", test_hardware_result, &sDEV),
		_FLOAT_R_ ("CPU temperature, \u00B0C", cpu_temperature, &sDEV),

		/* Display both VDDA supply and raw sensor voltages */
		_FLOAT_R_ ("VDDA (Real Vref) [V]", real_vref, &sDEV),
		_FLOAT_R_ ("Temp Sensor [V]", adc_voltage, &sDEV),

		_STRING_R_("FPGA SPI Flash", flash_desc, &sDEV),

SIGNALS_END(DSPA_SIGNALS_NAME)

int init_terminal_signals(void){
	snprintf(flash_desc, sizeof(flash_desc), "%s | %02X %02X %02X | %02X%02X%02X%02X%02X%02X%02X%02X",
	         W25Q128_GetName(&bsp_flash),
	         bsp_flash.manufacturer_id, bsp_flash.memory_type, bsp_flash.capacity_id,
	         bsp_flash.unique_id[0], bsp_flash.unique_id[1], bsp_flash.unique_id[2], bsp_flash.unique_id[3],
	         bsp_flash.unique_id[4], bsp_flash.unique_id[5], bsp_flash.unique_id[6], bsp_flash.unique_id[7]);

	return SIG_INIT(DSPA_SIGNALS_NAME);
}


/*
 * 	Reaction to changes in FPGA control signals (eAssist)
 */

void signal_change_handler(void *s) {

}
