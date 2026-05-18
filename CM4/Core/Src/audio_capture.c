#include "audio_capture.h"

#include "main.h"
#include "sai_a.h"
#include "shared_audio.h"

#define AUDIO_DMA_EVENT_HALF 0U
#define AUDIO_DMA_EVENT_FULL 1U
#define AUDIO_EVENT_BLOCK_A  0x10U
#define AUDIO_EVENT_BLOCK_B  0x20U

#define AUDIO_SAMPLE_RATE_HZ 16000U
#define AUDIO_FRAME_SAMPLES  SHARED_AUDIO_FRAME_SAMPLES
#define AUDIO_CHANNELS_USED  4U

#define AUDIO_SAI_WORD_SIZE_BITS 32U
#define AUDIO_SAI_SLOTS          2U
#define AUDIO_SAI_FRAME_BITS     (AUDIO_SAI_WORD_SIZE_BITS * AUDIO_SAI_SLOTS)

#define AUDIO_DMA_HALF_SAMPLES   (AUDIO_FRAME_SAMPLES * AUDIO_SAI_SLOTS)
#define AUDIO_DMA_TOTAL_SAMPLES  (AUDIO_DMA_HALF_SAMPLES * 2U)

static osMessageQueueId_t s_audio_queue = NULL;
static uint32_t s_ready_a = 0U;
static uint32_t s_ready_b = 0U;

static int32_t s_sai_a_dma[AUDIO_DMA_TOTAL_SAMPLES];
static int32_t s_sai_b_dma[AUDIO_DMA_TOTAL_SAMPLES];
static int16_t s_frame_buffer[SHARED_AUDIO_NUM_CHANNELS][SHARED_AUDIO_FRAME_SAMPLES];

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
  MX_SAI1_Init();
}

void AudioCapture_Start(void)
{
  (void)HAL_SAI_Receive_DMA(&hsai1_a, (uint8_t *)s_sai_a_dma, AUDIO_DMA_TOTAL_SAMPLES);
  (void)HAL_SAI_Receive_DMA(&hsai1_b, (uint8_t *)s_sai_b_dma, AUDIO_DMA_TOTAL_SAMPLES);
}

void AudioCapture_RegisterQueue(osMessageQueueId_t queue)
{
  s_audio_queue = queue;
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  uint32_t event = AUDIO_DMA_EVENT_HALF;

  if (hsai->Instance == SAI1_Block_A)
  {
    event |= AUDIO_EVENT_BLOCK_A;
  }
  else if (hsai->Instance == SAI1_Block_B)
  {
    event |= AUDIO_EVENT_BLOCK_B;
  }

  if (s_audio_queue != NULL)
  {
    osMessageQueuePut(s_audio_queue, &event, 0U, 0U);
  }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
  uint32_t event = AUDIO_DMA_EVENT_FULL;

  if (hsai->Instance == SAI1_Block_A)
  {
    event |= AUDIO_EVENT_BLOCK_A;
  }
  else if (hsai->Instance == SAI1_Block_B)
  {
    event |= AUDIO_EVENT_BLOCK_B;
  }

  if (s_audio_queue != NULL)
  {
    osMessageQueuePut(s_audio_queue, &event, 0U, 0U);
  }
}

static void AudioCapture_AssembleFrame(uint32_t half_index)
{
  uint32_t i;
  uint32_t base = half_index * AUDIO_DMA_HALF_SAMPLES;

  for (i = 0U; i < AUDIO_FRAME_SAMPLES; i++)
  {
    int32_t a_left = s_sai_a_dma[base + (i * 2U) + 0U];
    int32_t a_right = s_sai_a_dma[base + (i * 2U) + 1U];
    int32_t b_left = s_sai_b_dma[base + (i * 2U) + 0U];
    int32_t b_right = s_sai_b_dma[base + (i * 2U) + 1U];

    s_frame_buffer[0][i] = (int16_t)(a_left >> 8);
    s_frame_buffer[1][i] = (int16_t)(a_right >> 8);
    s_frame_buffer[2][i] = (int16_t)(b_left >> 8);
    s_frame_buffer[3][i] = (int16_t)(b_right >> 8);

    s_frame_buffer[4][i] = 0;
    s_frame_buffer[5][i] = 0;
    s_frame_buffer[6][i] = 0;
    s_frame_buffer[7][i] = 0;
  }

  AudioCapture_CopyToShared(s_frame_buffer);
}

void AudioCapture_ProcessEvent(uint32_t event)
{
  uint32_t is_full = (event & AUDIO_DMA_EVENT_FULL) != 0U;
  uint32_t half_index = is_full ? 1U : 0U;
  uint32_t mask = (half_index == 0U) ? 0x1U : 0x2U;

  if ((event & AUDIO_EVENT_BLOCK_A) != 0U)
  {
    s_ready_a |= mask;
  }
  if ((event & AUDIO_EVENT_BLOCK_B) != 0U)
  {
    s_ready_b |= mask;
  }

  if (((s_ready_a & mask) != 0U) && ((s_ready_b & mask) != 0U))
  {
    s_ready_a &= ~mask;
    s_ready_b &= ~mask;
    AudioCapture_AssembleFrame(half_index);
  }
}
