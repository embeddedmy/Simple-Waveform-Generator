
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

#include <stdio.h>
#include "timer.h"

static void timer_extract_base_pointer(enum timer_index idx,
								TIM_TypeDef **tim);

/** @name Callback function handlers */
/** @{*/

void (*timer6_callback)(void) = NULL;
void (*timer7_callback)(void) = NULL;

/** @}*/

static void timer_extract_base_pointer(enum timer_index idx,
								TIM_TypeDef **tim)
{
	if (idx == TIMER_IDX_6)
		*tim = TIM6;
	else if (idx == TIMER_IDX_7)
		*tim = TIM7;
}

/** @brief Initializes a basic Timer
 *	@param idx The timer to be initialized.
 *	@returns 0 if successful and -1 if otherwise.
 *
 *	A basic timer is initialized for the puspose of being a triggering source
 *	for DAC.
 */
int timer_init(enum timer_index idx, bool interrupt, void (*callback)(void))
{
	TIM_TypeDef *tim;
	
	if ((idx != TIMER_IDX_6) & (idx != TIMER_IDX_7))
		return -1;
	
	timer_extract_base_pointer(idx, &tim);
	
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
	
	/* Enable interrupt */
	if (interrupt && (tim == TIM6)) {
		if (callback != NULL) {
			NVIC_EnableIRQ(TIM6_DAC_IRQn);
			timer6_callback = callback;
		} else {
			return -1;
		}
	} else if (interrupt && (tim == TIM7)) {
		if (callback != NULL) {
			NVIC_EnableIRQ(TIM7_IRQn);
			timer7_callback = callback;	
		} else {
			return -1;
		}
	}
	
	return 0;
}

/** @brief Writes to the counter register
 *	@param idx The timer to configure.
 *	@param val The value of the counter.
 *	@returns 0 if successful and -1 if otherwise.
 *
 *	This function writes to the counter register which is the ARR register.
 *
 *	@note It is best to disable the timer before updating the counter.
 */
int timer_write_counter(enum timer_index idx, uint16_t val)
{
	TIM_TypeDef *tim;
	
	if ((idx != TIMER_IDX_6) & (idx != TIMER_IDX_7))
		return -1;
	
	timer_extract_base_pointer(idx, &tim);

	tim->ARR = val;
	return 0;
}

/** @brief Writes to the prescaler register
 *	@param idx The timer to configure.
 *	@param val The value of the prescaler.
 *	@returns 0 if successful and -1 if otherwise.
 *
 *	@note It is best to disable the timer before updating the prescaler.
 */
int timer_write_prescaler(enum timer_index idx, uint16_t val)
{
	TIM_TypeDef *tim;
	
	if ((idx != TIMER_IDX_6) & (idx != TIMER_IDX_7))
		return -1;
	
	timer_extract_base_pointer(idx, &tim);

	tim->PSC = val;
	return 0;
}

/** @brief Enables Timer interrupt
 *	@param idx The timer to configure.
 *	@returns 0 if successful and -1 if otherwise.
 *
 *	@note The timer needs to be configured to enable interrupt in timer_init in
 *	order for this to work.
 */
int timer_enable_interrupt(enum timer_index idx)
{
	TIM_TypeDef *tim;
	
	if ((idx != TIMER_IDX_6) & (idx != TIMER_IDX_7))
		return -1;
	
	timer_extract_base_pointer(idx, &tim);
	
	tim->SR	  &= ~(TIM_SR_UIF); /* Clear interrupt flag */
	tim->DIER |= TIM_DIER_UIE;	/* Enable interrupt */
	
	return 0;
}

/** @brief Disables Timer interrupt
 *	@param idx The timer to configure.
 *	@returns 0 if successful and -1 if otherwise.
 */
int timer_disable_interrupt(enum timer_index idx)
{
	TIM_TypeDef *tim;
	
	if ((idx != TIMER_IDX_6) & (idx != TIMER_IDX_7))
		return -1;
	
	timer_extract_base_pointer(idx, &tim);
	
	tim->SR	  &= ~(TIM_SR_UIF); 	/* Clear interrupt flag */
	tim->DIER &= ~(TIM_DIER_UIE);	/* Disable interrupt */
	
	return 0;
}

/** @brief Disables timer counting
 *	@param idx Timer to be configured.
 *	@returns 0 if successful and -1 if otherwise.
 */
int timer_disable(enum timer_index idx)
{
	TIM_TypeDef *tim;
	
	if ((idx != TIMER_IDX_6) & (idx != TIMER_IDX_7))
		return -1;
	
	timer_extract_base_pointer(idx, &tim);

	tim->CR1 &= ~(TIM_CR1_CEN);
	return 0;
}

/** @brief Enables timer counting
 *	@param idx Timer to be configured.
 *	@returns 0 if successful and -1 if otherwise.
 */
int timer_enable(enum timer_index idx)
{
	TIM_TypeDef *tim;
	
	if ((idx != TIMER_IDX_6) & (idx != TIMER_IDX_7))
		return -1;
	
	timer_extract_base_pointer(idx, &tim);

	tim->CR1 |= TIM_CR1_CEN;
	return 0;
}

/** @name Timer Interrupt Service Routine. */
/** @{*/

/** @brief IRQ Handler for Timer 6
 */
void TIM6_DAC_IRQHandler(void)
{
	TIM6->SR &= ~(TIM_SR_UIF);

	if (timer6_callback)
		timer6_callback();
}

/** @brief IRQ Handler for Timer 7
 */
void TIM7_IRQHandler(void)
{
	TIM7->SR &= ~(TIM_SR_UIF);

	if (timer7_callback)
		timer7_callback();
}

/** @}*/
