
/** @file dac.h
 *  @brief DAC driver include file
 *  @author Dennis Law
 *  @date April 2016
 */
 
#ifndef DAC_H
#define DAC_H

#include "stm32f0xx.h"

enum dac_channel {
	DAC_CHN_1 = 0,
	DAC_CHN_2 = 1
};

int dac_init(enum dac_channel chn);

int dac_disable(enum dac_channel chn);
int dac_enable(enum dac_channel chn);

#endif	/* DAC_H */
