
/** @file dma.c
 *  @brief DMA driver
 *  @author Dennis Law
 *  @date May 2016
 */

#include "dma.h"

static void dma_extract_base_pointer(enum dma_channel chn,
								DMA_Channel_TypeDef **dma);

/** @brief Extracts the DMA channel base pointer.
 *	@param The channel of interest.
 *	@param dma Handle to DMA base pointer.
 *	@returns 0 if successful and -1 if otherwise.
 */
static void dma_extract_base_pointer(enum dma_channel chn,
								DMA_Channel_TypeDef **dma)
{
	if (chn == DMA_CHN_3)
		*dma = DMA1_Channel3;
	else if (chn == DMA_CHN_4)
		*dma = DMA1_Channel4;
}

/** @brief Initializes the DMA for DAC use.
 *	@param chn The DMA channel to initialize.
 *	@param read_mem Handle to DMA read memory location.
 *	@param num_read Size of read_mem.
 *	@returns 0 if successful and -1 if otherwise.
 */
int dma_init(enum dma_channel chn, uint32_t *read_mem, uint32_t num_read)
{
	DMA_Channel_TypeDef *dma;
	
	if ((chn != DMA_CHN_3) && (chn != DMA_CHN_4))
		return -1;
	
	dma_extract_base_pointer(chn, &dma);

	/* Enable clock for DMA */
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	
	/* Assign DMA transfer read and write memory locations */
	dma->CMAR = (uint32_t)(read_mem);
	
	if (chn == DMA_CHN_3)
		dma->CPAR = (uint32_t)(&DAC->DHR12R1);
	else
		dma->CPAR = (uint32_t)(&DAC->DHR12R2);
	
	dma->CNDTR = num_read;
	
	/* 1. Enable increment mode,
	   2. Set read memory size to 32 bits,
	   3. Set write memory size to 32 bits,
	   4. Enable circular mode
	   5. Set to read from memory mode */
	dma->CCR |=DMA_CCR_MINC | DMA_CCR_MSIZE_1 | DMA_CCR_PSIZE_1
				| DMA_CCR_CIRC | DMA_CCR_DIR;
	
	return 0;
}

/** @brief Disables a DMA channel.
 *	@param chn The DMA channel to disable.
 *	@returns 0 if successful and -1 if otherwise.
 */
int dma_disable(enum dma_channel chn)
{
	DMA_Channel_TypeDef *dma;
	
	if ((chn != DMA_CHN_3) && (chn != DMA_CHN_4))
		return -1;
	
	dma_extract_base_pointer(chn, &dma);
	
	dma->CCR &= ~(DMA_CCR_EN);
	return 0;
}

/**	@brief Enables a DMA channel.
 *	@param chn The DMA channel to enable.
 *	@returns 0 if successful and -1 if otherwise.
 */
int dma_enable(enum dma_channel chn)
{
	DMA_Channel_TypeDef *dma;
	
	if ((chn != DMA_CHN_3) && (chn != DMA_CHN_4))
		return -1;
	
	dma_extract_base_pointer(chn, &dma);
	
	dma->CCR |= DMA_CCR_EN;
	return 0;
}
