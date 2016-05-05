
/** @file dma.h
 *  @brief DMA driver include file
 *  @author Dennis Law
 *  @date May 2016
 */

#ifndef DMA_H
#define DMA_H

#include "stm32f0xx.h"

/** Enumeration for DMA channels */
enum dma_channel {
	DMA_CHN_3 = 0,	/** Used with DAC channel 1 */
	DMA_CHN_4 = 1	/** Used with DAC channel 2 */
};

int dma_init(enum dma_channel chn, uint32_t *read_mem, uint32_t num_read);

int dma_disable(enum dma_channel chn);
int dma_enable(enum dma_channel chn);

#endif	/* DMA_H */
