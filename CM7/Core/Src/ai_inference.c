#include "ai_inference.h"

#include <math.h>

#include "ai_platform.h"
#include "network.h"
#include "network_data.h"

static ai_handle s_network = AI_HANDLE_NULL;
static ai_buffer *s_inputs = NULL;
static ai_buffer *s_outputs = NULL;

AI_ALIGNED(4)
static ai_i8 s_in_data[AI_NETWORK_IN_1_SIZE_BYTES];
AI_ALIGNED(4)
static ai_i8 s_out_data[AI_NETWORK_OUT_1_SIZE_BYTES];

static ai_bool AI_CreateNetwork(void)
{
  ai_error err;
  ai_network_params params;

  err = ai_network_create(&s_network, AI_NETWORK_DATA_CONFIG);
  if ((err.type != AI_ERROR_NONE) || (err.code != AI_ERROR_CODE_NONE))
  {
    s_network = AI_HANDLE_NULL;
    return false;
  }

  if (ai_network_data_params_get(&params) == false)
  {
    ai_network_destroy(s_network);
    s_network = AI_HANDLE_NULL;
    return false;
  }

  if (ai_network_init(s_network, &params) == false)
  {
    ai_network_destroy(s_network);
    s_network = AI_HANDLE_NULL;
    return false;
  }

  s_inputs = ai_network_inputs_get(s_network, NULL);
  s_outputs = ai_network_outputs_get(s_network, NULL);
  s_inputs[0].data = AI_HANDLE_PTR(s_in_data);
  s_outputs[0].data = AI_HANDLE_PTR(s_out_data);

  return true;
}

void AI_Inference_Init(void)
{
  if (s_network == AI_HANDLE_NULL)
  {
    (void)AI_CreateNetwork();
  }
}

void Run_Audio_Inference(float *mfcc_features, uint8_t *predicted_class, float *confidence)
{
  uint32_t i;
  int32_t max_idx = 0;
  ai_i8 max_val;

  if ((mfcc_features == NULL) || (predicted_class == NULL) || (confidence == NULL))
  {
    return;
  }

  if (s_network == AI_HANDLE_NULL)
  {
    if (AI_CreateNetwork() == false)
    {
      *predicted_class = (uint8_t)AI_CLASS_BACKGROUND;
      *confidence = 0.0f;
      return;
    }
  }

  /* Simple int8 quantization: clamp float features to [-128, 127]. */
  for (i = 0U; i < AI_NETWORK_IN_1_SIZE; i++)
  {
    float x = mfcc_features[i];
    int32_t q = (int32_t)lrintf(x);
    if (q > 127)
    {
      q = 127;
    }
    else if (q < -128)
    {
      q = -128;
    }
    s_in_data[i] = (ai_i8)q;
  }

  if (ai_network_run(s_network, s_inputs, s_outputs) <= 0)
  {
    *predicted_class = (uint8_t)AI_CLASS_BACKGROUND;
    *confidence = 0.0f;
    return;
  }

  max_val = s_out_data[0];
  for (i = 1U; i < AI_NETWORK_OUT_1_SIZE; i++)
  {
    if (s_out_data[i] > max_val)
    {
      max_val = s_out_data[i];
      max_idx = (int32_t)i;
    }
  }

  *predicted_class = (uint8_t)max_idx;
  *confidence = (float)max_val / 127.0f;
}
