/** @file wave_gen.c
 *  @brief Waveform Generation Control
 *
 *	@details The current logic uses DAC on Timer and DMA to generate different
 *	types of waveform (Sine, Sawtooth..) with diferent frequency and amplitude
 *
 *  @author Leong Keit
 *  @date April 2016
 */

#include <math.h>
#include "wave_gen.h"

/*data structure to store waveform sampling data*/
uint32_t DMAData[MAX_MEMORY_ALLOWED];

/** @brief Process Waveform parameter and calculate the timing and number of sample
 *	@param waveform is the waveform types
 *	frequency is the waveform frequency in Hz
 *	amplitude is the floating point value of waveform amplitude in v 
 *	pTiming_ns is pointer to store output sampling timing in ns
 *	pNoofSample is pointer to store output sample number 
 *	@returns 1 if parameter acceptable and 0 if otherwise.
 */
static uint8_t process_waveform_param(enum waveform waveform, uint32_t frequency, float amplitude, uint32_t* pTiming_ns, uint32_t* pNoofSample)
{
	uint32_t period_in_ns;

	period_in_ns=1000000000/frequency;
	
	if(amplitude>MAX_AMPLITUDE_FLOAT||amplitude<MIN_AMPLITUDE_FLOAT)
	{
		return 0;
	}
	
	switch(waveform)
	{
		case SINE:
		case SAWTOOTH :
		case TRIANGLE:
			*pNoofSample = period_in_ns/DAC_SAMPLE_WAIT_TIME_NS;
		
			if(*pNoofSample<MIN_SAMPLE_PER_CYCLE)
				return 0;
				
			if(*pNoofSample>MAX_MEMORY_ALLOWED)
			{
				*pNoofSample=period_in_ns/DAC_SAMPLE_MAX_DRAG_TIME_NS;
				if(*pNoofSample>MAX_MEMORY_ALLOWED)
				{
					return 0;
				}
				else
				{
					*pTiming_ns=period_in_ns/MAX_MEMORY_ALLOWED;
					*pNoofSample=MAX_MEMORY_ALLOWED;
				}
			}
			else
			{
				//timing = DAC_SAMPLE_WAIT_TIME_MS;
				*pTiming_ns =  period_in_ns/(*pNoofSample);
			}
		break;
		case SQUARE:
			*pNoofSample=2;
			*pTiming_ns =  period_in_ns/(*pNoofSample);
		break;
		default:
			return 0;
	}
	
	return 1;
}

/** @brief Generate SawTooth WaveForm Sampling Data
 *	@param NoOfSample is the number of sample for this waveform
 *	amplitude_in_resolution is amplitude of waveform in DAC resolution
 */
static void generate_sawtooth_table(uint32_t NoOfSample, uint32_t amplitude_in_resolution)
{
	uint32_t i;
	for(i=0;i<NoOfSample;i++)
	{
		DMAData[i]=(amplitude_in_resolution*i/NoOfSample);
	}
}

/** @brief Generate Triangular WaveForm Sampling Data
 *	@param NoOfSample is the number of sample for this waveform
 *	amplitude_in_resolution is amplitude of waveform in DAC resolution
 */
static void generate_triangular_table(uint32_t NoOfSample, uint32_t amplitude_in_resolution)
{
	uint32_t i;

	for(i=0;i<NoOfSample/2;i++)
	{
		DMAData[i]=2*(amplitude_in_resolution*i/NoOfSample);
	}

	for(i=0;i<NoOfSample/2;i++)
	{
		DMAData[i+NoOfSample/2]=amplitude_in_resolution-(2*(amplitude_in_resolution*i/NoOfSample));
	}
}

/** @brief Generate Sine WaveForm Sampling Data
 *	@param NoOfSample is the number of sample for this waveform
 *	amplitude_in_resolution is amplitude of waveform in DAC resolution
 */
static void generate_sine_table(uint32_t NoOfSample, uint32_t amplitude_in_resolution)
{
	uint32_t i;
	
	for(i=0;i<NoOfSample;i++)
	{
		DMAData[i]=(sin(i*2*PI_VALUE/NoOfSample)+1)*(amplitude_in_resolution+1)/2;
	}
}

/** @brief Generate Square WaveForm Sampling Data
 *	@param NoOfSample is the number of sample for this waveform
 *	amplitude_in_resolution is amplitude of waveform in DAC resolution
 */
static void generate_square_table(uint32_t amplitude_in_resolution)
{
	DMAData[0]=0;
	DMAData[1]=amplitude_in_resolution;
}

/** @brief Generate WaveForm Sampling Data according to types
 *	@param  waveform indicates the types of waveform
 *	NoOfSample is the number of sample for this waveform
 *	amplitude_in_resolution is amplitude of waveform in DAC resolution
 */
static void generate_waveform_table(enum waveform waveform, uint32_t NoOfSample, uint32_t amplitude_in_resolution)
{
	switch (waveform)
	{
		case SINE:
			generate_sine_table(NoOfSample,amplitude_in_resolution);
		break;
		case SAWTOOTH:
			generate_sawtooth_table(NoOfSample,amplitude_in_resolution);
		break;
		case TRIANGLE:
			generate_triangular_table(NoOfSample,amplitude_in_resolution);
		break;
		case SQUARE:
			generate_square_table(amplitude_in_resolution);
		break;
	}
}

/** @brief configure the DAC, DMA and timer to trigger waveform generation
 *	@param  noofsample is the number of sample for this waveform
 *	period_in_ns is timing in to grab a sample to output in DAC in nano seconds
 */
static void configure_dac(uint32_t noofsample, uint32_t period_in_ns)
{
	uint32_t timer_count;
	uint32_t timer_prescalar;
	
	//disable all peripheral to make changes
	timer_disable(TIMER_IDX);
	dma_disable(DMA_CHN);
	dac_disable(DAC_CHN);
	

	/* Initialize DAC */
	dac_init(DAC_CHN);
	dac_enable(DAC_CHN);
	
	/* Initialize DMA */
	dma_init(DMA_CHN,DMAData,noofsample);
	dma_enable(DMA_CHN);

	/* Initialize Timer */
	
	timer_prescalar=1;
	//prescalar and count calculation
	while(1)
	{
		timer_count=period_in_ns/(TIMER_TICK_NS*(timer_prescalar+1));
		if(timer_count>65535)
		{
			timer_prescalar=timer_prescalar*2;
		}
		else
		{
			break;
		}
	}

	timer_init(TIMER_IDX, 0, 0);
	timer_write_counter(TIMER_IDX, timer_count);
	timer_write_prescaler(TIMER_IDX,timer_prescalar);
	timer_enable(TIMER_IDX);
}

/** @brief Draw waveform in DAC output port according to processed information
 *	@param  waveform indicates the types of waveform
 *	amplitude_in_resolution is amplitude of waveform in DAC resolution
 *	timing_ns is timing in to grab a sample to output in DAC in nano seconds
 *	noOfSample is the number of sample for this waveform
 */
static void draw_waveform(enum waveform waveform, uint32_t amplitude_in_resolution, uint32_t timing_ns,	uint32_t noOfSample)
{
	generate_waveform_table(waveform,noOfSample,amplitude_in_resolution);
	configure_dac(noOfSample, timing_ns);
}

/** @brief Draw waveform in DAC output port according to waveform parameter
 *	@param  waveform indicates the types of waveform
 *	frequency is the waveform frequency in Hz
 *	amplitude is the floating point value of waveform amplitude in v 
 */
void generate_waveform(enum waveform waveform, uint32_t frequency, float amplitude)
{
	uint32_t timing_ns;
	uint32_t noOfSample;
	uint32_t amplitude_in_resolution;
	
	if(process_waveform_param(waveform, frequency, amplitude, &timing_ns, &noOfSample))
	{
		amplitude_in_resolution = amplitude*DAC_RESOLUTION/DAC_VREF;
		draw_waveform(waveform,amplitude_in_resolution,timing_ns,noOfSample);
	}
	else
	{
		timer_disable(TIMER_IDX);
	}
}

/** @brief Retrieve the maximum waveform frequncy that the system supports
 *	@returns value for maximum frequency in Hz that the system supports.
*/
uint32_t get_max_freq(void)
{
		return MAX_FREQUENCY;
}

/** @brief Retrieve the minimum waveform amplitude that the system supports
 *	@returns value for minimum amplitude in V that the system supports.
*/

uint32_t get_min_freq(void)
{
		return MIN_FREQUENCY;
}
/** @brief Retrieve the maximum waveform amplitude that the system supports
 *	@returns value for maximum amplitude in V that the system supports.
*/
float get_max_amplitude(void)
{
		return MAX_AMPLITUDE_FLOAT;
}
/** @brief Retrieve the minimum waveform amplitude that the system supports
 *	@returns value for minimum amplitude in V that the system supports.
*/
float get_min_amplitude(void)
{
		return MIN_AMPLITUDE_FLOAT;
}

