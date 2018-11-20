/*
 * spi.c
 *
 * Created: 11/11/2018 11:35:47
 *  Author: Parents
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

void SpiInit(void) 
{
	// set MISO as output
	DDRB |= 1<<DDB1;
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

ISR(USI_OVF_vect)
{
	USISR |= 1<<USIOIF;	
}
