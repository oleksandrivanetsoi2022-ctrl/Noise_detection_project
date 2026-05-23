#ifndef LORA_HAL_H
#define LORA_HAL_H

#include <stdint.h>

/* Basic HAL wrappers used by SX1262 driver. Implemented with STM32 HAL. */

/* Initialize SPI and control pins for LoRa */
void LORA_HAL_Init(void);

/* NSS / RESET control */
void LORA_HAL_NssLow(void);
void LORA_HAL_NssHigh(void);
void LORA_HAL_ResetAssert(void);
void LORA_HAL_ResetDeassert(void);
void LORA_HAL_Select(void);
void LORA_HAL_Unselect(void);
void LORA_HAL_ResetToggle(void);

/* Busy line and blocking helpers */
int  LORA_HAL_IsBusy(void);
int  LORA_HAL_WaitWhileBusy(uint32_t timeout_ms);

/* Basic SPI helpers (blocking) */
int LORA_HAL_Transmit(uint8_t *data, uint16_t len, uint32_t timeout);
int LORA_HAL_Receive(uint8_t *data, uint16_t len, uint32_t timeout);
int LORA_HAL_TransmitReceive(const uint8_t *tx, uint8_t *rx, uint16_t len, uint32_t timeout);

/* Convenience wrappers used by earlier code */
int LORA_HAL_SPI_Transmit(const uint8_t *data, uint16_t len);
int LORA_HAL_SPI_Receive(uint8_t *data, uint16_t len);
int LORA_HAL_SPI_TransmitReceive(const uint8_t *tx, uint8_t *rx, uint16_t len);

/* High-level operations */
void LORA_HAL_SetTx(void);
void LORA_HAL_SetRx(void);

/* Read Rx buffer from radio (implementation/driver specific) */
int LORA_HAL_ReadRx(uint8_t *buf, uint16_t maxlen, uint16_t *outlen);

#endif /* LORA_HAL_H */
