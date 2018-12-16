/*
 * spi.c
 *
 * Created: 11/11/2018 11:35:47
 *  Author: Parents
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer_simplified.h"

void SpiInit(void) 
{
	// set MISO as output
#ifdef __AVR_ATtiny2313A__ 
	DDRB |= 1<<DDB6;
#elif defined __AVR_ATtiny44__
	DDRA |= 1<<DDA1;
#endif
	// enable interrupt, 3 wire mode, ext clk rising edge
	USICR = (1<<USIOIE)|(1<<USIWM0)|(1<<USICS1);
}
	
uint8_t SpiGetData(void)
{
	return USIDR;	
}

void SpiSetData(uint8_t data)
{
	USIDR = data;
}

ISR(USI_OVERFLOW_vect)
{
	USISR |= 1<<USIOIF;
	Event_Signal(SPIRX_EVENT);
}
