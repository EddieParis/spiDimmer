/*
 * CPPFile1.cpp
 *
 * Created: 05/12/2022 22:39:05
 *  Author: Parents
 */ 


#include "Channel.h"

#define STEP 1

Channel::Channel()
{
	value = 0;
	time_cnt = 0;
	delta = 1;
	top_pause = 0;
	last_value = 100;
}

void Channel::setValue(int8_t val)
{
	if (val > 100)
	{
		value = 100;
	}
	else if (val < 0)
	{
		value = 0;
	}
	else 
	{
		value = val;
	}
}

void Channel::Periodic(bool button)
{
	if (button==true) {
		// button pressed, process dimming every 10 cycles if we are not at top pause
		if (time_cnt > 50 && time_cnt % 10 == 0 && top_pause == 0)
		{
			setValue(value+delta);
			if (value == 0)
			{
				delta = STEP;
			}
			else if (value >= 100)
			{
				delta = -STEP;
				top_pause = 100;
			}
		}
		else
		{
			if (top_pause)
			{
				top_pause -= 1;
			}
		}
		time_cnt += 1;
	}
	else if (8 < time_cnt && time_cnt <= 50)
	{
		// button has been released, if it is a short press 8 < t <= 50,
		// light is toggled on or off
		if (value == 0)
		{
			setValue(last_value);
		}
		else
		{
			last_value = value;
			value = 0;
		}
		top_pause = 0;
		time_cnt = 0;
	}
	else
	{
		top_pause = 0;
		time_cnt = 0;
	}
			
}