#include <string.h>
#include "dspa_command.h"
#include "dspa_system.h"
#include "dspa_utils.h"
#include "dspa_program.h"
#include "dspa_signals.h"
#include "dspa_telemetry.h"
#include "dspa_dispatcher.h"

#define ACK_OK  0x00

static du8 *tx_buf;

static dboolean Scan_Mode = dfalse;
static char *STR_MSG_SELFTEST_SCANMODE = "Impossible perform Selftest in scan mode";
static char *pStr;

static Dev_descriptor_t *dev_descr_arr;
static du16 num_devices = 0;

static const char *system_info;

//------------------------------------------------------------------------------------------
dboolean command_run(const Disp_COM_t com, du8 *const data)
{
    du32 len;
    int dev;
    int sig;
    Dev_descriptor_t *pDescr;
    int cnt;
    dboolean btmp;
    du8 tmp_u8;
    du16 tmp_u16;
    du32 tmp_u32;

    switch (com)
    {    //**********************************************************
        case SYS_CHECKCHANNEL:
            tx_buf[0] = ACK_OK;
            memcpy(&tx_buf[1], data, 8);
            Send_ChkSum(tx_buf, 9);
        break;
        //**********************************************************
        case SYS_RESET:
            if (Scan_Mode) Send_ACK(dtrue);
            else
            {
                Send_ACK(dtrue);
                SYS_Reset();
            }
        break;
        //**********************************************************
        case SYS_REQUESTDEVICEINFO:
            tx_buf[0] = ACK_OK;
            len = strlen(system_info);
            memcpy(&tx_buf[1], system_info, len + 1);
            Send_ChkSum(tx_buf, 1 + len + 1);
        break;
        //**********************************************************
        case SYS_SETSCANMODE:
            Scan_Mode = (data[0] == 0x01);
            Send_ACK(dtrue);
        break;
        //**********************************************************
        case SYS_SELFTEST:
            tx_buf[0] = ACK_OK;
            if (Scan_Mode) pStr = STR_MSG_SELFTEST_SCANMODE;
            else pStr = SYS_SelfTest();
            len = strlen(pStr);
            memcpy(&tx_buf[1], pStr, len + 1);
            Send_ChkSum(tx_buf, 1 + len + 1);
        break;
        //**********************************************************
        case SYS_SAVE_SETTINGS:
            if (Scan_Mode) Send_ACK(dtrue);
            else Send_ACK(SYS_SaveSettings());
        break;
        //**********************************************************
        case SYS_TESTCHANNEL:
            for (len = 0; data[len] != 0; len++);
            tx_buf[0] = ACK_OK;
            memcpy(&tx_buf[1], data, len + 1);
            Send_ChkSum(tx_buf, 1 + len + 1);
        break;
        //**********************************************************
#if DSPA_PROT_VERSION >= 3
        case SYS_REQUESTVERSION:
            cnt = 0;
            tx_buf[cnt++] = ACK_OK;
            tx_buf[cnt++] = DSPA_PROT_VERSION;
            Send_ChkSum(tx_buf, cnt);
        break;
#endif
        //**********************************************************
        case PROG_REQUESTDEVICENUM:
            tx_buf[0] = ACK_OK;
            tx_buf[1] = num_devices;
            Send_ChkSum(tx_buf, 2);
        break;
        //**********************************************************
        case PROG_READDESCR:
            dev = data[0];
            if (dev > (num_devices - 1)) Send_ACK(dfalse);
            else
            {
                pDescr = &dev_descr_arr[dev];
                tx_buf[0] = ACK_OK; cnt = 1;
                cnt += string_to_buf(pDescr->name, &tx_buf[cnt]);
                cnt += u32_to_buf(pDescr->cnt_wr_block, &tx_buf[cnt]);
                cnt += u32_to_buf(pDescr->cnt_rd_block, &tx_buf[cnt]);
                cnt += u32_to_buf(pDescr->max_size, &tx_buf[cnt]);
                cnt += u16_to_buf(pDescr->wr_timeout, &tx_buf[cnt]);
                cnt += u16_to_buf(pDescr->prep_timeout, &tx_buf[cnt]);
                if (pDescr->restart) tx_buf[cnt] = 0x01;
                else tx_buf[cnt] = 0x00;
                cnt++;
                Send_ChkSum(tx_buf, cnt);
            }
        break;
        //**********************************************************
        case PROG_READID:
            dev = data[0];
            if (dev > (num_devices - 1)) Send_ACK(dfalse);
            else
            {
                tx_buf[0] = ACK_OK; cnt = 1;
                cnt += u32_to_buf(PROG_ReadId(dev), &tx_buf[cnt]);
                Send_ChkSum(tx_buf, cnt);
            }
        break;
        //**********************************************************
        case PROG_READ:
            dev = data[0];
            if (dev > (num_devices - 1)) Send_ACK(dfalse);
            else
            {
                if (Scan_Mode)
                {
                    tx_buf[0] = ACK_OK; cnt = 1;
                    len = PROG_GetCntRdBlock(dev);
                    memset(&tx_buf[cnt], 0, len);
                    Send_ChkSum(tx_buf, cnt + len);
                }
                else
                {
                    len = PROG_Read(dev, &tx_buf[1]);
                    if (len != 0)
                    {
                        tx_buf[0] = ACK_OK;
                        Send_ChkSum(tx_buf, 1 + len);
                    }
                    else Send_ACK(dfalse);
                }

            }
        break;
        //**********************************************************
        case PROG_PREPAREFORPROG:
            if (Scan_Mode) Send_ACK(dtrue);
            else
            {
                dev = data[0];
                if (dev > (num_devices - 1)) Send_ACK(dfalse);
                else
                {
                    if (data[1] == 0x01) btmp = dtrue;//erase
                    else btmp = dfalse;
                    if (dev > (num_devices - 1)) Send_ACK(dfalse);
                    else Send_ACK(PROG_PrepareForProg(dev, btmp));
                }
            }
        break;
        //**********************************************************
        case PROG_WRITE:
            if (Scan_Mode) Send_ACK(dtrue);
            else
            {
                dev = data[0];
                if (dev > (num_devices-1)) Send_ACK(dfalse);
                else Send_ACK(PROG_Write(dev, &data[1]));
            }
        break;
        //**********************************************************
        case PROG_FINISHPROG:
            if (Scan_Mode) Send_ACK(dtrue);
            else
            {
                dev = data[0];
                if (dev > (num_devices - 1)) Send_ACK(dfalse);
                else Send_ACK(PROG_Finish(dev));
            }
        break;
        //**********************************************************
        case PROG_RESTART:
            if (Scan_Mode) Send_ACK(dtrue);
            else
            {
                dev = data[0];
                if (dev > (num_devices - 1)) Send_ACK(dfalse);
                else
                {
                    Send_ACK(dtrue);
                    PROG_Reset(dev);
                }
            }
        break;
        //**********************************************************
        case SIG_REQUESTNUM:
            tx_buf[0] = ACK_OK;
            u16_to_buf(SIG_Get_num_signals(), &tx_buf[1]);
            Send_ChkSum(tx_buf, 3);
        break;
        //**********************************************************
        case SIG_READDESCR:
            sig = data[0] | (data[1] << 8);
            if (sig > SIG_Get_num_signals()) Send_ACK(dfalse);
            else
            {
                tx_buf[0] = ACK_OK;
                len = SIG_ReadDSCR(sig, &tx_buf[1]);
                Send_ChkSum(tx_buf, 1 + len);
            }
        break;
        //**********************************************************
        case SIG_CONTROL:
            if (Scan_Mode) Send_ACK(dtrue);
            else
            {
                sig = u16_from_buf(&data[0]);
                Send_ACK(SIG_Control(sig, (SIG_Ctrl_type_t)data[2], &data[3]));
            }
        break;
        //**********************************************************
        case SIG_SETSIGNATURE:
            //not implemented
            Send_ACK(dfalse);
        break;
        //**********************************************************
        case SIG_READVALUE:
            sig = data[0] | (data[1] << 8);
            if (sig > SIG_Get_num_signals()) Send_ACK(dfalse);
            else
            {
                tx_buf[0] = ACK_OK;
                len = SIG_ReadValue(sig, &tx_buf[1]);
                Send_ChkSum(tx_buf, 1 + len);
            }
        break;
        //**********************************************************
        case TEL_READDESCR:
            tx_buf[0] = ACK_OK;
            len = TEL_ReadDSCR(&tx_buf[1]);
            Send_ChkSum(tx_buf, 1 + len);
        break;
        //**********************************************************
        case TEL_SETMODE:
            if (Scan_Mode) Send_ACK(dtrue);
            else
            {
                tmp_u8 = data[0];                 //mode
                tmp_u32 = u32_from_buf(&data[1]); //kdiv
                tmp_u16 = u16_from_buf(&data[5]); //frame_size
                Send_ACK(TEL_SetMode(tmp_u8, tmp_u32, tmp_u16));
            }
        break;
        //**********************************************************
        case TEL_ADDSIGNAL:
            if (Scan_Mode) Send_ACK(dtrue);
            else
            {
                sig = data[0] | (data[1] << 8);
                if (sig > SIG_Get_num_signals()) Send_ACK(dfalse);
                else Send_ACK(TEL_AddSignal(sig));
            }
        break;
        //**********************************************************
        case TEL_GETDATA:
            if (Scan_Mode)
            {
                tx_buf[0] = ACK_OK;
                tx_buf[1] = 0;
                tx_buf[2] = 0;
                tx_buf[3] = 0; //status
                Send_ChkSum(tx_buf, 1 + 3);
            }
            else
            {
                len = TEL_Getdata(&tx_buf[4], &tmp_u16, &tmp_u8);
                tx_buf[0] = ACK_OK;
                u16_to_buf(tmp_u16, &tx_buf[1]);
                tx_buf[3] = tmp_u8;
                Send_ChkSum(tx_buf, 1 + 2 + 1 + len);
            }
        break;

        default: Send_ACK(dfalse); break;
    }
    return dtrue;
}
//------------------------------------------------------------------------------------------
void command_set_dev_descriptor(Dev_descriptor_t *const dev_descr_arr_value, const du16 size)
{
    dev_descr_arr = dev_descr_arr_value;
    num_devices = size;
}
//--------------------------------------------------------------------------------------------
void command_set_system_info(const char *const system_info_value)
{
    system_info = system_info_value;
}
//--------------------------------------------------------------------------------------------
void commnad_set_tx_buf(du8 *const pbuf)
{
    tx_buf = pbuf;
}
