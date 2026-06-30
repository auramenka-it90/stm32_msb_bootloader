#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dspa_signals.h"
#include "dspa_config.h"
#include "dspa_utils.h"
#include "dspa_telemetry.h"
#include "dspa.h"

static        int      get_signal_handle (const du16 sig_num);
static        dboolean ctrl_add          (Signal_t *const signal);
static        dboolean ctrl_remove       (Signal_t *const signal);
static inline du16     get_value_size    (Signal_t *const signal);

static int num_signals = 0;
static SigArrRec_t arr_sig_arr[SIG_NUM_SIGNALS_ARR];
static int cnt_arr_sig_arr = 0;



__attribute__((weak)) void	signal_change_handler(void*	s){

}


//--------------------------------------------------------------------------------------------
// Read signal descriptor (command handler)
du16 SIG_ReadDSCR(const du16 sig_num, du8 *const buf)
{
    int cnt = 0;
    Signal_t *s = SIG_get_signal(sig_num);

    DSPA_ENTER_CRITICAL
        cnt += string_to_buf(s->dscr.name, &buf[cnt]);
        buf[cnt] = (du8)s->dscr.class_type; cnt++;
        buf[cnt] = (du8)s->dscr.type; cnt++;
        cnt += u32_to_buf(s->dscr.attributes, &buf[cnt]);
        cnt += u16_to_buf(s->dscr.parent_node, &buf[cnt]);
        cnt += u32_to_buf(s->dscr.period, &buf[cnt]);
        buf[cnt] = (du8)s->dscr.meas_physics; cnt++;
        cnt += float_to_buf(s->dscr.coefficient, &buf[cnt]);
    DSPA_EXIT_CRITICAL

    return cnt;
}
//--------------------------------------------------------------------------------------------
// Init and register signals array
// Return signals array handl
int SIG_Init(Signal_t *const sig_arr, const int size)
{
    int k = 0;
    Signal_t *s;
    Signal_t *s_i;
    int i;
    SigArrRec_t *sar;
    int handle;

    if (cnt_arr_sig_arr == SIG_NUM_SIGNALS_ARR) return -1; // overflow

    DSPA_ENTER_CRITICAL
        // register signals array in arr_sig_arr
        sar = &arr_sig_arr[cnt_arr_sig_arr];
        sar->arr = sig_arr;
        if(cnt_arr_sig_arr == 0) sar->num_offset = 0;
        else sar->num_offset = arr_sig_arr[cnt_arr_sig_arr - 1].size + arr_sig_arr[cnt_arr_sig_arr - 1].num_offset;
        sar->size = size;
        sar->ctrl_num = 0;
        handle = cnt_arr_sig_arr;
        cnt_arr_sig_arr++;
        num_signals += size;

        // init contents of signals array
        for (k = 0; k < size; k++)
        {
            s = &sig_arr[k];
            // find parent signal and get its id
            for (i = 0; i < size; i++)
            {
                s_i = &sig_arr[i];
                if (s->pParentSignal == s_i->pValue)
                {
                    s->dscr.parent_node = i + sar->num_offset;
                    break;
                }
            }


            // init Value_size
            s->Value_size = get_value_size(s);

            s->handle = cnt_arr_sig_arr - 1;
        }
    DSPA_EXIT_CRITICAL

    TEL_Init(arr_sig_arr);

    return handle;
}
//--------------------------------------------------------------------------------------------
// Return pointer to signal with position == sig_num
Signal_t *SIG_get_signal(const du16 sig_num)
{
	const int handle = get_signal_handle(sig_num);
	if (handle < 0)
	{
		return NULL;
	}
	return &(arr_sig_arr[handle].arr[sig_num - arr_sig_arr[handle].num_offset]);
}
//--------------------------------------------------------------------------------------------
int get_signal_handle(const du16 sig_num)
{
    int i;

    for (i = 0; i < cnt_arr_sig_arr; i++)
        if (sig_num <= arr_sig_arr[i].num_offset + arr_sig_arr[i].size - 1) return i;

    return -1;
}
//--------------------------------------------------------------------------------------------
// Signal control (command handler)
dboolean SIG_Control(const du16 sig_num, const SIG_Ctrl_type_t ctrl_type, du8 *const data)
{
    Signal_t *s = SIG_get_signal(sig_num);
    dboolean ok = dtrue;

    if (s == NULL) return dfalse;

    DSPA_ENTER_CRITICAL
        memcpy(s->pValue, data, s->Value_size);
    DSPA_EXIT_CRITICAL

    if (s->ctrl != ctrl_type) // changing signal control type
    {
        switch (ctrl_type)
        {
            case SCT_NONE      : ok = ctrl_remove(s);      break;
            case SCT_FIXED     : ok = ctrl_add(s);         break;
            case SCT_SIGNATURE : /* not yet implemented */ break;
        }

        s->ctrl = ctrl_type;
    }

    signal_change_handler(s->pValue);

    return ok;
}
//--------------------------------------------------------------------------------------------
// Read signal value (commnad handler)
du16 SIG_ReadValue(const du16 sig_num, du8 *const data)
{
	Signal_t *s = SIG_get_signal(sig_num);
	if (s == NULL)
	{
		return 0;
	}

	du16 len;

	DSPA_ENTER_CRITICAL
	if (s->pValue == NULL)
	{
		memset(data, 0, s->Value_size);
		len = s->Value_size;
	}
	else if (s->dscr.type == STYPE_STRING)
	{
		len = string_to_buf(((char *)(char **)s->pValue), data);
	}
	else
	{
		memcpy(data, s->pValue, s->Value_size);
		len = s->Value_size;
	}
	DSPA_EXIT_CRITICAL

	return len;
}
//--------------------------------------------------------------------------------------------
static inline du16 get_value_size(Signal_t *const signal)
{
	switch (signal->dscr.type)
	{
	case STYPE_STRING:
		return strlen((char *)(char **)signal->pValue);

	case STYPE_BOOL:
		return sizeof(bool);

	case STYPE_BYTE:
		return sizeof(uint8_t);

	case STYPE_SBYTE:
		return sizeof(int8_t);

	case STYPE_USHORT:
		return sizeof(uint16_t);

	case STYPE_SHORT:
		return sizeof(int16_t);

	case STYPE_ULONG:
		return sizeof(uint32_t);

	case STYPE_LONG:
		return sizeof(int32_t);

	case STYPE_FLOAT:
		return sizeof(float);

	case STYPE_UINT64:
		return sizeof(uint64_t);

	case STYPE_INT64:
		return sizeof(int64_t);

	case STYPE_DOUBLE:
		return sizeof(long double);

	default:
		return 0;
	}
}
//--------------------------------------------------------------------------------------------
// add signal to control list
dboolean ctrl_add(Signal_t *const signal)
{
    int i;
    du16 handle = signal->handle;
    int ctrl_num = arr_sig_arr[handle].ctrl_num;
    Signal_t **ctrl = arr_sig_arr[handle].ctrl;

    if (ctrl_num >= SIG_MAX_SIGNALS_2_CONTROL) return dfalse;

    //check for duplication
    for (i = 0; i < ctrl_num; i++)
    {
        if (ctrl[i] == signal)
        {
            return dfalse;
        }
    }

    //add
    DSPA_ENTER_CRITICAL
        arr_sig_arr[handle].ctrl[ctrl_num] = signal;
        arr_sig_arr[handle].ctrl_num++;
    DSPA_EXIT_CRITICAL

    return dtrue;
}
//--------------------------------------------------------------------------------------------
// remove signal from control list
dboolean ctrl_remove(Signal_t *const signal)
{
    if (signal == NULL)
	{
		return dfalse;
	}

    int i, k;
    int ind_empty;
    du16 handle = signal->handle;
    int ctrl_num = arr_sig_arr[handle].ctrl_num;
    Signal_t **ctrl = arr_sig_arr[handle].ctrl;

    if (ctrl_num <= 0) return dfalse;

    for (i = 0; i < SIG_MAX_SIGNALS_2_CONTROL; i++)
    {
        if (ctrl[i] == signal)
        {
            DSPA_ENTER_CRITICAL
                arr_sig_arr[handle].ctrl[i] = NULL;
                arr_sig_arr[handle].ctrl_num--;

                //move data
                ind_empty = i;
                for (k = i + 1; k < SIG_MAX_SIGNALS_2_CONTROL; k++)
                {
                    if (ctrl[k] != NULL)
                    {
                        ctrl[ind_empty] = ctrl[k];
                        ctrl[k] = NULL;
                        ind_empty++;
                    }
                    else break;
                }
            DSPA_EXIT_CRITICAL
            break;
        }
    }

    return dtrue;
}
//--------------------------------------------------------------------------------------------
Signal_t *SIG_tel_addsignal(const du16 sig_num, const du16 sample_offset)
{
    Signal_t *s = SIG_get_signal(sig_num);
    if (s == NULL)
	{
		return NULL;
	}

    du16 h = s->handle;

    if (arr_sig_arr[h].tel_num >= TEL_MAX_SIGNALS) return NULL;

    DSPA_ENTER_CRITICAL
        arr_sig_arr[h].tel[arr_sig_arr[h].tel_num] = s;
        arr_sig_arr[h].tel[arr_sig_arr[h].tel_num]->tel_sample_offset = sample_offset;
        arr_sig_arr[h].tel_num++;
    DSPA_EXIT_CRITICAL

    return s;
}
//--------------------------------------------------------------------------------------------
void SIG_tel_reset(void)
{
	for (du16 i = 0; i < cnt_arr_sig_arr; i++)
	{
		arr_sig_arr[i].tel_num = 0;
	}
}
//--------------------------------------------------------------------------------------------
int SIG_Get_num_signals(void)
{
    return num_signals;
}
//--------------------------------------------------------------------------------------------
du16 SIG_get_signal_size(const du16 sig_num)
{
	if (sig_num >= num_signals)
	{
		return 0;
	}
	Signal_t *s = SIG_get_signal(sig_num);
	if (s == NULL)
	{
		return 0;
	}
	return SIG_get_signal(sig_num)->Value_size;
}
