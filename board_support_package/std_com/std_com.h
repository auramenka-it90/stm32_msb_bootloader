
#ifndef	_STD_COM_H
#define	_STD_COM_H




#include	"board_support_package.h"



//#define		_G_SERIES_DMA_



typedef void 	(*FuncPtr_TxCpltCallback)(void* par);
typedef void 	(*FuncPtr_UARTErrorCallback)(void* par);

#define	_MAX_UART_ITEM		8

typedef struct Uart_ctrl_t {
	uint8_t *tx_buf;
	uint8_t *rx_buf;
	uint32_t tx_buf_size;
	uint32_t rx_buf_size;

	UART_HandleTypeDef *huart;
	DMA_HandleTypeDef *dma_handle_rx;
	DMA_HandleTypeDef *dma_handle_tx;

	uint32_t id;
	FuncPtr_TxCpltCallback TxCpltCallback;		//	Completed translate buffer
	FuncPtr_UARTErrorCallback ErrorUartCallback;
	bool flag_tx_cplt;

	uint32_t rd_ptr;

	//	pointer to wrapper
	void *wrapper;

} Uart_ctrl_t;


void com_init(Uart_ctrl_t *par);
void com_deinit(Uart_ctrl_t *par);
uint32_t get_num_rx(Uart_ctrl_t *par);
uint32_t com_inbuf_used(Uart_ctrl_t *par);
uint32_t com_inbuf_fetch(Uart_ctrl_t *par, uint8_t *dst_buf, uint32_t size);
bool com_send(Uart_ctrl_t *par, uint8_t *data, uint32_t size);
bool __com_send(Uart_ctrl_t *par, uint8_t *data, uint16_t size,		uint32_t timeout);
void com_flush_in_buffer(Uart_ctrl_t *par);
void com_start_receive(Uart_ctrl_t *par);
uint8_t com_pop_byte(Uart_ctrl_t *par);
uint16_t com_pop_word(Uart_ctrl_t *par);

void com_set_tx_cplt(Uart_ctrl_t *par, FuncPtr_TxCpltCallback fptr);
void com_set_error(Uart_ctrl_t *par, FuncPtr_UARTErrorCallback fptr);



#endif

