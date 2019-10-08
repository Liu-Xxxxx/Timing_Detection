/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "tim.h"
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
osThreadId SendPlcStaTaskHandle;
PLC_StaTypeDef plc;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osMessageQId PlcStaQueueHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void Send_PLC_StaTask(void const* argument);
void CleanTimeCnt(void);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

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

  /* Create the queue(s) */
  /* definition and creation of PlcStaQueue */
  osMessageQDef(PlcStaQueue, 45, PLC_StaTypeDef);
  PlcStaQueueHandle = osMessageCreate(osMessageQ(PlcStaQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  osThreadDef(SendPlcStaTask, Send_PLC_StaTask, osPriorityLow, 0, 128);
  SendPlcStaTaskHandle = osThreadCreate(osThread(SendPlcStaTask), NULL);
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
  vTaskDelete( NULL );
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void Send_PLC_StaTask(void const *argument)
{
  PLC_StaTypeDef plc;
  for (;;)
  {        
    TX_EN = 1;
    xQueueReceive(PlcStaQueueHandle,                    /* 消息队列句柄 */
                            (void *)&plc,   /* 存储接收到的数据到变量plc中 */
                            portMAX_DELAY); /* 设置阻塞时间 */
			#ifdef debug
			/* 打印一个由SystemView格式化的字符串. */
      SEGGER_SYSVIEW_OnTaskStartExec((unsigned)SendPlcStaTaskHandle);
      SEGGER_SYSVIEW_PrintfHost("0x%02X,%05d\r\n", plc.CAPTURE_STA,plc.CAPTURE_VAL);
			#endif
      /* 成功接收，并通过串口将数据打印出来 */
//      printf("%X,%d\r\n\r\n", plc.CAPTURE_STA,plc.CAPTURE_VAL); 
    TX_EN = 0;
  }
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  #ifdef debug
	SEGGER_SYSVIEW_RecordEnterISR();
  #endif
  uint16_t ExtiLine;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  ExtiLine=GPIO_Pin;
  if ((EXTI->RTSR & ExtiLine) != 0)
  {
    EXTI->RTSR &= ~ExtiLine;
    EXTI->FTSR |= ExtiLine;
    plc.CAPTURE_STA |= ExtiLine;
  }
  else if ((EXTI->FTSR & ExtiLine) != 0)
  {
    EXTI->FTSR &= ~ExtiLine;
    EXTI->RTSR |= ExtiLine;
    plc.CAPTURE_STA &= ~ExtiLine;
  }
  plc.CAPTURE_VAL = __HAL_TIM_GET_COUNTER(&htim2);
  xQueueSendFromISR(PlcStaQueueHandle,
                    (void *)&plc,
                    &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  #ifdef debug
	SEGGER_SYSVIEW_RecordExitISR();
  #endif
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
