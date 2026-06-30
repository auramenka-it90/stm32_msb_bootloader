#ifndef DSPA_PROGRAM_H_
#define DSPA_PROGRAM_H_

#include "dspa.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Services
void PROG_Init(PROG_Config_t *const PROG_Config_value);

// Command handlers
du32     PROG_ReadId         (const du8 dev_num);
du32     PROG_Read           (const du8 dev_num, du8 *const buf);
dboolean PROG_PrepareForProg (const du8 dev_num, const dboolean erase);
dboolean PROG_Write          (const du8 dev_num, du8 *const data);
dboolean PROG_Finish         (const du8 dev_num);
void     PROG_Reset          (const du8 dev_num);

du32     PROG_GetCntRdBlock  (const du8 dev_num);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DSPA_PROGRAM_H_ */
