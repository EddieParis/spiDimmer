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

#include "Channel.h"

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
* 12 PB0 - CS for SPI (in-pullup)
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

Channel channels[4];

uint8_t old_net_a = 0;
uint8_t old_net_b = 0;

uint8_t val = 0;

uint8_t init = 1;
uint8_t sequence = -1;

uint8_t input_msg[2], output_msg[2] = {0,0};

uint8_t spi_step=0;
uint8_t spi_cmd=0;


static const uint8_t timerValue[101] = { 0,  1,  2,  2,  3,  4,  5,  5,  6,  7,  8,  9,  9, 10, 11, 12, 12, 13, 14, 15,
	                                    16, 16, 17, 18, 19, 20, 20, 21, 22, 23, 23, 24, 25, 26, 27, 27, 28, 29, 30, 30,
								        31, 32, 33, 34, 34, 35, 36, 37, 37, 38, 39, 40, 41, 41, 42, 43, 44, 44, 45, 46,
								        47, 48, 48, 49, 50, 51, 51, 52, 53, 54, 55, 55, 56, 57, 58, 59, 59, 60, 61, 62,
								        62, 63, 64, 65, 66, 66, 67, 68, 69, 69, 70, 71, 72, 73, 73, 74, 75, 76, 76, 77, 78 };

int main(void)
{
	
	// configure output for mos commands
	DDRB |= (1<<DDB4)|(1<<DDB3)|(1<<DDB2);
	PORTB &= ~((1<<PORTB4)|(1<<PORTB3)|(1<<PORTB2));

	DDRD |= (1<<DDD5);
	PORTD &= ~((1<<PORTD5));
		
	Event_Init();
	//SpiInit();
	
	SpiSetData(0xFF);
	
	for (int i=0; i<4; i++)
	{
		channels[i] = Channel();
		channels[i].setValue(0);
	}
	
	// Clear on compare match
	TCCR0A = (1<<COM0B1) | (1<<COM0A1);
	// Mode 0
	// divide by 1024 -> 128us per tick 78 ticks for 100Hz 
	// divide by 256 -> 32us per tick 312 ticks for 100Hz 
	// -> keep 64
	TCCR0B = (1<<CS02)|(1<<CS00);
	
	//pull up on Pin of input switches
	PORTD |= (1<<PORTD6)|(1<<PORTD4)|(1<<PORTD3)|(1<<PORTD2);
	
	// pull up for CS
	PORTB |= (1<<PORTB0);
	// enable pin change interrupt for CS
	PCMSK |= 1<<PCINT0;

	
	//pull up on Pin of ZDC
	PORTA |= (1<<PORTA0);
	// enable pin change interrupt for ZDC
	PCMSK1 |= 1<<PCINT8;

	// enable pin change int
	GIMSK |= (1<<PCIE1)|(1<<PCIE0);
	
	sei();
	
    while(1)
    {
        Event_WaitNext();
    }
}

void SpiRxCB(void * arg)
{
	Event_ClearSignal(SPIRX_EVENT);
	
	if (spi_step == 0)
	{
		spi_cmd = SpiGetData();
		
		if ((spi_cmd&0x80) == 0)
		{
			uint8_t chan = spi_cmd&0x03;
			SpiSetData(channels[chan].getValue());
		}
		else
		{
			SpiSetData(0xFF);	
		}
		spi_step++;
	}
	else if (spi_step == 1 && (spi_cmd&0x80))
	{
		uint8_t chan = spi_cmd&0x03;
		// write mode
		channels[chan].setValue(SpiGetData());	
		SpiSetData(0xFF);
		spi_step = 0;
	}
	else 
	{
		// data read was already prepared
		SpiSetData(0xFF);
		spi_step = 0;
	}
	
	
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

/* CS interrupt */
ISR(PCINT0_vect)
{
	if (PINB&(1<<PINB0))
	{
		// CS high, chip deselected.
		SpiUninit();
	}
	else
	{
		SpiSetData(0xff);
		SpiInit();
		spi_step = 0;
	}
	
}


/* ZDC interrupt */
ISR(PCINT1_vect)
{
	
	if (PINA&(1<<PINA0)) {
		// Reset the timer
		GTCCR = 1<<PSR10;
		TCNT0 = 0;

		if (channels[0].getValue() != 0)
		{
			// forces toggle on set mode -> set output high
			TCCR0A |= (1<<COM0A1) | (1<<COM0A0);
		}
		else
		{
			uint8_t old = TCCR0A & ~((1<<COM0A1)|(1<<COM0A0));
			// force clear output
			TCCR0A = old | (1<<COM0A1);
		}

		if (channels[1].getValue() != 0)
		{
			// forces toggle on set mode -> set output high
			TCCR0A |= (1<<COM0B1) | (1<<COM0B0);
		}
		else
		{
			uint8_t old = TCCR0A & ~((1<<COM0B1)|(1<<COM0B0));
			// force clear output
			TCCR0A = old | (1<<COM0B1);
		}
		TCCR0B |= (1<<FOC0B) | (1<<FOC0A);

		OCR0A = timerValue[channels[0].getValue()];
		OCR0B = timerValue[channels[1].getValue()];
	
		TCCR0A = (1<<COM0B1) | (1<<COM0A1);

		channels[0].Periodic((PIND&(1<<PIND2))==0);
		channels[1].Periodic((PIND&(1<<PIND3))==0);
		channels[2].Periodic((PIND&(1<<PIND4))==0);
		channels[3].Periodic((PIND&(1<<PIND6))==0);
	}
}
