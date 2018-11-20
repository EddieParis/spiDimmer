/*
 * SpiDimmer.c
 *
 * Created: 11/11/2018 11:09:25
 *  Author: Parents
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>

#include "spi.h"
#include "timer_simplified.h"

/*
* PIN assignments
* 1 (PB5-reset) external sw (in weak pull-up)
* 2 PB3 Zero Detect (in)
* 3 PB4 Transistors command (out)
* 4 GND
* 5 PBO - MOSI (in)
* 6 PB1 - MISO (out)
* 7 PB2 - SCK (in)
* 8 VCC
*/

/* message format


*/


#define COMMAND_BIT PB4

void SpiRxCB(void * arg);

event_t Events[]={ {SpiRxCB, (void*)0, 0 } };

uint8_t channel_a = 0;
uint8_t channel_b = 0;

uint8_t val = 0;

int main(void)
{
	
	DDRB |= 1<<DDB4;
	PORTB &= ~(1<<COMMAND_BIT);
	
	Event_Init();
	SpiInit();
	
	// Mode 0
	TCCR0A = (1<<COM0B1) | (1<<COM0A1);
	// divide by 256 -> freq = 60Hz
	TCCR0B = 1<<CS02;
	
	sei();
	
    while(1)
    {
        //TODO:: Please write your application code 
    }
}

void SpiRxCB(void * arg)
{
	if (val == 0)
	{
		channel_a = SpiGetData();
		val = 1;
	}
	else
	{
		channel_b = SpiGetData();
		val = 0;
	}
}



/* ZDC interrupt */
ISR(INT0_vect)
{
	// Reset the timer
	if (channel_a != 0)
	{
		// forces toggle on set mode -> set output high
		TCCR0A |= (1<<COM0A1) | (1<<COM0A0);
		TCCR0B |= 1<<FOC0A;
	}

	if (channel_b != 0)
	{
		// forces toggle on set mode -> set output high
		TCCR0A |= (1<<COM0B1) | (1<<COM0B0);
		TCCR0B |= 1<<FOC0B;
	}
	
	OCR0A = channel_a;
	OCR0B = channel_b;
	
	TCCR0A = (1<<COM0B1) | (1<<COM0A1);
	
}
