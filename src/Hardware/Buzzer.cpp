/*
 * Buzzer.cpp
 *
 * Created: 13/11/2014 22:56:18
 *  Author: David
 * The piezo sounder is connected to the complementary outputs of PWM channel 0, PWMH0 and PWML0, aka PB0 peripheral A and PB5 peripheral B.
 * The backlight control is included in this module because it also uses PWM. Output PWMH1 (aka PB1 peripheral A) drives the backlight pin.
 */

#include "ecv.h"
#undef array
#undef result
#undef value
#include "asf.h"
#include "Buzzer.hpp"
#include "SysTick.hpp"
#include "Configuration.hpp"

namespace Buzzer
{
	extern const uint32_t pwmClockFrequency = 2000000;		// 2MHz clock (OK down to 30Hz PWM frequency)
	static uint32_t beepTicksToGo = 0;
	static bool inBuzzer = true;

	static const uint32_t volumeTable[MaxVolume] = { 3, 9, 20, 40, 80 };

	static pwm_channel_t buzzer_pwm_channel_instance =
	{
		.channel = 0,
		.ul_prescaler = PWM_CMR_CPRE_CLKA,
		.alignment = PWM_ALIGN_LEFT,
		.polarity = static_cast<pwm_level_t>(0),
		.ul_duty = 0,
		.ul_period = 0,
#if (SAM3U || SAM3S || SAM3XA || SAM4S || SAM4E)
		.counter_event = static_cast<pwm_counter_event_t>(0),
		.b_deadtime_generator = 0,
		.b_pwmh_output_inverted = false,
		.b_pwml_output_inverted = false,
		.us_deadtime_pwmh = 0,
		.us_deadtime_pwml = 0,
		.output_selection = {
			.b_override_pwmh = false,
			.b_override_pwml = false,
			.override_level_pwmh = static_cast<pwm_level_t>(0),
			.override_level_pwml = static_cast<pwm_level_t>(0),
		},
		.b_sync_ch = false,
		.fault_id = static_cast<pwm_fault_id_t>(0),
		.ul_fault_output_pwmh = static_cast<pwm_level_t>(0),
		.ul_fault_output_pwml = static_cast<pwm_level_t>(0),
#endif
#if SAM4E
		.ul_spread = 0,
		.spread_spectrum_mode = PWM_SPREAD_SPECTRUM_MODE_TRIANGULAR,
		.ul_additional_edge = 0,
		.additional_edge_mode = static_cast<pwm_additional_edge_mode_t>(0),
#endif
	};

	// Initialize the buzzer and the PWM system. Must be called before using the buzzer or backlight.
	void Init()
	{
		pio_configure(PIOB, PIO_OUTPUT_0, PIO_PB0 | PIO_PB5, 0);	// set both piezo pins low
		beepTicksToGo = 0;
		inBuzzer = false;
	}

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

}

// End
