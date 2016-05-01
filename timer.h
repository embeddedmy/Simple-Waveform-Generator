
/** @file timer.h
 *  @brief Timer driver include file
 *  @author Dennis Law
 *  @date April 2016
 */

#ifndef TIMER_H
#define TIMER_H

#include "stm32f0xx.h"

int timer_write_counter(TIM_TypeDef *tim, uint16_t val);
int timer_write_prescaler(TIM_TypeDef *tim, uint16_t val);
int timer_init(TIM_TypeDef *tim);

#endif	/* TIMER_H */
