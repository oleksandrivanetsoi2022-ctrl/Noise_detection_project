#ifndef DSP_PROCESSING_H
#define DSP_PROCESSING_H

#include "arm_math.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DSP_FFT_SIZE 256U
#define DSP_MFCC_FEATURES 40U
#define DSP_MEL_BINS 40U
#define DSP_DCT_BINS 40U
#define DSP_DOA_MAX_SAMPLES 256U

void Extract_MFCC(float32_t *pAudioBuffer, float32_t *pMfccOut);
uint16_t Calculate_DOA(const float32_t *pAudioBuffer, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* DSP_PROCESSING_H */
