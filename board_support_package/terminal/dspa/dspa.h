#ifndef DSPA_H_
#define DSPA_H_

#include <stdint.h>

#define du8      uint8_t
#define du16     uint16_t
#define du32     uint32_t
#define du64     uint64_t
#define dint16   int16_t
#define dint32   int32_t
#define dboolean int

#define dfalse 0
#define dtrue  1

#define invalid_handle -1

typedef int      (*FuncPtr_Send_t)          (du8 *const, const unsigned int);
typedef int      (*FuncPtr_GetData_t)       (du8 *const, const unsigned int);
typedef int      (*FuncPtr_InBufUsed_t)     (void);
typedef void     (*FuncPtr_FlushInBuffer_t) (void);

typedef void     (*FuncPtr_SysReset_t)      (void);
typedef char*    (*FuncPtr_SelfTest_t)      (void);
typedef dboolean (*FuncPtr_SaveSettings_t)  (void);

typedef du32     (*FuncPtr_ReadId_t)        (const du8 dev_num);
typedef dboolean (*FuncPtr_Read_t)          (const du8 dev_num, du8 *const pusData, const du32 ulStartAddress);
typedef dboolean (*FuncPtr_Erase_t)         (const du8 dev_num);
typedef dboolean (*FuncPtr_Write_t)         (const du8 dev_num, du8 *const pusData, const du32 ulStartAddress);
typedef void     (*FuncPtr_Reset_t)         (const du8 dev_num);
typedef dboolean (*Func_Ptr_Finish_t)       (const du8 dev_num);

typedef struct Dev_descriptor
{
    char*    name;         // Name
    du32     cnt_wr_block; // Write data block size
    du32     cnt_rd_block; // Read data block size
    du32     max_size;     // Max firmware size
    du16     wr_timeout;   // Write timeout
    du16     prep_timeout; // Prepare for program timeout
    dboolean restart;      // Need of restart after programming
    du64     Addr;         // address pointer
    du64     Addr_base;    // base address
} Dev_descriptor_t;

typedef struct
{
    du32 period;           // Period
    du16 max_num_signals;  // maximum number signals permitted to use in telemetry
    du16 max_frame_size;   // maximal permitted size of frame size
    du32 attributes;       // attributes
} TEL_Descriptor_t;

typedef struct SYS_Config
{
    char *system_info;
    FuncPtr_SysReset_t     FuncPtr_SysReset;
    FuncPtr_SelfTest_t     FuncPtr_SelfTest;
    FuncPtr_SaveSettings_t FuncPtr_SaveSettings;
} SYS_Config_t;

typedef struct PROG_Config
{
    Dev_descriptor_t  *descriptor;
    du16              descriptor_size;
    FuncPtr_ReadId_t  FuncPtr_ReadId;
    FuncPtr_Read_t    FuncPtr_Read;
    FuncPtr_Erase_t   FuncPtr_Erase;
    FuncPtr_Write_t   FuncPtr_Write;
    FuncPtr_Reset_t   FuncPtr_Reset;
    Func_Ptr_Finish_t Func_Ptr_Finish;
} PROG_Config_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void dspa_dispatcher_init     (const FuncPtr_Send_t pfSend, const FuncPtr_GetData_t pfGetData,
                               const FuncPtr_InBufUsed_t pfInBufUsed, const FuncPtr_FlushInBuffer_t pfFlushInBuffer);
void dspa_dispatcher          (const int elapsed_time_us);

void SYS_Init                 (SYS_Config_t *const SYS_Config_value);

void PROG_Init                (PROG_Config_t *const PROG_Config_value);

void TEL_Set_descriptor       (TEL_Descriptor_t *const TEL_dscr_value);
void TEL_Sample_Update        (const int handle);
void TEL_Sample_Save          (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* DSPA_H_ */
