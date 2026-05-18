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
#include "cmsis_os2.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "ai_inference.h"
#include "dsp_processing.h"
#include "lora_app.h"
#include "shared_audio.h"

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

typedef struct
{
  float mfcc[DSP_MFCC_FEATURES];
} MfccMessage;

typedef struct
{
  uint8_t predicted_class;
  float confidence;
  uint16_t doa;
} TelemetryMessage;

osMessageQueueId_t g_telemetry_queue;

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

static void DSP_AI_Task(void *argument);
static void TelemetryTask(void *argument);

/* USER CODE END FunctionPrototypes */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void MX_FREERTOS_Init(void)
{
  const osThreadAttr_t dsp_task_attr = {
    .name = "DSP_AI_Task",
    .priority = (osPriority_t)osPriorityHigh,
    .stack_size = 2048U
  };

  const osThreadAttr_t telemetry_task_attr = {
    .name = "TelemetryTask",
    .priority = (osPriority_t)osPriorityNormal,
    .stack_size = 1024U
  };

  g_telemetry_queue = osMessageQueueNew(4U, sizeof(TelemetryMessage), NULL);

  (void)osThreadNew(DSP_AI_Task, NULL, &dsp_task_attr);
  (void)osThreadNew(TelemetryTask, NULL, &telemetry_task_attr);
}

static void DSP_AI_Task(void *argument)
{
  float32_t audio_float[DSP_FFT_SIZE];
  MfccMessage mfcc_msg;
  TelemetryMessage telemetry_msg;
  uint32_t i;

  (void)argument;

  for (;;)
  {
    if (g_shared_audio_frame.ready == 0U)
    {
      osDelay(1U);
      continue;
    }

    /* If DCache is enabled, clean/invalidate shared buffers before use. */
    for (i = 0U; i < DSP_FFT_SIZE; i++)
    {
      audio_float[i] = (float32_t)g_shared_audio_frame.audio[0][i];
    }

    g_shared_audio_frame.ready = 0U;

    Extract_MFCC(audio_float, mfcc_msg.mfcc);
    Run_Audio_Inference(mfcc_msg.mfcc, &telemetry_msg.predicted_class, &telemetry_msg.confidence);
    telemetry_msg.doa = Calculate_DOA(audio_float, DSP_FFT_SIZE);

    (void)osMessageQueuePut(g_telemetry_queue, &telemetry_msg, 0U, 0U);
  }
}

static void TelemetryTask(void *argument)
{
  TelemetryMessage telemetry_msg;

  (void)argument;

  for (;;)
  {
    if (osMessageQueueGet(g_telemetry_queue, &telemetry_msg, NULL, osWaitForever) == osOK)
    {
      LoRa_Pack_And_Send(telemetry_msg.predicted_class, telemetry_msg.confidence, telemetry_msg.doa);
    }
  }
}

/* USER CODE END Application */

