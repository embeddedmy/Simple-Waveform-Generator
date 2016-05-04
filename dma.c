
/** @file dma.c
 *  @brief DAC Driver for the STM32F072RB.
 *  @author Dennis Law
 *  @date April 2016
 */
 
#include "dma.h"

/** @brief Extracts the DMA channel base pointer.
 *	@param The channel of interest.
 *	@param dma The container to the Channel base pointer for
 *	holding the output of this operation.
 *	@returns 0 if successful and -1 if otherwise.
 */
int DMA_extractBasePointer(int chn, DMA_Channel_TypeDef **dma)
{
	if (chn == 1)
		*dma = DMA1_Channel1;
	else if (chn == 2)
		*dma = DMA1_Channel2;
	else if (chn == 3)
		*dma = DMA1_Channel3;
	else if (chn == 4)
		*dma = DMA1_Channel4;
	else if (chn == 5)
		*dma = DMA1_Channel5;
	else if (chn == 6)
		*dma = DMA1_Channel6;
	else if (chn == 7)
		*dma = DMA1_Channel7;
	else
		return -1;
	
	return 0;
}

/** @brief Disables a DMA channel.
 *	@param chn The DMA channel to disable.
 *	@returns 0 if successfula and -1 if otherwise.
 */
int DMA_disable(int chn)
{
	DMA_Channel_TypeDef *dma;
	
	if (DMA_extractBasePointer(chn, &dma))
		return -1;
	
	dma->CCR &= ~(DMA_CCR_EN);
	return 0;
}

/**	@brief Enables a DMA channel.
 *	@param chn The DMA channel to enable.
 *	@returns 0 if successful and -1 if otherwise.
 */
int DMA_enable(int chn)
{
	DMA_Channel_TypeDef *dma;
	
	if (DMA_extractBasePointer(chn, &dma))
		return -1;
	
	dma->CCR |= DMA_CCR_EN;
	return 0;
}

/** @brief Initializes the DMA
 *	@param chn The DMA channel to initialize
 *	@param size The DMA size
 *	@returns 0 if successful and -1 if otherwise.
 */
int DMA_set_size(int chn, uint32_t size)
{
	DMA_Channel_TypeDef *dma;
	
	if (DMA_extractBasePointer(chn, &dma))
		return -1;
	dma->CNDTR = size;
	
	return 0;
}
/** @brief Initializes the DMA
 *	@param chn The DMA channel to initialize
 *	@param pstarthere The DMA read address
 *	@returns 0 if successful and -1 if otherwise.
 */
int DMA_set_memory(int chn, uint32_t* pstarthere)
{
	DMA_Channel_TypeDef *dma;
	
	if (DMA_extractBasePointer(chn, &dma))
		return -1;

	dma->CMAR = (uint32_t) pstarthere;
	
	return 0;
}

/** @brief Initializes the DMA
 *	@param chn The DMA channel to initialize
 *	@returns 0 if successful and -1 if otherwise.
 */
int DMA_init(int chn)
{
	DMA_Channel_TypeDef *dma;
	
	if (DMA_extractBasePointer(chn, &dma))
		return -1;

	RCC->AHBENR |= RCC_AHBENR_DMA1EN; /* Enable clock for DMA */
	
	dma->CPAR = (uint32_t)(&DAC->DHR12R1);
	
	dma->CCR |=DMA_CCR_MINC | DMA_CCR_MSIZE_1 | DMA_CCR_PSIZE_1
				| DMA_CCR_CIRC | DMA_CCR_DIR;
	
	return 0;
}



