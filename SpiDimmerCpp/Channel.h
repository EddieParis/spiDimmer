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
	
	inline uint8_t getValue() const { return value; }
	inline void setValue(uint8_t val) { value = val; }

private:
	uint8_t value;
	uint8_t time_cnt;
	int8_t delta;
	uint8_t top_pause;
	uint8_t last_value;
	
	
};



#endif /* CHANNEL_H_ */