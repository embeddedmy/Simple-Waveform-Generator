
/** @file dac.h
 *  @brief DAC driver include file
 *  @author Dennis Law
 *  @date April 2016
 */
 
 #ifndef DAC_H
 #define DAC_H
 
 #include "stm32f0xx.h"
 
 int dac_init(int chn);
 
 int DAC_disable(int chn);
 int DAC_enable(int chn);
 
 #endif	/* DAC_H */
