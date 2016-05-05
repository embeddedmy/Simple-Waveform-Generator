
/** @file main.c
 *  @brief Implements the main function for the waveform generator.
 *  @author Dennis Law
 *  @date April 2016
 */

#include "stm32f0xx.h"
#include "timer.h"

#include "dac.h"
#include "timer.h"
#include "dma.h"
#include "apptree.h"
#include "wave_gen.h"

#include "serial.h"

/*system setting structure*/
struct system_settings {
	enum waveform wave;
	unsigned int frequency;
	float amplitude;

	bool changed;
};

/*system setting default*/
struct system_settings settings = {
	SINE,	/* wave */
	1000,	/* frequency */
	3.3,	/* amplitude */
	true	/* changed */
};

/** @brief Draw blank screen in serial terminal
 */
void print_blankscreen(void)
{
	int i;
	for(i = 0; i < 24; i++)
		printf("\r\n");
}

/** @brief update waveform type from user input
 *	@param *parent parent structure of apptree menu
 *	@param child_idx handle the waveform type from selection
 */
void change_waveform(struct apptree_node *parent, int child_idx)
{
	print_blankscreen();
	
	switch (child_idx) {
	case SINE:
		settings.wave = SINE;
		printf("Waveform changed to SINE!\r\n");
		break;
	case SQUARE:
		settings.wave = SQUARE;
		printf("Waveform changed to SQUARE!\r\n");
		break;
	case TRIANGLE:
		settings.wave = TRIANGLE;
		printf("Waveform changed to TRIANGLE!\r\n");
		break;
	case SAWTOOTH:
		settings.wave = SAWTOOTH;
		printf("Waveform changed to SAWTOOTH!\r\n");
		break;
	default:
		return;
	}
	
	printf("Press any key to continue ...\r\n");
	getchar();
	
	settings.changed = true;
}

/** @brief update waveform frequency from user input
 *	@param *parent parent structure of apptree menu
 *	@param child_idx handle the waveform frequency that user input
 */
void change_frequency(struct apptree_node *parent, int child_idx)
{
	unsigned int max_freq;
	unsigned int min_freq;
	unsigned int new_freq;
	int ret;
	
	//max_freq = 2000;
	max_freq = get_max_freq();
	//min_freq = 1;
	min_freq = get_min_freq();
	
	print_blankscreen();
	
repeat:
	printf("Current frequency: %d\r\n", settings.frequency);
	printf("Maximum allowable frequency: %d\r\n", max_freq);
	printf("Minimum allowable frequency: %d\r\n", min_freq);
	printf("\r\n");
	printf("Enter new freqency: ");
	
	ret = scanf("%d", &new_freq);
	printf("\r\n");
	
	if (ret <= 0) {
		printf("Error! Invalid input\r\n");
		printf("\r\n");
		goto repeat;
	}
	
	if (new_freq > max_freq) {
		printf("Error! Value exceeded maximum limit!\r\n");
		printf("\r\n");
		goto repeat;
	} else if (new_freq < min_freq) {
		printf("Error! Value preceeded minimum limit!\r\n");
		printf("\r\n");
		goto repeat;
	}
	
	printf("Frequency changed to %d!\r\n", new_freq);
	printf("Press any key to continue ...\r\n");
	getchar();
	
	settings.frequency = new_freq;
	settings.changed = true;
}

/** @brief update waveform amplitude from user input
 *	@param *parent parent structure of apptree menu
 *	@param child_idx handle the waveform amplitude from user input
 */
void change_amplitude(struct apptree_node *parent, int child_idx)
{
	float max_amp;
	float min_amp;
	float new_amp;
	int ret;
	
	//max_amp = 3.3;
	max_amp = get_max_amplitude();
	//min_amp = 1.0;
	min_amp = get_min_amplitude();
	
	print_blankscreen();
	
repeat:
	printf("Current amplitude: %.1f\r\n", settings.amplitude);
	printf("Maximum allowable amplitude: %.1f\r\n", max_amp);
	printf("Minimum allowable amplitude: %.1f\r\n", min_amp);
	printf("\r\n");
	printf("Enter new amplitude: ");
	
	ret = scanf("%f", &new_amp);
	printf("\r\n");
	
	if (ret <= 0) {
		printf("Error! Invalid input\r\n");
		printf("\r\n");
		goto repeat;
	}
	
	if (new_amp > max_amp) {
		printf("Error! Value exceeded maximum limit!\r\n");
		printf("\r\n");
		goto repeat;
	} else if (new_amp < min_amp) {
		printf("Error! Value preceeded minimum limit!\r\n");
		printf("\r\n");
		goto repeat;
	}
	
	/* Change new_amp to .1 precision */
	new_amp = ((float)((int)(new_amp * 10)))/10;
	
	printf("Amplitude changed to %.1f!\r\n", new_amp);
	printf("Press any key to continue ...\r\n");
	getchar();
	
	settings.amplitude = new_amp;
	settings.changed = true;
}

/** @brief printout waveform setting status
 *	@param *parent parent structure of apptree menu
 *	@param child_idx is not used
 */
void print_status(struct apptree_node *parent, int child_idx)
{
	print_blankscreen();
	
	printf("Current system settings are as follows:\r\n");
	printf("\r\n");
	
	switch (settings.wave) {
	case SINE:
		settings.wave = SINE;
		printf("\tWaveform:\tSINE\r\n");
		break;
	case SQUARE:
		settings.wave = SQUARE;
		printf("\tWaveform:\tSQUARE\r\n");
		break;
	case TRIANGLE:
		settings.wave = TRIANGLE;
		printf("\tWaveform:\tTRIANGLE\r\n");
		break;
	case SAWTOOTH:
		settings.wave = SAWTOOTH;
		printf("\tWaveform:\tSAWTOOTH\r\n");
		break;
	default:
		return;
	}
	
	printf("\tFrequency:\t%d\r\n", settings.frequency);
	printf("\tAmplitude:\t%.1f\r\n", settings.amplitude);
	printf("\r\n");
	printf("Press any key to continue ...\r\n");
	getchar();
}

/** @brief read serial input from user
 *	@param *input point to user input key
 *	@return 0 = read sucess -1 = read failed
 */
int read(char *input)
{
	return (serial_getchar_nonblocking((unsigned char *)input));
}


/** @brief Configures the SystemCoreClock using HSI.
 *	@note HSE is not populated on Nucleo board.
 */
void SystemCoreClockConfigure(void)
{
	RCC->CR |= ((uint32_t)RCC_CR_HSION);                     /* Enable HSI */
	while ((RCC->CR & RCC_CR_HSIRDY) == 0);                  /* Wait for HSI Ready */

	RCC->CFGR = RCC_CFGR_SW_HSI;                             /* HSI is system clock */
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);  /* Wait for HSI used as system clock */

	FLASH->ACR  = FLASH_ACR_PRFTBE;                          /* Enable Prefetch Buffer */
	FLASH->ACR |= FLASH_ACR_LATENCY;                         /* Flash 1 wait state */

	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;                         /* HCLK = SYSCLK */
	RCC->CFGR |= RCC_CFGR_PPRE_DIV1;                         /* PCLK = HCLK */

	RCC->CR &= ~RCC_CR_PLLON;                                /* Disable PLL */

	/* PLL configuration:  = HSI/2 * 12 = 48 MHz */
	RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL);
	RCC->CFGR |=  (RCC_CFGR_PLLSRC_HSI_Div2 | RCC_CFGR_PLLMULL12);

	RCC->CR |= RCC_CR_PLLON;                                 /* Enable PLL */
	while((RCC->CR & RCC_CR_PLLRDY) == 0) __NOP();           /* Wait till PLL is ready */

	RCC->CFGR &= ~RCC_CFGR_SW;                               /* Select PLL as system clock source */
	RCC->CFGR |=  RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  /* Wait till PLL is system clock src */
}

/** @brief main function
 */
int main (void)
{
	struct apptree_keybindings keys;
	
	struct apptree_node *n_master;
	
	struct apptree_node *n_waveform;
	struct apptree_node *n_frequency;
	struct apptree_node *n_amplitude;
	struct apptree_node *n_status;
	
	struct apptree_node *n_sine;
	struct apptree_node *n_square;
	struct apptree_node *n_triangle;
	struct apptree_node *n_sawtooth;
	
	SystemCoreClockConfigure();                 /* Configure HSI as System Clock */
	SystemCoreClockUpdate();
	
	serial_init(115200);
	
	keys.up		= 'i';
	keys.down 	= 'k';
	keys.select = 'l';
	keys.back 	= 'j';
	keys.home 	= 'u';
	
	apptree_init(&n_master, "Simple Waveform Generator v0.1", &keys, &read);
	
	apptree_create_node(&n_waveform, n_master, "Waveform", "Change output waveform", NULL);
	apptree_create_node(&n_frequency, n_master, "Frequency", "Change output frequency", &change_frequency);
	apptree_create_node(&n_amplitude, n_master, "Amplitude", "Change output amplitude", &change_amplitude);
	apptree_create_node(&n_status, n_master, "Status", "View system status", &print_status);
	
	apptree_create_node(&n_sine, n_waveform, "Sine", "Change to sine wave", &change_waveform);
	apptree_create_node(&n_square, n_waveform, "Sawtooth", "Change to square wave", &change_waveform);
	apptree_create_node(&n_triangle, n_waveform, "Triangle", "Change to triangle wave", &change_waveform);
	apptree_create_node(&n_sawtooth, n_waveform, "Square", "Change to sawtooth wave", &change_waveform);
	
	apptree_enable();
	
	while (1){
		apptree_handle_input();
		if(settings.changed==true)
		{
			generate_waveform(settings.wave, settings.frequency, settings.amplitude);
			settings.changed=false;
		}
	}
	
	
}
