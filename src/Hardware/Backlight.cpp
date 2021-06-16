#include "Hardware/Backlight.hpp"

Backlight::Backlight(pwm_channel_t *pwm,
		uint32_t pwmFrequency, uint32_t frequency,
		uint32_t dimBrightness, uint32_t normalBrightness,
		uint32_t minDuty, uint32_t maxDuty)
{
	//assert(backlight);
	//assert(channel);
	//assert(pwmClockFrequency >= frequency);

	this->pwm = pwm;

	this->frequency = frequency;
	this->period = pwmFrequency / frequency;

	this->dimBrightness = dimBrightness;
	this->normalBrightness = normalBrightness;

	this->minDuty = minDuty;
	this->maxDuty = maxDuty;

	this->state = BacklightStateNormal;

	this->pwm->ul_period = this->period;
	this->pwm->ul_duty = this->maxDuty;

	pwm_channel_init(PWM, this->pwm);
	pwm_channel_enable(PWM, this->pwm->channel);
}

void Backlight::SetBrightness(uint32_t brightness)
{
	this->pwm->ul_period = this->period;
	this->pwm->ul_duty = this->minDuty + brightness * this->maxDuty / Backlight::MaxBrightness;
	if (this->pwm->ul_duty > this->maxDuty)
	{
		this->pwm->ul_duty = this->maxDuty;
	}

	pwm_channel_init(PWM, this->pwm);
	pwm_channel_enable(PWM, this->pwm->channel);
}

void Backlight::SetState(enum BacklightState state)
{
	//assert(backlight);
	//assert(backlight->pwm);
	//assert(backlight->maxBrightness >= brightness);

	//if (this->state == state)
		//return;

	uint32_t brightness = 100;

	switch (state)
	{
	case BacklightStateDimmed:
		brightness = this->dimBrightness;
		break;
	case BacklightStateNormal:
		brightness = this->normalBrightness;
		break;
	default:
		// TODO throw exception
		break;
	}

	this->state = state;
	this->SetBrightness(brightness);
}

enum BacklightState Backlight::GetState()
{
	return this->state;
}

void Backlight::SetNormalBrightness(uint32_t normalBrightness)
{
	// TODO add checks?
	this->normalBrightness = normalBrightness;
}

void Backlight::SetDimBrightness(uint32_t dimBrightness)
{
	// TODO add checks?
	this->dimBrightness = dimBrightness;
}
