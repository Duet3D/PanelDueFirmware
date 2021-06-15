#ifndef HARDWARE_BACKLIGHT_HPP
#define HARDWARE_BACKLIGHT_HPP 1

#include "asf.h"
#include <stdint.h>

enum BacklightState {
	BacklightStateOff,
	BacklightStateNormal,
	BacklightStateDimmed
};

class Backlight
{
public:
	Backlight(pwm_channel_t *pwm,
		uint32_t pwmFrequency, uint32_t frequency,
		uint32_t dimBrightness, uint32_t normalBrightness,
		uint32_t minBrightness, uint32_t maxBrightness);
	virtual ~Backlight() {};

	void SetDimBrightness(uint32_t dimBrightness);
	void SetNormalBrightness(uint32_t normalBrightness);

	void SetState(enum BacklightState state);
	enum BacklightState GetState();

private:
	pwm_channel_t *pwm;

	uint32_t frequency;
	uint32_t period;
	uint32_t channel;

	uint32_t dimBrightness;
	uint32_t normalBrightness;

	uint32_t minBrightness;
	uint32_t maxBrightness;

	enum BacklightState state;

	void SetBrightness(uint32_t brightness);
};

#endif /* ifndef HARDWARE_BACKLIGHT_HPP */
