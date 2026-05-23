#include "ipc_cm7.h"
#include "../../../../Common/Inc/shared_ipc.h"
#include <string.h>
#include "stm32h7xx.h"

void IPC_ConsumerInit(void)
{
    SharedAudioBuffer *buf = get_shared_audio_buffer();
    /* Ensure we have a sane starting point */
    buf->tail = 0;
    /* head may be set by producer */
    __DSB();
}

static inline uint32_t cache_round_down(uint32_t addr)
{
    const uint32_t line = 32;
    return addr & ~(line - 1);
}

static inline uint32_t cache_round_up(uint32_t addr, uint32_t size)
{
    const uint32_t line = 32;
    uint32_t end = addr + size;
    return (end + (line - 1)) & ~(line - 1);
}

int IPC_ConsumerReadFrame(uint8_t *out_buf, uint32_t buf_len, uint32_t *out_len)
{
    SharedAudioBuffer *buf = get_shared_audio_buffer();
    uint32_t head = buf->head;
    uint32_t tail = buf->tail;
    if (tail >= head) {
        return -1; /* empty */
    }
    uint32_t idx = tail % SHARED_FRAME_COUNT;
    SharedFrame *f = &buf->frames[idx];

    /* Invalidate D‑cache for the frame memory region before reading */
    uint32_t addr = (uint32_t)f;
    uint32_t addr_aligned = cache_round_down(addr);
    uint32_t size_round = cache_round_up(addr, sizeof(SharedFrame)) - addr_aligned;
    SCB_InvalidateDCache_by_Addr((uint32_t *)addr_aligned, (int32_t)size_round);
    __DSB(); __ISB();

    uint32_t copy_len = f->len;
    if (copy_len > buf_len) copy_len = buf_len;
    memcpy(out_buf, f->data, copy_len);
    if (out_len) *out_len = copy_len;

    /* Advance tail to mark consumption */
    buf->tail = tail + 1;
    __DSB();
    return 0;
}
