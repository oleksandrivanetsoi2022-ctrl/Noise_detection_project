#include "lora_hal.h"
#include "main.h"
#include "stm32h7xx_hal.h"

extern SPI_HandleTypeDef hspi1;

/* Default pin assignments (can be changed to match hardware)
 * SPI1: SCK=PA5, MISO=PA6, MOSI=PA7
 * NSS  = PA4
 * RESET= PB0
 * BUSY = PB1
 * DIO1 = PB2
 */

extern SPI_HandleTypeDef hspi1;
static GPIO_TypeDef* LORA_NSS_PORT = GPIOA;
static const uint16_t LORA_NSS_PIN = GPIO_PIN_4;
static GPIO_TypeDef* LORA_RESET_PORT = GPIOB;
static const uint16_t LORA_RESET_PIN = GPIO_PIN_0;
static GPIO_TypeDef* LORA_BUSY_PORT = GPIOB;
static const uint16_t LORA_BUSY_PIN = GPIO_PIN_1;
static GPIO_TypeDef* LORA_DIO1_PORT = GPIOB;
static const uint16_t LORA_DIO1_PIN = GPIO_PIN_2;

void LORA_HAL_Init(void)
{
  /* SPI1 is initialized by CubeMX-generated MX_SPI1_Init().
     Here we only prepare control GPIOs and EXTI. */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* NSS */
  GPIO_InitStruct.Pin = LORA_NSS_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LORA_NSS_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(LORA_NSS_PORT, LORA_NSS_PIN, GPIO_PIN_SET);

  /* RESET */
  GPIO_InitStruct.Pin = LORA_RESET_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(LORA_RESET_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(LORA_RESET_PORT, LORA_RESET_PIN, GPIO_PIN_SET);

  /* BUSY */
  GPIO_InitStruct.Pin = LORA_BUSY_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LORA_BUSY_PORT, &GPIO_InitStruct);

  /* DIO1 as EXTI */
  GPIO_InitStruct.Pin = LORA_DIO1_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LORA_DIO1_PORT, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
}

void LORA_HAL_NssLow(void)
{
  HAL_GPIO_WritePin(LORA_NSS_PORT, LORA_NSS_PIN, GPIO_PIN_RESET);
}

void LORA_HAL_NssHigh(void)
{
  HAL_GPIO_WritePin(LORA_NSS_PORT, LORA_NSS_PIN, GPIO_PIN_SET);
}

void LORA_HAL_Select(void)
{
  LORA_HAL_NssLow();
}

void LORA_HAL_Unselect(void)
{
  LORA_HAL_NssHigh();
}

void LORA_HAL_ResetAssert(void)
{
  HAL_GPIO_WritePin(LORA_RESET_PORT, LORA_RESET_PIN, GPIO_PIN_RESET);
}

void LORA_HAL_ResetDeassert(void)
{
  HAL_GPIO_WritePin(LORA_RESET_PORT, LORA_RESET_PIN, GPIO_PIN_SET);
}

void LORA_HAL_ResetToggle(void)
{
  LORA_HAL_ResetAssert();
  HAL_Delay(1);
  LORA_HAL_ResetDeassert();
}

int LORA_HAL_IsBusy(void)
{
  return (HAL_GPIO_ReadPin(LORA_BUSY_PORT, LORA_BUSY_PIN) == GPIO_PIN_SET) ? 1 : 0;
}

int LORA_HAL_WaitWhileBusy(uint32_t timeout_ms)
{
  uint32_t t0 = HAL_GetTick();
  while (LORA_HAL_IsBusy())
  {
    if ((HAL_GetTick() - t0) > timeout_ms) return -1;
  }
  return 0;
}

int LORA_HAL_Transmit(uint8_t *data, uint16_t len, uint32_t timeout)
{
  return HAL_SPI_Transmit(&hspi1, data, len, timeout);
}

int LORA_HAL_Receive(uint8_t *data, uint16_t len, uint32_t timeout)
{
  return HAL_SPI_Receive(&hspi1, data, len, timeout);
}

int LORA_HAL_TransmitReceive(const uint8_t *tx, uint8_t *rx, uint16_t len, uint32_t timeout)
{
  return HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)tx, rx, len, timeout);
}

int LORA_HAL_SPI_Transmit(const uint8_t *data, uint16_t len)
{
  return LORA_HAL_Transmit((uint8_t*)data, len, 200);
}

int LORA_HAL_SPI_Receive(uint8_t *data, uint16_t len)
{
  return LORA_HAL_Receive(data, len, 200);
}

int LORA_HAL_SPI_TransmitReceive(const uint8_t *tx, uint8_t *rx, uint16_t len)
{
  return LORA_HAL_TransmitReceive(tx, rx, len, 500);
}

void LORA_HAL_SetTx(void)
{
  /* Placeholder: application may implement TX start indicator */
}

void LORA_HAL_SetRx(void)
{
  /* Placeholder: application may implement RX start indicator */
}

int LORA_HAL_ReadRx(uint8_t *buf, uint16_t maxlen, uint16_t *outlen)
{
  (void)buf; (void)maxlen; (void)outlen;
  return -1;
}

/* EXTI callback forwarded from HAL */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == LORA_DIO1_PIN)
  {
    extern int SX1262_HandleIrq(void);
    SX1262_HandleIrq();
  }
}
