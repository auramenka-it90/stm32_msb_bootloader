
#ifndef TERMINAL_SIGNALS_H_
#define TERMINAL_SIGNALS_H_

#include	"board_support_package.h"
#include 	"w25q128.h"

#define		DSPA_SIGNALS_NAME		dspa


//	test hardware
extern	uint32_t	test_hardware_result;

// reset source
extern	 bool is_soft_reset_detected;


//	ADC
extern	float adc_voltage;
extern	float	cpu_temperature;

// reset
extern 	bool is_soft_reset_detected;

int	init_terminal_signals(void);

#endif /* TERMINAL_SIGNALS_H_ */
