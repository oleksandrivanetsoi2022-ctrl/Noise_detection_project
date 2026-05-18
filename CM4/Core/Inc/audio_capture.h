#ifndef AUDIO_CAPTURE_H
#define AUDIO_CAPTURE_H

#include <stdint.h>
#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C" {
#endif

void AudioCapture_Init(void);
void AudioCapture_Start(void);
void AudioCapture_RegisterQueue(osMessageQueueId_t queue);
void AudioCapture_ProcessEvent(uint32_t event);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_CAPTURE_H */
