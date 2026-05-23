#include "lora_app.h"
#include "sx1262.h"
#include "lora_hal.h"
#include "main.h"

#include <string.h>
#include <stdio.h>

/* Simple mesh header: [src(1), dest(1), ttl(1), seq(2), flags(1)] + payload */
struct mesh_hdr {
  uint8_t src;
  uint8_t dest;
  uint8_t ttl;
  uint16_t seq;
  uint8_t flags;
} __attribute__((packed));

static uint16_t tx_seq = 0;
static lora_app_rx_cb_t app_cb = NULL;

/* module state */
static uint16_t s_seq_counter = 0U;
static int s_inited = 0;

static void sx_rx_cb(const uint8_t *buf, uint16_t len)
{
  if (len < sizeof(struct mesh_hdr)) return;
  struct mesh_hdr hdr;
  memcpy(&hdr, buf, sizeof(hdr));
  const uint8_t *payload = buf + sizeof(hdr);
  uint16_t payload_len = len - sizeof(hdr);

  /* If packet addressed to me or broadcast (0xFF), deliver */
  if (hdr.dest == LORA_NODE_ID || hdr.dest == 0xFF)
  {
    if (app_cb) app_cb(payload, payload_len, hdr.src);
    /* If ACK flag set, send ack back */
    if (hdr.flags & 0x01)
    {
      uint8_t ackbuf[8];
      struct mesh_hdr ack = { .src = LORA_NODE_ID, .dest = hdr.src, .ttl = 8, .seq = hdr.seq, .flags = 0 };
      memcpy(ackbuf, &ack, sizeof(ack));
      /* no payload for ack */
      SX1262_Send(ackbuf, sizeof(ack));
    }
  }
  else if (hdr.ttl > 0)
  {
    /* Forwarding: decrement TTL and re-send */
    struct mesh_hdr fwd = hdr;
    fwd.ttl--;
    uint8_t fwdbuf[256];
    memcpy(fwdbuf, &fwd, sizeof(fwd));
    memcpy(fwdbuf + sizeof(fwd), payload, payload_len);
    SX1262_Send(fwdbuf, sizeof(fwd) + payload_len);
  }
}

int LoRa_Init(void)
{
  LORA_HAL_Init();
  SX1262_Init();
  SX1262_RegisterRxCallback(sx_rx_cb);
  SX1262_SetRx();
  s_inited = 1;
  return 0;
}

int LoRa_SendRaw(const uint8_t *payload, uint16_t len, uint8_t dest)
{
  if (!payload || len == 0 || len > 240) return -1;
  if (!s_inited)
  {
    LoRa_Init();
  }
  uint8_t buf[256];
  struct mesh_hdr hdr = { .src = LORA_NODE_ID, .dest = dest, .ttl = 8, .seq = tx_seq++, .flags = 0x01 };
  memcpy(buf, &hdr, sizeof(hdr));
  memcpy(buf + sizeof(hdr), payload, len);
  return SX1262_Send(buf, sizeof(hdr) + len);
}

void LoRa_RegisterRxCallback(lora_app_rx_cb_t cb)
{
  app_cb = cb;
}
void LoRa_Pack_And_Send(uint8_t vehicle_class, float confidence, uint16_t doa_angle)
{
  uint8_t payload[16];
  uint16_t conf_q;
  uint16_t doa_q;
  uint32_t packed0;
  uint32_t packed1;

  if (!s_inited)
  {
    LoRa_Init();
  }

  if (confidence < 0.0f) confidence = 0.0f;
  if (confidence > 1.0f) confidence = 1.0f;

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

  /* send as broadcast by default */
  LoRa_SendRaw(payload, 6, 0xFF);
}
