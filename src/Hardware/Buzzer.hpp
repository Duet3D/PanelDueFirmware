/*
 * Buzzer.h
 *
 * Created: 13/11/2014 22:56:34
 *  Author: David
 */ 


#ifndef BUZZER_H_
#define BUZZER_H_

namespace Buzzer
{
	void Init();

	void Beep(uint32_t frequency, uint32_t ms, uint32_t volume);

	void Tick();
	
	bool Noisy();
	
	void SetBacklight(uint32_t brightness);
	
	const uint32_t MaxVolume = 5;
	const uint32_t DefaultVolume = 3;
	const uint32_t MaxBrightness = 100;
	const uint32_t MinBrightness = 5;		// avoid making it so dark that it cannot be seen
	const uint32_t DefaultBrightness = 100;	// default = maximum because lower levels make the backlight inverter buzz
}

#endif /* BUZZER_H_ */
