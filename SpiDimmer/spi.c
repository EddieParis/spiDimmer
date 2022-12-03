/*
 * spi.c
 *
 * Created: 11/11/2018 11:35:47
 *  Author: Parents
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer_simplified.h"

#define SEL (PINB&(1<<PINB3))
#define MOSI_V (PINB&(1<<PINB2))
#define MOSI_B PINB2

uint8_t phase = 0;
uint8_t value = 0;

void SpiInit(void) 
{
	// enable interrupt, 3 wire mode, ext clk rising edge
	//USICR = (1<<USIOIE)|(1<<USIWM0)|(1<<USICS1);
	
	//Interrupt on rising edge.
	MCUCR |= (1<<ISC01) + (1<<ISC00);
	GIMSK |= 1<<INT0;
}
	
uint8_t SpiGetData(void)
{
	phase = 0;
	//return USIDR;
	uint8_t temp = value;
	value = 0;
	return temp;
}

void SpiSetData(uint8_t data)
{
	//USIDR = data;
}

ISR(INT0_vect)
{
	//if (SEL == 0)
	//{
		if (phase<8)
		{
			
			if (PINB&(1<<MOSI_B))
			{
				value |= 1;
			}
			
			if (phase != 7)
			{
				value <<= 1;
			}
			else
			{
				Event_Signal(SPIRX_EVENT);
			}

			phase++;
			
		}
	//}	
}

/*
ISR(USI_OVERFLOW_vect)
{
	USISR |= 1<<USIOIF;
	Event_Signal(SPIRX_EVENT);
}
*/