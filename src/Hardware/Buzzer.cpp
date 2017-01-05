/*
 * Buzzer.cpp
 *
 * Created: 13/11/2014 22:56:18
 *  Author: David
 * The piezo sounder is connected to the complementary outputs of PWM channel 0, PWMH0 and PWML0, aka PB0 peripheral A and PB5 peripheral B.
 * The backlight control is included in this module because it also uses PWM. Output PWMH1 (aka PB1 peripheral A) drives the backlight pin.
 */ 

#include "ecv.h"
#include "asf.h"
#include "Buzzer.hpp"
#include "SysTick.hpp"
#include <cstring>

namespace Buzzer
{
	static const uint32_t pwmClockFrequency = 2000000;		// 2MHz clock (OK down to 30Hz PWM frequency)
	static const uint32_t backlightPwmFrequency = 300;		// Working range is about 100Hz to 1KHz. Some frequencies causes flickering on the 4.3" display.
	static const uint32_t backlightPeriod = pwmClockFrequency/backlightPwmFrequency; 

	static pwm_channel_t buzzer_pwm_channel_instance =
	{
		.channel = 0,
		.ul_prescaler = PWM_CMR_CPRE_CLKA		
	};
	static pwm_channel_t backlight_pwm_channel_instance =
	{
		.channel = 1,
		.ul_prescaler = PWM_CMR_CPRE_CLKA,
	};
	static uint32_t beepTicksToGo = 0;
	static bool inBuzzer = true;

	// Initialize the buzzer and the PWM system. Must be called before using the buzzer or backlight.
	void Init()
	{
		pwm_channel_disable(PWM, PWM_CHANNEL_0);					// make sure buzzer PWM is off
		pwm_channel_disable(PWM, PWM_CHANNEL_1);					// make sure backlight PWM is off
		pwm_clock_t clock_setting =
		{
			.ul_clka = pwmClockFrequency,
			.ul_clkb = 0,
			.ul_mck = SystemCoreClock
		};
		pwm_init(PWM, &clock_setting);								// set up the PWM clock
		pio_configure(PIOB, PIO_PERIPH_A, PIO_PB1, 0);				// enable HI output to backlight, but not to piezo yet
		pio_configure(PIOB, PIO_OUTPUT_0, PIO_PB0 | PIO_PB5, 0);	// set both piezo pins low

		beepTicksToGo = 0;
		inBuzzer = false;
	}

	static uint32_t volumeTable[MaxVolume] = { 3, 9, 20, 40, 80 };
		
	// Generate a beep of the given length and frequency. The volume goes from 0 to MaxVolume.
	void Beep(uint32_t frequency, uint32_t ms, uint32_t volume)
	{
		if (volume != 0)
		{
			if (volume > MaxVolume)
			{
				volume = MaxVolume;
			}
			
			inBuzzer = true;		// tell the tick interrupt to leave us alone
			if (beepTicksToGo == 0)
			{
				uint32_t period = pwmClockFrequency/frequency;
				// To get the maximum fundamental component, we want the dead time to be 1/6 of the period.
				// Larger dead times reduce the volume, at the expense of generating more high harmonics.
				uint32_t onTime = (period * volumeTable[volume - 1])/200;
				uint16_t deadTime = period/2 - onTime;
				buzzer_pwm_channel_instance.ul_period = period;
				buzzer_pwm_channel_instance.ul_duty = period/2;
				pwm_channel_init(PWM, &buzzer_pwm_channel_instance);
				PWM->PWM_CH_NUM[PWM_CHANNEL_0].PWM_CMR |= PWM_CMR_DTE;
				PWM->PWM_CH_NUM[PWM_CHANNEL_0].PWM_DT = (deadTime << 16) | deadTime;
				PWM->PWM_CH_NUM[PWM_CHANNEL_0].PWM_DTUPD = (deadTime << 16) | deadTime;
				pwm_channel_enable(PWM, PWM_CHANNEL_0);
				pio_configure(PIOB, PIO_PERIPH_A, PIO_PB0, 0);				// enable HI PWM output to piezo
				pio_configure(PIOB, PIO_PERIPH_B, PIO_PB5, 0);				// enable LO PWM output to piezo
				beepTicksToGo = ms;
			}
			inBuzzer = false;
		}
	}

	// This is called from the tick ISR
	void Tick()
	{
		if (!inBuzzer && beepTicksToGo != 0)
		{
			--beepTicksToGo;
			if (beepTicksToGo == 0)
			{
				// Turn the buzzer off
				pwm_channel_disable(PWM, PWM_CHANNEL_0);
				pio_configure(PIOB, PIO_OUTPUT_0, PIO_PB0 | PIO_PB5, 0);		// set both piezo pins low to silence the piezo
			}
		}
	}
	
	// Return true if the buzzer is (or should be) still sounding
	bool Noisy()
	{
		return beepTicksToGo != 0;
	}
	
	// Set the backlight brightness on a scale of 0 to MaxBrightness.
	// Must call Init before calling this.
	void SetBacklight(uint32_t brightness)
	{
		backlight_pwm_channel_instance.ul_period = backlightPeriod;
		backlight_pwm_channel_instance.ul_duty = ((backlightPeriod - 1) * (MaxBrightness - brightness))/MaxBrightness;
		pwm_channel_init(PWM, &backlight_pwm_channel_instance);
		pwm_channel_enable(PWM, PWM_CHANNEL_1);
	}
}

// End
