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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "arm_math.h"
#include "ai_inference.h"
#include "dsp_processing.h"
#include "lora_app.h"
#include "../../../Common/Inc/shared_audio.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define DSP_VAD_ENERGY_THRESHOLD  500.0f
#define DSP_VAD_ON_FRAMES         3U
#define DSP_VAD_OFF_FRAMES        5U
#define DSP_MFCC_EWMA_ALPHA       0.98f
#define DSP_MFCC_SMOOTH_ALPHA     0.85f
#define DSP_MFCC_NORM_EPS         1.0e-6f

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

static float s_mfcc_mean[DSP_MFCC_FEATURES];
static float s_mfcc_var[DSP_MFCC_FEATURES];
static float s_mfcc_smooth[DSP_MFCC_FEATURES];
static uint8_t s_mfcc_stats_ready = 0U;

typedef struct
{
  uint8_t on_count;
  uint8_t off_count;
  uint8_t state;
  uint8_t on_frames;
  uint8_t off_frames;
} VadState;

static VadState s_vad_state = {
  .on_count = 0U,
  .off_count = 0U,
  .state = 0U,
  .on_frames = DSP_VAD_ON_FRAMES,
  .off_frames = DSP_VAD_OFF_FRAMES
};

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

static void DSP_AI_Task(void *argument);
static void TelemetryTask(void *argument);

/* USER CODE END FunctionPrototypes */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

static void DSP_InvalidateSharedAudio(void)
{
  uint32_t addr = (uint32_t)&g_shared_audio_frame;
  uint32_t size = sizeof(g_shared_audio_frame);
  uint32_t start = addr & ~31U;
  uint32_t end = addr + size;
  uint32_t length = end - start;

  SCB_InvalidateDCache_by_Addr((uint32_t *)start, (int32_t)length);
}

static uint8_t DSP_VadIsActive(const float32_t *audio_frame)
{
  float32_t energy = 0.0f;
  arm_power_f32(audio_frame, DSP_FFT_SIZE, &energy);
  energy = energy / (float32_t)DSP_FFT_SIZE;
  return (energy > DSP_VAD_ENERGY_THRESHOLD) ? 1U : 0U;
}

static uint8_t DSP_VadDebounce(VadState *state, uint8_t vad_raw)
{
  if (vad_raw != 0U)
  {
    if (state->on_count < 255U)
    {
      state->on_count++;
    }
    state->off_count = 0U;

    if (state->on_count >= state->on_frames)
    {
      state->state = 1U;
    }
  }
  else
  {
    if (state->off_count < 255U)
    {
      state->off_count++;
    }
    state->on_count = 0U;

    if (state->off_count >= state->off_frames)
    {
      state->state = 0U;
    }
  }

  return state->state;
}

static void DSP_UpdateMfccStats(const float32_t *mfcc)
{
  uint32_t i;
  float32_t alpha = DSP_MFCC_EWMA_ALPHA;

  if (s_mfcc_stats_ready == 0U)
  {
    for (i = 0U; i < DSP_MFCC_FEATURES; i++)
    {
      s_mfcc_mean[i] = mfcc[i];
      s_mfcc_var[i] = 1.0f;
      s_mfcc_smooth[i] = mfcc[i];
    }
    s_mfcc_stats_ready = 1U;
    return;
  }

  for (i = 0U; i < DSP_MFCC_FEATURES; i++)
  {
    float32_t delta = mfcc[i] - s_mfcc_mean[i];
    s_mfcc_mean[i] = alpha * s_mfcc_mean[i] + (1.0f - alpha) * mfcc[i];
    s_mfcc_var[i] = alpha * s_mfcc_var[i] + (1.0f - alpha) * (delta * delta);
  }
}

static void DSP_NormalizeAndSmooth(const float32_t *mfcc_in, float32_t *mfcc_out)
{
  uint32_t i;
  float32_t smooth_alpha = DSP_MFCC_SMOOTH_ALPHA;

  for (i = 0U; i < DSP_MFCC_FEATURES; i++)
  {
    float32_t denom = arm_sqrt_f32(s_mfcc_var[i] + DSP_MFCC_NORM_EPS);
    float32_t norm = (mfcc_in[i] - s_mfcc_mean[i]) / denom;
    s_mfcc_smooth[i] = smooth_alpha * s_mfcc_smooth[i] + (1.0f - smooth_alpha) * norm;
    mfcc_out[i] = s_mfcc_smooth[i];
  }
}

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
  /* long buffer for 2s clips used by log-mel frontend */
  static float32_t long_audio[DSP_LM_SAMPLES_PER_CLIP];
  static uint32_t long_pos = 0U;
  static float32_t lm_features[DSP_LM_FEATURES];
  TelemetryMessage telemetry_msg;
  uint8_t vad_raw;
  uint8_t vad_active;
  uint32_t i;

  (void)argument;

  for (;;)
  {
    if (g_shared_audio_frame.ready == 0U)
    {
      osDelay(1U);
      continue;
    }

    DSP_InvalidateSharedAudio();

    for (i = 0U; i < DSP_FFT_SIZE; i++)
    {
      audio_float[i] = (float32_t)g_shared_audio_frame.audio[0][i];
    }

    /* accumulate into long buffer (non-overlapping) */
    if (long_pos + DSP_FFT_SIZE <= DSP_LM_SAMPLES_PER_CLIP)
    {
      for (i = 0U; i < DSP_FFT_SIZE; i++)
      {
        long_audio[long_pos + i] = audio_float[i];
      }
      long_pos += DSP_FFT_SIZE;
    }

    g_shared_audio_frame.ready = 0U;

    vad_raw = DSP_VadIsActive(audio_float);
    vad_active = DSP_VadDebounce(&s_vad_state, vad_raw);
    Extract_MFCC(audio_float, mfcc_msg.mfcc);

    DSP_UpdateMfccStats(mfcc_msg.mfcc);
    DSP_NormalizeAndSmooth(mfcc_msg.mfcc, mfcc_msg.mfcc);

    /* If VAD active and we have a full 2s buffer, run log-mel frontend + inference */
    if ((vad_active != 0U) && (long_pos >= DSP_LM_SAMPLES_PER_CLIP))
    {
      /* produce 64x63 log-mel features */
      Extract_LogMelSpectrogram(long_audio, lm_features);
      Run_Audio_Inference(lm_features, &telemetry_msg.predicted_class, &telemetry_msg.confidence);
      telemetry_msg.doa = Calculate_DOA(audio_float, DSP_FFT_SIZE);
      (void)osMessageQueuePut(g_telemetry_queue, &telemetry_msg, 0U, 0U);
      /* reset buffer for next capture */
      long_pos = 0U;
    }
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

