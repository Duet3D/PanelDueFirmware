/*
 * RequestTimer.h
 *
 * Created: 06/11/2015 14:22:05
 *  Author: David
 */ 


#ifndef REQUESTTIMER_H_
#define REQUESTTIMER_H_

#include <cstdint>
#include "ecv.h"
#undef array
#undef result
#undef value

class RequestTimer
{
	enum { stopped, running, ready } timerState;
	uint32_t startTime;
	uint32_t delayTime;
	const char * _ecv_array command;
	const char * _ecv_array null extra;
	bool quoteArgument;
	
public:
	RequestTimer(uint32_t del, const char * _ecv_array cmd, const char * _ecv_array null ex = nullptr);
	void SetCommand(const char * _ecv_array cmd) { command = cmd; }
	void SetArgument(const char * _ecv_array null arg, bool doQuote) { extra = arg; quoteArgument = doQuote; }
	void SetPending() { timerState = ready; }
	void Stop() { timerState = stopped; }
	bool Process();
};

#endif /* REQUESTTIMER_H_ */
