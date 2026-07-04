#ifndef DSPA_TELEMETRY_H_
#define DSPA_TELEMETRY_H_

#include "dspa_signals.h"
#include "dspa.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Command handlers
du16     TEL_ReadDSCR  (du8 *const buf);
dboolean TEL_SetMode   (const du8 mode_value, const du32 kdiv_value, const du16 frame_size_value);
dboolean TEL_AddSignal (const du16 sig_num);
du16     TEL_Getdata   (du8 *buf, du16 *const num_samples, du8 *const status);

// Services
void TEL_Init          (SigArrRec_t *const arr_sig_arr_value);
void TEL_Sample_Update (const int handle);
void TEL_Sample_Save   (void);
void TEL_Set_descriptor(TEL_Descriptor_t *const TEL_dscr_value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DSPA_TELEMETRY_H_ */
