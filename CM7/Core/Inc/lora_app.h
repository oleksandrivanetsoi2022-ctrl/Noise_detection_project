#ifndef LORA_APP_H
#define LORA_APP_H

#include <stdint.h>

/* Node ID for this device (set as needed) */
#ifndef LORA_NODE_ID
#define LORA_NODE_ID 0x01
#endif

int LoRa_Init(void);
int LoRa_SendRaw(const uint8_t *payload, uint16_t len, uint8_t dest);
/* Pack telemetry (vehicle class, confidence 0..1, doa angle degrees) and send */
void LoRa_Pack_And_Send(uint8_t vehicle_class, float confidence, uint16_t doa_angle);

/* Register application receive callback */
typedef void (*lora_app_rx_cb_t)(const uint8_t *buf, uint16_t len, uint8_t src);
void LoRa_RegisterRxCallback(lora_app_rx_cb_t cb);

#endif /* LORA_APP_H */
