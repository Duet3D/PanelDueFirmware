/*
 * SysTick.hpp
 *
 * Created: 13/11/2014 23:04:31
 *  Author: David
 */ 


#ifndef SYSTICK_H_
#define SYSTICK_H_

namespace SystemTick
{
	constexpr uint32_t TicksPerSecond = 1000;

	uint32_t GetTickCount();		// get the number of milliseconds since we started
}

#endif /* SYSTICK_H_ */
