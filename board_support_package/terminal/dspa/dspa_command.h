#ifndef DSPA_COMMAND_H_
#define DSPA_COMMAND_H_

#include "dspa_dispatcher.h"
#include "dspa_config.h"
#include "dspa.h"

typedef enum Disp_COM
{
    SYS_CHECKCHANNEL, SYS_RESET, SYS_REQUESTDEVICEINFO, SYS_SETSCANMODE, SYS_SELFTEST, SYS_SAVE_SETTINGS, SYS_TESTCHANNEL,
#if DSPA_PROT_VERSION >= 3
    SYS_REQUESTVERSION,
#endif
    PROG_REQUESTDEVICENUM, PROG_READDESCR, PROG_READID, PROG_READ, PROG_PREPAREFORPROG, PROG_WRITE, PROG_FINISHPROG, PROG_RESTART,
    SIG_REQUESTNUM, SIG_READDESCR, SIG_CONTROL, SIG_SETSIGNATURE, SIG_READVALUE,
    TEL_READDESCR, TEL_SETMODE, TEL_ADDSIGNAL, TEL_GETDATA
} Disp_COM_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

dboolean command_run                (const Disp_COM_t com, du8 *const data);
void     command_set_dev_descriptor (Dev_descriptor_t *const dev_descr_arr_value, const du16 size);
void     command_set_system_info    (const char *const system_info_value);
void     commnad_set_tx_buf         (du8 *const pbuf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DSPA_COMMAND_H_ */
