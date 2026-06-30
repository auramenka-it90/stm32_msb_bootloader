
#ifndef TERMINAL_WRAPPER_H_
#define TERMINAL_WRAPPER_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "std_com.h"

//typedef enum {WRP_BAD_START_BYTE, WRP_BAD_HEADER, WRP_NO_DATA, WRP_BAD_PAYLOAD, WRP_PACKET_OK} WRP_Rx_result_t;

typedef enum {	S_WAIT_START_BYTE,
				S_HEADER, S_PAYLOAD,
				S_WAIT_USER
} Uart_Wrapper_State_t;




typedef struct {
	Uart_Wrapper_State_t state;
	uint8_t addr;
	uint8_t addr_alt;
	uint8_t	addr_real;
	//uint32_t rd_ptr;
	uint32_t user_ptr;
	uint32_t payload_size;
	uint16_t length;
	bool padded;
	//uint32_t cnt_bad_header_checksum;
	//uint32_t cnt_bad_tail_checksum;
	uint32_t 	timeout; 				// in us
	uint32_t 	time_running;			// in us
} t_wrapper;


//
uint32_t 	com_inbuf_used(Uart_ctrl_t *uctrl);
bool 		com_send(Uart_ctrl_t *uctrl, uint8_t *data, uint32_t size);
void 	 	com_check_timeout(Uart_ctrl_t *uctrl, uint32_t elapsed_time);
void 		com_flush_in_buffer(Uart_ctrl_t *uctrl);
uint32_t 	com_inbuf_fetch(Uart_ctrl_t *uctrl, uint8_t *dst_buf, uint32_t size) ;

void		wrp_init(Uart_ctrl_t *uctrl, uint8_t addr, uint8_t addr_alt,uint32_t	timeout);
void		wrp_deinit(Uart_ctrl_t *uctrl);

// return amount of data to transmit
uint16_t 	wrp_make_tx_data( 	uint8_t  		*dst,
								const uint8_t   *payload,
								const uint16_t 	payload_size,
								const uint8_t   addr_dst,
								const uint8_t   addr_local );



void		wrp_set_addr_alt(Uart_ctrl_t *uctrl,	uint8_t s);
void		ten(bool par);


#endif /* TERMINAL_WRAPPER_H_ */
