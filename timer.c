
/** @file timer.c
 *  @brief Timer driver
 *
 *	Only timer 6 and 7 is configured in this file. The timer is used to trigger
 *	the DAC which will then trigger a DMA request. The timer is configured to
 *	trigger at every count overflow.
 *
 *  @author Dennis Law
 *  @date April 2016
 */

#include <stdbool.h>
#include "timer.h"

/** @brief Writes to the counter register
 *	@param tim The timer to configure.
 *	@param val The value of the counter.
 *	@returns 0 if successful and -1 if otherwise.
 *
 *	This function writes to the counter register which is the ARR register.
 *
 *	@note It is best to disable the timer before updating the counter.
 */
int timer_write_counter(TIM_TypeDef *tim, uint16_t val)
{
	if ((tim != TIM6) && (tim != TIM7))
		return -1;

	tim->ARR = val;
	return 0;
}

/** @brief Writes to the prescaler register
 *	@param tim The timer to configure.
 *	@param val The value of the prescaler.
 *	@returns 0 if successful and -1 if otherwise.
 *
 *	@note It is best to disable the timer before updating the prescaler.
 */
int timer_write_prescaler(TIM_TypeDef *tim, uint16_t val)
{
	if ((tim != TIM6) && (tim != TIM7))
		return -1;

	tim->PSC = val;
	return 0;
}

/** @brief Initializes a basic Timer
 *	@param tim The timer to be initialized. Its value can be TIM6 or TIM7
 *
 *	A basic timer is initialized for the puspose of being a triggering source
 *	for DAC.
 */
int timer_init(TIM_TypeDef *tim)
{
	if ((tim != TIM6) & (tim != TIM7))
		return -1;
	
	/* Enable clock */
	if (tim == TIM6) {
		RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
	} else if (tim == TIM7) {
		RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
	}
	
	/* Enable ARR register buffering */
	tim->CR1 |= TIM_CR1_ARPE;
	
	/* Set Mode as continuous */
	tim->CR1 &= ~(TIM_CR1_OPM);
	
	/* Set Mater Mode as Update */
	tim->CR2 &= ~(TIM_CR2_MMS);
	tim->CR2 |= TIM_CR2_MMS_1;
	
	/* Enable update DMA request */
	tim->DIER |= TIM_DIER_UDE;
	
	return 0;
}
