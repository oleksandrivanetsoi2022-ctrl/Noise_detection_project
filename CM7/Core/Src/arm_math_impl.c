#include "arm_math.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

int arm_rfft_fast_init_f32(arm_rfft_fast_instance_f32 *S, uint32_t fftLen)
{
    if (!S || fftLen == 0U) return -1;
    S->fftLen = fftLen;
    S->pTwiddle = NULL;
    return 0;
}

void arm_rfft_fast_f32(const arm_rfft_fast_instance_f32 *S, float32_t *p, float32_t *pOut, uint32_t ifftFlag)
{
    /* Simple (non-optimized) DFT implementation for real input.
       p: input array of length S->fftLen (real)
       pOut: output array of interleaved complex values of length 2*S->fftLen
       ifftFlag: ignored (we only implement forward transform)
    */
    uint32_t N = S ? S->fftLen : 0U;
    if (N == 0U) return;

    for (uint32_t k = 0U; k < N; k++)
    {
        double re = 0.0;
        double im = 0.0;
        for (uint32_t n = 0U; n < N; n++)
        {
            double angle = -2.0 * M_PI * (double)k * (double)n / (double)N;
            re += (double)p[n] * cos(angle);
            im += (double)p[n] * sin(angle);
        }
        pOut[2U * k] = (float32_t)re;
        pOut[2U * k + 1U] = (float32_t)im;
    }
}

void arm_cmplx_mag_f32(const float32_t *pSrc, float32_t *pDst, uint32_t numSamples)
{
    for (uint32_t i = 0U; i < numSamples; i++)
    {
        float32_t re = pSrc[2U * i];
        float32_t im = pSrc[2U * i + 1U];
        pDst[i] = sqrtf(re * re + im * im);
    }
}

void arm_mean_f32(const float32_t *pSrc, uint32_t blockSize, float32_t *pResult)
{
    double acc = 0.0;
    for (uint32_t i = 0U; i < blockSize; i++) acc += pSrc[i];
    *pResult = (float32_t)(acc / (double)blockSize);
}

void arm_offset_f32(const float32_t *pSrc, float32_t offset, float32_t *pDst, uint32_t blockSize)
{
    for (uint32_t i = 0U; i < blockSize; i++) pDst[i] = pSrc[i] + offset;
}

void arm_std_f32(const float32_t *pSrc, uint32_t blockSize, float32_t *pResult)
{
    float32_t mean;
    arm_mean_f32(pSrc, blockSize, &mean);
    double acc = 0.0;
    for (uint32_t i = 0U; i < blockSize; i++)
    {
        double d = (double)pSrc[i] - (double)mean;
        acc += d * d;
    }
    *pResult = (float32_t)sqrt(acc / (double)blockSize);
}

void arm_scale_f32(const float32_t *pSrc, float32_t scale, float32_t *pDst, uint32_t blockSize)
{
    for (uint32_t i = 0U; i < blockSize; i++) pDst[i] = pSrc[i] * scale;
}

void arm_mult_f32(const float32_t *pSrcA, const float32_t *pSrcB, float32_t *pDst, uint32_t blockSize)
{
    for (uint32_t i = 0U; i < blockSize; i++) pDst[i] = pSrcA[i] * pSrcB[i];
}

void arm_correlate_f32(const float32_t *pSrcA, uint32_t srcALen, const float32_t *pSrcB, uint32_t srcBLen, float32_t *pDst)
{
    /* full linear cross-correlation (non-optimized) */
    uint32_t outLen = srcALen + srcBLen - 1U;
    for (uint32_t n = 0U; n < outLen; n++)
    {
        double acc = 0.0;
        uint32_t kmin = (n >= srcBLen - 1U) ? (n - (srcBLen - 1U)) : 0U;
        uint32_t kmax = (n < srcALen - 1U) ? n : (srcALen - 1U);
        for (uint32_t k = kmin; k <= kmax; k++)
        {
            acc += (double)pSrcA[k] * (double)pSrcB[n - k];
        }
        pDst[n] = (float32_t)acc;
    }
}

void arm_max_f32(const float32_t *pSrc, uint32_t blockSize, float32_t *pResult, uint32_t *pIndex)
{
    float32_t maxv = pSrc[0];
    uint32_t idx = 0U;
    for (uint32_t i = 1U; i < blockSize; i++)
    {
        if (pSrc[i] > maxv)
        {
            maxv = pSrc[i];
            idx = i;
        }
    }
    *pResult = maxv;
    *pIndex = idx;
}

void arm_power_f32(const float32_t *pSrc, uint32_t blockSize, float32_t *pResult)
{
    double acc = 0.0;
    for (uint32_t i = 0U; i < blockSize; i++) acc += (double)pSrc[i] * (double)pSrc[i];
    *pResult = (float32_t)acc;
}

float32_t arm_sqrt_f32(float32_t in)
{
    return sqrtf(in);
}

float32_t arm_cos_f32(float32_t x)
{
    return cosf(x);
}
