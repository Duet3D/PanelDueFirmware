/*
 * RequestTimer.cpp
 *
 * Created: 06/11/2015 14:22:55
 *  Author: David
 */ 

#include "ecv.h"
#include "asf.h"
#include "RequestTimer.hpp"
#include "Hardware/SysTick.hpp"
#include "Hardware/SerialIo.hpp"

extern bool OkToSend();		// in PanelDue.cpp

RequestTimer::RequestTimer(uint32_t del, const char * array cmd, const char * array null ex)
	: delayTime(del), command(cmd), extra(ex)
{
	timerState = stopped;
}

bool RequestTimer::Process()
{
	if (timerState == running)
	{
		uint32_t now = SystemTick::GetTickCount();
		if (now - startTime > delayTime)
		{
			timerState = ready;
		}
	}

	if (timerState == ready && OkToSend())
	{
		SerialIo::SendString(command);
		if (extra != nullptr)
		{
			SerialIo::SendString(not_null(extra));
		}
		SerialIo::SendChar('\n');
		startTime = SystemTick::GetTickCount();
		timerState = running;
		return true;
	}
	return false;
}

// End
