/**
 ******************************************************************************
 * @file    version_control.h
 * @brief   Software/Hardware version information module for Bootloader.
 *          Supports dual-board configuration (MSB / PSPM) with Zero-RAM overhead.
 *          All comments in ASCII English.
 ******************************************************************************
 */

#ifndef VERSION_CONTROL_H_
#define VERSION_CONTROL_H_

#include "board_support_package.h"

/* ========================================================================= */
/*  STATIC STRING DEFINITIONS (BOOTLOADER)                                   */
/* ========================================================================= */

#if (BSP_MSB0_PSPM1_BOARD == 0)
    /* --- Settings for the MSB board --- */
    #define _VC_PRODUCT_NUMBER_     "Mode switching board (MSB)\n"
    #define _VC_BOARD_NUMBER_       "Board:  DIAS.901.02.00.200\n"
    #define _VC_CHIP_DESIGNATION_   "Chip:   DD16 - STM32F411RET6TR\n"
    #define _VC_SOFT_NAME_          "Software: bootloader\n"
    #define _VC_SOFT_NOTE_          "Produced by mr. Andrew Auramenka \n"
    #define _VC_SOFT_VERSION_       "Ver. 2.0\n"
#else
    /* --- Settings for the PSPM board --- */
    #define _VC_PRODUCT_NUMBER_     "Primary signal processing module (PSPM)\n"
    #define _VC_BOARD_NUMBER_       "Board:  DIAS.901.02.00.300\n"
    #define _VC_CHIP_DESIGNATION_   "Chip:   DD1 - STM32F411RET6TR\n"
    #define _VC_SOFT_NAME_          "Software: bootloader\n"
    #define _VC_SOFT_NOTE_          "Produced by mr. Andrew Auramenka \n"
    #define _VC_SOFT_VERSION_       "Ver. 2.0\n"
#endif

/* ========================================================================= */
/*  PUBLIC API                                                               */
/* ========================================================================= */

/**
 * @brief  Returns pointer to static device info string stored in Flash (.rodata).
 * @retval const char* Pointer to null-terminated formatted string.
 */
const char* get_device_info(void);

#endif /* VERSION_CONTROL_H_ */
