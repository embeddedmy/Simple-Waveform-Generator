/*----------------------------------------------------------------------------
 * Name:    Serial.c
 * Purpose: Low Level Serial Routines
 * Note(s): possible defines select the used communication interface:
 *                        - USART2 interface  (default)
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2014 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include "stm32f0xx.h"                  // Device header
#include "Serial.h"
#include <stdio.h>

/*----------------------------------------------------------------------------
 Define  USART
 *----------------------------------------------------------------------------*/
#define USARTx  USART2


/*----------------------------------------------------------------------------
 Define  Baudrate setting (BRR) for USART
 *----------------------------------------------------------------------------*/
#define __DIV(__PCLK, __BAUD)       ((__PCLK*25)/(4*__BAUD))
#define __DIVMANT(__PCLK, __BAUD)   (__DIV(__PCLK, __BAUD)/100)
#define __DIVFRAQ(__PCLK, __BAUD)   (((__DIV(__PCLK, __BAUD) - (__DIVMANT(__PCLK, __BAUD) * 100)) * 16 + 50) / 100)
#define __USART_BRR(__PCLK, __BAUD) ((__DIVMANT(__PCLK, __BAUD) << 4)|(__DIVFRAQ(__PCLK, __BAUD) & 0x0F))

/** Ring buffer size */
#define SER_RBUF_SIZE	500

/** The structure for a ring buffer */
struct SER_ringBuf {
	unsigned char buffer[SER_RBUF_SIZE];
	volatile int head;
	volatile int tail;
};

/** @name Intermediary ring buffers for sending and receiving data. */
/** @{*/

static struct SER_ringBuf rx_rbuf; 
static struct SER_ringBuf tx_rbuf;

/** @}*/

/** @brief Reads a single byte from the rx ring buffer.
 *	@param output The container for holding the output read.
 *	@returns 0 if successful and -1 if there is nothing to read.
 */
static int rx_rbuf_read(unsigned char *output)
{
	if (rx_rbuf.head == rx_rbuf.tail)
		return -1;
	
	*output = rx_rbuf.buffer[rx_rbuf.tail];
	rx_rbuf.tail = (unsigned int)((rx_rbuf.tail + 1) % SER_RBUF_SIZE);
	
	return 0;
}

/** @brief Writes a single byte into the rx ring buffer.
 *	@param input The char to be written to the buffer.
 */
static void rx_rbuf_write(unsigned char input)
{
	int i = (rx_rbuf.head + 1) % SER_RBUF_SIZE;
    
	rx_rbuf.buffer[rx_rbuf.head] = input;
	rx_rbuf.head = i;
}

/** @brief Reads a single byte from the tx ring buffer.
 *	@param output The container to store the read character.
 *	@returns 0 if successful and -1 if there is no character
 *	to be read.
 */
static int tx_rbuf_read(unsigned char *output)
{
	if (tx_rbuf.head == tx_rbuf.tail)
		return -1;
		
	*output = tx_rbuf.buffer[tx_rbuf.tail];
	tx_rbuf.tail = (unsigned int)((tx_rbuf.tail + 1) % SER_RBUF_SIZE);

	return 0;
}

/**	@brief Writes a single character to the tx ring buffer.
 *	@param input The character to be written to the tx ring buffer.
 */
static void tx_rbuf_write(unsigned char input)
{
	int i = (tx_rbuf.head + 1) % SER_RBUF_SIZE;
	
    tx_rbuf.buffer[tx_rbuf.head] = input;
	tx_rbuf.head = i;
}

/*----------------------------------------------------------------------------
  Initialize UART pins, Baudrate
 *----------------------------------------------------------------------------*/
void SER_Initialize (void) {

  RCC->AHBENR  |=  (   1ul << 17);         /* Enable GPIOA clock              */
  RCC->APB1ENR |=  (   1ul << 17);         /* Enable USART#2 clock            */

  /* Configure PA3 to USART2_RX, PA2 to USART2_TX */
  GPIOA->AFR[0] &= ~((15ul << 4* 2) | (15ul << 4* 3));
  GPIOA->AFR[0] |=  (( 1ul << 4* 2) | ( 1ul << 4* 3));
  GPIOA->MODER  &= ~(( 3ul << 2* 2) | ( 3ul << 2* 3));
  GPIOA->MODER  |=  (( 2ul << 2* 2) | ( 2ul << 2* 3));

  NVIC_EnableIRQ(USART2_IRQn);

  USARTx->BRR  = __USART_BRR(48000000ul, 115200ul);  /* 115200 baud @ 48MHz   */
  USARTx->CR3   = 0x0000;                  /* no flow control                 */
  USARTx->CR2   = 0x0000;                  /* 1 stop bit                      */
  USARTx->CR1   = ((   1ul <<  2) |        /* enable RX                       */
                   (   1ul <<  3) |        /* enable TX                       */
                   (   0ul << 12) |        /* 1 start bit, 8 data bits        */
                   (   1ul <<  0) |       /* enable USART                    */
					USART_CR1_RXNEIE );		/* Enable receive interrupt */
}


/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/
unsigned char SER_PutChar (unsigned char ch)
{
	tx_rbuf_write(ch);
	USARTx->CR1 |= USART_CR1_TXEIE;
	return ch;
}

/*----------------------------------------------------------------------------
  Read character from Serial Port
 *----------------------------------------------------------------------------*/
unsigned char SER_GetChar (void)
{
	unsigned char input;

	while(rx_rbuf_read(&input));
	return input;
}

int SER_GetChar_nonBlocking(unsigned char *output)
{
	return (rx_rbuf_read(output));
}

/** @brief Function for handling rx interrupts.
 */
static void SER_handleRxInterrupt(void)
{
	rx_rbuf_write((unsigned char)(USARTx->RDR & 0xFF));
}

/** @brief Function for handling tx interrupts .
 */
static void SER_handleTxInterrupt(void)
{
	unsigned char output;
	
	if (!tx_rbuf_read(&output)) {
		USARTx->TDR = (output & 0xFF);
	} else {
		USARTx->CR1 &= ~(USART_CR1_TXEIE);
	}
}

/** @brief IRQ Handler function for USART2 */
void USART2_IRQHandler(void)
{
	if (USARTx->ISR & USART_ISR_RXNE)
		SER_handleRxInterrupt();
	
	if (USARTx->ISR & USART_ISR_TXE)
		SER_handleTxInterrupt();
}
