/*
 * ColourScheme.h
 *
 * Created: 01/09/2016 13:13:05
 *  Author: David
 */ 


#ifndef COLOURSCHEME_H_
#define COLOURSCHEME_H_

#include <cstdint>
#include "Hardware/UTFT.hpp"

const size_t MaxColourSchemes = 5;				// the maximum number supported by firmware

// Some common colours
const Colour red = UTFT::fromRGB(255,0,0);
const Colour yellow = UTFT::fromRGB(128,128,0);
const Colour green = UTFT::fromRGB(0,255,0);
const Colour turquoise = UTFT::fromRGB(0,128,128);
const Colour blue = UTFT::fromRGB(0,0,255);
const Colour magenta = UTFT::fromRGB(128,0,128);
const Colour white = 0xFFFF;
const Colour black = 0x0000;

// Definition of a colour scheme
struct ColourScheme
{
	const char *name;
	Colour titleBarTextColour;
	Colour titleBarBackColour;
	Colour labelTextColour;
	Colour infoTextColour;
	Colour infoBackColour;
	Colour defaultBackColour;
	Colour activeBackColour;
	Colour standbyBackColour;
	Colour tuningBackColour;
	Colour errorTextColour;
	Colour errorBackColour;

	Colour popupBorderColour;
	Colour popupBackColour;
	Colour popupTextColour;
	Colour popupButtonTextColour;
	Colour popupButtonBackColour;
	Colour popupInfoTextColour;
	Colour popupInfoBackColour;

	Colour alertPopupBackColour;
	Colour alertPopupTextColour;

	Colour buttonTextColour;
	Colour buttonPressedTextColour;
	Colour buttonBackColour;
	Colour buttonGradColour;
	Colour buttonPressedBackColour;
	Colour buttonPressedGradColour;
	Colour buttonBorderColour;
	Colour homedButtonBackColour;
	Colour notHomedButtonBackColour;
	Colour pauseButtonBackColour;
	Colour resumeButtonBackColour;
	Colour resetButtonBackColour;

	Colour progressBarColour;
	Colour progressBarBackColour;
};

extern const size_t NumColourSchemes;
extern const ColourScheme colourSchemes[];

#endif /* COLOURSCHEME_H_ */