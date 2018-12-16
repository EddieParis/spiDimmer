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
uint8_t channel_b = 0;

uint8_t old_net_a = 0;
uint8_t old_net_b = 0;

uint8_t val = 0;

uint8_t init = 1;
uint8_t sequence = -1;

uint8_t input_msg[2], output_msg[2] = {0,0};

int main(void)
{
	
	DDRB |= 1<<DDB4;
	PORTB &= ~(1<<COMMAND_BIT);
	
	Event_Init();
	SpiInit();
	
	SpiSetData(0);
	
	// Clear on compare match
	TCCR0A = (1<<COM0B1) | (1<<COM0A1);
	// Mode 0
	// divide by 256 -> 64us per tik 156 ticks for 100Hz 
	TCCR0B = 1<<CS02;
	
	//pull up on Pin of int0
	PORTD |= 1<<PORTD6;
	// any change on int0 triggers interrupt
	MCUCR |= 1<<ISC00;
	// enable int0
	GIMSK |= 1<<INT0;
	
	sei();
	
    while(1)
    {
        Event_WaitNext();
    }
}

void SpiRxCB(void * arg)
{
	Event_ClearSignal(SPIRX_EVENT);
	input_msg[val] = SpiGetData();
	
	/* First w are in init mode, try to count the number of messages that are 00,00
	It is the sequence number we are in the chain.
	Master sends 00,00 then 10,00 then 20,00
	So first device get 00,00 10,00 20,00 00,00 10,00 20,00
	second device get   00,00 00,00 10,00 20,00 00,00 10,00
	third device get    00,00 00,00 00,00 10,00 20,00 00,00
	*/
	if (init == 1 && val == 1)
	{
		if ( input_msg[0] == 0 && input_msg[1] == 0 )
		{
			sequence += 1;
		}
		else
		{
			init = 0;
		}
		output_msg[0] = input_msg[0];
		output_msg[1] = input_msg[1];
	}
	else
	{
		if (val==1)
		{
			uint8_t msg_seq = input_msg[0]>>4;
			if (sequence == msg_seq)
			{
				uint8_t temp_a = ((input_msg[0]&0x0F) <<2 ) | input_msg[1]>>6;
				uint8_t temp_b = input_msg[1] & 0x3F;
			
				if (old_net_a != temp_a)
				{
					channel_a = temp_a;
					old_net_a = temp_a;
				}
			
				if (old_net_b != temp_b)
				{
					channel_b = temp_b;
					old_net_b = temp_b;
				}			

				output_msg[0] = (sequence<<4) | (channel_a>>2);
				output_msg[1] = (channel_a<<6) | channel_b;	
			}
			else
			{
				output_msg[0] = input_msg[0];
				output_msg[1] = input_msg[1];
			}
		
		}
	}

	val ^= 1;

	SpiSetData(output_msg[val]);

/*
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
*/
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
