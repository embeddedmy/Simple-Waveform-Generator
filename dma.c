
/** @file dma.c
 *  @brief DMA driver
 *  @author Dennis Law
 *  @date May 2016
 */

#include "dma.h"

static void dma_extract_base_pointer(int chn, DMA_Channel_TypeDef **dma);

/** @brief Extracts the DMA channel base pointer.
 *	@param The channel of interest. The value can either be 3 (for DAC chn 6)
 *	or 4 (for DAC chn 7).
 *	@param dma The container to the Channel base pointer for holding the output
 *	of this operation.
 *	@returns 0 if successful and -1 if otherwise.
 */
static void dma_extract_base_pointer(int chn, DMA_Channel_TypeDef **dma)
{
	if (chn == 3)
		*dma = DMA1_Channel3;
	else if (chn == 4)
		*dma = DMA1_Channel4;
}

/** @brief Initializes the DMA for DAC use.
 *	@param chn The DMA channel to initialize. The value can either be
 *	3 (for DAC chn 6) or 4 (for DAC chn 7).
 *	@param read_mem Handle to memory location to be read for DMA transfer.
 *	@param num_read Size of read mem.
 *	@returns 0 if successful and -1 if otherwise.
 */
int dma_init(int chn, uint32_t *read_mem, uint32_t num_read)
{
	DMA_Channel_TypeDef *dma;
	
	if ((chn != 3) && (chn != 4))
		return -1;
	
	dma_extract_base_pointer(chn, &dma);

	/* Enable clock for DMA */
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	
	/* Assign DMA transfer read and write memory locations */
	dma->CMAR = (uint32_t)(read_mem);
	
	if (chn == 3)
		dma->CPAR = (uint32_t)(&DAC->DHR12R1);
	else
		dma->CPAR = (uint32_t)(&DAC->DHR12R2);
	
	dma->CNDTR = num_read;
	
	/* Enable increment mode,
	   Set read memory size to 32 bits,
	   Set write memory size to 32 bits,
	   Enable circular mode
	   Set to read from memory mode */
	dma->CCR |=DMA_CCR_MINC | DMA_CCR_MSIZE_1 | DMA_CCR_PSIZE_1
				| DMA_CCR_CIRC | DMA_CCR_DIR;
	
	return 0;
}

/** @brief Disables a DMA channel.
 *	@param chn The DMA channel to disable. The value can either be
 *	3 (for DAC chn 6) or 4 (for DAC chn 7).
 *	@returns 0 if successfula and -1 if otherwise.
 */
int dma_disable(int chn)
{
	DMA_Channel_TypeDef *dma;
	
	if ((chn != 3) && (chn != 4))
		return -1;
	
	dma_extract_base_pointer(chn, &dma);
	
	dma->CCR &= ~(DMA_CCR_EN);
	return 0;
}

/**	@brief Enables a DMA channel.
 *	@param chn The DMA channel to enable.
 *	@returns 0 if successful and -1 if otherwise.
 */
int dma_enable(int chn)
{
	DMA_Channel_TypeDef *dma;
	
	if ((chn != 3) && (chn != 4))
		return -1;
	
	dma_extract_base_pointer(chn, &dma);
	
	dma->CCR |= DMA_CCR_EN;
	return 0;
}
