#ifndef SAI_A_H
#define SAI_A_H

#include "stm32h7xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void MX_SAI1_Init(void);

extern SAI_HandleTypeDef hsai1_a;
extern SAI_HandleTypeDef hsai1_b;
extern DMA_HandleTypeDef hdma_sai1_a;
extern DMA_HandleTypeDef hdma_sai1_b;

#ifdef __cplusplus
}
#endif

#endif /* SAI_A_H */
