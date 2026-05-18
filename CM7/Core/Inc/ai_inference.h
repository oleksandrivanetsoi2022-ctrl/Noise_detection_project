#ifndef AI_INFERENCE_H
#define AI_INFERENCE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  AI_CLASS_BACKGROUND = 0,
  AI_CLASS_CAR = 1,
  AI_CLASS_MOTORCYCLE = 2
} AiClass;

void Run_Audio_Inference(float *mfcc_features, uint8_t *predicted_class, float *confidence);

#ifdef __cplusplus
}
#endif

#endif /* AI_INFERENCE_H */
