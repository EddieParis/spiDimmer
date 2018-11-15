/*
 * spi.c
 *
 * Created: 11/11/2018 11:35:47
 *  Author: Parents
 */ 

#include <avr/io.h>

void Init(void) 
{
	// set MISO as output
	DDRB |= 1<<DDB1;
	USICR = 1<<USIWM0|(1<<USICS1);
}
	
	
	init:
	ldi r16,(1<<USIWM0)|(1<<USICS1)
	out USICR,r16
	...
	SlaveSPITransfer:
	out USIDR,r16
	ldi r16,(1<<USIOIF)
	out USISR,r16
	SlaveSPITransfer_loop:
	in r16, USISR
	sbrs r16, USIOIF
	rjmp SlaveSPITransfer_loop
	in r16,USIDR
}