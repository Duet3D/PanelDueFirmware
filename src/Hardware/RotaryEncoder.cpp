/*
 * RotaryEncoder.cpp
 *
 *  Created on: 13 Mar 2020
 *      Author: David
 */

#include "Hardware/RotaryEncoder.hpp"
#include "Hardware/SysTick.hpp"
#include "Library/Misc.hpp"
#include <cmath>

RotaryEncoder::RotaryEncoder(unsigned int p0, unsigned int p1, unsigned int pb) noexcept
	: pin0(p0), pin1(p1), pinButton(pb),
	  ppc(2), encoderChange(0), encoderState(0), buttonState(0),
	  newPress(false), reverseDirection(false), whenSame(0) {}

inline unsigned int RotaryEncoder::ReadEncoderState() const noexcept
{
	return (pin0.read() ? 1u : 0u) | (pin1.read() ? 2u : 0u);
}

void RotaryEncoder::Init(int pulsesPerClick) noexcept
{
	ppc = max<unsigned int>(abs(pulsesPerClick), 1);
	reverseDirection = (pulsesPerClick < 0);

	// Set up pins
	pin0.setMode(OneBitPort::InputPullup);
	pin1.setMode(OneBitPort::InputPullup);
	pinButton.setMode(OneBitPort::InputPullup);
	OneBitPort::delay(200 * OneBitPort::delay_100ns);			// ensure we read the initial state correctly

	// Initialise encoder variables
	encoderChange = 0;
	encoderState = ReadEncoderState();

	// Initialise button variables
	buttonState = !pinButton.read();
	whenSame = SystemTick::GetTickCount();
	newPress = false;
}

void RotaryEncoder::Poll() noexcept
{
	// State transition table. Each entry has the following meaning:
	// 0 - the encoder hasn't moved
	// 1 or 2 - the encoder has moved 1 or 2 units clockwise
	// -1 or -2 = the encoder has moved 1 or 2 units anticlockwise
	static const int tbl[16] =
	{
		 0, +1, -1,  0,		// position 3 = 00 to 11, can't really do anything, so 0
		-1,  0, -2, +1,		// position 2 = 01 to 10, assume it was a bounce and should be 01 -> 00 -> 10
		+1, +2,  0, -1,		// position 1 = 10 to 01, assume it was a bounce and should be 10 -> 00 -> 01
		 0, -1, +1,  0		// position 0 = 11 to 00, can't really do anything
	};

	// Poll the encoder
	const unsigned int t = ReadEncoderState();
	const int movement = tbl[(encoderState << 2) | t];
	if (movement != 0)
	{
		encoderChange += movement;
		encoderState = t;
	}

	// Poll the button
	const uint32_t now = SystemTick::GetTickCount();
	const bool b = !pinButton.read();
	if (b == buttonState)
	{
		whenSame = now;
	}
	else if (now - whenSame > DebounceMillis)
	{
		buttonState = b;
		whenSame = now;
		if (buttonState)
		{
			newPress = true;
		}
	}
}

int RotaryEncoder::GetChange() noexcept
{
	const int rounding = (ppc - 1)/2;
	int r;
	if (encoderChange + rounding >= ppc - rounding)
	{
		r = (encoderChange + rounding)/ppc;
	}
	else if (encoderChange - rounding <= -ppc)
	{
		r = -((rounding - encoderChange)/ppc);
	}
	else
	{
		r = 0;
	}
	encoderChange -= (r * ppc);
	return (reverseDirection) ? -r : r;
}

bool RotaryEncoder::GetButtonPress() noexcept
{
	const bool ret = newPress;
	newPress = false;
	return ret;
}

// End
