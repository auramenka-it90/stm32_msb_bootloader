/**
 * @file version_control.h
 * @brief Модуль информации о версии ПО и аппаратуры (Zero-RAM Overhead)
 */

#ifndef VERSION_CONTROL_H_
#define VERSION_CONTROL_H_

#include "board_support_package.h" /* Для BSP_MSB0_PSPM1_BOARD */

/*==============================================================================
 * МАКРОСЫ КОНФИГУРАЦИИ (Строковые литералы)
 *============================================================================*/

#if BSP_MSB0_PSPM1_BOARD == 0
    /* --- Настройки для платы MSB --- */
    #define _VC_PRODUCT_NUMBER_     "Mode switching board(MSB)\n"
    #define _VC_BOARD_NUMBER_       "Board:  7198.30.03.100\n" /* Замени на реальный номер MSB */
    #define _VC_CHIP_DESIGNATION_   "Chip: DD16 "
    #define _VC_SOFT_NAME_          "STM32F411RET6TR\n"
    #define _VC_SOFT_NOTE_          "SW:  produced by mr. Andrew Auramenka \n"
    #define _VC_SOFT_VERSION_       "Ver. 1.0\n"
#else
    /* --- Настройки для платы PSPM --- */
    #define _VC_PRODUCT_NUMBER_     "Primary signal processing module(PSPM)\n"
    #define _VC_BOARD_NUMBER_       "Board:  xxxx.xx.xx.xxx\n" /* Замени на реальный номер PSPM */
    #define _VC_CHIP_DESIGNATION_   "Chip: DD1 "
    #define _VC_SOFT_NAME_          "STM32F411RET6TR\n"
    #define _VC_SOFT_NOTE_          "SW:  produced by mr. Andrew Auramenka \n"
    #define _VC_SOFT_VERSION_       "Ver. 1.0\n"
#endif

/*==============================================================================
 * PUBLIC API
 *============================================================================*/
/**
 * @brief Получить указатель на статическую строку с информацией об устройстве
 * @return const char* Указатель на null-terminated строку во Flash (.rodata)
 */
const char* get_device_info(void);

#endif /* VERSION_CONTROL_H_ */
