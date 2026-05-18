#include "ai_inference.h"

static uint8_t s_model_loaded = 0U;

static void Load_Int8_Model(void)
{
  s_model_loaded = 1U;
}

void Run_Audio_Inference(float *mfcc_features, uint8_t *predicted_class, float *confidence)
{
  float score = 0.0f;
  uint32_t i;

  if (s_model_loaded == 0U)
  {
    Load_Int8_Model();
  }

  for (i = 0U; i < 16U; i++)
  {
    score += mfcc_features[i];
  }

  if (score > 1000.0f)
  {
    *predicted_class = (uint8_t)AI_CLASS_MOTORCYCLE;
    *confidence = 0.85f;
  }
  else
  {
    *predicted_class = (uint8_t)AI_CLASS_CAR;
    *confidence = 0.72f;
  }
}
