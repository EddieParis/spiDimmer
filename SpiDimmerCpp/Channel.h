/*
 * Channel.h
 *
 * Created: 05/12/2022 22:39:36
 *  Author: Parents
 */ 


#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <stdint.h>

class Channel {

public:
	Channel();
	void Periodic(bool button);
	
	inline int8_t getValue() const { return value; }
	void setValue(int8_t val);

private:
	int8_t value;
	uint16_t time_cnt;
	int8_t delta;
	uint8_t top_pause;
	int8_t last_value;
	
	
};



#endif /* CHANNEL_H_ */