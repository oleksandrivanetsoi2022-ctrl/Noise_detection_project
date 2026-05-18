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

/* Log-Mel frontend (training parameters)
 * SR = 16000, DURATION = 2.0s -> 32000 samples
 * N_FFT = 1024, HOP_LENGTH = 512, N_MELS = 64 -> 64 x 63 = 4032
 */
#define DSP_LM_FFT_SIZE 1024U
#define DSP_LM_MEL_BINS 64U
#define DSP_LM_HOP_LENGTH 512U
#define DSP_LM_FRAMES 63U
#define DSP_LM_SAMPLES_PER_CLIP 32000U
#define DSP_LM_FEATURES (DSP_LM_MEL_BINS * DSP_LM_FRAMES)

void Extract_MFCC(float32_t *pAudioBuffer, float32_t *pMfccOut);
/* Produces a log-mel spectrogram with shape (64,63) flattened in
 * height-major order (mel bins first). Input must be DSP_LM_SAMPLES_PER_CLIP samples.
 */
void Extract_LogMelSpectrogram(float32_t *pAudioBuffer, float32_t *pOut64x63);
uint16_t Calculate_DOA(const float32_t *pAudioBuffer, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* DSP_PROCESSING_H */
