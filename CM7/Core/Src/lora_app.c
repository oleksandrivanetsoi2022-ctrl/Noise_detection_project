#include "lora_app.h"

#include "main.h"

static SPI_HandleTypeDef s_lora_spi;
static uint16_t s_seq_counter = 0U;

void LoRa_Pack_And_Send(uint8_t vehicle_class, float confidence, uint16_t doa_angle)
{
  uint8_t payload[6];
  uint16_t conf_q;
  uint16_t doa_q;
  uint32_t packed0;
  uint32_t packed1;

  if (confidence < 0.0f)
  {
    confidence = 0.0f;
  }
  if (confidence > 1.0f)
  {
    confidence = 1.0f;
  }

  conf_q = (uint16_t)(confidence * 1023.0f);
  doa_q = (uint16_t)(doa_angle % 360U);

  packed0 = ((uint32_t)(vehicle_class & 0x03U)) |
            ((uint32_t)(conf_q & 0x03FFU) << 2U) |
            ((uint32_t)(doa_q & 0x03FFU) << 12U);

  packed1 = (uint32_t)s_seq_counter++;

  payload[0] = (uint8_t)(packed0 & 0xFFU);
  payload[1] = (uint8_t)((packed0 >> 8U) & 0xFFU);
  payload[2] = (uint8_t)((packed0 >> 16U) & 0xFFU);
  payload[3] = (uint8_t)(packed1 & 0xFFU);
  payload[4] = (uint8_t)((packed1 >> 8U) & 0xFFU);
  payload[5] = (uint8_t)(payload[0] ^ payload[1] ^ payload[2] ^ payload[3] ^ payload[4]);

  (void)HAL_SPI_Transmit(&s_lora_spi, payload, sizeof(payload), HAL_MAX_DELAY);
}
