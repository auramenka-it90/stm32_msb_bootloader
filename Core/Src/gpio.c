/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED_GREEN_Pin|TP1_Pin|TP2_Pin|TP3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SP1_FPGA_CSO_GPIO_Port, SP1_FPGA_CSO_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, STM32_2_FPGA_MISC0_Pin|STM32_2_FPGA_MISC2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MR_PROG_GPIO_Port, MR_PROG_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USART1_KPA_TE_GPIO_Port, USART1_KPA_TE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_GREEN_Pin TP1_Pin TP2_Pin TP3_Pin */
  GPIO_InitStruct.Pin = LED_GREEN_Pin|TP1_Pin|TP2_Pin|TP3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : TP4_Pin */
  GPIO_InitStruct.Pin = TP4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TP4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DONE_Pin */
  GPIO_InitStruct.Pin = DONE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DONE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SP1_FPGA_CSO_Pin */
  GPIO_InitStruct.Pin = SP1_FPGA_CSO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SP1_FPGA_CSO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : STM32_2_FPGA_MISC0_Pin STM32_2_FPGA_MISC2_Pin MR_PROG_Pin */
  GPIO_InitStruct.Pin = STM32_2_FPGA_MISC0_Pin|STM32_2_FPGA_MISC2_Pin|MR_PROG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : FPGA_2_STM32_MISC1_Pin MR_FPGA_Pin STLINK_GND_TEST_Pin */
  GPIO_InitStruct.Pin = FPGA_2_STM32_MISC1_Pin|MR_FPGA_Pin|STLINK_GND_TEST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : FPGA_2_STM32_MISC3_Pin */
  GPIO_InitStruct.Pin = FPGA_2_STM32_MISC3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(FPGA_2_STM32_MISC3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USART1_KPA_TE_Pin */
  GPIO_InitStruct.Pin = USART1_KPA_TE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USART1_KPA_TE_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
