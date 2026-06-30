

#include	"std_com.h"


static	uint32_t	max_id=	0;
static	Uart_ctrl_t* uart_ref[_MAX_UART_ITEM]=	{NULL};


//Init dspa communication
void com_init(Uart_ctrl_t *par) {
	if (par == NULL) {
		return;
	}

	par->TxCpltCallback = NULL;
	par->ErrorUartCallback = NULL;

	par->rx_buf = malloc(par->rx_buf_size);
	par->tx_buf = malloc(par->tx_buf_size);

	par->rd_ptr = 0;

	par->id = max_id++;
	uart_ref[par->id] = par;

	//	!!!!
	par->wrapper=	NULL;

}

// Deinit	
void com_deinit(Uart_ctrl_t *par) {
	if (par != NULL) {
		free(par->rx_buf);
		free(par->tx_buf);

		uart_ref[par->id] = NULL;
		max_id--;
	}
}	

void com_set_tx_cplt(Uart_ctrl_t *par, FuncPtr_TxCpltCallback fptr) {
	if (par != NULL) {
		par->TxCpltCallback = fptr;

	}
}	


void com_set_error(Uart_ctrl_t *par, FuncPtr_UARTErrorCallback fptr) {
	if (par != NULL) {
		par->ErrorUartCallback = fptr;

	}
}


/*	get number of bytes received
//	!!!! __weak
*/
__weak	uint32_t com_inbuf_used(Uart_ctrl_t *par) {
	return get_num_rx(par);
}

//	capture from input buffer to user buffer
__weak	uint32_t com_inbuf_fetch(Uart_ctrl_t *par, uint8_t *dst_buf, uint32_t size) {

	uint32_t result = 0;

	uint32_t num = com_inbuf_used(par);

	size = (num >= size) ? size : num;

	for (int i = 0; i < size; ++i) {

		dst_buf[i] = par->rx_buf[par->rd_ptr++];
		par->rd_ptr &= (par->rx_buf_size - 1);
		result++;
	}

	return result;

}

//	send "size" bytes  with DMA
__weak	bool com_send(Uart_ctrl_t *par, uint8_t *data, uint32_t size) {
	par->flag_tx_cplt=	false;
	memcpy(par->tx_buf, data, size);
	return (HAL_UART_Transmit_DMA(par->huart, par->tx_buf, size) == HAL_OK);
}

//	send "size" bytes  without DMA & interrupt
bool __com_send(Uart_ctrl_t *par, uint8_t *data, uint16_t size,	uint32_t timeout) {
	memcpy(par->tx_buf, data, size);
	return (HAL_UART_Transmit(par->huart, par->tx_buf, size, timeout) == HAL_OK);
}

__weak	void com_flush_in_buffer(Uart_ctrl_t *par) {
#ifdef _G_SERIES_DMA_
	volatile uint32_t ndtr = par->dma_handle_rx->Instance->CNDTR;
#else
	volatile uint32_t ndtr = par->dma_handle_rx->Instance->NDTR;
#endif
	par->rd_ptr = (par->rx_buf_size - ndtr) & (par->rx_buf_size - 1);
}	


//	Start receive
void com_start_receive(Uart_ctrl_t *par) {
	HAL_UART_Receive_DMA(par->huart, par->rx_buf, par->rx_buf_size);
}

//	pop byte
uint8_t com_pop_byte(Uart_ctrl_t *par) {
	//return	par->rx_buf[ par->rd_ptr++ & (par->rx_buf_size - 1) ];

	uint8_t ret = par->rx_buf[par->rd_ptr++];

	par->rd_ptr &= (par->rx_buf_size - 1);

	return ret;

}

//	pop word
uint16_t com_pop_word(Uart_ctrl_t *par) {
	volatile uint16_t temp = com_pop_byte(par);
	temp |= com_pop_byte(par) << 8;
	return temp;

}



//	Static functionS 
uint32_t get_num_rx(Uart_ctrl_t *par) {

#ifdef _G_SERIES_DMA_
	volatile uint32_t ndtr = par->dma_handle_rx->Instance->CNDTR;
#else
	volatile uint32_t ndtr = par->dma_handle_rx->Instance->NDTR;
#endif

	uint32_t wr_ptr = (par->rx_buf_size - ndtr) & (par->rx_buf_size - 1);

	if (wr_ptr >= par->rd_ptr)
		return wr_ptr - par->rd_ptr;
	else
		return par->rx_buf_size - (par->rd_ptr - wr_ptr);
}

// ISR TX Callback
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {



    if (max_id) {
        for (uint32_t i = 0; i < max_id; i++) {
            if (huart == uart_ref[i]->huart) {

                uart_ref[i]->flag_tx_cplt = true;
                if (uart_ref[i]->TxCpltCallback != NULL) {
                    uart_ref[i]->TxCpltCallback(uart_ref[i]);
                }
                break;
            }
        }
    }
}

// ISR Error Callback
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {

    if (max_id) {
        for (uint32_t i = 0; i < max_id; i++) {
            if (huart == uart_ref[i]->huart) {

                if (uart_ref[i]->ErrorUartCallback != NULL) {
                    uart_ref[i]->ErrorUartCallback(uart_ref[i]);
                }
                break;
            }
        }
    }
}





