#include <stddef.h>
#include <string.h>
#include "dspa_telemetry.h"
#include "dspa_config.h"
#include "dspa_utils.h"

#define TEL_STATUS_OK          0x01
#define TEL_STATUS_OVERFLOW    0x02
#define TEL_STATUS_ERR1        0x04
#define TEL_STATUS_ERR2        0x08
#define TEL_STATUS_ERR3        0x10
#define TEL_STATUS_ERR4        0x20
#define TEL_STATUS_WAIT        0x40
#define TEL_STATUS_FRAME_ERROR 0x80

typedef enum {TM_STOP = 0, TM_START_STREAM = 1, TM_START_FRAME = 2, TM_START_BUF = 3, TM_PAUSE = 4} TEL_Mode_t;

static void tel_stop(void);

// settings controlled by host
static volatile TEL_Mode_t mode;
static du32 kdiv;
//static du16 frame_size;// not used

//variables to manage samples in buffer
static          int sample_size     = 0;
static          int added_signals   = 0;
static volatile int buf_num_samples = 0;
static volatile int buf_sample_wr   = 0;
static volatile int buf_sample_rd   = 0;

static volatile dboolean overflow_fl = dfalse;

static du8 tel_buffer[TEL_BUFFER_SIZE]; // data buffer with samples values
static du8 sample[TEL_SIZE_SAMPLE_BUFFER]; // memory for sample data

static SigArrRec_t *arr_sig_arr = NULL;
static TEL_Descriptor_t *TEL_dscr;

//------------------------------------------------------------------------------------------
void TEL_Init(SigArrRec_t *const arr_sig_arr_value)
{
    arr_sig_arr = arr_sig_arr_value;
}
//------------------------------------------------------------------------------------------
void TEL_Set_descriptor(TEL_Descriptor_t *const TEL_dscr_value)
{
    TEL_dscr = TEL_dscr_value;
}
//------------------------------------------------------------------------------------------
du16 TEL_ReadDSCR(du8 *const buf)
{
    int cnt = 0;

    cnt += u32_to_buf(TEL_dscr->period,          &buf[cnt]);
    cnt += u16_to_buf(TEL_dscr->max_num_signals, &buf[cnt]);
    cnt += u16_to_buf(TEL_dscr->max_frame_size,  &buf[cnt]);
    cnt += u32_to_buf(TEL_dscr->attributes,      &buf[cnt]);

    return cnt;
}
//------------------------------------------------------------------------------------------
dboolean TEL_SetMode(const du8 mode_value, const du32 kdiv_value, const du16 frame_size_value)
{
    dboolean ok = dtrue;

    if ((kdiv_value == 0) || (frame_size_value > TEL_dscr->max_frame_size)) return dfalse;

    switch ((TEL_Mode_t)mode_value)
    {
        case TM_START_FRAME: //not supported
        case TM_START_STREAM://not supported
            ok = dfalse;
            tel_stop();
            break;

        case TM_STOP:
            ok = dtrue;
            tel_stop();
            break;

        case TM_START_BUF:
            if (added_signals == 0)
            {
                ok = dfalse;
                tel_stop();
            }
            else
            {
                if (sample_size == 0) return dfalse;
                ok = dtrue;
                kdiv = kdiv_value;
                //frame_size = frame_size_value;
                buf_num_samples = TEL_BUFFER_SIZE / sample_size;
                buf_sample_wr = 0;
                buf_sample_rd = 0;

                mode = (TEL_Mode_t)mode_value;//last to activate telemetry
            }
            break;

        case TM_PAUSE:
            ok = dtrue;
            mode = (TEL_Mode_t)mode_value; //first to pause telemetry
            kdiv = kdiv_value;
            //frame_size = frame_size_value;

            buf_sample_wr = 0;
            buf_sample_rd = 0;
            break;
    }
    return ok;
}
//------------------------------------------------------------------------------------------
void tel_stop(void)
{
    mode = TM_STOP;//first to stop telemetry
    sample_size     = 0;
    added_signals   = 0;
    buf_num_samples = 0;
    buf_sample_wr   = 0;
    buf_sample_rd   = 0;
    SIG_tel_reset();
}
//------------------------------------------------------------------------------------------
dboolean TEL_AddSignal(const du16 sig_num)
{
    Signal_t *s;

    if ( added_signals >= TEL_MAX_SIGNALS )    return dfalse;
    if (mode != TM_STOP)                       return dfalse;
    if (sample_size >= TEL_SIZE_SAMPLE_BUFFER) return dfalse;

    s = SIG_tel_addsignal(sig_num, sample_size);
    if (s == NULL) return dfalse;

    sample_size += s->Value_size;
    added_signals++;

    return dtrue;
}
//------------------------------------------------------------------------------------------
du16 TEL_Getdata(du8 *buf, du16 *const num_samples, du8 *const status)
{
    int samples_ready;
    int samples_to_get;
    int ind;
    int buf_sample_wr_tmp = buf_sample_wr;
    int i;

    *status = TEL_STATUS_OK;

    if ((mode == TM_PAUSE) || (mode == TM_STOP)) // telemetry not active
    {
        *num_samples = 0;
        return 0;
    }

    if (buf_sample_wr_tmp == buf_sample_rd) // empty
    {
        *num_samples = 0;
        *status |= TEL_STATUS_WAIT;
        return 0;
    }
    // calc samples ready to read
    if (buf_sample_wr_tmp > buf_sample_rd) samples_ready = buf_sample_wr_tmp - buf_sample_rd;
    else                                   samples_ready = buf_sample_wr_tmp + (buf_num_samples - buf_sample_rd);

    // calc samples to send to the host
    if (samples_ready * sample_size > (DSPA_DISPATCHER_TX_BUF_SIZE - 8)) samples_to_get = (DSPA_DISPATCHER_TX_BUF_SIZE - 8) / sample_size;
    else                                                                 samples_to_get = samples_ready;

    // finalizing
    for (i = 0; i < samples_to_get; i++)
    {
        ind = buf_sample_rd * sample_size; // calc ind for tel_buffer
        memcpy(buf, &tel_buffer[ind], sample_size);
        buf_sample_rd++;
        buf += sample_size;
        if (buf_sample_rd >= buf_num_samples) buf_sample_rd = 0;
    }

    *num_samples = samples_to_get;
    if (overflow_fl) // overflow happened
    {
        overflow_fl = dfalse; // reset overflow flag
        *status |= TEL_STATUS_OVERFLOW;
    }

    return samples_to_get * sample_size;
}
//------------------------------------------------------------------------------------------
// TEL_Sample_Update may be called from interrupt or separate thread
// Update data to send over telemetry
void TEL_Sample_Update(const int handle)
{
    if (handle < 0)
    {
        return;
    }
    
    int i;

    SigArrRec_t *sar = &arr_sig_arr[handle];

    if (sar->tel_num == 0) return;

    DSPA_ENTER_CRITICAL
        for (i = 0; i < sar->tel_num; i++)
            memcpy(&sample[sar->tel[i]->tel_sample_offset], sar->tel[i]->pValue, sar->tel[i]->Value_size);
    DSPA_EXIT_CRITICAL
}
//------------------------------------------------------------------------------------------
// TEL_Sample_Save may be called from interrupt or separate thread
// Push sample data to telemetry buffer
void TEL_Sample_Save(void)
{
    static du32 cnt_kdiv = 0;
    int ind;

    if ((mode == TM_PAUSE) || (mode == TM_STOP)) return;

    cnt_kdiv++;
    if (cnt_kdiv >= kdiv)
    {
        cnt_kdiv = 0;

        // check for overflow
        if (((buf_sample_rd - buf_sample_wr) == 1) ||
            ((buf_sample_rd == 0) && (buf_sample_wr == (buf_num_samples - 1))))
        {
            overflow_fl = dtrue;
            return;
        }

        // calculate ind for tel_buffer
        ind = buf_sample_wr * sample_size;

        memcpy(&tel_buffer[ind], sample, sample_size);

        // calculate next value of buf_sample_wr
        buf_sample_wr++;
        if (buf_sample_wr >= buf_num_samples) buf_sample_wr = 0;
    }
}
