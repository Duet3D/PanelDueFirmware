#ifndef HARDWARE_BACKLIGHT_HPP
#define HARDWARE_BACKLIGHT_HPP 1

#include "asf.h"
#include <stdint.h>

enum BacklightState {
	BacklightStateNormal,
	BacklightStateDimmed
};

class Backlight
{
public:
	static const uint32_t MinBrightness = 0;
	static const uint32_t MaxBrightness = 100;

	Backlight(pwm_channel_t *pwm,
		uint32_t pwmFrequency, uint32_t frequency,
		uint32_t dimBrightness, uint32_t normalBrightness,
		uint32_t minDuty, uint32_t maxDuty);

	void SetDimBrightness(uint32_t p_dimBrightness) { dimBrightness = p_dimBrightness; }
	void SetNormalBrightness(uint32_t p_normalBrightness) { normalBrightness = p_normalBrightness; }

	void SetState(enum BacklightState state);
	enum BacklightState GetState() { return state; }

private:
	pwm_channel_t *pwm;

	uint32_t frequency;
	uint32_t period;
	uint32_t channel;

	uint32_t dimBrightness;
	uint32_t normalBrightness;

	uint32_t minDuty;
	uint32_t maxDuty;

	enum BacklightState state;

	void SetBrightness(uint32_t brightness);
};

#endif /* ifndef HARDWARE_BACKLIGHT_HPP */
