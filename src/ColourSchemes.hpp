/*
 * ColourScheme.h
 *
 * Created: 01/09/2016 13:13:05
 *  Author: David
 */

#ifndef COLOURSCHEME_H_
#define COLOURSCHEME_H_

#include "Hardware/UTFT.hpp"

// Some common colours
const Colour
	white = 0xFFFF,
	black = 0x0000,
	grey = UTFT::fromRGB(128, 128, 128),
	darkGrey = UTFT::fromRGB(60, 60, 60),
	midGrey = UTFT::fromRGB(80, 80, 80),
	veryDarkGrey = UTFT::fromRGB(40, 40, 40),
	red = UTFT::fromRGB(255,0,0),
	lightRed =  UTFT::fromRGB(255, 128, 128),
	darkRed = UTFT::fromRGB(128, 0, 0),
	yellow = UTFT::fromRGB(128,128,0),
	lightYellow = UTFT::fromRGB(255, 255, 128),
	darkYellow = UTFT::fromRGB(64, 64, 0),
	lightOrange = UTFT::fromRGB(255, 224, 192),
	orange = UTFT::fromRGB(255, 199, 89),
	darkOrange = UTFT::fromRGB(128, 64, 0),
	green = UTFT::fromRGB(0,255,0),
	lightGreen = UTFT::fromRGB(192, 255, 192),
	midGreen =  UTFT::fromRGB(0, 160, 0),
	darkGreen = UTFT::fromRGB(0, 96, 0),
	turquoise = UTFT::fromRGB(0,128,128),
	blue = UTFT::fromRGB(0,0,255),
	byzantine = UTFT::fromRGB(170,24,170),	// we need the 24 green to not wrap around with gradient
	lightBlue = UTFT::fromRGB(224, 224, 255),
	darkBlue = UTFT::fromRGB(0, 0, 64);

// Definition of a colour scheme
struct ColourScheme
{
	size_t index;
	Palette pal;

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
	Colour buttonTextBackColour;
	Colour buttonImageBackColour;
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

	Colour stopButtonTextColour;
	Colour stopButtonBackColour;
};

extern const ColourScheme colourSchemes[];

#endif /* COLOURSCHEME_H_ */
