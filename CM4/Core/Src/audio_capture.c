#include "audio_capture.h"

#include "main.h"
#include "shared_audio.h"

#define AUDIO_DMA_EVENT_HALF 0U
#define AUDIO_DMA_EVENT_FULL 1U

static osMessageQueueId_t s_audio_queue = NULL;
static uint32_t s_ping_pong_index = 0U;

static int16_t s_dma_ping_buffer[SHARED_AUDIO_NUM_CHANNELS][SHARED_AUDIO_FRAME_SAMPLES];
static int16_t s_dma_pong_buffer[SHARED_AUDIO_NUM_CHANNELS][SHARED_AUDIO_FRAME_SAMPLES];

static void AudioCapture_CopyToShared(const int16_t src[SHARED_AUDIO_NUM_CHANNELS][SHARED_AUDIO_FRAME_SAMPLES])
{
  uint32_t ch;
  uint32_t i;

  for (ch = 0U; ch < SHARED_AUDIO_NUM_CHANNELS; ch++)
  {
    for (i = 0U; i < SHARED_AUDIO_FRAME_SAMPLES; i++)
    {
      g_shared_audio_frame.audio[ch][i] = src[ch][i];
    }
  }

  if (g_shared_audio_frame.ready != 0U)
  {
    g_shared_audio_frame.lost_frames++;
  }

  g_shared_audio_frame.frame_counter++;
  g_shared_audio_frame.ready = 1U;
}

void AudioCapture_Init(void)
{
  /* Placeholder for SAI + DMA init; fill with CubeMX-generated init later. */
}

void AudioCapture_Start(void)
{
  /* Placeholder for SAI DMA start; fill with HAL_SAI_Receive_DMA later. */
}

void AudioCapture_RegisterQueue(osMessageQueueId_t queue)
{
  s_audio_queue = queue;
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  (void)hsai;
  if (s_audio_queue != NULL)
  {
    uint32_t event = AUDIO_DMA_EVENT_HALF;
    osMessageQueuePut(s_audio_queue, &event, 0U, 0U);
  }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
  (void)hsai;
  if (s_audio_queue != NULL)
  {
    uint32_t event = AUDIO_DMA_EVENT_FULL;
    osMessageQueuePut(s_audio_queue, &event, 0U, 0U);
  }
}

void AudioCapture_ProcessEvent(uint32_t event)
{
  const int16_t (*src)[SHARED_AUDIO_FRAME_SAMPLES] = NULL;

  if (event == AUDIO_DMA_EVENT_HALF)
  {
    s_ping_pong_index ^= 1U;
  }

  if (s_ping_pong_index == 0U)
  {
    src = s_dma_ping_buffer;
  }
  else
  {
    src = s_dma_pong_buffer;
  }

  AudioCapture_CopyToShared(src);
}
