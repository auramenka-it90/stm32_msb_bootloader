/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications (Strictly within USER CODE blocks)
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "board_support_package.h"  /* Safe central registry include */
#include "pin_mgmt.h"               /* Safe Pin access include */
#include "terminal.h"               /* Safe Terminal control include */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
/* Extern declarations to access analog values from board_support_package.c */
extern float cpu_temperature;
extern float adc_voltage;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for TerminalTask */
osThreadId_t TerminalTaskHandle;
const osThreadAttr_t TerminalTask_attributes = {
  .name = "TerminalTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TemperatureTask */
osThreadId_t TemperatureTaskHandle;
const osThreadAttr_t TemperatureTask_attributes = {
  .name = "TemperatureTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTerminalTask(void *argument);
void StartTemperatureTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of TerminalTask */
  TerminalTaskHandle = osThreadNew(StartTerminalTask, NULL, &TerminalTask_attributes);

  /* creation of TemperatureTask */
  TemperatureTaskHandle = osThreadNew(StartTemperatureTask, NULL, &TemperatureTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */

	osThreadSuspend(TemperatureTaskHandle);
	osThreadSuspend(TerminalTaskHandle);

    /* Run board-level POST and hardware initialization */
	init_hardware();

    /* Cascade resume tasks based on POST diagnostic results */
	if (test_status_hardware(_B_FAULT_TERMINAL_)) {
		osThreadResume(TerminalTaskHandle);
	}

    /* CPU internal temperature is highly reliable, always resume */
	if (test_status_hardware(_B_FAULT_PINS_)) {
		osThreadResume(TemperatureTaskHandle);
	}

	for (;;) {
        /* Direct fast port toggle on Test Point 1 (No mutex overhead) */
		PIN_Toggle_F(&pin_tp1);
		osDelay(10);
	}
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTerminalTask */
/**
* @brief Function implementing the TerminalTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTerminalTask */
void StartTerminalTask(void *argument)
{
  /* USER CODE BEGIN StartTerminalTask */
  /* Infinite loop */
  for(;;)
  {
    terminal_task();
  }
  /* USER CODE END StartTerminalTask */
}

/* USER CODE BEGIN Header_StartTemperatureTask */
/**
* @brief Function implementing the TemperatureTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTemperatureTask */
void StartTemperatureTask(void *argument)
{
  /* USER CODE BEGIN StartTemperatureTask */
  /* Infinite loop */
  for(;;)
  {
    /* Read CPU internal core temperature every 100 ms */
    cpu_temperature = Read_Temperature();
    osDelay(100);
  }
  /* USER CODE END StartTemperatureTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

