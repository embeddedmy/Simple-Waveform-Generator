
/** @file retarget.c
 *  @brief Retarget stdio
 *  @author Dennis Law
 *  @date May 2016
 */

#include <stdio.h>
#include <rt_misc.h>
#include "serial.h"

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

int fputc(int c, FILE *f)
{
	serial_putchar_blocking(c);
	return c;
}

int fgetc(FILE *f)
{
	unsigned char c;
	serial_getchar_blocking(&c);
	return c;
}

int ferror(FILE *f)
{
	/* Your implementation of ferror */
	return EOF;
}

void _ttywrch(int c)
{
	serial_putchar_blocking(c);
}

void _sys_exit(int return_code)
{
label:  goto label;  /* endless loop */
}
