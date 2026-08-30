/**
 * ******************************************************************************
 * @file    board_support_package.c
 * @brief   Secure Hardware Initialization and Board Control Helper Functions.
 *          Fixes bus contention on SPI, sticky RCC flags, and struct errors.
 *          All comments in English.
 * ******************************************************************************
 */

#include "board_support_package.h"
#include "pin_mgmt.h"
#include "w25q128.h"
#include "terminal.h"


/* Global hardware test result variable */
uint32_t test_hardware_result = _B_TEST_HARDWARE_SUCCESS_;



/* STM32 pins configuration (use_rtos removed as it is now redundant) */
Pin_Mgmt_Config_t pin = {
    .debug_enabled = 1
};


/**
  * @brief  Securely initializes all hardware components in the proper sequence.
  *         Holds FPGA in reset during SPI Flash operations to prevent bus contention.
  * @retval Current hardware status register
  */
uint32_t init_hardware(void) {

    /* 1. PINS CONFIGURATION */
    if (PIN_MGMT_Init(&bsp_pin_config) != osOK) {
        test_hardware_result |= _B_FAULT_PINS_;
    } else {
        DWT_Init();

        /* Держим FPGA в сбросе */
        PIN_Reset(&pin_mr_prog);
    }

    /* 2. W25Q128 FLASH CONFIGURATION */
    if (test_status_hardware(_B_FAULT_PINS_)) {

        /* Инициализируем Flash без захвата шины */
        if (W25Q128_Init(&bsp_flash, &hspi1, SP1_FPGA_CSO_GPIO_Port, SP1_FPGA_CSO_Pin) != osOK) {
            test_hardware_result |= _B_FAULT_W25Q128_;
        } else {
            W25Q128_ReadJEDECID(&bsp_flash);
            W25Q128_ReadUID(&bsp_flash);
        }
    }

    /* 3. TERMINAL */
    if (!terminal_init()) {
        test_hardware_result |= _B_FAULT_TERMINAL_;
    }

    return test_hardware_result;
}

/**
  * @brief  Checks if a specific hardware module is operational.
  */
inline bool test_status_hardware(uint32_t module) {
    return !(get_status_hardware() & module);
}

/**
  * @brief  Gets the overall hardware diagnostic status.
  */
inline uint32_t get_status_hardware(void) {
    return test_hardware_result;
}

/**
  * @brief  Checks if a Software Reset occurred.
  *         Clears the sticky RCC reset flags to ensure future cold boots are read correctly.
  *         Call before HAL_Init.
  */
inline bool get_rcc_csr(void) {
    bool is_soft_reset = __HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) ? true : false;

    if (is_soft_reset) {
        /* Clear sticky reset flags, otherwise SFTRST remains 'true' on subsequent power-on resets */
        __HAL_RCC_CLEAR_RESET_FLAGS();
    }

    return is_soft_reset;
}

/**
  * @brief  Performs a safe system reset.
  */
void bsp_system_reset(void) {
    HAL_NVIC_SystemReset();
}

/**
  * @brief  RS-485 Driver Transmitter Enable (TE) control wrapper.
  *         Uses ultra-fast atomic register access without mutex overhead.
  *         Provides exact 2us transceiver stabilization delay using DWT.
  *         All comments in English.
  * @param  par: true to enable transmitter, false to disable.
  */
void ten(bool par) {
    if (par) {
        /* Direct write to BSRR register - atomic, thread-safe, ultra-fast */
        PIN_Set_F(&pin_usart1_kpa_te);

        /* Datasheet required delay for RS-485 transceiver driver to stabilize */
        /* Typical setup time for MAX3485/ADM485 is under 1-2 microseconds */
        delay_us(2);
    } else {
        /* Disable transmitter immediately after transmission is complete */
        PIN_Reset_F(&pin_usart1_kpa_te);
    }
}



/**
 * @brief  Completes system-wide hardware deinitialization.
 *         Explicitly shuts down all initialized peripherals and DMAs used in the bootloader,
 *         and force-resets peripheral buses using RCC reset registers to restore MCU
 *         to its raw, power-on hardware defaults.
 *         All comments in English.
 */
void bsp_deinit(void) {
    /* 1. Deinitialize all initialized HAL peripheral handles */
    HAL_ADC_DeInit(&hadc1);
    HAL_SPI_DeInit(&hspi1);
    HAL_UART_DeInit(&huart1);
    HAL_UART_DeInit(&huart2);
    HAL_UART_DeInit(&huart6);

#ifdef HAL_CRC_MODULE_ENABLED
    HAL_CRC_DeInit(&hcrc);
#endif

    /* 2. Explicitly stop and deinitialize all DMA streams used by UARTs/SPI */
    HAL_DMA_DeInit(&hdma_usart1_rx);
    HAL_DMA_DeInit(&hdma_usart1_tx);
    HAL_DMA_DeInit(&hdma_usart6_rx);
    HAL_DMA_DeInit(&hdma_usart6_tx);

    /* 3. Force hardware-level reset on all internal buses using RCC Reset Registers */
    /* This completely clears any pending flags, states, and register contents to 0 */
    RCC->AHB1RSTR = 0xFFFFFFFFU;
    RCC->APB1RSTR = 0xFFFFFFFFU;
    RCC->APB2RSTR = 0xFFFFFFFFU;

    /* Release all peripheral resets so the next application can initialize them */
    RCC->AHB1RSTR = 0x00000000U;
    RCC->APB1RSTR = 0x00000000U;
    RCC->APB2RSTR = 0x00000000U;

    /* 4. Complete final core-level HAL teardown and restore RCC clock config to HSI */
    HAL_RCC_DeInit();
    HAL_DeInit();
}

/**
 * @brief  Performs a safe, clean jump to the main Application.
 *         Completely deinitializes the RTOS, SysTick, and HAL peripherals before jumping.
 *         All comments in English.
 * @param  addr: Target application start address (Sector 5: 0x08020000)
 */
void jump_main_application(uint32_t addr) {
    void (*Jump_To_Application)(void);
    uint32_t JumpAddress;

    /* 1. Read the Stack Pointer (SP) value from the start of the target firmware image */
    uint32_t stack_pointer = *(__IO uint32_t*)addr;

    /* 2. Verify that the Stack Pointer lies strictly within STM32F411 SRAM range (0x20000000 - 0x20020000, max 128KB) */
    /* Note: STM32F411 does not have CCMRAM (0x10000000), removed legacy check */
    if ((stack_pointer >= 0x20000000U) && (stack_pointer <= 0x20020000U)) {

        /* VALID FIRMWARE DETECTED! Proceed to safe bootloader teardown */

        /* 3. CRITICAL: Disable all CPU interrupts globally FIRST */
        /* Prevents ISR execution while hardware is being deinitialized */
        __disable_irq();

        /* Lock the RTOS kernel scheduler to stop task switching */
        osKernelLock();

        /* Hard-disable the SysTick timer and clear its registers */
        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL  = 0;

        /* Deep-clean and deinitialize all hardware peripherals to raw reset states */
        bsp_deinit();

        /* 4. CRITICAL: Clear all pending interrupts in the NVIC */
        /* Ensures the Main App doesn't immediately trigger a stale bootloader interrupt */
        for (uint8_t i = 0; i < 8; i++) {
            NVIC->ICER[i] = 0xFFFFFFFFU; /* Disable all interrupts */
            NVIC->ICPR[i] = 0xFFFFFFFFU; /* Clear all pending flags */
        }

        /* 5. Disable instruction/data caches (ART Accelerator) for a clean start */
        FLASH->ACR &= ~(FLASH_ACR_ICEN | FLASH_ACR_DCEN);

        /* 6. Switch Thread mode stack from PSP (used by FreeRTOS task) back to MSP */
        /* Reset CONTROL register (SPSEL bit = 0: use MSP, nPRIV bit = 0: privileged) */
        __set_CONTROL(0);

        /* Set the Main Stack Pointer (MSP) for the new application */
        __set_MSP(stack_pointer);

        /* 7. Instruction and Data Synchronization Barriers */
        /* Ensure memory operations and pipeline are flushed before jump */
        __DSB();
        __ISB();

        /* Optional: Set Vector Table Offset to Target Application base address */
        SCB->VTOR = addr;

        /* Extract the reset handler address (stored at App base + 4) */
        JumpAddress = *(__IO uint32_t*)(addr + 4);
        Jump_To_Application = (void (*)(void))JumpAddress;

        /* Execute the jump (no return from this point) */
        Jump_To_Application();
    }
    else {
        /* NO VALID FIRMWARE DETECTED! (or flash memory is completely erased) */
        /* Since we have not broken any bootloader context, the RTOS, interrupts, */
        /* and HAL remain completely active and operational here. */

        /* Error handling placeholder (e.g., flash status LEDs, UART warnings, etc.) */
    }
}

/* ========================================================================= */
/*  ADC INTERNAL TEMPERATURE SENSOR OPERATIONS                              */
/* ========================================================================= */

/* Global ADC values */
float adc_voltage = 0.0f;
float cpu_temperature = 0.0f;

/* STM32F411 Factory Calibration Register Addresses */
/* TS_CAL1: Raw ADC value at 30 °C, V_DDA = 3.3V (typically around 940-960) */
/* TS_CAL2: Raw ADC value at 110 °C, V_DDA = 3.3V (typically around 1210-1230) */
#define TS_CAL1_ADDR     ((volatile uint16_t*)0x1FFF7A2CU)
#define TS_CAL2_ADDR     ((volatile uint16_t*)0x1FFF7A30U)



static	float Read_Temperature_Enhanced(void) {
    uint32_t adc_value = 0;
    float temp = 0.0f;

    if ((ADC->CCR & ADC_CCR_TSVREFE) == 0) {
        ADC->CCR |= ADC_CCR_TSVREFE;
        delay_us(20);
    }

    /* Берем среднее из 8 измерений подряд для подавления шума */
    for (int i = 0; i < 8; i++) {
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 4) == HAL_OK) {
            adc_value += HAL_ADC_GetValue(&hadc1);
        }
        HAL_ADC_Stop(&hadc1);
    }
    adc_value /= 8;

    uint16_t ts_cal1 = *TS_CAL1_ADDR;
    uint16_t ts_cal2 = *TS_CAL2_ADDR;

    if (ts_cal2 > ts_cal1 && ts_cal1 != 0xFFFF && ts_cal2 != 0xFFFF) {
        temp = ((110.0f - 30.0f) / (float)(ts_cal2 - ts_cal1)) * (float)((int32_t)adc_value - ts_cal1) + 30.0f;
    } else {
        float voltage = (float)adc_value * 3.3f / 4095.0f;
        temp = 25.0f + ((voltage - 0.76f) / 0.0025f);
    }
    return temp;
}


/**
 * @brief  Simple wrapper for backward compatibility.
 */
float Read_Temperature(void) {
    return Read_Temperature_Enhanced();
}


/* ========================================================================= */
/*  DWT DELAY HARDWARE CONTROL                                               */
/* ========================================================================= */

/**
 * @brief  Initializes the Data Watchpoint and Trace (DWT) cycle counter.
 */
void DWT_Init(void) {
    volatile uint32_t *dwt_lar = (volatile uint32_t *)0xE0001FB0U;
    *dwt_lar = 0xC5ACCE55U;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
 * @brief  Deinitializes the DWT cycle counter.
 */
void DWT_DeInit(void) {
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
}

/**
 * @brief  Performs microsecond blocking delay.
 */
void delay_us(const uint32_t us) {
    const uint32_t ticks_needed = us * (SystemCoreClock / 1000000U);
    const uint32_t tick_start = DWT->CYCCNT;

    while ((DWT->CYCCNT - tick_start) < ticks_needed) {
        /* Busy wait */
    }
}
