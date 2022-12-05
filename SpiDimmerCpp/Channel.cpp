/*
 * CPPFile1.cpp
 *
 * Created: 05/12/2022 22:39:05
 *  Author: Parents
 */ 


#include "Channel.h"


Channel::Channel()
{
	value = 0;
	time_cnt = 0;
	delta = 1;
	top_pause=0;
	last_value=0;
}

void Channel::Periodic(bool button)
{
	if (button==true) {
		if (time_cnt > 50 && time_cnt % 10 == 0 && top_pause == 0)
		{
			time_cnt += 1;
			value += delta;
			if (value == 0)
			{
				delta = -delta;
			}
			else if (value == 78)
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
		if (value == 0)
		{
			value = last_value;
		}
		else
		{
			last_value = value;
			value = 0;
		}
		time_cnt = 0;
	}
	else
	{
		time_cnt = 0;
	}
			
}