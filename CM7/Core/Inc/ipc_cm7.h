#ifndef IPC_CM7_H
#define IPC_CM7_H

#include <stdint.h>

/* Initialize consumer-side (CM7) view of the buffer. */
void IPC_ConsumerInit(void);

/* Read a frame from shared buffer. Returns 0 on success, -1 if no frame available.
 * out_len will contain the number of bytes copied into out_buf.
 */
int IPC_ConsumerReadFrame(uint8_t *out_buf, uint32_t buf_len, uint32_t *out_len);

#endif /* IPC_CM7_H */
