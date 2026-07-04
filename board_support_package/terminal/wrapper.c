

#include	"board_support_package.h"
#include	"wrapper.h"



static uint16_t CalcCS(uint16_t* data, uint32_t len);
static	void	com_send_cplt(void *);
static	void	com_error_cplt(void * s);




void		wrp_init(Uart_ctrl_t *uctrl, uint8_t addr, uint8_t addr_alt, uint32_t	timeout){
	if	(uctrl!=NULL){

		uctrl->wrapper = malloc(sizeof(t_wrapper));
		t_wrapper* w=	(t_wrapper*)uctrl->wrapper;

		w->addr=		addr;
		w->addr_alt=	addr_alt;
		w->timeout=		timeout;

		w->state = S_WAIT_START_BYTE;
		w->time_running = 0;
		w->payload_size = 0;

		com_set_tx_cplt(uctrl,com_send_cplt);
		com_set_error(uctrl,com_error_cplt);


		ten(false);
	}
}

void		wrp_deinit(Uart_ctrl_t *uctrl){
	if(uctrl->wrapper!=NULL){
		free(uctrl->wrapper);
		uctrl->wrapper=	NULL;
	}

}
//----------------------------------------------------------------------------------------------------------
// return amount of data to transmit
uint16_t wrp_make_tx_data(uint8_t *dst, const uint8_t *payload,
		const uint16_t payload_size, const uint8_t addr_dst,
		const uint8_t addr_local) {
	uint16_t cnt = 0;
	bool padding = (payload_size & 1) != 0;
	uint16_t length = (padding) ? payload_size + 1 : payload_size;

	// create wrapper header
	dst[0] = 0x5A;							    //start byte
	dst[1] = 0;								    	//destinatyion address
	dst[2] = (length >> 1);				  //length lsb
	dst[3] = (length >> (8 + 1));			//length msb
	if (padding)
		dst[3] |= 0x80;
	uint16_t tmp = CalcCS((uint16_t*) dst, 2);	// header checksum
	dst[4] = (tmp >> 8);
	dst[5] = tmp;
	cnt += 6;

	// payload
	memcpy(&dst[6], payload, payload_size);
	cnt += payload_size;
	if (padding) 					// add null byte to 16-bit data allignment
	{
		dst[cnt] = 0;
		++cnt;
	}

	// payload checksum
	tmp = CalcCS((uint16_t*) dst, cnt >> 1);	// header checksum
	dst[cnt] = (tmp >> 8);
	dst[cnt + 1] = tmp;
	cnt += 2;

	return cnt;
}									


//----------------------------------------------------------------------------------------------------------
#define SET_INITIAL_STATE(par)   { com_flush_in_buffer(par); \
                              	  	  w->state = S_WAIT_START_BYTE; }

#define BYTE(X)             uctrl->rx_buf[ (uctrl->rd_ptr + X) & (uctrl->rx_buf_size - 1) ]
#define WORD16_LSB(X)       ( (BYTE(X+1)<<8 ) | BYTE(X) )
#define CALC_CS_FINISH(X)   uint32_t carry; \
                                    do { \
                                    carry = X >> 16; \
                                    X = ( X & 0xFFFF ) + carry; \
                                } while (carry != 0);


uint32_t com_inbuf_used(Uart_ctrl_t *uctrl) {

	if (uctrl->wrapper == NULL) {
		return get_num_rx(uctrl);

	} else {


		t_wrapper* w=	(t_wrapper*)(uctrl->wrapper);
		uint32_t num_rx = get_num_rx(uctrl);

		if ((w->payload_size == 0) && (num_rx != 0)) { // parsing input stream

			while (num_rx != 0) {

				if (w->state == S_WAIT_START_BYTE) {

					uint8_t start_byte = BYTE(0);
					if (start_byte == 0x5A) {
						w->state = S_HEADER;
					} else {
						SET_INITIAL_STATE(uctrl)
						;
						break;
					}

				} else if (w->state == S_HEADER) {

					if (num_rx >= 6) {

						uint32_t cs = 0;
						uint8_t dst_addr = BYTE(1);
						w->length = WORD16_LSB(2);
						uint16_t header_chksum = WORD16_LSB(4);

						cs += 0x5A | (dst_addr << 8);
						cs += w->length;
						cs += header_chksum;
						CALC_CS_FINISH(cs);

						if ((cs == 0xFFFF)	&& ((w->addr == dst_addr)|| (w->addr_alt == dst_addr))) {
							w->addr_real=	dst_addr;
							w->padded = (w->length & 0x8000) != 0;
							w->length &= ~0x8000;
							w->state = S_PAYLOAD;
						} else {
							SET_INITIAL_STATE(uctrl)
							;
							break;
						}

					}
					break;

				} else if (w->state == S_PAYLOAD) {

					if (num_rx >= (6 + 2 * w->length + 2)) {

						uint32_t cs = 0;
						uint32_t i;

						for (i = 0; i < 3 + w->length + 1; ++i) {
							cs += WORD16_LSB(i * 2);
						}

						CALC_CS_FINISH(cs);

						if (cs == 0xFFFF) {
							w->payload_size = 2 * w->length;
							if (w->padded) {
								w->payload_size -= 1;
							}

							w->user_ptr = (uctrl->rd_ptr + 6) & (uctrl->rx_buf_size - 1);
							w->state = S_WAIT_USER;

						} else {
							SET_INITIAL_STATE(uctrl)
						}

					}
					break;
				} else if (w->state == S_WAIT_USER) {

					break;
				}

			}
		}

		return w->payload_size;

	}

}

// elapsed_time in us
void com_check_timeout(Uart_ctrl_t *uctrl, uint32_t elapsed_time) {

	if (uctrl->wrapper != NULL) {

		t_wrapper *w = (t_wrapper*) (uctrl->wrapper);

		if (w->state != S_WAIT_START_BYTE) {
			w->time_running += elapsed_time;

			if (w->time_running > w->timeout) {
			//	com_start_receive(uctrl);
				SET_INITIAL_STATE(uctrl)

			}
		} else {
			w->time_running = 0;
		}
	}
}




bool 	com_send(Uart_ctrl_t *uctrl, uint8_t *data, uint32_t size) {

	uctrl->flag_tx_cplt=	false;

	if	(uctrl->wrapper==NULL){
		memcpy(uctrl->tx_buf, data, size);
		return (HAL_UART_Transmit_DMA(uctrl->huart, uctrl->tx_buf, size) == HAL_OK);
	}

	/*
	 *	insert translate buffer enable, if ...
	 */

	CLEAR_BIT(uctrl->huart->Instance->CR1, USART_CR1_RE);

	ten(true);

	t_wrapper* w=	(t_wrapper*)(uctrl->wrapper);

	uint32_t num = wrp_make_tx_data(uctrl->tx_buf, data, size, 0, w->addr_real);					//wrapper
	return (HAL_UART_Transmit_DMA(uctrl->huart, uctrl->tx_buf, num) == HAL_OK);
}




uint32_t com_inbuf_fetch(Uart_ctrl_t *uctrl, uint8_t *dst_buf, uint32_t size) {

	uint32_t result = 0;
	uint32_t num = com_inbuf_used(uctrl);
	size = (num >= size) ? size : num;

	if (uctrl->wrapper == NULL) {
		for (uint32_t i = 0; i < size; ++i) {

			dst_buf[i] = uctrl->rx_buf[uctrl->rd_ptr++];
			uctrl->rd_ptr &= (uctrl->rx_buf_size - 1);
			result++;
		}
	} else {
		t_wrapper* w=	(t_wrapper*)(uctrl->wrapper);
		for (uint32_t i = 0; i < size; ++i) {

			dst_buf[i] = uctrl->rx_buf[w->user_ptr];
			w->user_ptr++;
			w->payload_size--;
			result++;
			w->user_ptr &= (uctrl->rx_buf_size - 1);
		}

		if(w->payload_size == 0){
			SET_INITIAL_STATE(uctrl); // ready for new wrapper
		}

	}
	return result;
}



void com_flush_in_buffer(Uart_ctrl_t *uctrl) {

#ifdef _G_SERIES_DMA_
	volatile uint32_t ndtr = uctrl->dma_handle_rx->Instance->CNDTR;
#else
	volatile uint32_t ndtr = uctrl->dma_handle_rx->Instance->NDTR;
#endif
	uctrl->rd_ptr = (uctrl->rx_buf_size - ndtr) & (uctrl->rx_buf_size - 1);

	if (uctrl->wrapper != NULL) {
		t_wrapper* w=	(t_wrapper*)(uctrl->wrapper);
		w->payload_size=	0;
	}
}

void wrp_set_addr_alt(Uart_ctrl_t *uctrl, uint8_t s) {

	if (uctrl->wrapper != NULL) {
		t_wrapper *w = (t_wrapper*) (uctrl->wrapper);
		w->addr_alt = s;
	}
}
//----------------------------------------------------------------------------------------------------------
#define L_ENDIAN 1
static uint16_t CalcCS(uint16_t *data, uint32_t len) {
	uint32_t cs = 0;
	uint32_t i;
	uint16_t *dataptr = data;

	for (i = 0; i < len; i++) {
#if L_ENDIAN
		cs += ((*dataptr & 0xff) << 8) | ((*dataptr & 0xff00) >> 8);
#else
   		cs += *dataptr;
#endif
		dataptr++;
	}
	uint32_t carry;
	do {
		carry = cs >> 16;
		cs = (cs & 0xFFFF) + carry;
	} while (carry != 0);
	return ~cs;
}

static	void	com_send_cplt(void * s){

	Uart_ctrl_t* p=	(Uart_ctrl_t*)s;

	ten(false);

	SET_BIT(p->huart->Instance->CR1, USART_CR1_RE);

}


static	void	com_error_cplt(void * s){
	Uart_ctrl_t *uctrl=	s;
	t_wrapper* w=	(t_wrapper*)(uctrl->wrapper);

	com_start_receive(uctrl);
	SET_INITIAL_STATE(uctrl);
}

bool	fl_ten;
__weak	void	ten(bool par){
	fl_ten=	par;
}


