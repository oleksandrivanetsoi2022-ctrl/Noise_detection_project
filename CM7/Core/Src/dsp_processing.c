#include "dsp_processing.h"

#include <math.h>
#include <string.h>

/* Log-Mel frontend static data */
static float32_t s_lm_window[DSP_LM_FFT_SIZE];
static float32_t s_lm_fft_in[DSP_LM_FFT_SIZE];
static float32_t s_lm_fft_out[DSP_LM_FFT_SIZE];
static float32_t s_lm_power[(DSP_LM_FFT_SIZE / 2U)];
static float32_t s_lm_filters[DSP_LM_MEL_BINS][(DSP_LM_FFT_SIZE / 2U)];
static uint8_t s_lm_ready = 0U;

static void DSP_InitLogMel(void)
{
  uint32_t i, m, k;
  const uint32_t n_fft = DSP_LM_FFT_SIZE;
  const uint32_t n_bins = n_fft / 2U;
  const uint32_t n_mels = DSP_LM_MEL_BINS;

  /* Hann window */
  for (i = 0U; i < n_fft; i++)
  {
    float32_t phase = (2.0f * PI * (float32_t)i) / (float32_t)(n_fft - 1U);
    s_lm_window[i] = 0.5f - 0.5f * arm_cos_f32(phase);
  }

  /* Build mel filterbank (triangular) using HTK-style mel scale approximation */
  float32_t sr = 16000.0f;
  float32_t fmax = sr / 2.0f;
  /* helper functions */
  float32_t hz_to_mel_f(float32_t f) { return 2595.0f * log10f(1.0f + f / 700.0f); }
  float32_t mel_to_hz_f(float32_t m) { return 700.0f * (powf(10.0f, m / 2595.0f) - 1.0f); }

  float32_t mel_low = hz_to_mel_f(0.0f);
  float32_t mel_high = hz_to_mel_f(fmax);

  for (m = 0U; m < n_mels + 2U; m++)
  {
    /* compute bin centers later in loop */
  }

  /* compute mel bin edges in Hz and then to fft bin indices */
  float32_t mel_points[DSP_LM_MEL_BINS + 2U];
  uint32_t bin_points[DSP_LM_MEL_BINS + 2U];
  for (m = 0U; m < n_mels + 2U; m++)
  {
    float32_t mel = mel_low + (mel_high - mel_low) * (float32_t)m / (float32_t)(n_mels + 1U);
    float32_t hz = mel_to_hz_f(mel);
    mel_points[m] = hz;
    uint32_t bin = (uint32_t)floorf((n_fft + 1U) * hz / sr);
    if (bin > n_bins - 1U) bin = n_bins - 1U;
    bin_points[m] = bin;
  }

  /* zero filters */
  for (m = 0U; m < n_mels; m++)
  {
    for (k = 0U; k < n_bins; k++)
    {
      s_lm_filters[m][k] = 0.0f;
    }
  }

  for (m = 0U; m < n_mels; m++)
  {
    uint32_t start = bin_points[m];
    uint32_t center = bin_points[m + 1U];
    uint32_t end = bin_points[m + 2U];
    if (center < start) center = start;
    if (end < center) end = center;
    for (k = start; k <= end && k < n_bins; k++)
    {
      float32_t w;
      if (k <= center)
      {
        float32_t denom = (float32_t)(center - start);
        w = (denom > 0.0f) ? ((float32_t)(k - start) / denom) : 0.0f;
      }
      else
      {
        float32_t denom = (float32_t)(end - center);
        w = (denom > 0.0f) ? ((float32_t)(end - k) / denom) : 0.0f;
      }
      if (w < 0.0f) w = 0.0f;
      s_lm_filters[m][k] = w;
    }
  }

  s_lm_ready = 1U;
}

void Extract_LogMelSpectrogram(float32_t *pAudioBuffer, float32_t *pOut64x63)
{
  uint32_t i, m, fidx;
  const uint32_t n_fft = DSP_LM_FFT_SIZE;
  const uint32_t hop = DSP_LM_HOP_LENGTH;
  const uint32_t frames = DSP_LM_FRAMES;
  const uint32_t n_mels = DSP_LM_MEL_BINS;
  const uint32_t n_bins = n_fft / 2U;

  static float32_t spectro[DSP_LM_MEL_BINS * DSP_LM_FRAMES];

  if (!s_lm_ready)
  {
    DSP_InitLogMel();
  }

  /* center padding by n_fft/2 (simple zero padding) */
  const uint32_t pad = n_fft / 2U;
  uint32_t padded_len = DSP_LM_SAMPLES_PER_CLIP + 2U * pad;
  static float32_t padded[ DSP_LM_SAMPLES_PER_CLIP + DSP_LM_FFT_SIZE ];
  memset(padded, 0, sizeof(padded));
  memcpy(padded + pad, pAudioBuffer, DSP_LM_SAMPLES_PER_CLIP * sizeof(float32_t));

  arm_rfft_fast_instance_f32 rfft;
  (void)arm_rfft_fast_init_f32(&rfft, n_fft);

  /* for each frame */
  for (fidx = 0U; fidx < frames; fidx++)
  {
    uint32_t start = fidx * hop;
    /* copy windowed frame */
    for (i = 0U; i < n_fft; i++)
    {
      s_lm_fft_in[i] = padded[start + i] * s_lm_window[i];
    }

    arm_rfft_fast_f32(&rfft, s_lm_fft_in, s_lm_fft_out, 0U);
    /* compute magnitude for first n_bins (ignore Nyquist bin ordering differences) */
    arm_cmplx_mag_f32(s_lm_fft_out, s_lm_power, n_bins);
    /* power */
    for (i = 0U; i < n_bins; i++)
    {
      s_lm_power[i] = s_lm_power[i] * s_lm_power[i];
    }

    /* apply mel filters */
    for (m = 0U; m < n_mels; m++)
    {
      float32_t sum = 0.0f;
      uint32_t k;
      for (k = 0U; k < n_bins; k++)
      {
        sum += s_lm_power[k] * s_lm_filters[m][k];
      }
      spectro[m * frames + fidx] = sum;
    }
  }

  /* convert to dB: power_to_db(S, ref=np.max, amin=1e-10, top_db=80) */
  const float32_t amin = 1.0e-10f;
  float32_t maxv = 0.0f;
  for (i = 0U; i < (n_mels * frames); i++)
  {
    if (spectro[i] > maxv) maxv = spectro[i];
  }
  if (maxv < amin) maxv = amin;

  for (i = 0U; i < (n_mels * frames); i++)
  {
    float32_t val = spectro[i];
    if (val < amin) val = amin;
    float32_t db = 10.0f * log10f(val) - 10.0f * log10f(maxv);
    /* clip to [-80, 0] */
    if (db < -80.0f) db = -80.0f;
    if (db > 0.0f) db = 0.0f;
    pOut64x63[i] = db;
  }
}


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
