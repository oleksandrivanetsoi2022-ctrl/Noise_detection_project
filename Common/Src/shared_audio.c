#include "shared_audio.h"

SHARED_AUDIO_SECTION SharedAudioFrame g_shared_audio_frame = {
  .ready = 0U,
  .frame_counter = 0U,
  .lost_frames = 0U,
  .frame_samples = SHARED_AUDIO_FRAME_SAMPLES,
  .num_channels = SHARED_AUDIO_NUM_CHANNELS
};
