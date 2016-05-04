
/** @file dma.h
 *  @brief DMA driver include file
 *  @author Dennis Law
 *  @date May 2016
 */

#ifndef DMA_H
#define DMA_H

#include "stm32f0xx.h"

int dma_init(int chn, uint32_t *read_mem, uint32_t num_read);

int dma_disable(int chn);
int dma_enable(int chn);

#endif	/* DMA_H */
