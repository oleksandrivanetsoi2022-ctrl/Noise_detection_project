#ifndef AI_INFERENCE_H
#define AI_INFERENCE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  AI_CLASS_CAR = 0,
  AI_CLASS_MOTORCYCLE = 1,
  AI_CLASS_TRUCK = 2,
  AI_CLASS_TRAFFIC_HORN = 3,
  AI_CLASS_ENGINE_IDLING = 4,
  AI_CLASS_BACKGROUND = 5
} AiClass;

void AI_Inference_Init(void);
void Run_Audio_Inference(float *mfcc_features, uint8_t *predicted_class, float *confidence);

#ifdef __cplusplus
}
#endif

#endif /* AI_INFERENCE_H */
