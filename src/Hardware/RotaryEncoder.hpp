/*
 * RotaryEncoder.h
 *
 *  Created on: 13 Mar 2020
 *      Author: David
 */

#ifndef __RotaryEncoderIncluded
#define __RotaryEncoderIncluded

#include "OneBitPort.hpp"

// Class to manage a rotary encoder with a push button
class RotaryEncoder
{
	const OneBitPort pin0, pin1, pinButton;
	int ppc;
	int encoderChange;
	unsigned int encoderState;
	bool buttonState;
	bool newPress;
	bool reverseDirection;
	uint32_t whenSame;

	unsigned int ReadEncoderState() const noexcept;

	static constexpr uint32_t DebounceMillis = 5;

public:
	RotaryEncoder(unsigned int p0, unsigned int p1, unsigned int pb) noexcept;

	void Init(int pulsesPerClick) noexcept;
	void Poll() noexcept;
	int GetChange() noexcept;
	bool GetButtonPress() noexcept;
	int GetPulsesPerClick() const noexcept { return ppc; }
};

#endif
