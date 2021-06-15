#include "Hardware/Backlight.hpp"

Backlight::Backlight(pwm_channel_t *pwm,
		uint32_t pwmFrequency, uint32_t frequency,
		uint32_t dimBrightness, uint32_t normalBrightness,
		uint32_t minBrightness, uint32_t maxBrightness)
{
	//assert(backlight);
	//assert(channel);
	//assert(pwmClockFrequency >= frequency);

	this->pwm = pwm;

	this->frequency = frequency;
	this->period = pwmFrequency / frequency - 1;

	this->dimBrightness = dimBrightness;
	this->normalBrightness = normalBrightness;

	this->minBrightness = minBrightness;
	this->maxBrightness = maxBrightness;

	this->state = BacklightStateOff;

	this->pwm->ul_period = this->period;
	this->pwm->ul_duty = 0;

	// configure pb13, pb13 tells us which frequency to use. should move to paneldue
	pio_configure(PIOB, PIO_INPUT, PIO_PB13, PIO_PULLUP);
	pio_get(PIOB, PIO_INPUT, PIO_PB13);

	// backlight pwm pin
	pio_configure(PIOB, PIO_PERIPH_A, PIO_PB1, 0);				// enable HI output to backlight, but not to piezo yet

	pwm_channel_init(PWM, this->pwm);
	pwm_channel_disable(PWM, this->pwm->channel);
}

void Backlight::SetBrightness(uint32_t brightness)
{
	if (brightness == 0)
	{
		pwm_channel_disable(PWM, this->pwm->channel);
		return;
	}

	this->pwm->ul_period = this->period;

	pwm_channel_init(PWM, this->pwm);
	pwm_channel_enable(PWM, this->pwm->channel);
}

void Backlight::SetState(enum BacklightState state)
{
	//assert(backlight);
	//assert(backlight->pwm);
	//assert(backlight->maxBrightness >= brightness);

	if (this->state == state)
		return;

	uint32_t brightness;

	switch (state)
	{
	case BacklightStateOff:
		brightness = 0;
		break;
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
