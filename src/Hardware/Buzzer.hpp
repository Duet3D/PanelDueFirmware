/*
 * Buzzer.h
 *
 * Created: 13/11/2014 22:56:34
 *  Author: David
 */ 


#ifndef BUZZER_H_
#define BUZZER_H_

#include <cstdint>

namespace Buzzer
{
	void Init();

	void Beep(uint32_t frequency, uint32_t ms, uint32_t volume);

	void Tick();
	
	bool Noisy();
	
	const uint32_t MaxVolume = 5;
	const uint32_t DefaultVolume = 3;
}

#endif /* BUZZER_H_ */
