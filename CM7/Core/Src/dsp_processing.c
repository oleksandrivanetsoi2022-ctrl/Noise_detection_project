#include "dsp_processing.h"

#include <string.h>

static float32_t s_window[DSP_FFT_SIZE];
static float32_t s_fft_in[DSP_FFT_SIZE];
static float32_t s_fft_out[DSP_FFT_SIZE];
static float32_t s_mag[DSP_FFT_SIZE / 2U];
static uint8_t s_window_ready = 0U;

static void DSP_InitWindow(void)
{
  uint32_t i;
  for (i = 0U; i < DSP_FFT_SIZE; i++)
  {
    float32_t phase = (2.0f * PI * (float32_t)i) / (float32_t)(DSP_FFT_SIZE - 1U);
    s_window[i] = 0.5f - 0.5f * arm_cos_f32(phase);
  }
  s_window_ready = 1U;
}

void Extract_MFCC(float32_t *pAudioBuffer, float32_t *pMfccOut)
{
  arm_rfft_fast_instance_f32 rfft;
  uint32_t i;
  uint32_t bin;

  if (!s_window_ready)
  {
    DSP_InitWindow();
  }

  for (i = 0U; i < DSP_FFT_SIZE; i++)
  {
    s_fft_in[i] = pAudioBuffer[i] * s_window[i];
  }

  (void)arm_rfft_fast_init_f32(&rfft, DSP_FFT_SIZE);
  arm_rfft_fast_f32(&rfft, s_fft_in, s_fft_out, 0U);

  for (bin = 0U; bin < (DSP_FFT_SIZE / 2U); bin++)
  {
    float32_t re = s_fft_out[2U * bin];
    float32_t im = s_fft_out[2U * bin + 1U];
    s_mag[bin] = arm_sqrt_f32((re * re) + (im * im));
  }

  for (i = 0U; i < DSP_MFCC_FEATURES; i++)
  {
    uint32_t start = (i * (DSP_FFT_SIZE / 2U)) / DSP_MFCC_FEATURES;
    uint32_t end = ((i + 1U) * (DSP_FFT_SIZE / 2U)) / DSP_MFCC_FEATURES;
    float32_t sum = 0.0f;

    for (bin = start; bin < end; bin++)
    {
      sum += s_mag[bin];
    }

    pMfccOut[i] = sum;
  }
}

uint16_t Calculate_DOA(const float32_t *pAudioBuffer, uint32_t length)
{
  (void)pAudioBuffer;
  (void)length;

  return 90U;
}
