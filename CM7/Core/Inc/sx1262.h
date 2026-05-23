/* Minimal SX1262 driver header */
#ifndef SX1262_H
#define SX1262_H

#include <stdint.h>
#include <stddef.h>

int SX1262_Init(void);
int SX1262_Send(const uint8_t *buf, uint16_t len);
int SX1262_SetRx(void);
int SX1262_HandleIrq(void);
int SX1262_GetRxPacket(uint8_t *buf, uint16_t *len);

/* Register receive callback (called from IRQ context) */
typedef void (*sx1262_rx_cb_t)(const uint8_t *buf, uint16_t len);
void SX1262_RegisterRxCallback(sx1262_rx_cb_t cb);

#endif /* SX1262_H */
#ifndef SX1262_H
#define SX1262_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize SX1262 (resets, sets standby, packet type LoRa, frequency) */
bool SX1262_Init(void);

/* Send raw payload (blocking until TX finishes) */
bool SX1262_Send(const uint8_t *data, uint16_t len, uint32_t timeout_ms);

/* Low-level buffer write */
bool SX1262_WriteBuffer(uint8_t offset, const uint8_t *data, uint8_t len);

/* Wait until radio not busy */
void SX1262_WaitOnBusy(void);

#ifdef __cplusplus
}
#endif

#endif /* SX1262_H */
