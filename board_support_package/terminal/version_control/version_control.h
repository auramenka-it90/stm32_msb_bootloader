#ifndef VERSION_CONTROL_H_
#define VERSION_CONTROL_H_

#include "board_support_package.h"

/*
 * Comment out if you need to change the compilation date
 */
//#define _VC_MACRO_ENA_DATE_COMPILE_
#define _VC_SIZE_DEVICE_INFO_STR_ (4*256)

/*
 * Software version 2.1
 */
#define _SOFT_VERSION_      0
#define _SOFT_SUBVERSION_   1

/*
 * Version 1.1
 */
//#define _SOFT_VERSION_BUK_ ((_SOFT_VERSION_<<4)|_SOFT_SUBVERSION_)

// Function to create device info string with password in hex format
char* device_info_create(void);

// Function to destroy device info string and free memory
void device_info_destroy(void);

#endif /* VERSION_CONTROL_H_ */
