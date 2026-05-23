/* Lightweight replacement for a subset of CMSIS-DSP declarations used
 * by this project. This header is intended to be paired with
 * CM7/Core/Src/arm_math_impl.c which provides plain-C implementations
 * of the commonly used functions so the project can build and link
 * without the optimized CMSIS-DSP library. Replace with the official
 * CMSIS-DSP headers+library for best performance.
 */

#ifndef ARM_MATH_SIMPLE_H
#define ARM_MATH_SIMPLE_H

/* Indicate Cortex-M7 optimized variant (used by some source checks) */
#ifndef ARM_MATH_CM7
#define ARM_MATH_CM7
#endif

#include <stdint.h>
#include <stddef.h>

typedef float float32_t;

typedef struct
{
  uint32_t fftLen;
  void *pTwiddle; /* unused in simple implementation */
} arm_rfft_fast_instance_f32;

int arm_rfft_fast_init_f32(arm_rfft_fast_instance_f32 *S, uint32_t fftLen);
void arm_rfft_fast_f32(const arm_rfft_fast_instance_f32 *S, float32_t *p, float32_t *pOut, uint32_t ifftFlag);

void arm_cmplx_mag_f32(const float32_t *pSrc, float32_t *pDst, uint32_t numSamples);

void arm_mean_f32(const float32_t *pSrc, uint32_t blockSize, float32_t *pResult);
void arm_offset_f32(const float32_t *pSrc, float32_t offset, float32_t *pDst, uint32_t blockSize);
void arm_std_f32(const float32_t *pSrc, uint32_t blockSize, float32_t *pResult);
void arm_scale_f32(const float32_t *pSrc, float32_t scale, float32_t *pDst, uint32_t blockSize);
void arm_mult_f32(const float32_t *pSrcA, const float32_t *pSrcB, float32_t *pDst, uint32_t blockSize);
void arm_correlate_f32(const float32_t *pSrcA, uint32_t srcALen, const float32_t *pSrcB, uint32_t srcBLen, float32_t *pDst);
void arm_max_f32(const float32_t *pSrc, uint32_t blockSize, float32_t *pResult, uint32_t *pIndex);
void arm_power_f32(const float32_t *pSrc, uint32_t blockSize, float32_t *pResult);

float32_t arm_sqrt_f32(float32_t in);
float32_t arm_cos_f32(float32_t x);

#endif /* ARM_MATH_SIMPLE_H */
