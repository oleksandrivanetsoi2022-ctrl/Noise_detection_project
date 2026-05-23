#ifndef IPC_CM4_H
#define IPC_CM4_H

#include <stdint.h>

/* Initialize producer-side shared buffer (called once at startup). */
void IPC_ProducerInit(void);

/*
 * Write a frame into the shared buffer. Returns 0 on success, -1 if buffer full
 * or len exceeds SHARED_FRAME_SIZE.
 */
int IPC_ProducerWriteFrame(const void *data, uint32_t len);

#endif /* IPC_CM4_H */
