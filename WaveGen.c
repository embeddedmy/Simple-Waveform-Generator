/** @file WaveGen.c
 *  @brief Waveform Generation Control
 *
 *	@details The current logic uses DAC on Timer and DMA to generate different
 *	types of waveform (Sine, Sawtooth..) with diferent frequency and amplitude
 *
 *  @author Leong Keit
 *  @date April 2016
 */

#include <math.h>
#include "WaveGen.h"

/*data structure to store waveform sampling data*/
uint32_t DMAData[MAX_MEMORY_ALLOWED];

/** @brief Process Waveform parameter and calculate the timing and number of sample
 *	@param waveform_types is the waveform types
 *	frequency is the waveform frequency in Hz
 *	amplitude is the floating point value of waveform amplitude in v 
 *	pTiming_ns is pointer to store output sampling timing in ns
 *	pNoofSample is pointer to store output sample number 
 *	@returns 1 if parameter acceptable and 0 if otherwise.
 */
static uint8_t ProcessWaveformParam(enum WAVEFORM_TYPES waveform_types, uint32_t frequency, float amplitude, uint32_t* pTiming_ns, uint32_t* pNoofSample)
{
	uint32_t period_in_ns;

	period_in_ns=1000000000/frequency;
	
	if(amplitude>MAX_AMPLITUDE_FLOAT||amplitude<MIN_AMPLITUDE_FLOAT)
	{
		return 0;
	}
	
	switch(waveform_types)
	{
		case WAVEFORM_TYPE_SINE:
		case WAVEFORM_TYPE_SAWTOOTH :
		case WAVEFORM_TYPE_TRIANGULAR:
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
		case WAVEFORM_TYPE_SQUARE:
			*pNoofSample=2;
			*pTiming_ns = period_in_ns/2;
		break;
		default:
			return 0;
	}
	
	return 1;
}

/** @brief Generate SawTooth WaveForm Sampling Data
 *	@param NoOfSample is the number of sample for this waveform
 *	Amplitude_In_Resolution is amplitude of waveform in DAC resolution
 */
static void GenerateSawToothTable(uint32_t NoOfSample, uint32_t Amplitude_In_Resolution)
{
	uint32_t i;
	for(i=0;i<NoOfSample;i++)
	{
		DMAData[i]=(Amplitude_In_Resolution*i/NoOfSample);
	}
}

/** @brief Generate Triangular WaveForm Sampling Data
 *	@param NoOfSample is the number of sample for this waveform
 *	Amplitude_In_Resolution is amplitude of waveform in DAC resolution
 */
static void GenerateTriangularTable(uint32_t NoOfSample, uint32_t Amplitude_In_Resolution)
{
	uint32_t i;

	for(i=0;i<NoOfSample/2;i++)
	{
		DMAData[i]=2*(Amplitude_In_Resolution*i/NoOfSample);
	}

	for(i=0;i<NoOfSample/2;i++)
	{
		DMAData[i+NoOfSample/2]=Amplitude_In_Resolution-(2*(Amplitude_In_Resolution*i/NoOfSample));
	}
}

/** @brief Generate Sine WaveForm Sampling Data
 *	@param NoOfSample is the number of sample for this waveform
 *	Amplitude_In_Resolution is amplitude of waveform in DAC resolution
 */
static void GenerateSineTable(uint32_t NoOfSample, uint32_t Amplitude_In_Resolution)
{
	uint32_t i;
	
	for(i=0;i<NoOfSample;i++)
	{
		DMAData[i]=(sin(i*2*PI_VALUE/NoOfSample)+1)*(Amplitude_In_Resolution+1)/2;
	}
}

/** @brief Generate Square WaveForm Sampling Data
 *	@param NoOfSample is the number of sample for this waveform
 *	Amplitude_In_Resolution is amplitude of waveform in DAC resolution
 */
static void GenerateSquareTable(uint32_t Amplitude_In_Resolution)
{
	DMAData[0]=0;
	DMAData[1]=Amplitude_In_Resolution;
}

/** @brief Generate WaveForm Sampling Data according to types
 *	@param  waveform_types indicates the types of waveform
 *	NoOfSample is the number of sample for this waveform
 *	Amplitude_In_Resolution is amplitude of waveform in DAC resolution
 */
static void GenerateWaveFormTable(enum WAVEFORM_TYPES waveform_types, uint32_t NoOfSample, uint32_t Amplitude_In_Resolution)
{
	switch (waveform_types)
	{
		case WAVEFORM_TYPE_SINE:
			GenerateSineTable(NoOfSample,Amplitude_In_Resolution);
		break;
		case WAVEFORM_TYPE_SAWTOOTH:
			GenerateSawToothTable(NoOfSample,Amplitude_In_Resolution);
		break;
		case WAVEFORM_TYPE_TRIANGULAR:
			GenerateTriangularTable(NoOfSample,Amplitude_In_Resolution);
		break;
		case WAVEFORM_TYPE_SQUARE:
			GenerateSquareTable(Amplitude_In_Resolution);
		break;
	}
}

/** @brief configure the DAC, DMA and timer to trigger waveform generation
 *	@param  noofsample is the number of sample for this waveform
 *	periodinns is timing in to grab a sample to output in DAC in nano seconds
 */
static void ConfigureDAC(uint32_t noofsample, uint32_t periodinns)
{
	uint32_t timercount;
	uint32_t timerprescalar;
	
	//disable all peripheral to make changes
	TIMER_disable(TIM6);
	dma_disable(DMA_CHN);
	DAC_disable(DAC_CHN);
	

	/* Initialize DAC */
	dac_init(DAC_CHN);
	DAC_enable(DAC_CHN);
	
	/* Initialize DMA */
	dma_init(DMA_CHN,DMAData,noofsample);
	dma_enable(DMA_CHN);

	/* Initialize Timer */
	
	timerprescalar=1;
	//prescalar and count calculation
	while(1)
	{
		timercount=periodinns/(TIMER_TICK_NS*(timerprescalar+1));
		if(timercount>65535)
		{
			timerprescalar=timerprescalar*2;
		}
		else
		{
			break;
		}
	}
	
	timer_write_counter(TIM6, timercount);
	timer_write_prescaler(TIM6,timerprescalar);
	timer_init(TIM6, 0, 0);
	TIMER_enable(TIM6);
}

/** @brief Draw waveform in DAC output port according to processed information
 *	@param  waveform_types indicates the types of waveform
 *	Amplitude_In_Resolution is amplitude of waveform in DAC resolution
 *	timing_ns is timing in to grab a sample to output in DAC in nano seconds
 *	noOfSample is the number of sample for this waveform
 */
static void DrawWaveform(enum WAVEFORM_TYPES waveform_types, uint32_t amplitude_in_resolution, uint32_t timing_ns,	uint32_t noOfSample)
{
	GenerateWaveFormTable(waveform_types,noOfSample,amplitude_in_resolution);
	ConfigureDAC(noOfSample, timing_ns);
}

/** @brief Draw waveform in DAC output port according to waveform parameter
 *	@param  waveform_types indicates the types of waveform
 *	frequency is the waveform frequency in Hz
 *	amplitude is the floating point value of waveform amplitude in v 
 */
void GenerateWaveform(enum WAVEFORM_TYPES waveform_types, uint32_t frequency, float amplitude)
{
	uint32_t timing_ns;
	uint32_t noOfSample;
	uint32_t amplitude_in_resolution;
	
	if(ProcessWaveformParam(waveform_types, frequency, amplitude, &timing_ns, &noOfSample))
	{
		amplitude_in_resolution = amplitude*DAC_RESOLUTION/DAC_VREF;
		DrawWaveform(waveform_types,amplitude_in_resolution,timing_ns,noOfSample);
	}
	else
	{
		TIMER_disable(TIM6);
	}
}

/** @brief Retrieve the maximum waveform frequncy that the system supports
 *	@returns value for maximum frequency in Hz that the system supports.
*/
uint32_t GetMaxFreq(void)
{
		return MAX_FREQUENCY;
}

/** @brief Retrieve the minimum waveform amplitude that the system supports
 *	@returns value for minimum amplitude in V that the system supports.
*/

uint32_t GetMinFreq(void)
{
		return MIN_FREQUENCY;
}
/** @brief Retrieve the maximum waveform amplitude that the system supports
 *	@returns value for maximum amplitude in V that the system supports.
*/
float GetMaxAmplitude(void)
{
		return MAX_AMPLITUDE_FLOAT;
}
/** @brief Retrieve the minimum waveform amplitude that the system supports
 *	@returns value for minimum amplitude in V that the system supports.
*/
float GetMinAmplitude(void)
{
		return MIN_AMPLITUDE_FLOAT;
}

