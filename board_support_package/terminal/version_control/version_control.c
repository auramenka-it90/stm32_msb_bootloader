#include "version_control.h"
#include <string.h>
#include <stdlib.h>


#ifdef BSP_MSB0_PSPM1_BOARD
#if BSP_MSB0_PSPM1_BOARD==0
// Product information strings
	const char _VC_PRODUCT_NUMBER_[] =      "Mode switching board(MSB)\n";
	const char _VC_BOARD_NUMBER_[] =        "Board:  xxxx.xx.xx.xxx\n";
	const char _VC_CHIP_DESIGNATION_[] =    "Chip: DD16 ";
	const char _VC_SOFT_NAME_[] =           "STM32F411RET6TR\n";
	const char _VC_SOFT_NOTE_[] =           "SW:  produced by mr. Andrew Auramenka \n";
	const char _VC_SOFT_VERSION_[] =        "Ver. 1.0\n";
#else
	const char _VC_PRODUCT_NUMBER_[] =      "Primary signal processing module(PSPM)\n";
	const char _VC_BOARD_NUMBER_[] =        "Board:  xxxx.xx.xx.xxx\n";
	const char _VC_CHIP_DESIGNATION_[] =    "Chip: DD1 ";
	const char _VC_SOFT_NAME_[] =           "STM32F411RET6TR\n";
	const char _VC_SOFT_NOTE_[] =           "SW:  produced by mr. Andrew Auramenka \n";
	const char _VC_SOFT_VERSION_[] =        "Ver. 1.0\n";
#endif
#endif

// Build timestamp (compilation date and time)
static const char* build = __DATE__"  "__TIME__"\n";

// Pointer to device info string
static char* device_info = NULL;

/**
 * Convert uint32_t to hex string in format "0xABCDEF12"
 * 
 * @param value  32-bit value to convert
 * @param buffer Output buffer (must be at least 11 bytes)
 */
/*
static void uint32_to_hex_str(uint32_t value, char* buffer) {
    if (buffer == NULL) return;
    
    const char hex_digits[] = "0123456789ABCDEF";
    
    // Start with "0x" prefix
    buffer[0] = '0';
    buffer[1] = 'x';
    
    // Fill 8 hex digits (most significant first)
    for (int i = 0; i < 8; i++) {
        // Get hex digit (from most significant to least)
        int digit = (value >> (28 - i * 4)) & 0xF;
        buffer[i + 2] = hex_digits[digit];
    }
    
    // Null-terminate the string
    buffer[10] = '\0';
}
*/

/**
 * Create device information string
 * All comments in English.
 * 
 * @return Pointer to the created string, or NULL if allocation failed
 */
char* device_info_create(void) {
    uint32_t len = 0;
    
    // Free previous string if exists
    device_info_destroy();
    
    // Allocate memory for device info string
    device_info = malloc(_VC_SIZE_DEVICE_INFO_STR_);
    
    if (device_info != NULL) {
        // Copy product number
        strcpy(device_info + len, _VC_PRODUCT_NUMBER_);
        len += strlen(_VC_PRODUCT_NUMBER_);
        
        // Copy board number
        strcpy(device_info + len, _VC_BOARD_NUMBER_);
        len += strlen(_VC_BOARD_NUMBER_);
        
        // Copy chip designation
        strcpy(device_info + len, _VC_CHIP_DESIGNATION_);
        len += strlen(_VC_CHIP_DESIGNATION_);
        
        // Copy software name
        strcpy(device_info + len, _VC_SOFT_NAME_);
        len += strlen(_VC_SOFT_NAME_);
        
        // Copy build timestamp
        strcpy(device_info + len, build);
        len += strlen(build);
        
        // Copy software note
        strcpy(device_info + len, _VC_SOFT_NOTE_);
        len += strlen(_VC_SOFT_NOTE_);
        
        // Copy software version
        strcpy(device_info + len, _VC_SOFT_VERSION_);
        len += strlen(_VC_SOFT_VERSION_);
        
        // Add newline at the end
        device_info[len++] = '\n';
        device_info[len] = '\0';
    }
    
    return device_info;
}

/**
 * Destroy device info string and free allocated memory
 */
void device_info_destroy(void) {
    if (device_info != NULL) {
        free(device_info);
        device_info = NULL;
    }
}
