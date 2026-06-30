/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_GREEN_Pin GPIO_PIN_13
#define LED_GREEN_GPIO_Port GPIOC
#define CLK25MHZ_Pin GPIO_PIN_0
#define CLK25MHZ_GPIO_Port GPIOH
#define TP1_Pin GPIO_PIN_0
#define TP1_GPIO_Port GPIOC
#define TP2_Pin GPIO_PIN_1
#define TP2_GPIO_Port GPIOC
#define TP3_Pin GPIO_PIN_2
#define TP3_GPIO_Port GPIOC
#define TP4_Pin GPIO_PIN_3
#define TP4_GPIO_Port GPIOC
#define DONE_Pin GPIO_PIN_1
#define DONE_GPIO_Port GPIOA
#define USART2_FPGA_TX_Pin GPIO_PIN_2
#define USART2_FPGA_TX_GPIO_Port GPIOA
#define USART2_FPGA_RX_Pin GPIO_PIN_3
#define USART2_FPGA_RX_GPIO_Port GPIOA
#define SP1_FPGA_CSO_Pin GPIO_PIN_4
#define SP1_FPGA_CSO_GPIO_Port GPIOA
#define SP1_FPGA_SCK_Pin GPIO_PIN_5
#define SP1_FPGA_SCK_GPIO_Port GPIOA
#define SPI1_FPGA_MISO_Pin GPIO_PIN_6
#define SPI1_FPGA_MISO_GPIO_Port GPIOA
#define SPI1_FPGA_MOSI_Pin GPIO_PIN_7
#define SPI1_FPGA_MOSI_GPIO_Port GPIOA
#define STM32_2_FPGA_MISC0_Pin GPIO_PIN_0
#define STM32_2_FPGA_MISC0_GPIO_Port GPIOB
#define FPGA_2_STM32_MISC1_Pin GPIO_PIN_1
#define FPGA_2_STM32_MISC1_GPIO_Port GPIOB
#define STM32_2_FPGA_MISC2_Pin GPIO_PIN_2
#define STM32_2_FPGA_MISC2_GPIO_Port GPIOB
#define FPGA_2_STM32_MISC3_Pin GPIO_PIN_10
#define FPGA_2_STM32_MISC3_GPIO_Port GPIOB
#define MR_FPGA_Pin GPIO_PIN_13
#define MR_FPGA_GPIO_Port GPIOB
#define MR_PROG_Pin GPIO_PIN_15
#define MR_PROG_GPIO_Port GPIOB
#define USART6_DEBUG_TX_Pin GPIO_PIN_6
#define USART6_DEBUG_TX_GPIO_Port GPIOC
#define USART6_DEBUG_RX_Pin GPIO_PIN_7
#define USART6_DEBUG_RX_GPIO_Port GPIOC
#define USART1_KPA_TE_Pin GPIO_PIN_8
#define USART1_KPA_TE_GPIO_Port GPIOA
#define USART1_KPA_TX_Pin GPIO_PIN_9
#define USART1_KPA_TX_GPIO_Port GPIOA
#define USART1_KPA_RX_Pin GPIO_PIN_10
#define USART1_KPA_RX_GPIO_Port GPIOA
#define STLINK_GND_TEST_Pin GPIO_PIN_7
#define STLINK_GND_TEST_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
