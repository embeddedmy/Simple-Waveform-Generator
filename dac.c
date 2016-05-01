
/** @file dac.c
 *  @brief DAC driver
 *  @author Dennis Law
 *  @date April 2016
 */

#include "dac.h"

/**	@brief Initializes a DAC channel.
 *	@param chn Channel to initialize. The value is either 1 or 2.
 *	@returns Returns 0 if successful and -1 if otherwise.
 *
 *	Initializes either channel 1 or 2 of the DAC. The channel is configured to
 *	use timer as the triggering source. The assignment of the timer to the
 *	channel is done with reference to Table 31 of the Reference Manual. Also,
 *	DMA is enabled.
 */
int dac_init(int chn)
{
	/* Check if channel index is correct */
	if ((chn != 1) && (chn != 2))
		return -1;

	/* Enable clock for DAC peripheral */
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;
	
	if (chn == 1) {
		/* Configure Timer 6 as trigger */
		DAC->CR &= ~(DAC_CR_TSEL1);
		DAC->CR |= DAC_CR_TEN1;
		
		/* Configure DMA */
		DAC->CR |= DAC_CR_DMAEN1;
		
		/* Congigure GPIO */
		GPIOA->MODER |= GPIO_MODER_MODER4; 		/* Set pin as AIN */
		GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR4); 	/* Disable pull resistor */
		
		/* Enable output buffer */
		DAC->CR |= DAC_CR_BOFF1;
		
	} else if (chn == 2) {
		/* Configure Timer 7 as trigger */
		DAC->CR &= ~(DAC_CR_TSEL2);
		DAC->CR |= DAC_CR_TSEL2_1;
		DAC->CR |= DAC_CR_TEN2;
		
		/* Configure DMA */
		DAC->CR |= DAC_CR_DMAEN2;
		
		/* Congigure GPIO */
		GPIOA->MODER |= GPIO_MODER_MODER5; 		/* Set pin as AIN */
		GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR5); 	/* Disable pull resistor */
		
		/* Enable output buffer */
		DAC->CR |= DAC_CR_BOFF2;
	}
	
	return 0;
}
