/*
 * SpiDimmer.c
 *
 * Created: 11/11/2018 11:09:25
 *  Author: Parents
 */ 


#include <avr/io.h>

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

#define COMMAND_BIT PB4


int main(void)
{
	
	DDRB |= 1<<DDB4;
	PORTB &= ~(1<<COMMAND_BIT);
	
    while(1)
    {
        //TODO:: Please write your application code 
    }
}