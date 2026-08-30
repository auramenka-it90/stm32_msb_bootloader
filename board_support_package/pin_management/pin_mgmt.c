/**
  ******************************************************************************
  * @file    pin_mgmt.c
  * @brief   Pin Management Library Implementation.
  *          Features Standard (Mutex Protected) and Fast (Register Direct) I/O.
  *          All comments in English.
  ******************************************************************************
  */

#include "pin_mgmt.h"

/* Global structures instantiation */
Pin_Mgmt_Config_t bsp_pin_config = PIN_MGMT_CONFIG_DEFAULT;

/* Private variables ---------------------------------------------------------*/
static osMutexId_t gpio_mutex = NULL;

/* Pin descriptors */
const Pin_Descriptor_t pin_led_green = PIN_DESC(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 0, "LED_GREEN");
const Pin_Descriptor_t pin_clk25mhz  = PIN_DESC(CLK25MHZ_GPIO_Port, CLK25MHZ_Pin, 0, "CLK25MHZ");

const Pin_Descriptor_t pin_tp1        = PIN_DESC(TP1_GPIO_Port, TP1_Pin, 0, "TP1");
const Pin_Descriptor_t pin_tp2        = PIN_DESC(TP2_GPIO_Port, TP2_Pin, 0, "TP2");
const Pin_Descriptor_t pin_tp3        = PIN_DESC(TP3_GPIO_Port, TP3_Pin, 0, "TP3");
const Pin_Descriptor_t pin_tp4        = PIN_DESC(TP4_GPIO_Port, TP4_Pin, 0, "TP4");

const Pin_Descriptor_t pin_fpga_done  = PIN_DESC(DONE_GPIO_Port, DONE_Pin, 0, "FPGA_DONE");

const Pin_Descriptor_t pin_usart2_fpga_tx = PIN_DESC(USART2_FPGA_TX_GPIO_Port, USART2_FPGA_TX_Pin, 0, "USART2_FPGA_TX");
const Pin_Descriptor_t pin_usart2_fpga_rx = PIN_DESC(USART2_FPGA_RX_GPIO_Port, USART2_FPGA_RX_Pin, 0, "USART2_FPGA_RX");

const Pin_Descriptor_t pin_spi_cso   = PIN_DESC(SP1_FPGA_CSO_GPIO_Port, SP1_FPGA_CSO_Pin, 1, "SPI_CSO");
const Pin_Descriptor_t pin_spi_sck   = PIN_DESC(SP1_FPGA_SCK_GPIO_Port, SP1_FPGA_SCK_Pin, 0, "SPI_SCK");
const Pin_Descriptor_t pin_spi_miso  = PIN_DESC(SPI1_FPGA_MISO_GPIO_Port, SPI1_FPGA_MISO_Pin, 0, "SPI_MISO");
const Pin_Descriptor_t pin_spi_mosi  = PIN_DESC(SPI1_FPGA_MOSI_GPIO_Port, SPI1_FPGA_MOSI_Pin, 0, "SPI_MOSI");

const Pin_Descriptor_t pin_stm32_2_fpga_misc0 = PIN_DESC(STM32_2_FPGA_MISC0_GPIO_Port, STM32_2_FPGA_MISC0_Pin, 0, "STM32_MISC0");
const Pin_Descriptor_t pin_fpga_2_stm32_misc1 = PIN_DESC(FPGA_2_STM32_MISC1_GPIO_Port, FPGA_2_STM32_MISC1_Pin, 0, "FPGA_MISC1");
const Pin_Descriptor_t pin_stm32_2_fpga_misc2 = PIN_DESC(STM32_2_FPGA_MISC2_GPIO_Port, STM32_2_FPGA_MISC2_Pin, 0, "STM32_MISC2");
const Pin_Descriptor_t pin_fpga_2_stm32_misc3 = PIN_DESC(FPGA_2_STM32_MISC3_GPIO_Port, FPGA_2_STM32_MISC3_Pin, 0, "FPGA_MISC3");

const Pin_Descriptor_t pin_mr_fpga   = PIN_DESC(MR_FPGA_GPIO_Port, MR_FPGA_Pin, 0, "MR_FPGA");
const Pin_Descriptor_t pin_mr_prog   = PIN_DESC(MR_PROG_GPIO_Port, MR_PROG_Pin, 1, "MR_PROG");

const Pin_Descriptor_t pin_usart6_debug_tx = PIN_DESC(USART6_DEBUG_TX_GPIO_Port, USART6_DEBUG_TX_Pin, 0, "USART6_DEBUG_TX");
const Pin_Descriptor_t pin_usart6_debug_rx = PIN_DESC(USART6_DEBUG_RX_GPIO_Port, USART6_DEBUG_RX_Pin, 0, "USART6_DEBUG_RX");

const Pin_Descriptor_t pin_usart1_kpa_te = PIN_DESC(USART1_KPA_TE_GPIO_Port, USART1_KPA_TE_Pin, 0, "USART1_KPA_TE");
const Pin_Descriptor_t pin_usart1_kpa_tx = PIN_DESC(USART1_KPA_TX_GPIO_Port, USART1_KPA_TX_Pin, 0, "USART1_KPA_TX");
const Pin_Descriptor_t pin_usart1_kpa_rx = PIN_DESC(USART1_KPA_RX_GPIO_Port, USART1_KPA_RX_Pin, 0, "USART1_KPA_RX");

const Pin_Descriptor_t pin_stlink_detect = PIN_DESC(STLINK_GND_TEST_GPIO_Port, STLINK_GND_TEST_Pin, 1, "STLINK_DETECT");

/* Private function prototypes -----------------------------------------------*/
static osStatus_t _create_gpio_mutex(void);
static osStatus_t _delete_gpio_mutex(void);

/* Private functions implementation ------------------------------------------*/

static osStatus_t _create_gpio_mutex(void)
{
    if (gpio_mutex == NULL) {
        const osMutexAttr_t mutex_attr = {
            .name = "PIN_MGMT_GPIO_Mutex",
            .attr_bits = osMutexRecursive | osMutexPrioInherit,
            .cb_mem = NULL,
            .cb_size = 0
        };
        gpio_mutex = osMutexNew(&mutex_attr);
        if (gpio_mutex == NULL) {
            return osErrorResource;
        }
    }
    return osOK;
}

static osStatus_t _delete_gpio_mutex(void)
{
    if (gpio_mutex != NULL) {
        osMutexDelete(gpio_mutex);
        gpio_mutex = NULL;
    }
    return osOK;
}

/* Public API Functions ------------------------------------------------------*/

osStatus_t PIN_MGMT_Init(Pin_Mgmt_Config_t* config)
{
    if (config) {
        bsp_pin_config = *config;
    }
    return _create_gpio_mutex();
}

osStatus_t PIN_MGMT_DeInit(void)
{
    _delete_gpio_mutex();
    return osOK;
}

/* ========================================================================= */
/*  STANDARD PIN OPERATIONS (MUTEX PROTECTED)                                */
/* ========================================================================= */

osStatus_t PIN_Set(const Pin_Descriptor_t* pin)
{
    if (!pin || !pin->port) return osErrorParameter;

    osStatus_t status = PIN_GPIO_Mutex_Acquire(osWaitForever);
    if (status != osOK) return status;

    HAL_GPIO_WritePin(pin->port, pin->pin, GPIO_PIN_SET);

    PIN_GPIO_Mutex_Release();
    return osOK;
}

osStatus_t PIN_Reset(const Pin_Descriptor_t* pin)
{
    if (!pin || !pin->port) return osErrorParameter;

    osStatus_t status = PIN_GPIO_Mutex_Acquire(osWaitForever);
    if (status != osOK) return status;

    HAL_GPIO_WritePin(pin->port, pin->pin, GPIO_PIN_RESET);

    PIN_GPIO_Mutex_Release();
    return osOK;
}

osStatus_t PIN_Toggle(const Pin_Descriptor_t* pin)
{
    if (!pin || !pin->port) return osErrorParameter;

    osStatus_t status = PIN_GPIO_Mutex_Acquire(osWaitForever);
    if (status != osOK) return status;

    HAL_GPIO_TogglePin(pin->port, pin->pin);

    PIN_GPIO_Mutex_Release();
    return osOK;
}

uint8_t PIN_Read(const Pin_Descriptor_t* pin)
{
    if (!pin || !pin->port) return 0;

    uint8_t state = 0;
    if (PIN_GPIO_Mutex_Acquire(osWaitForever) == osOK) {
        state = (HAL_GPIO_ReadPin(pin->port, pin->pin) == GPIO_PIN_SET) ? 1 : 0;
        PIN_GPIO_Mutex_Release();
    }
    return state;
}

/* ========================================================================= */
/*  FAST PIN OPERATIONS (NO MUTEX, DIRECT REGISTER ACCESS)                    */
/* ========================================================================= */

osStatus_t PIN_Set_F(const Pin_Descriptor_t* pin)
{
    if (!pin || !pin->port) return osErrorParameter;
    pin->port->BSRR = pin->pin;
    return osOK;
}

osStatus_t PIN_Reset_F(const Pin_Descriptor_t* pin)
{
    if (!pin || !pin->port) return osErrorParameter;
    pin->port->BSRR = (uint32_t)pin->pin << 16U;
    return osOK;
}

osStatus_t PIN_Toggle_F(const Pin_Descriptor_t* pin)
{
    if (!pin || !pin->port) return osErrorParameter;

    /* Atomic BSRR-based toggle to prevent RMW corruption of adjacent pins on the same port */
    uint32_t odr = pin->port->ODR;
    if (odr & pin->pin) {
        pin->port->BSRR = (uint32_t)pin->pin << 16U; /* Reset pin */
    } else {
        pin->port->BSRR = pin->pin;                   /* Set pin */
    }
    return osOK;
}

uint8_t PIN_Read_F(const Pin_Descriptor_t* pin)
{
    if (!pin || !pin->port) return 0;
    return (pin->port->IDR & pin->pin) ? 1 : 0;
}

/* ========================================================================= */
/*  MUTEX OPERATIONS                                                         */
/* ========================================================================= */

osStatus_t PIN_GPIO_Mutex_Acquire(uint32_t timeout)
{
    if (gpio_mutex == NULL) {
        osStatus_t status = _create_gpio_mutex();
        if (status != osOK) {
            return status;
        }
    }
    return osMutexAcquire(gpio_mutex, timeout);
}

osStatus_t PIN_GPIO_Mutex_Release(void)
{
    if (gpio_mutex == NULL) {
        return osErrorResource;
    }
    return osMutexRelease(gpio_mutex);
}

uint8_t PIN_GPIO_Mutex_Is_Locked(void)
{
    if (gpio_mutex == NULL) {
        return 0;
    }

    osStatus_t status = osMutexAcquire(gpio_mutex, 0);
    if (status == osOK) {
        osMutexRelease(gpio_mutex);
        return 0;
    }
    return 1;
}

/* ========================================================================= */
/*  STLINK DETECTION & UTILITIES                                             */
/* ========================================================================= */

uint8_t STLINK_Is_Connected(void)
{
    /* Use fast atomic read: zero overhead and safe to call before RTOS scheduler starts */
    return (PIN_Read_F(&pin_stlink_detect) == 0) ? 1 : 0;
}

osStatus_t PIN_Delay(uint32_t ms)
{
    return osDelay(ms);
}

osStatus_t PIN_Blink(const Pin_Descriptor_t* pin, uint8_t count, uint32_t delay_ms)
{
    if (!pin) return osErrorParameter;
    
    for (uint8_t i = 0; i < count; i++) {
        PIN_Set(pin);
        PIN_Delay(delay_ms);
        PIN_Reset(pin);
        PIN_Delay(delay_ms);
    }
    
    return osOK;
}
