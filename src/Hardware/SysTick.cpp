/*
 * SysTick.cpp
 *
 * Created: 13/11/2014 23:03:37
 *  Author: David
 */ 

#include "SysTick.hpp"
#include <UI/UserInterface.hpp>
#include "asf.h"
#include "Buzzer.hpp"

namespace SystemTick
{
	volatile uint32_t tickCount;

	uint32_t GetTickCount()
	{
		return tickCount;
	}
}

void SysTick_Handler()
{
	wdt_restart(WDT);
	++SystemTick::tickCount;
	Buzzer::Tick();
	UI::Tick();
}

// End
