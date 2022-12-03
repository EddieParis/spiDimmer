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

event_t Events[]={ {SpiRxCB, (void*)0, 0, 1 } };

uint8_t channel_a = 0;

uint8_t old_net_a = 0;
uint8_t old_net_b = 0;

uint8_t val = 0;

uint8_t init = 1;
uint8_t sequence = -1;

uint8_t input_msg[2], output_msg[2] = {0,0};

// 7 MOSI (input) PB2
// 6 INT0 used for SCK (input) PB1
// 5 OC0A (output) PB0
// 3 ZDC (input) PB4
// 2 _SEL (input) PB3
// 1 debug


int main(void)
{
	
	//OC0A as output
	DDRB |= 1<<DDB0;
	
	//ZDC as Wpullup
	PORTB |= 1<<PB4;
	
	Event_Init();
	SpiInit();
	
	SpiSetData(0);
	
	// Clear on compare match
	TCCR0A = (1<<COM0A1);
	// Mode 0
	// clk=9.6MHz divide by 1024 -> 100.6us per tick 94 ticks for 100Hz (10.027 ms)
	TCCR0B = 1<<CS02 | 1<<CS00;
	
	channel_a = 39;
	OCR0A = 39;	
	
	//pull up on Pin of int0
	//PORTD |= 1<<PORTD6;
	// any change on int0 triggers interrupt
	//MCUCR |= 1<<ISC00;
	
	
	
	PCMSK = 1<<PCINT4;
	// enable int0
	GIMSK |= 1<<PCIE;
	
	sei();
	
    while(1)
    {
        Event_WaitNext();
    }
}

void SpiRxCB(void * arg)
{
	Event_ClearSignal(SPIRX_EVENT);
	
	channel_a = SpiGetData();
}

/* ZDC interrupt */
ISR(PCINT0_vect)
{
	if (PINB&(1<<PINB4)) {
		GTCCR = (1<<TSM) + (1<<PSR10);
		TCNT0 = 0;
		GTCCR &= (1<<TSM);
			
		if (channel_a != 0)
		{
			// forces toggle on set mode -> set output high
			TCCR0A |= (1<<COM0A1) | (1<<COM0A0);
			TCCR0B |= 1<<FOC0A;
		}
			
		OCR0A = channel_a;
			
		TCCR0A = (1<<COM0A1);
	}
}

/* ZDC interrupt */
//ISR(PCINT0_vect)
//{
	//if (!(PINB&(1<<PINB4))) {
		//GTCCR = (1<<TSM) + (1<<PSR10);
		//TCNT0 = 255-6;
		//GTCCR &= (1<<TSM);
//
		//TIMSK0 = 1<<TOIE0;	
	//}
//}
//
//ISR(TIM0_OVF_vect)
//{
	//GTCCR = (1<<TSM) + (1<<PSR10);
	//TCNT0 = 0;
	//GTCCR &= (1<<TSM);
			//
	//if (channel_a != 0)
	//{
		//// forces toggle on set mode -> set output high
		//TCCR0A |= (1<<COM0A1) | (1<<COM0A0);
		//TCCR0B |= 1<<FOC0A;
	//}
			//
	//OCR0A = channel_a;
			//
	//TCCR0A = (1<<COM0A1);
//
	//TIMSK0 = 0;
	//
//}