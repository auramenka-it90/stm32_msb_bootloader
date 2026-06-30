/**
  ******************************************************************************
  * @file    pin_mgmt.h
  * @brief   Pin Management Library for STM32 with Standard and Fast Pin Access.
  *          All comments in English.
  ******************************************************************************
  */

#ifndef __PIN_MGMT_H
#define __PIN_MGMT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "board_support_package.h"  /* Bring in BSP configuration and math */

/* Exported types ------------------------------------------------------------*/

/**
  * @brief  Pin descriptor structure
  */
typedef struct {
    GPIO_TypeDef* port;      /*!< GPIO port (GPIOA, GPIOB, etc.) */
    uint16_t pin;            /*!< Pin mask (GPIO_PIN_0, GPIO_PIN_1, etc.) */
    uint8_t default_state;   /*!< Default state (0: LOW, 1: HIGH) */
    const char* name;        /*!< Pin name for debugging */
} Pin_Descriptor_t;

/**
  * @brief  SPI bus configuration modes
  */
typedef enum {
    SPI_BUS_AF_MODE = 0,     /*!< Alternate Function mode (SPI peripheral) */
    SPI_BUS_HIZ_MODE         /*!< High impedance mode (tristate) */
} SPI_Bus_Mode_t;

/**
  * @brief  Library configuration structure
  */
typedef struct {
    uint8_t debug_enabled;   /*!< Enable debug output */
} Pin_Mgmt_Config_t;

/* Exported constants --------------------------------------------------------*/

/* Default configuration */
#define PIN_MGMT_CONFIG_DEFAULT { \
    .debug_enabled = 0 \
}

/* Macros for creating pin descriptors */
#define PIN_DESC(_port, _pin, _def_state, _name) \
    { .port = _port, .pin = _pin, .default_state = _def_state, .name = _name }

/* ========================================================================= */
/*  GLOBAL CONFIGURATION HANDLES (Now in driver header!)                      */
/* ========================================================================= */
extern Pin_Mgmt_Config_t bsp_pin_config; /* Global Pin Management configuration instance */

/* Exported functions --------------------------------------------------------*/

/* ==================== INITIALIZATION ==================== */
osStatus_t PIN_MGMT_Init(Pin_Mgmt_Config_t* config);
osStatus_t PIN_MGMT_DeInit(void);

/* ==================== STANDARD PIN OPERATIONS (MUTEX PROTECTED) ==================== */
osStatus_t PIN_Set(const Pin_Descriptor_t* pin);
osStatus_t PIN_Reset(const Pin_Descriptor_t* pin);
osStatus_t PIN_Toggle(const Pin_Descriptor_t* pin);
uint8_t PIN_Read(const Pin_Descriptor_t* pin);

/* ==================== FAST PIN OPERATIONS (NO MUTEX, FAST REGISTERS!) ==================== */
osStatus_t PIN_Set_F(const Pin_Descriptor_t* pin);
osStatus_t PIN_Reset_F(const Pin_Descriptor_t* pin);
osStatus_t PIN_Toggle_F(const Pin_Descriptor_t* pin);
uint8_t PIN_Read_F(const Pin_Descriptor_t* pin);

/* ==================== GPIO MUTEX OPERATIONS ==================== */
osStatus_t PIN_GPIO_Mutex_Acquire(uint32_t timeout);
osStatus_t PIN_GPIO_Mutex_Release(void);
uint8_t PIN_GPIO_Mutex_Is_Locked(void);

/* ==================== FPGA CONTROL ==================== */
osStatus_t FPGA_Reset_OS(uint32_t reset_time_ms);
osStatus_t FPGA_Wait_Ready(uint32_t timeout_ms);
uint8_t FPGA_Is_Ready(void);
uint8_t FPGA_Is_In_Reset(void);

/* ==================== SPI BUS CONTROL ==================== */
osStatus_t SPI_Bus_Configure(SPI_Bus_Mode_t mode);
osStatus_t SPI_Bus_Acquire_For_STM32(void);
osStatus_t SPI_Bus_Release_To_FPGA(void);
osStatus_t SPI_Bus_Hold_For_Reset(void);
uint8_t SPI_Bus_Is_Acquired(void);

/* ==================== STLINK DETECTION ==================== */
uint8_t STLINK_Is_Connected(void);

/* ==================== UTILITIES ==================== */
osStatus_t PIN_Delay(uint32_t ms);
osStatus_t PIN_Blink(const Pin_Descriptor_t* pin, uint8_t count, uint32_t delay_ms);

/* ==================== CONVENIENCE MACROS ==================== */

/* Standard Macros */
#define PIN_ON(pin)          PIN_Set(pin)
#define PIN_OFF(pin)         PIN_Reset(pin)
#define PIN_TOGGLE(pin)      PIN_Toggle(pin)
#define PIN_STATE(pin)       PIN_Read(pin)

/* Fast Macros */
#define PIN_ON_F(pin)        PIN_Set_F(pin)
#define PIN_OFF_F(pin)       PIN_Reset_F(pin)
#define PIN_TOGGLE_F(pin)    PIN_Toggle_F(pin)
#define PIN_STATE_F(pin)     PIN_Read_F(pin)

/* FPGA macros */
#define FPGA_RESET(time_ms)  FPGA_Reset_OS(time_ms)
#define FPGA_READY()         FPGA_Is_Ready()
#define FPGA_IN_RESET()      FPGA_Is_In_Reset()

/* SPI Bus Control macros */
#define SPI_BUS_ACQUIRE_STM32()     SPI_Bus_Acquire_For_STM32()
#define SPI_BUS_RELEASE_FPGA()      SPI_Bus_Release_To_FPGA()
#define SPI_BUS_HOLD_RESET()        SPI_Bus_Hold_For_Reset()
#define SPI_BUS_IS_ACQUIRED()       SPI_Bus_Is_Acquired()

/* LED macros */
#define LED_GREEN_ON()       PIN_Set(&pin_led_green)
#define LED_GREEN_OFF()      PIN_Reset(&pin_led_green)
#define LED_GREEN_TOGGLE()   PIN_Toggle(&pin_led_green)
#define LED_GREEN_ON_F()     PIN_Set_F(&pin_led_green)
#define LED_GREEN_OFF_F()    PIN_Reset_F(&pin_led_green)

/* Test Point macros (Fast variants are critical for logic analyzer debugging!) */
#define TP1_ON()             PIN_Set_F(&pin_tp1)
#define TP1_OFF()            PIN_Reset_F(&pin_tp1)
#define TP2_ON()             PIN_Set_F(&pin_tp2)
#define TP2_OFF()            PIN_Reset_F(&pin_tp2)
#define TP3_ON()             PIN_Set_F(&pin_tp3)
#define TP3_OFF()            PIN_Reset_F(&pin_tp3)
#define TP4_ON()             PIN_Set_F(&pin_tp4)
#define TP4_OFF()            PIN_Reset_F(&pin_tp4)

/* SPI Pins macros */
#define SPI_CSO_ON()         PIN_Set(&pin_spi_cso)
#define SPI_CSO_OFF()        PIN_Reset(&pin_spi_cso)
#define SPI_CSO_ON_F()       PIN_Set_F(&pin_spi_cso)  /* High-speed CS toggle */
#define SPI_CSO_OFF_F()      PIN_Reset_F(&pin_spi_cso)

/* Exported Pin descriptors - mapped to actual hardware ports */
extern const Pin_Descriptor_t pin_led_green;
extern const Pin_Descriptor_t pin_clk25mhz;
extern const Pin_Descriptor_t pin_tp1;
extern const Pin_Descriptor_t pin_tp2;
extern const Pin_Descriptor_t pin_tp3;
extern const Pin_Descriptor_t pin_tp4;
extern const Pin_Descriptor_t pin_fpga_done;
extern const Pin_Descriptor_t pin_usart2_fpga_tx;
extern const Pin_Descriptor_t pin_usart2_fpga_rx;
extern const Pin_Descriptor_t pin_spi_cso;
extern const Pin_Descriptor_t pin_spi_sck;
extern const Pin_Descriptor_t pin_spi_miso;
extern const Pin_Descriptor_t pin_spi_mosi;
extern const Pin_Descriptor_t pin_stm32_2_fpga_misc0;
extern const Pin_Descriptor_t pin_fpga_2_stm32_misc1;
extern const Pin_Descriptor_t pin_stm32_2_fpga_misc2;
extern const Pin_Descriptor_t pin_fpga_2_stm32_misc3;
extern const Pin_Descriptor_t pin_mr_fpga;
extern const Pin_Descriptor_t pin_mr_prog;
extern const Pin_Descriptor_t pin_usart6_debug_tx;
extern const Pin_Descriptor_t pin_usart6_debug_rx;
extern const Pin_Descriptor_t pin_usart1_kpa_te;
extern const Pin_Descriptor_t pin_usart1_kpa_tx;
extern const Pin_Descriptor_t pin_usart1_kpa_rx;
extern const Pin_Descriptor_t pin_stlink_detect;

#ifdef __cplusplus
}
#endif

#endif /* __PIN_MGMT_H */
