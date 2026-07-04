#include <stddef.h>
#include "dspa_system.h"
#include "dspa_command.h"

static SYS_Config_t *SYS_Config;

//------------------------------------------------------------------------------------------
// Reset entire system
void SYS_Reset(void)
{
    if(SYS_Config->FuncPtr_SysReset != (FuncPtr_SysReset_t)0) SYS_Config->FuncPtr_SysReset();
}
//------------------------------------------------------------------------------------------
// Selftest
char* SYS_SelfTest(void)
{
    if(SYS_Config->FuncPtr_SelfTest != (FuncPtr_SelfTest_t)0) return SYS_Config->FuncPtr_SelfTest();
    else return "ERROR: SelfTest handler not assigned";
}
//------------------------------------------------------------------------------------------
void SYS_Init(SYS_Config_t *const SYS_Config_value)
{
    SYS_Config = SYS_Config_value;
    command_set_system_info(SYS_Config->system_info);
}
//------------------------------------------------------------------------------------------
dboolean SYS_SaveSettings(void)
{
    if(SYS_Config->FuncPtr_SaveSettings != (FuncPtr_SaveSettings_t)0) return SYS_Config->FuncPtr_SaveSettings();
    else return dfalse;
}
