
/** @file serial.c
 *  @brief Serial driver
 *
 *	This serial driver is a modified version of the Serial driver from
 *	uVision/ARM development tool. This driver initializes USART2 which is
 *	connected to the ST-Link Debugger available on the NUCLEO-F072RB board.
 *	Two sets of ring buffers (one for tx and one for rx) are added for smooth
 *	receiving and transmitting of data.
 *
 *  @author Dennis Law
 *  @date May 2016
 */
 
#include <stdio.h>
#include "stm32f0xx.h"
#include "serial.h"

/** Baud rate settings */
#define __DIV(__PCLK, __BAUD)       ((__PCLK*25)/(4*__BAUD))
#define __DIVMANT(__PCLK, __BAUD)   (__DIV(__PCLK, __BAUD)/100)
#define __DIVFRAQ(__PCLK, __BAUD)   (((__DIV(__PCLK, __BAUD) - (__DIVMANT(__PCLK, __BAUD) * 100)) * 16 + 50) / 100)
#define __USART_BRR(__PCLK, __BAUD) ((__DIVMANT(__PCLK, __BAUD) << 4)|(__DIVFRAQ(__PCLK, __BAUD) & 0x0F))

/** The structure for a ring buffer */
struct serial_ringbuf {
	unsigned char buffer[SERIAL_RBUF_SIZE];
	volatile int head;
	volatile int tail;
};

static int rx_rbuf_read(unsigned char *output);
static int rx_rbuf_write(unsigned char input);
static int tx_rbuf_read(unsigned char *output);
static int tx_rbuf_write(unsigned char input);

static void serial_handle_rx_interrupt(void);
static void serial_handle_tx_interrupt(void);

/** @name Intermediary ring buffers
 *	Used as a FIFO buffer for queing data received and data to be sent.
 */
/** @{*/

static struct serial_ringbuf rx_rbuf; 
static struct serial_ringbuf tx_rbuf;

/** @}*/

/** @name Ring buffer functions
 *	Functions for writing into and reading from the ring buffers.
 */
/** @{*/

/** @brief Reads a single byte from the rx ring buffer.
 *	@param output The container for holding the output read.
 *	@returns 0 if successful and -1 if there is nothing to read.
 */
static int rx_rbuf_read(unsigned char *output)
{
	if (rx_rbuf.head == rx_rbuf.tail)
		return -1;
	
	*output = rx_rbuf.buffer[rx_rbuf.tail];
	rx_rbuf.tail = (unsigned int)((rx_rbuf.tail + 1) % SERIAL_RBUF_SIZE);
	
	return 0;
}

/** @brief Writes a single byte into the rx ring buffer.
 *	@param input The char to be written to the buffer.
 *	@returns 0 if successful and -1 if the buffer is full.
 */
static int rx_rbuf_write(unsigned char input)
{
	if ((rx_rbuf.tail + 1) % SERIAL_RBUF_SIZE == rx_rbuf.head)
		return -1;
	
	rx_rbuf.buffer[rx_rbuf.head] = input;
	rx_rbuf.head = (rx_rbuf.head + 1) % SERIAL_RBUF_SIZE;;
	return 0;
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
	tx_rbuf.tail = (unsigned int)((tx_rbuf.tail + 1) % SERIAL_RBUF_SIZE);

	return 0;
}

/**	@brief Writes a single character to the tx ring buffer.
 *	@param input The character to be written to the tx ring buffer.
 */
static int tx_rbuf_write(unsigned char input)
{
	if ((tx_rbuf.tail + 1) % SERIAL_RBUF_SIZE == tx_rbuf.head)
		return -1;
	
    tx_rbuf.buffer[tx_rbuf.head] = input;
	tx_rbuf.head = (tx_rbuf.head + 1) % SERIAL_RBUF_SIZE;
	return 0;
}

/** @}*/

/**	@brief Initializes USART2
 *	
 *	This function initializes USART2 which is connected to the RX and TX of the
 *	ST-Link Debugger.
 */
void serial_init(int baud)
{
	/* Enable clock */
	RCC->AHBENR  |=  (   1ul << 17);	/* Enable GPIOA clock */
	RCC->APB1ENR |=  (   1ul << 17);    /* Enable USART2 clock */

	/* Configure PA3 to USART2_RX, PA2 to USART2_TX */
	GPIOA->AFR[0] &= ~((15ul << 4* 2) | (15ul << 4* 3));
	GPIOA->AFR[0] |=  (( 1ul << 4* 2) | ( 1ul << 4* 3));
	GPIOA->MODER  &= ~(( 3ul << 2* 2) | ( 3ul << 2* 3));
	GPIOA->MODER  |=  (( 2ul << 2* 2) | ( 2ul << 2* 3));

	NVIC_EnableIRQ(USART2_IRQn);

	USART2->BRR  = __USART_BRR(48000000ul, baud); /* Clock at 48MHz */
	USART2->CR3   = 0x0000;             /* no flow control */
	USART2->CR2   = 0x0000;             /* 1 stop bit */
	USART2->CR1   = ((   1ul <<  2) |	/* enable RX */
				     (   1ul <<  3) |	/* enable TX */
				     (   0ul << 12) |  	/* 1 start bit, 8 data bits */
				     (   1ul <<  0) |   /* enable USART */
					USART_CR1_RXNEIE );	/* Enable receive interrupt */
}

/** @brief Writes a character into tx_rbuf
 *	@param ch The character to be written.
 *
 *	This waits until tx_rbuf has a slot available before writing into it.
 */
void serial_putchar_blocking(unsigned char ch)
{
	while (tx_rbuf_write(ch));
	USART2->CR1 |= USART_CR1_TXEIE;
}

/** @brief Writes a character into tx_rbuf
 *	@param ch The character to be written.
 *	@returns 0 if successful and -1 if the tx buffer is full.
 *
 *	This attempts to write into tx_rbuf once. It will return regardless of
 *	whether it succeeds or fails.
 */
int serial_putchar_nonblocking(unsigned char ch)
{
	if (tx_rbuf_write(ch))
		return -1;
	
	USART2->CR1 |= USART_CR1_TXEIE;
	return 0;
}

/** @brief Reads a char from the rx_ringbuf
 *	@param ch Container for holding output.
 *	@returns 0 if a new character is read and -1 if otherwise.
 *
 *	This function attemps to read form the rx ring buffer in blocking manner.
 *	It will wait until a new character is received before returning.
 */
void serial_getchar_blocking(unsigned char *ch)
{
	while (rx_rbuf_read(ch));
}

/** @brief Reads a char from the rx_ringbuf
 *	@param ch Container for holding output.
 *	@returns 0 if a new character is read and -1 if otherwise.
 *
 *	This function attemps to read form the rx ring buffer in a non-blocking
 *	manner. The function will return regardless of whether a new character is
 *	available or not.
 */
int serial_getchar_nonblocking(unsigned char *ch)
{
	return (rx_rbuf_read(ch));
}

/** @brief Function for handling rx interrupts
 */
static void serial_handle_rx_interrupt(void)
{
	rx_rbuf_write((unsigned char)(USART2->RDR & 0xFF));
}

/** @brief Function for handling tx interrupts
 */
static void serial_handle_tx_interrupt(void)
{
	unsigned char output;
	
	if (!tx_rbuf_read(&output)) {
		USART2->TDR = (output & 0xFF);
	} else {
		USART2->CR1 &= ~(USART_CR1_TXEIE);
	}
}

/** @brief IRQ Handler function for USART2
 */
void USART2_IRQHandler(void)
{
	if (USART2->ISR & USART_ISR_RXNE)
		serial_handle_rx_interrupt();
	
	if (USART2->ISR & USART_ISR_TXE)
		serial_handle_tx_interrupt();
}
