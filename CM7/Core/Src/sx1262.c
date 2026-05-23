/* Minimal SX1262 driver (synchronous, small subset) */
#include "sx1262.h"
#include "lora_hal.h"
#include "main.h"

#include <string.h>

/* Simple internal RX buffer */
static uint8_t rx_buf[256];
static uint16_t rx_len = 0;
static volatile int rx_ready = 0;
static sx1262_rx_cb_t rx_cb = NULL;

/* Placeholder: send command over SPI */
static int sx1262_spi_write(const uint8_t *data, size_t len)
{
  return LORA_HAL_SPI_Transmit(data, len);
}

static int sx1262_spi_read(uint8_t *data, size_t len)
{
  return LORA_HAL_SPI_Receive(data, len);
}

int SX1262_Init(void)
{
  LORA_HAL_Init();
  /* Reset radio */
  LORA_HAL_ResetToggle();
  HAL_Delay(10);
  /* Minimal configuration could go here (packet type, freq, power) */
  return 0;
}

int SX1262_Send(const uint8_t *buf, uint16_t len)
{
  if (!buf || len == 0 || len > 240) return -1;
  /* Write payload into buffer and start TX. This is a simplified flow. */
  LORA_HAL_Select();
  /* In real driver: WRITE_BUFFER opcode + offset + payload */
  sx1262_spi_write(buf, len);
  LORA_HAL_Unselect();
  /* Start TX (placeholder) */
  LORA_HAL_SetTx();
  return 0;
}

int SX1262_SetRx(void)
{
  /* Configure radio to continuous RX (placeholder) */
  LORA_HAL_SetRx();
  return 0;
}

int SX1262_HandleIrq(void)
{
  /* Called from EXTI callback when DIO1 asserted. Query IRQ status and
     if RxDone: read payload into rx_buf and set rx_ready. This is simplified. */
  uint8_t tmp[4];
  /* In real driver: READ_IRQ_STATUS and READ_BUFFER */
  /* For now, read available bytes from radio buffer (driver-dependent) */
  /* We'll simulate by calling LORA_HAL_ReadRx with maximum */
  int r = LORA_HAL_ReadRx(rx_buf, sizeof(rx_buf), &rx_len);
  if (r == 0 && rx_len > 0)
  {
    rx_ready = 1;
    if (rx_cb) rx_cb(rx_buf, rx_len);
  }
  return 0;
}

int SX1262_GetRxPacket(uint8_t *buf, uint16_t *len)
{
  if (!rx_ready) return -1;
  if (!buf || !len) return -2;
  uint16_t l = rx_len;
  memcpy(buf, rx_buf, l);
  *len = l;
  rx_ready = 0;
  rx_len = 0;
  return 0;
}

void SX1262_RegisterRxCallback(sx1262_rx_cb_t cb)
{
  rx_cb = cb;
}

