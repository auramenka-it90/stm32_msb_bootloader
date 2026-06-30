#ifndef DSPA_DISPATCHER_H_
#define DSPA_DISPATCHER_H_

#include "dspa.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    // User services
void dspa_dispatcher_init          (const FuncPtr_Send_t pfSend, const FuncPtr_GetData_t pfGetData,
                                    const FuncPtr_InBufUsed_t pfInBufUsed, const FuncPtr_FlushInBuffer_t pfFlushInBuffer);
void dispatcher_set_dev_descriptor (Dev_descriptor_t *const dev_descr_arr_value);
void dspa_dispatcher               (const int elapsed_time_us); //elapsed_time_us - time from last call in us

void Send_ChkSum                   (du8 *const buf, const unsigned int num);
void Send_ACK                      (const dboolean ack);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DSPA_DISPATCHER_H_ */
