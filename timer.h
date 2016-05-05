
/** @file timer.h
 *  @brief Timer driver include file
 *  @author Dennis Law
 *  @date April 2016
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include "stm32f0xx.h"

enum timer_index {
	TIMER_IDX_6 = 0,
	TIMER_IDX_7 = 1
};

int timer_init(enum timer_index idx, bool interrupt,
			void (*callback)(void));

int timer_write_counter(enum timer_index idx, uint16_t val);
int timer_write_prescaler(enum timer_index idx, uint16_t val);

int timer_disable_interrupt(enum timer_index idx);
int timer_enable_interrupt(enum timer_index idx);

int timer_disable(enum timer_index idx);
int timer_enable(enum timer_index idx);

#endif	/* TIMER_H */
