#include <stddef.h>
#include "dspa.h"
#include "dspa_program.h"
#include "dspa_command.h"

static PROG_Config_t *PROG_Config;

//--------------------------------------------------------------------------------------------
du32 PROG_GetCntRdBlock(const du8 dev_num)
{
    return PROG_Config->descriptor[dev_num].cnt_rd_block;
}
//--------------------------------------------------------------------------------------------
du32 PROG_ReadId(const du8 dev_num)
{
    if(PROG_Config->FuncPtr_ReadId != (FuncPtr_ReadId_t)0)
        return PROG_Config->FuncPtr_ReadId(dev_num);
    else return 0xFFFFFFFF;
}
//--------------------------------------------------------------------------------------------
du32 PROG_Read(const du8 dev_num, du8 *const buf)
{
    if (PROG_Config->FuncPtr_Read != (FuncPtr_Read_t)0)
    {
        dboolean result = PROG_Config->FuncPtr_Read(dev_num, buf, PROG_Config->descriptor[dev_num].Addr);

        PROG_Config->descriptor[dev_num].Addr += PROG_Config->descriptor[dev_num].cnt_rd_block;
        if (result == dtrue) return PROG_Config->descriptor[dev_num].cnt_rd_block;
        else return 0;
    }
    return 0xFFFFFFFF;
}
//--------------------------------------------------------------------------------------------
dboolean PROG_PrepareForProg(const du8 dev_num, const dboolean erase)
{
    dboolean result = dtrue;

    if (PROG_Config->FuncPtr_Erase != (FuncPtr_Erase_t)0)
    {
        PROG_Config->descriptor[dev_num].Addr = PROG_Config->descriptor[dev_num].Addr_base;
        if (erase) result = PROG_Config->FuncPtr_Erase(dev_num);
    }
    else result = dfalse;

    return result;
}
//--------------------------------------------------------------------------------------------
dboolean PROG_Write(const du8 dev_num, du8 *const data)
{
    dboolean bResult;

    if (PROG_Config->FuncPtr_Write != (FuncPtr_Write_t)0)
    {
        bResult = PROG_Config->FuncPtr_Write(dev_num, data, PROG_Config->descriptor[dev_num].Addr);
        PROG_Config->descriptor[dev_num].Addr += PROG_Config->descriptor[dev_num].cnt_wr_block;
    }
    else bResult = dfalse;

    return bResult;
}
//--------------------------------------------------------------------------------------------
dboolean PROG_Finish(const du8 dev_num)
{
    if (PROG_Config->Func_Ptr_Finish != (Func_Ptr_Finish_t)0)
    {
        PROG_Config->descriptor[dev_num].Addr = PROG_Config->descriptor[dev_num].Addr_base;
        PROG_Config->Func_Ptr_Finish(dev_num);
        return dtrue;
    }
    else return dfalse;
}
//--------------------------------------------------------------------------------------------
void PROG_Reset(const du8 dev_num)
{
    if (PROG_Config->FuncPtr_Reset != (FuncPtr_Reset_t)0)
        PROG_Config->FuncPtr_Reset(dev_num);
}
//--------------------------------------------------------------------------------------------
void PROG_Init(PROG_Config_t *const PROG_Config_value)
{
    PROG_Config = PROG_Config_value;

    command_set_dev_descriptor(PROG_Config->descriptor, PROG_Config->descriptor_size);
    dispatcher_set_dev_descriptor(PROG_Config->descriptor);
}

