#ifndef DSPA_SYSTEM_H_
#define DSPA_SYSTEM_H_

#include "dspa.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void     SYS_Init        (SYS_Config_t *const SYS_Config_value);
void     SYS_Reset       (void);
char*    SYS_SelfTest    (void);
dboolean SYS_SaveSettings(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DSPA_SYSTEM_H_ */
