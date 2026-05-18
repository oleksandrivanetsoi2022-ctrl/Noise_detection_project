#include "sai_a.h"

#include "main.h"

SAI_HandleTypeDef hsai1_a;
SAI_HandleTypeDef hsai1_b;
DMA_HandleTypeDef hdma_sai1_a;
DMA_HandleTypeDef hdma_sai1_b;

void MX_SAI1_Init(void)
{
  hsai1_a.Instance = SAI1_Block_A;
  hsai1_a.Init.AudioMode = SAI_MODEMASTER_RX;
  hsai1_a.Init.Synchro = SAI_ASYNCHRONOUS;
  hsai1_a.Init.DataSize = SAI_DATASIZE_32;
  hsai1_a.Init.FirstBit = SAI_FIRSTBIT_MSB;
  hsai1_a.Init.ClockStrobing = SAI_CLOCKSTROBING_FALLINGEDGE;
  hsai1_a.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
  hsai1_a.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
  hsai1_a.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
  hsai1_a.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_16K;
  hsai1_a.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
  hsai1_a.Init.MonoStereoMode = SAI_STEREOMODE;
  hsai1_a.Init.CompandingMode = SAI_NOCOMPANDING;
  hsai1_a.Init.TriState = SAI_OUTPUT_NOTRELEASED;

  hsai1_a.FrameInit.FrameLength = 64;
  hsai1_a.FrameInit.ActiveFrameLength = 32;
  hsai1_a.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
  hsai1_a.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
  hsai1_a.FrameInit.FSOffset = SAI_FS_FIRSTBIT;

  hsai1_a.SlotInit.FirstBitOffset = 0U;
  hsai1_a.SlotInit.SlotSize = SAI_SLOTSIZE_32B;
  hsai1_a.SlotInit.SlotNumber = 2U;
  hsai1_a.SlotInit.SlotActive = SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1;

  if (HAL_SAI_Init(&hsai1_a) != HAL_OK)
  {
    Error_Handler();
  }

  hsai1_b.Instance = SAI1_Block_B;
  hsai1_b.Init.AudioMode = SAI_MODESLAVE_RX;
  hsai1_b.Init.Synchro = SAI_SYNCHRONOUS;
  hsai1_b.Init.DataSize = SAI_DATASIZE_32;
  hsai1_b.Init.FirstBit = SAI_FIRSTBIT_MSB;
  hsai1_b.Init.ClockStrobing = SAI_CLOCKSTROBING_FALLINGEDGE;
  hsai1_b.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
  hsai1_b.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
  hsai1_b.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
  hsai1_b.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_16K;
  hsai1_b.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
  hsai1_b.Init.MonoStereoMode = SAI_STEREOMODE;
  hsai1_b.Init.CompandingMode = SAI_NOCOMPANDING;
  hsai1_b.Init.TriState = SAI_OUTPUT_NOTRELEASED;

  hsai1_b.FrameInit.FrameLength = 64;
  hsai1_b.FrameInit.ActiveFrameLength = 32;
  hsai1_b.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
  hsai1_b.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
  hsai1_b.FrameInit.FSOffset = SAI_FS_FIRSTBIT;

  hsai1_b.SlotInit.FirstBitOffset = 0U;
  hsai1_b.SlotInit.SlotSize = SAI_SLOTSIZE_32B;
  hsai1_b.SlotInit.SlotNumber = 2U;
  hsai1_b.SlotInit.SlotActive = SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1;

  if (HAL_SAI_Init(&hsai1_b) != HAL_OK)
  {
    Error_Handler();
  }
}
