
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

#include "timer.h"
