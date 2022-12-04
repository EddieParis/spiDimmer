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
*  1 (PB5-reset) external sw (in weak pull-up)
*  2 NC
*  3 NC
*  4 NC
*  5 PA0 - Zero Detect (in-pullup) - PCINT8
*  6 PD2 - Switch 1 (in-pullup) - PCINT13
*  7 PD3 - Switch 2 (in-pullup) - PCINT14
*  8 PD4 - Switch 3 (in-pullup) - PCINT15
*  9 PD5 - OCC0B - Transistors ch 2 command (out)
* 10 GND
* 11 PD6 - Switch 4 (in-pullup) - PCINT17
* 12 NC
* 13 NC
* 14 PB2 - OCC0A - Transistors ch 1 command (out)
* 15 PB3 - OCC1A - Transistors ch 3 command (out)
* 16 PB4 - OCC1B - Transistors ch 4 command (out)
* 17 PB5 - MOSI (in)  <- we are the slave
* 18 PB6 - MISO (out) <- we are the slave
* 19 PB7 - SCK (in)   <- we are the slave
* 20 VCC
*/

/* message format


*/


#define COMMAND_BIT PB4

void SpiRxCB(void * arg);

event_t Events[]={ {SpiRxCB, (void*)0, 0, 1 } };

uint8_t channel_a = 0x10;
uint8_t channel_b = 0;

uint8_t time_cnt = 0;
int8_t delta = 1;
uint8_t top_pause=0;
uint8_t last_value=0;

uint8_t old_net_a = 0;
uint8_t old_net_b = 0;

uint8_t val = 0;

uint8_t init = 1;
uint8_t sequence = -1;

uint8_t input_msg[2], output_msg[2] = {0,0};

int main(void)
{
	
	DDRB |= (1<<DDB4)|(1<<DDB3)|(1<<DDB2);
	PORTB &= ~((1<<PORTB4)|(1<<PORTB3)|(1<<PORTB2));

	DDRD |= (1<<DDD5);
	PORTD &= ~((1<<PORTD5));
		
	Event_Init();
	SpiInit();
	
	SpiSetData(0);
	
	// Clear on compare match
	TCCR0A = (1<<COM0B1) | (1<<COM0A1);
	// Mode 0
	// divide by 1024 -> 128us per tick 78 ticks for 100Hz 
	// divide by 256 -> 32us per tick 312 ticks for 100Hz 
	// -> keep 64
	TCCR0B = (1<<CS02)|(1<<CS00);
	
	//pull up on Pin of input switches
	PORTD |= (1<<PORTD6)|(1<<PORTD4)|(1<<PORTD3)|(1<<PORTD2);
	
	//pull up on Pin of ZDC
	PORTA |= (1<<PORTA0);
	// enable pin change interrupt for ZDC
	PCMSK1 |= 1<<PCINT8;

	// enable pin change int
	GIMSK |= 1<<PCIE1;
	
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
	
#if 0	
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
#endif
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
ISR(PCINT1_vect)
{
	
	if (PINA&(1<<PINA0)) {
		GTCCR = 1<<PSR10;
		TCNT0 = 0;
			
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

		if ((PIND&(1<<PIND2))==0) {		
            if (time_cnt > 50 && time_cnt % 10 == 0 && top_pause == 0)
			{
				time_cnt += 1;
				channel_a += delta;
				if (channel_a == 0)
				{
					delta = -delta;
				}
				else if (channel_a == 78)
				{
					delta = -delta;
					top_pause = 100;
				}
			}
			else
			{
				time_cnt += 1;
				if (top_pause)
				{
					top_pause -= 1;
				}
			}
		}
		else if (0 < time_cnt && time_cnt <= 50)
		{
			top_pause = 0;
			if (channel_a == 0)
			{
				channel_a = last_value;
			}
			else
			{
				last_value = channel_a;
				channel_a = 0;
			}
			time_cnt = 0;
		}
		else
		{
			time_cnt = 0;
		}
		
	}
}
