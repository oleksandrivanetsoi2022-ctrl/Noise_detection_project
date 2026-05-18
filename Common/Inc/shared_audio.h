#ifndef SHARED_AUDIO_H
#define SHARED_AUDIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHARED_AUDIO_NUM_CHANNELS 8U
#define SHARED_AUDIO_FRAME_SAMPLES 256U

#define SHARED_AUDIO_SECTION __attribute__((section(".shared_audio"), aligned(32)))

typedef struct
{
  volatile uint32_t ready;
  uint32_t frame_counter;
  uint32_t lost_frames;
  uint16_t frame_samples;
  uint16_t num_channels;
  int16_t  audio[SHARED_AUDIO_NUM_CHANNELS][SHARED_AUDIO_FRAME_SAMPLES];
} SharedAudioFrame;

extern SharedAudioFrame g_shared_audio_frame;

#ifdef __cplusplus
}
#endif

#endif /* SHARED_AUDIO_H */
