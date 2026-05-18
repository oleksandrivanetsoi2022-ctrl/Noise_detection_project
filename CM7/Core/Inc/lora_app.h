#ifndef LORA_APP_H
#define LORA_APP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void LoRa_Pack_And_Send(uint8_t vehicle_class, float confidence, uint16_t doa_angle);

#ifdef __cplusplus
}
#endif

#endif /* LORA_APP_H */
