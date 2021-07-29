/*
 * ColourScheme.h
 *
 * Created: 01/09/2016 13:13:05
 *  Author: David
 */

#ifndef COLOURSCHEME_H_
#define COLOURSCHEME_H_

#include <Hardware/UTFT.hpp>

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

const size_t NumColourSchemes = 3;

extern const ColourScheme colourSchemes[];

#endif /* COLOURSCHEME_H_ */
