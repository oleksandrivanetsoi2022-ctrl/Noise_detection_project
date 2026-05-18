#include "dsp_processing.h"

#include <math.h>
#include <string.h>

static float32_t s_window[DSP_FFT_SIZE];
static float32_t s_fft_in[DSP_FFT_SIZE];
static float32_t s_fft_out[DSP_FFT_SIZE];
static float32_t s_mag[DSP_FFT_SIZE / 2U];
static float32_t s_power[DSP_FFT_SIZE / 2U];
static float32_t s_mel_energies[DSP_MEL_BINS];
static float32_t s_dct_out[DSP_DCT_BINS];
static float32_t s_preemph[DSP_FFT_SIZE];
static float32_t s_corr[2U * DSP_DOA_MAX_SAMPLES - 1U];
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

static void DSP_PreEmphasis(const float32_t *input, float32_t *output)
{
  const float32_t alpha = 0.97f;
  uint32_t i;

  output[0] = input[0];
  for (i = 1U; i < DSP_FFT_SIZE; i++)
  {
    output[i] = input[i] - alpha * input[i - 1U];
  }
}

static void DSP_ComputeMelBank(const float32_t *power_spectrum, float32_t *mel_out)
{
  uint32_t i;
  uint32_t bin;
  const uint32_t bins = DSP_FFT_SIZE / 2U;

  for (i = 0U; i < DSP_MEL_BINS; i++)
  {
    float32_t sum = 0.0f;
    uint32_t start = (i * bins) / DSP_MEL_BINS;
    uint32_t center = ((i + 1U) * bins) / DSP_MEL_BINS;
    uint32_t end = ((i + 2U) * bins) / DSP_MEL_BINS;

    if (center > bins)
    {
      center = bins;
    }
    if (end > bins)
    {
      end = bins;
    }

    for (bin = start; bin < end; bin++)
    {
      float32_t w;
      if (bin <= center)
      {
        w = (float32_t)(bin - start) / (float32_t)((center - start) + 1U);
      }
      else
      {
        w = (float32_t)(end - bin) / (float32_t)((end - center) + 1U);
      }
      sum += power_spectrum[bin] * w;
    }

    mel_out[i] = sum;
  }
}

static void DSP_DCT(const float32_t *mel_in, float32_t *dct_out)
{
  uint32_t k;
  uint32_t n;

  for (k = 0U; k < DSP_DCT_BINS; k++)
  {
    float32_t acc = 0.0f;
    for (n = 0U; n < DSP_MEL_BINS; n++)
    {
      float32_t angle = (PI / (float32_t)DSP_MEL_BINS) * ((float32_t)n + 0.5f) * (float32_t)k;
      acc += mel_in[n] * arm_cos_f32(angle);
    }
    dct_out[k] = acc;
  }
}

void Extract_MFCC(float32_t *pAudioBuffer, float32_t *pMfccOut)
{
  arm_rfft_fast_instance_f32 rfft;
  uint32_t i;
  float32_t mean = 0.0f;
  float32_t std = 1.0f;

  if (!s_window_ready)
  {
    DSP_InitWindow();
  }

  DSP_PreEmphasis(pAudioBuffer, s_preemph);
  arm_mean_f32(s_preemph, DSP_FFT_SIZE, &mean);
  arm_offset_f32(s_preemph, -mean, s_fft_in, DSP_FFT_SIZE);
  arm_std_f32(s_fft_in, DSP_FFT_SIZE, &std);
  if (std > 0.000001f)
  {
    arm_scale_f32(s_fft_in, 1.0f / std, s_fft_in, DSP_FFT_SIZE);
  }

  arm_mult_f32(s_fft_in, s_window, s_fft_in, DSP_FFT_SIZE);

  (void)arm_rfft_fast_init_f32(&rfft, DSP_FFT_SIZE);
  arm_rfft_fast_f32(&rfft, s_fft_in, s_fft_out, 0U);

  arm_cmplx_mag_f32(s_fft_out, s_mag, DSP_FFT_SIZE / 2U);
  arm_mult_f32(s_mag, s_mag, s_power, DSP_FFT_SIZE / 2U);

  DSP_ComputeMelBank(s_power, s_mel_energies);
  for (i = 0U; i < DSP_MEL_BINS; i++)
  {
    s_mel_energies[i] = logf(s_mel_energies[i] + 1.0e-6f);
  }

  DSP_DCT(s_mel_energies, s_dct_out);
  for (i = 0U; i < DSP_MFCC_FEATURES; i++)
  {
    pMfccOut[i] = s_dct_out[i];
  }
}

uint16_t Calculate_DOA(const float32_t *pAudioBuffer, uint32_t length)
{
  float32_t max_val = 0.0f;
  uint32_t max_index = 0U;
  uint32_t samples = length / 2U;
  const float32_t *left = pAudioBuffer;
  const float32_t *right = pAudioBuffer + 1U;
  uint32_t i;

  if (samples > DSP_DOA_MAX_SAMPLES)
  {
    samples = DSP_DOA_MAX_SAMPLES;
  }

  for (i = 0U; i < samples; i++)
  {
    s_preemph[i] = left[i * 2U];
    s_fft_in[i] = right[i * 2U];
  }

  arm_correlate_f32(s_preemph, samples, s_fft_in, samples, s_corr);
  arm_max_f32(s_corr, (2U * samples - 1U), &max_val, &max_index);

  (void)max_val;

  if (max_index > (samples - 1U))
  {
    int32_t lag = (int32_t)max_index - (int32_t)(samples - 1U);
    float32_t angle = 90.0f + (float32_t)lag * 2.0f;
    if (angle < 0.0f)
    {
      angle = 0.0f;
    }
    if (angle > 180.0f)
    {
      angle = 180.0f;
    }
    return (uint16_t)angle;
  }

  return 90U;
}
