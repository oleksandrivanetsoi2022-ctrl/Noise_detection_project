/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "audio_capture.h"

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

osMessageQueueId_t g_audio_dma_queue;

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

static void AudioAcquisitionTask(void *argument);

/* USER CODE END FunctionPrototypes */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void MX_FREERTOS_Init(void)
{
  const osThreadAttr_t audio_task_attr = {
    .name = "AudioAcquisitionTask",
    .priority = (osPriority_t)osPriorityHigh,
    .stack_size = 1024U
  };

  g_audio_dma_queue = osMessageQueueNew(8U, sizeof(uint32_t), NULL);
  AudioCapture_RegisterQueue(g_audio_dma_queue);

  AudioCapture_Init();
  AudioCapture_Start();

  (void)osThreadNew(AudioAcquisitionTask, NULL, &audio_task_attr);
}

static void AudioAcquisitionTask(void *argument)
{
  uint32_t event = 0U;

  (void)argument;

  for (;;)
  {
    if (osMessageQueueGet(g_audio_dma_queue, &event, NULL, osWaitForever) == osOK)
    {
      AudioCapture_ProcessEvent(event);
    }
  }
}

/* USER CODE END Application */

