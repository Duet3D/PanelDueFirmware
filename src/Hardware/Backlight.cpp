#include "Hardware/Backlight.hpp"

Backlight::Backlight(pwm_channel_t *p_pwm,
		uint32_t p_pwmFrequency, uint32_t p_frequency,
		uint32_t p_dimBrightness, uint32_t p_normalBrightness,
		uint32_t p_minDuty, uint32_t p_maxDuty)
{
	pwm = p_pwm;

	frequency = p_frequency;
	period = p_pwmFrequency / p_frequency;

	dimBrightness = p_dimBrightness;
	normalBrightness = p_normalBrightness;

	minDuty = p_minDuty;
	maxDuty = p_maxDuty;

	state = BacklightStateNormal;

	pwm->ul_period = period;
	pwm->ul_duty = maxDuty;

	pwm_channel_init(PWM, pwm);
	pwm_channel_enable(PWM, pwm->channel);
}

void Backlight::SetBrightness(uint32_t brightness)
{
	pwm->ul_period = period;
	pwm->ul_duty = minDuty + brightness * (maxDuty - minDuty) / Backlight::MaxBrightness;
	if (pwm->ul_duty > maxDuty)
	{
		pwm->ul_duty = maxDuty;
	}

	pwm_channel_init(PWM, pwm);
	pwm_channel_enable(PWM, pwm->channel);
}

void Backlight::SetState(enum BacklightState newState)
{
	uint32_t brightness = 100;

	switch (newState)
	{
	case BacklightStateDimmed:
		brightness = dimBrightness;
		break;
	case BacklightStateNormal:
		brightness = normalBrightness;
		break;
	default:
		break;
	}

	state = newState;
	SetBrightness(brightness);
}

enum BacklightState Backlight::GetState()
{
	return state;
}

void Backlight::SetNormalBrightness(uint32_t p_normalBrightness)
{
	normalBrightness = p_normalBrightness;
}

void Backlight::SetDimBrightness(uint32_t p_dimBrightness)
{
	dimBrightness = p_dimBrightness;
}
