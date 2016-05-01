
/** @file main.c
 *  @brief Implements the main function for the waveform generator.
 *  @author Dennis Law
 *  @date April 2016
 */

#include "stm32f0xx.h"

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
	SystemCoreClockConfigure();                 /* Configure HSI as System Clock */
	SystemCoreClockUpdate();
	
	while (1);
}
