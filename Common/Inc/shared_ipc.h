#ifndef SHARED_IPC_H
#define SHARED_IPC_H

#include <stdint.h>

/* Default shared SRAM3 base for STM32H7 series. Override by defining
 * SHARED_MEM_BASE in your project build if different.
 */
#ifndef SHARED_MEM_BASE
#define SHARED_MEM_BASE 0x38000000UL
#endif

#define SHARED_FRAME_COUNT 4
#define SHARED_FRAME_SIZE 4096

typedef struct {
    volatile uint32_t seq;   /* Frame sequence number */
    volatile uint32_t len;   /* Valid bytes in data[] */
    volatile uint32_t flags; /* User flags */
    uint8_t data[SHARED_FRAME_SIZE];
} SharedFrame;

typedef struct {
    volatile uint32_t head; /* producer index (monotonic) */
    volatile uint32_t tail; /* consumer index (monotonic) */
    SharedFrame frames[SHARED_FRAME_COUNT];
} SharedAudioBuffer;

static inline SharedAudioBuffer *get_shared_audio_buffer(void)
{
    return (SharedAudioBuffer *)(SHARED_MEM_BASE);
}

#endif /* SHARED_IPC_H */
