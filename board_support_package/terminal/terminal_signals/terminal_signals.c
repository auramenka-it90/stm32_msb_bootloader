
#include	"dspa.h"
#include	"dspa_defs.h"
#include 	"dspa_sigdefs.h"

#include	"terminal_signals.h"

static	char *sDEV = "";
static	char *sFLASH = "";

SIGNALS_BEGIN(DSPA_SIGNALS_NAME)
	_STRING_R_  ("Mode switching board", sDEV, NULL),
		_U32_R_	("Test hardware",	test_hardware_result,&sDEV),
		_STRING_R_("W25Q128JVS", sFLASH, &sDEV),
			_BYTE_R_("JDECID0", bsp_flash.manufacturer_id,&sFLASH),
			_BYTE_R_("JDECID1", bsp_flash.memory_type,&sFLASH),
			_BYTE_R_("JDECID2", bsp_flash.capacity_id,&sFLASH),
			_U64_R_("UID", 	bsp_flash.unique_id,&sFLASH),

SIGNALS_END(DSPA_SIGNALS_NAME)





int	init_terminal_signals(void){
	return	SIG_INIT(DSPA_SIGNALS_NAME);
}


/*
 * 	Reaction to changes in FPGA control signals (eAssist)
 */

void signal_change_handler(void *s) {


}
