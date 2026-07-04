/**
 * ******************************************************************************
 * @file    gpio_fallbacks.h
 * @brief   Fallback definitions for all GPIO pins of the current project.
 *          Ensures each pin macro has a safe default value based on main.h.
 *          All comments in English as requested.
 * ******************************************************************************
 */

#ifndef GPIO_FALLBACKS_H
#define GPIO_FALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* LED_GREEN */
#ifndef LED_GREEN_Pin
#define LED_GREEN_Pin               GPIO_PIN_13
#endif
#ifndef LED_GREEN_GPIO_Port
#define LED_GREEN_GPIO_Port         GPIOC
#endif

/* CLK25MHZ */
#ifndef CLK25MHZ_Pin
#define CLK25MHZ_Pin                GPIO_PIN_0
#endif
#ifndef CLK25MHZ_GPIO_Port
#define CLK25MHZ_GPIO_Port          GPIOH
#endif

/* Test Points (TP1 - TP4) */
#ifndef TP1_Pin
#define TP1_Pin                     GPIO_PIN_0
#endif
#ifndef TP1_GPIO_Port
#define TP1_GPIO_Port               GPIOC
#endif

#ifndef TP2_Pin
#define TP2_Pin                     GPIO_PIN_1
#endif
#ifndef TP2_GPIO_Port
#define TP2_GPIO_Port               GPIOC
#endif

#ifndef TP3_Pin
#define TP3_Pin                     GPIO_PIN_2
#endif
#ifndef TP3_GPIO_Port
#define TP3_GPIO_Port               GPIOC
#endif

#ifndef TP4_Pin
#define TP4_Pin                     GPIO_PIN_3
#endif
#ifndef TP4_GPIO_Port
#define TP4_GPIO_Port               GPIOC
#endif

/* FPGA DONE Pin */
#ifndef DONE_Pin
#define DONE_Pin                    GPIO_PIN_1
#endif
#ifndef DONE_GPIO_Port
#define DONE_GPIO_Port              GPIOA
#endif

/* USART2 FPGA Communication (TX / RX) */
#ifndef USART2_FPGA_TX_Pin
#define USART2_FPGA_TX_Pin          GPIO_PIN_2
#endif
#ifndef USART2_FPGA_TX_GPIO_Port
#define USART2_FPGA_TX_GPIO_Port    GPIOA
#endif

#ifndef USART2_FPGA_RX_Pin
#define USART2_FPGA_RX_Pin          GPIO_PIN_3
#endif
#ifndef USART2_FPGA_RX_GPIO_Port
#define USART2_FPGA_RX_GPIO_Port    GPIOA
#endif

/* SPI1 FPGA Bus (CSO, SCK, MISO, MOSI) */
#ifndef SP1_FPGA_CSO_Pin
#define SP1_FPGA_CSO_Pin            GPIO_PIN_4
#endif
#ifndef SP1_FPGA_CSO_GPIO_Port
#define SP1_FPGA_CSO_GPIO_Port      GPIOA
#endif

#ifndef SP1_FPGA_SCK_Pin
#define SP1_FPGA_SCK_Pin            GPIO_PIN_5
#endif
#ifndef SP1_FPGA_SCK_GPIO_Port
#define SP1_FPGA_SCK_GPIO_Port      GPIOA
#endif

#ifndef SPI1_FPGA_MISO_Pin
#define SPI1_FPGA_MISO_Pin          GPIO_PIN_6
#endif
#ifndef SPI1_FPGA_MISO_GPIO_Port
#define SPI1_FPGA_MISO_GPIO_Port    GPIOA
#endif

#ifndef SPI1_FPGA_MOSI_Pin
#define SPI1_FPGA_MOSI_Pin          GPIO_PIN_7
#endif
#ifndef SPI1_FPGA_MOSI_GPIO_Port
#define SPI1_FPGA_MOSI_GPIO_Port    GPIOA
#endif

/* STM32 to FPGA MISC Communication lines */
#ifndef STM32_2_FPGA_MISC0_Pin
#define STM32_2_FPGA_MISC0_Pin      GPIO_PIN_0
#endif
#ifndef STM32_2_FPGA_MISC0_GPIO_Port
#define STM32_2_FPGA_MISC0_GPIO_Port GPIOB
#endif

#ifndef FPGA_2_STM32_MISC1_Pin
#define FPGA_2_STM32_MISC1_Pin      GPIO_PIN_1
#endif
#ifndef FPGA_2_STM32_MISC1_GPIO_Port
#define FPGA_2_STM32_MISC1_GPIO_Port GPIOB
#endif

#ifndef STM32_2_FPGA_MISC2_Pin
#define STM32_2_FPGA_MISC2_Pin      GPIO_PIN_2
#endif
#ifndef STM32_2_FPGA_MISC2_GPIO_Port
#define STM32_2_FPGA_MISC2_GPIO_Port GPIOB
#endif

#ifndef FPGA_2_STM32_MISC3_Pin
#define FPGA_2_STM32_MISC3_Pin      GPIO_PIN_10
#endif
#ifndef FPGA_2_STM32_MISC3_GPIO_Port
#define FPGA_2_STM32_MISC3_GPIO_Port GPIOB
#endif

/* Master Reset (MR) Lines */
#ifndef MR_FPGA_Pin
#define MR_FPGA_Pin                 GPIO_PIN_13
#endif
#ifndef MR_FPGA_GPIO_Port
#define MR_FPGA_GPIO_Port           GPIOB
#endif

#ifndef MR_PROG_Pin
#define MR_PROG_Pin                 GPIO_PIN_15
#endif
#ifndef MR_PROG_GPIO_Port
#define MR_PROG_GPIO_Port           GPIOB
#endif

/* USART6 Debug port */
#ifndef USART6_DEBUG_TX_Pin
#define USART6_DEBUG_TX_Pin         GPIO_PIN_6
#endif
#ifndef USART6_DEBUG_TX_GPIO_Port
#define USART6_DEBUG_TX_GPIO_Port   GPIOC
#endif

#ifndef USART6_DEBUG_RX_Pin
#define USART6_DEBUG_RX_Pin         GPIO_PIN_7
#endif
#ifndef USART6_DEBUG_RX_GPIO_Port
#define USART6_DEBUG_RX_GPIO_Port   GPIOC
#endif

/* USART1 KPA */
#ifndef USART1_KPA_TE_Pin
#define USART1_KPA_TE_Pin           GPIO_PIN_8
#endif
#ifndef USART1_KPA_TE_GPIO_Port
#define USART1_KPA_TE_GPIO_Port     GPIOA
#endif

#ifndef USART1_KPA_TX_Pin
#define USART1_KPA_TX_Pin           GPIO_PIN_9
#endif
#ifndef USART1_KPA_TX_GPIO_Port
#define USART1_KPA_TX_GPIO_Port     GPIOA
#endif

#ifndef USART1_KPA_RX_Pin
#define USART1_KPA_RX_Pin           GPIO_PIN_10
#endif
#ifndef USART1_KPA_RX_GPIO_Port
#define USART1_KPA_RX_GPIO_Port     GPIOA
#endif

/* STLINK Test Pin */
#ifndef STLINK_GND_TEST_Pin
#define STLINK_GND_TEST_Pin         GPIO_PIN_7
#endif
#ifndef STLINK_GND_TEST_GPIO_Port
#define STLINK_GND_TEST_GPIO_Port   GPIOB
#endif

#ifdef __cplusplus
}
#endif

#endif /* GPIO_FALLBACKS_H */
