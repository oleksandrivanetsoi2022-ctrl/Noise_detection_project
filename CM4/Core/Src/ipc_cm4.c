#include "ipc_cm4.h"
#include "../../../../Common/Inc/shared_ipc.h"
#include <string.h>
#include "stm32h7xx.h"

void IPC_ProducerInit(void)
{
    SharedAudioBuffer *buf = get_shared_audio_buffer();
    buf->head = 0;
    buf->tail = 0;
    for (uint32_t i = 0; i < SHARED_FRAME_COUNT; ++i) {
        buf->frames[i].seq = 0;
        buf->frames[i].len = 0;
        buf->frames[i].flags = 0;
    }
    __DSB();
}

int IPC_ProducerWriteFrame(const void *data, uint32_t len)
{
    if (len > SHARED_FRAME_SIZE) return -1;
    SharedAudioBuffer *buf = get_shared_audio_buffer();
    uint32_t head = buf->head;
    uint32_t tail = buf->tail;
    if ((head - tail) >= SHARED_FRAME_COUNT) {
        /* buffer full */
        return -1;
    }
    uint32_t idx = head % SHARED_FRAME_COUNT;
    SharedFrame *f = &buf->frames[idx];
    f->seq = head + 1;
    f->len = len;
    f->flags = 0;
    memcpy((void *)f->data, data, len);
    __DSB(); /* ensure data written before head update */
    buf->head = head + 1;
    __DSB();
    return 0;
}
