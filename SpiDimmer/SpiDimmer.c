/*
 * SpiDimmer.c
 *
 * Created: 11/11/2018 11:09:25
 *  Author: Parents
 */ 


#include <avr/io.h>

/*
* PIN assignements
* 1 (PB5-reset) external sw (in weak pullup)
* 2 PB3 Zero Detect (in)
* 3 PB4 Transistors command (out)
* 4 GND
* 5 PBO - MOSI (in)
* 6 PB1 - MISO (out)
* 7 PB2 - SCK (in)
* 8 VCC
*/


int main(void)
{
	DDRB |= 1<<PB4;
	
    while(1)
    {
        //TODO:: Please write your application code 
    }
}