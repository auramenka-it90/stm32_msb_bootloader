#ifndef DSPA_SIGNALS_H_
#define DSPA_SIGNALS_H_

#include "dspa.h"
#include "dspa_config.h"

typedef enum SIG_Ctrl_type
{
    SCT_NONE = 0, SCT_FIXED = 1, SCT_SIGNATURE = 2
} SIG_Ctrl_type_t;

typedef enum SIG_Class
{
    SCL_STD = 0, SCL_ENUM = 1, SCL_ARRAY1 = 2, SCL_ARRAY2 = 3
} SIG_Class_t;

typedef enum SIG_Type
{
	STYPE_STRING = 0,
	STYPE_BOOL = 1,
	STYPE_BYTE = 2,
	STYPE_USHORT = 3,
	STYPE_SHORT = 4,
	STYPE_LONG = 5,
	STYPE_ULONG = 6,
	STYPE_FLOAT = 7,
	STYPE_SBYTE = 8,
	STYPE_INT64 = 9,
	STYPE_UINT64 = 10,
	STYPE_DOUBLE = 11
} SIG_Type_t;

typedef enum SIG_Measurement_type
{
    SMT_NONE = 0, SMT_VOLTAGE = 1, SMT_AMPERE = 2, SMT_ANG_DEGREE = 3,
    SMT_RADIAN = 4, SMT_METER = 5, SMT_VOLT_ANG_DEGREE = 6,
    SMT_METER_SECOND = 7, SMT_CELSIUS = 8, SMT_ANG_DEGREE_SECOND = 9
} SIG_Measurement_type_t;

typedef struct SIG_Descriptor
{
    char*                  name;         // Name
    SIG_Class_t            class_type;   // Structure (class)
    SIG_Type_t             type;         // Type
    du32                   attributes;   // Attributes
    dint16                 parent_node;  // ID of parent signal
    du32                   period;       // Period of signals change
    SIG_Measurement_type_t meas_physics; // Measurement type (physics type)
    float                  coefficient;  // Coefficient
} SIG_Descriptor_t;

typedef struct Signal
{
    SIG_Descriptor_t dscr;              // Descriptor
    void            *pParentSignal;     // Parent signal
    void            *pValue;            // Signals value
    du16             Value_size;        // Size of signals value
    SIG_Ctrl_type_t  ctrl;              // True for under-control signals
    du16             handle;            // Handle to signals array
    du16             tel_sample_offset; // Offset in telemetry sample
} Signal_t;

typedef struct SigArrRec
{
    Signal_t *arr;
    int       size;
    int       num_offset;
    Signal_t *ctrl[SIG_MAX_SIGNALS_2_CONTROL];
    int       ctrl_num;
    Signal_t *tel[TEL_MAX_SIGNALS];
    du16      tel_num;
} SigArrRec_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Command handlers
du16      SIG_ReadDSCR (const du16 sig_num, du8 *const buf);
dboolean  SIG_Control  (const du16 sig_num, const SIG_Ctrl_type_t ctrl_type, du8 *const data);
du16      SIG_ReadValue(const du16 sig_num, du8 *const data);

// Services
int       SIG_Init           (Signal_t *const sig_arr, const int size);

Signal_t *SIG_get_signal     (const du16 sig_num);
Signal_t *SIG_tel_addsignal  (const du16 sig_num, const du16 sample_offset);
void      SIG_tel_reset      (void);
int       SIG_Get_num_signals(void);
du16      SIG_get_signal_size(const du16 sig_num);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DSPA_SIGNALS_H_ */
