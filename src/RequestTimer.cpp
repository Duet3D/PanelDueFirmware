/*
 * RequestTimer.cpp
 *
 * Created: 06/11/2015 14:22:55
 *  Author: David
 */ 

#include "RequestTimer.hpp"
#include "asf.h"
#include "Hardware/SysTick.hpp"
#include "Hardware/SerialIo.hpp"

extern bool OkToSend();		// in PanelDue.cpp

RequestTimer::RequestTimer(uint32_t del, const char * _ecv_array cmd, const char * _ecv_array null ex)
	: startTime(SystemTick::GetTickCount()), delayTime(del), command(cmd), extra(ex), quoteArgument(false)
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
			if (quoteArgument)
			{
				SerialIo::SendQuoted(not_null(extra));
			}
			else
			{
				SerialIo::SendString(not_null(extra));
			}
		}
		SerialIo::SendChar('\n');
		startTime = SystemTick::GetTickCount();
		timerState = running;
		return true;
	}
	return false;
}

// End
