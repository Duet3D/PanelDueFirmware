/*
 * RequestTimer.h
 *
 * Created: 06/11/2015 14:22:05
 *  Author: David
 */ 


#ifndef REQUESTTIMER_H_
#define REQUESTTIMER_H_

class RequestTimer
{
	enum { stopped, running, ready } timerState;
	uint32_t startTime;
	uint32_t delayTime;
	const char * array command;
	const char * array null extra;
	bool quoteArgument;
	
public:
	RequestTimer(uint32_t del, const char * array cmd, const char * array null ex = nullptr);
	void SetCommand(const char * array cmd) { command = cmd; }
	void SetArgument(const char * array null arg) { extra = arg; }
	void SetQuoteArgument(const bool arg) { quoteArgument = arg; }
	void SetPending() { timerState = ready; }
	void Stop() { timerState = stopped; }
	bool Process();
};

#endif /* REQUESTTIMER_H_ */
