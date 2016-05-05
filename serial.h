
/** @file serial.h
 *  @brief Serial driver include file
 *  @author Dennis Law
 *  @date May 2016
 */
 
#ifndef SERIAL_H
#define SERIAL_H

/** Ring buffer size */
#define SERIAL_RBUF_SIZE		200

void serial_init(int baud);

void serial_putchar_blocking(unsigned char ch);
int serial_putchar_nonblocking(unsigned char ch);
void serial_getchar_blocking(unsigned char *ch);
int serial_getchar_nonblocking(unsigned char *ch);

#endif	/* SERIAL_H */
