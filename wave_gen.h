/** @file wave_gen.h
 *  @brief WaveGen Control include file 
 *  @author Leong Keit
 *  @date April 2016
 */

#ifndef WAVE_GEN_H
#define WAVE_GEN_H

#include "dac.h"
#include "dma.h"
#include "timer.h"

/*define DAC channel and DMA channel to use DAC and DMA driver*/
#define DAC_CHN			1
#define DMA_CHN			3

/* DAC peripheral limitation defines*/ 
#define TIMER_TICK_NS		21
#define CLOCK_SPEED			42000000
#define DAC_RESOLUTION 4096
#define DAC_VREF				3.3


/*enum for the waveform types supported*/
enum waveform {
	SINE 	 = 0,
	SAWTOOTH	 = 1,
	TRIANGLE = 2,
	SQUARE = 3
};

/*define for waveform data calculations*/
#define PI_VALUE 3.14159
#define MIN_SAMPLE_PER_CYCLE		50
#define DAC_SAMPLE_WAIT_TIME_NS		10000
#define DAC_SAMPLE_MAX_DRAG_TIME_NS	1000000
#define MAX_MEMORY_ALLOWED			2000

/*waveform parameter limitation defines*/
#define MAX_AMPLITUDE_FLOAT		3.3
#define MIN_AMPLITUDE_FLOAT		1.0
#define MAX_FREQUENCY (1000000000/(DAC_SAMPLE_WAIT_TIME_NS*MIN_SAMPLE_PER_CYCLE))
#define MIN_FREQUENCY 1

extern void generate_waveform(enum waveform waveform, uint32_t frequency, float amplitude);
extern uint32_t get_max_freq(void);
extern uint32_t get_min_freq(void);
extern float get_max_amplitude(void);
extern float get_min_amplitude(void);

#endif	/* WAVE_GEN_H */
