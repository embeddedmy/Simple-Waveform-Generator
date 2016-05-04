
/** @file dma.h
 *  @brief DMA Driver include file for the STM32F072RB.
 *  @author Dennis Law
 *  @date April 2016
 */
 
#ifndef DMA_DRV_H
#define DMA_DRV_H
 
#include "stm32f0xx.h"

int DMA_extractBasePointer(int chn, DMA_Channel_TypeDef **dma);
int DMA_disable(int chn);
int DMA_enable(int chn);
int DMA_set_size(int chn, uint32_t size);
int DMA_set_memory(int chn, uint32_t* pstarthere);
int DMA_init(int chn);

#endif	/* DMA_DRV_H */
