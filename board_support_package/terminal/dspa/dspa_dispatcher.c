#include <stdlib.h>
#include <string.h>
#include "dspa.h"
#include "dspa_command.h"
#include "dspa_telemetry.h"
#include "dspa_signals.h"
#include "dspa_utils.h"

//KPA Commands ID's
#define ID_SYS_CHECKCHANNEL      0x0101
#define ID_SYS_RESET             0x0102
#define ID_SYS_REQUESTDEVICEINFO 0x0103
#define ID_SYS_SETSCANMODE       0x0104
#define ID_SYS_SELFTEST          0x0105
#define ID_SYS_SAVE_SETTINGS     0x0106
#define ID_SYS_TESTCHANNEL       0x0107
#if DSPA_PROT_VERSION >= 3
#define ID_SYS_REQUESTVERSION 0x0108
#endif
#define ID_PROG_REQUESTDEVICENUM 0x0201
#define ID_PROG_READDESCR        0x0202
#define ID_PROG_READID           0x0203
#define ID_PROG_READ             0x0204
#define ID_PROG_PREPAREFORPROG   0x0205
#define ID_PROG_WRITE            0x0206
#define ID_PROG_FINISHPROG       0x0207
#define ID_PROG_RESTART          0x0208
#define ID_SIG_REQUESTNUM        0x0301
#define ID_SIG_READDESCR         0x0302
#define ID_SIG_CONTROL           0x0303
#define ID_SIG_SETSIGNATURE      0x0304
#define ID_SIG_READVALUE         0x0305
#define ID_TEL_READDESCR         0x0401
#define ID_TEL_SETMODE           0x0402
#define ID_TEL_ADDSIGNAL         0x0403
#define ID_TEL_GETDATA           0x0404

typedef enum DISP_STATE {DISP_NONE = 0, DISP_ID = 1, DISP_DATA = 2, DISP_CHECKSUM = 3} DISP_STATE_t;

static inline unsigned int get_command_id(void);
static inline dboolean VerifyChecksum(void);
static inline int get_num_data(const Disp_COM_t com);

static FuncPtr_Send_t Send;
static FuncPtr_GetData_t GetData;
static FuncPtr_InBufUsed_t InBufUsed;
static FuncPtr_FlushInBuffer_t FlushInBuffer;

static du8 rx_buf[DSPA_DISPATCHER_RX_BUF_SIZE];
static du8 tx_buf[DSPA_DISPATCHER_TX_BUF_SIZE];

static Dev_descriptor_t *dev_descr_arr;

static int rx_cnt;                     //num bytes in input stream
static DISP_STATE_t state = DISP_NONE; // dispacher state machine
static unsigned int id;                // command ID
static Disp_COM_t com;                 // command
static unsigned int data_size;         // size of expected data

static unsigned int cnt_rx_buf;        // counter of data in rx_buf

static du8 crc;                        //checksum

//------------------------------------------------------------------------------------------
// Init dispatcher
void dspa_dispatcher_init(const FuncPtr_Send_t pfSend, const FuncPtr_GetData_t pfGetData,
                          const FuncPtr_InBufUsed_t pfInBufUsed, const FuncPtr_FlushInBuffer_t pfFlushInBuffer)
{
    Send = pfSend;
    GetData = pfGetData;
    InBufUsed = pfInBufUsed;
    FlushInBuffer = pfFlushInBuffer;

    commnad_set_tx_buf(tx_buf);
}
//------------------------------------------------------------------------------------------
// Poll input data stream
void dspa_dispatcher(const int elapsed_time_us)
{
    unsigned int num;
    unsigned int i;
    du16 sig_num, sig_size;
    static int total_elapsed_time = 0; // in us

    if ((state == DISP_CHECKSUM) || (state == DISP_NONE)) total_elapsed_time = 0;
    else total_elapsed_time += elapsed_time_us;

    rx_cnt = InBufUsed();

    if (total_elapsed_time > DISPATCHER_TIMEOUT_MS*1000) //reset on timeout
    {
        state = DISP_NONE;
        FlushInBuffer();
        rx_cnt = 0;
        Send_ACK(dfalse);
    }

    switch (state)
    {
        case DISP_NONE:
            if (rx_cnt > 0) state = DISP_ID;
        break;

        case DISP_ID:
            if (rx_cnt > 2)
            {
                crc = 0;
                cnt_rx_buf = 0;
                id = get_command_id();
                state = DISP_DATA;
                switch (id)
                {
                    case ID_SYS_CHECKCHANNEL      : com = SYS_CHECKCHANNEL;                             break;
                    case ID_SYS_RESET             : com = SYS_RESET;             state = DISP_CHECKSUM; break;
                    case ID_SYS_REQUESTDEVICEINFO : com = SYS_REQUESTDEVICEINFO; state = DISP_CHECKSUM; break;
                    case ID_SYS_SETSCANMODE       : com = SYS_SETSCANMODE;                              break;
                    case ID_SYS_SELFTEST          : com = SYS_SELFTEST;          state = DISP_CHECKSUM; break;
                    case ID_SYS_SAVE_SETTINGS     : com = SYS_SAVE_SETTINGS;     state = DISP_CHECKSUM; break;
                    case ID_SYS_TESTCHANNEL       : com = SYS_TESTCHANNEL;                              break;
#if DSPA_PROT_VERSION >= 3
			        case ID_SYS_REQUESTVERSION    : com = SYS_REQUESTVERSION;    state = DISP_CHECKSUM; break;
#endif
                    case ID_PROG_REQUESTDEVICENUM : com = PROG_REQUESTDEVICENUM; state = DISP_CHECKSUM; break;
                    case ID_PROG_READDESCR        : com = PROG_READDESCR;                               break;
                    case ID_PROG_READID           : com = PROG_READID;                                  break;
                    case ID_PROG_READ             : com = PROG_READ;                                    break;
                    case ID_PROG_PREPAREFORPROG   : com = PROG_PREPAREFORPROG;                          break;
                    case ID_PROG_WRITE            : com = PROG_WRITE;                                   break;
                    case ID_PROG_FINISHPROG       : com = PROG_FINISHPROG;                              break;
                    case ID_PROG_RESTART          : com = PROG_RESTART;                                 break;
                    case ID_SIG_REQUESTNUM        : com = SIG_REQUESTNUM;        state = DISP_CHECKSUM; break;
                    case ID_SIG_READDESCR         : com = SIG_READDESCR;                                break;
                    case ID_SIG_CONTROL           : com = SIG_CONTROL;                                  break;
                    case ID_SIG_SETSIGNATURE      : com = SIG_SETSIGNATURE;                             break;
                    case ID_SIG_READVALUE         : com = SIG_READVALUE;                                break;
                    case ID_TEL_READDESCR         : com = TEL_READDESCR;         state = DISP_CHECKSUM; break;
                    case ID_TEL_SETMODE           : com = TEL_SETMODE;                                  break;
                    case ID_TEL_ADDSIGNAL         : com = TEL_ADDSIGNAL;                                break;
                    case ID_TEL_GETDATA           : com = TEL_GETDATA;           state = DISP_CHECKSUM; break;
                    default:
                        state = DISP_NONE;
                        FlushInBuffer();
                        Send_ACK(dfalse);
                        break;
                }
                data_size = get_num_data(com);
            }
            break;

        case DISP_DATA:
            if (rx_cnt > 0)
            {
                num = GetData(&rx_buf[cnt_rx_buf], rx_cnt - 1);
                for (i = cnt_rx_buf; i < cnt_rx_buf + num; i++) crc ^= rx_buf[i];
                rx_cnt -= num;
                cnt_rx_buf += num;
            }
            switch (com)
            {
                //fixed data length commands
                case SYS_CHECKCHANNEL:
                case SYS_SETSCANMODE:
                case PROG_READDESCR:
                case PROG_READID:
                case PROG_READ:
                case PROG_PREPAREFORPROG:
                case PROG_FINISHPROG:
                case PROG_RESTART:
                case SIG_READDESCR:
                case SIG_READVALUE:
                case TEL_SETMODE:
                case TEL_ADDSIGNAL:
                    if (cnt_rx_buf >= data_size) state = DISP_CHECKSUM;
                    break;
                //variable data length commands
                case PROG_WRITE:
                    if (dev_descr_arr[rx_buf[0]].cnt_wr_block <= (cnt_rx_buf - 1)) state = DISP_CHECKSUM;
                    break;
                case SYS_TESTCHANNEL:
                    if (cnt_rx_buf > 0)
                            if (rx_buf[cnt_rx_buf - 1] == 0) state = DISP_CHECKSUM;
                    break;
                case SIG_CONTROL:
                    if (cnt_rx_buf > 2)
                    {
                        sig_num = u16_from_buf(&rx_buf[0]);
                        sig_size = SIG_get_signal_size(sig_num);
                        if (cnt_rx_buf >= (2 + 1 + sig_size)) state = DISP_CHECKSUM;
                    }
                    break;
                case SIG_SETSIGNATURE:
                    //not implemented
                    state = DISP_NONE;
                    FlushInBuffer();
                    Send_ACK(dfalse);
                    break;
                default: state = DISP_CHECKSUM; break;
            }
            break;

        case DISP_CHECKSUM:
            if (rx_cnt >= 1)
            {
                if (VerifyChecksum()) command_run(com, rx_buf);
                else Send_ACK(dfalse);

                FlushInBuffer();
                state = DISP_NONE;
            }
            break;
    }
}
//-----------------------------------------------------------------------------------------
inline unsigned int get_command_id(void)
{
    int num;

    num = GetData(rx_buf, 2);
    rx_cnt -= num;

    crc ^= rx_buf[0];
    crc ^= rx_buf[1];

    if (num != 2) return 0;
    else return (rx_buf[0] << 8) | rx_buf[1];
}
//-----------------------------------------------------------------------------------------
void Send_ACK(const dboolean ack)
{
    if (ack == dfalse) tx_buf[0] = 0xFF;
    else tx_buf[0] = 0x0;
    Send_ChkSum(tx_buf, 1);
}
//-----------------------------------------------------------------------------------------
inline int get_num_data(const Disp_COM_t com)
{
    int num;

    switch (com)
    {
        case SYS_CHECKCHANNEL:      num =  8; break;
        case SYS_RESET:             num =  0; break;
        case SYS_REQUESTDEVICEINFO: num =  0; break;
        case SYS_SETSCANMODE:       num =  1; break;
        case SYS_SELFTEST:          num =  0; break;
        case SYS_SAVE_SETTINGS:     num =  0; break;
        case SYS_TESTCHANNEL:       num = -1; break;
#if DSPA_PROT_VERSION >= 3
		case SYS_REQUESTVERSION:    num =  0; break;
#endif
        case PROG_REQUESTDEVICENUM: num =  0; break;
        case PROG_READDESCR:        num =  1; break;
        case PROG_READID:           num =  1; break;
        case PROG_READ:             num =  1; break;
        case PROG_PREPAREFORPROG:   num =  2; break;
        case PROG_WRITE:            num = -1; break;
        case PROG_FINISHPROG:       num =  1; break;
        case PROG_RESTART:          num =  1; break;
        case SIG_REQUESTNUM:        num =  0; break;
        case SIG_READDESCR:         num =  2; break;
        case SIG_CONTROL:           num = -1; break;
        case SIG_SETSIGNATURE:      num = -1; break;
        case SIG_READVALUE:         num =  2; break;
        case TEL_READDESCR:         num = -1; break;
        case TEL_SETMODE:           num =  7; break;
        case TEL_ADDSIGNAL:         num =  2; break;
        case TEL_GETDATA:           num = -1; break;
        default:                    num = -1; break;
    }

    return num;
}
//----------------------------------------------------------------------------------------
dboolean VerifyChecksum()
{
    du8 buf[1] = {0};
    unsigned int num;

    num = GetData(buf, 1);
    rx_cnt -= num;
    if(num != 1) return dfalse;
    if(buf[0] == crc) return dtrue;
    else return dfalse;
}
//----------------------------------------------------------------------------------------
void Send_ChkSum(du8 *const buf, const unsigned int num)
{
    du8 checksum = 0;
    unsigned int i;

    for (i = 0; i < num; i++) checksum ^= buf[i];
    buf[num] = checksum;
    Send(buf, num + 1);
}
//----------------------------------------------------------------------------------------
void dispatcher_set_dev_descriptor(Dev_descriptor_t *const dev_descr_arr_value)
{
    dev_descr_arr = dev_descr_arr_value;
}
