/*
 * ColourSchemes.cpp
 *
 * Created: 01/09/2016 13:13:29
 *  Author: David
 */

#ifndef OEM_COLOURS

#include "ColourSchemes.hpp"
#include "UserInterfaceConstants.hpp"
#include "Icons/Icons.hpp"

const ColourScheme colourSchemes[NumColourSchemes] =
{
	// Light colour schema. As this one comes first, it is the default.
	{
		.index = 0,
		.pal = IconPaletteLight,

		.titleBarTextColour = white,
		.titleBarBackColour = red,
		.labelTextColour = black,
		.infoTextColour = black,
		.infoBackColour = lightBlue,
		.defaultBackColour = white,
		.activeBackColour = lightRed,
		.standbyBackColour = lightYellow,
		.tuningBackColour = lightGreen,
		.errorTextColour = white,
		.errorBackColour = byzantine,

		.popupBorderColour = black,
		.popupBackColour = lightBlue,
		.popupTextColour = black,
		.popupButtonTextColour = black,
		.popupButtonBackColour = white,
		.popupInfoTextColour = black,
		.popupInfoBackColour = white,

		.alertPopupBackColour = lightGreen,
		.alertPopupTextColour = black,

		.buttonTextColour = black,
		.buttonPressedTextColour = black,
		.buttonTextBackColour = white,
		.buttonImageBackColour = white,
		.buttonGradColour = UTFT::fromRGB(255-8-8, 255-8-4, 255-8),
		.buttonPressedBackColour = lightGreen,
		.buttonPressedGradColour = UTFT::fromRGB(255-8-8, 255-8-4, 255-8),
		.buttonBorderColour = black,
		.homedButtonBackColour = lightBlue,
		.notHomedButtonBackColour = lightOrange,
		.pauseButtonBackColour = lightOrange,
		.resumeButtonBackColour = lightYellow,
		.resetButtonBackColour = lightRed,

		.progressBarColour = midGreen,
		.progressBarBackColour = white,

		.stopButtonTextColour = white,
		.stopButtonBackColour = UTFT::fromRGB(255, 24, 32)			// need enough G and B to allow for the gradient
	},

	// Dark colour scheme #1

	{
		.index = 1,
		.pal = IconPaletteDark,

		.titleBarTextColour = white,
		.titleBarBackColour = darkRed,
		.labelTextColour = white,
		.infoTextColour = white,
		.infoBackColour = darkBlue,
		.defaultBackColour = black,
		.activeBackColour = darkRed,
		.standbyBackColour = yellow,
		.tuningBackColour = darkGreen,
		.errorTextColour = white,
		.errorBackColour = byzantine,

		.popupBorderColour = white,
		.popupBackColour = darkBlue,
		.popupTextColour = white,
		.popupButtonTextColour = white,
		.popupButtonBackColour = black,
		.popupInfoTextColour = white,
		.popupInfoBackColour = black,

		.alertPopupBackColour = darkGreen,
		.alertPopupTextColour = white,

		.buttonTextColour = white,
		.buttonPressedTextColour = white,
		.buttonTextBackColour = black,
		.buttonImageBackColour = grey,
		.buttonGradColour = UTFT::fromRGB(8, 4, 8),
		.buttonPressedBackColour = darkGreen,
		.buttonPressedGradColour = UTFT::fromRGB(8, 8, 8),
		.buttonBorderColour = white,
		.homedButtonBackColour = UTFT::fromRGB(0,0,255-16), // blue needs to reduce to allow for gradient
		.notHomedButtonBackColour = darkOrange,
		.pauseButtonBackColour = darkOrange,
		.resumeButtonBackColour = darkYellow,
		.resetButtonBackColour = darkRed,

		.progressBarColour = midGreen,
		.progressBarBackColour = black,

		.stopButtonTextColour = white,
		.stopButtonBackColour = UTFT::fromRGB(255-16, 0, 0)			// need reduce R to allow for the gradient
	},

	// Dark colour scheme #2

	{
		.index = 2,
		.pal = IconPaletteDark,

		.titleBarTextColour = white,
		.titleBarBackColour = midGrey,
		.labelTextColour = white,
		.infoTextColour = white,
		.infoBackColour = veryDarkGrey,
		.defaultBackColour = veryDarkGrey,
		.activeBackColour = red,
		.standbyBackColour = yellow,
		.tuningBackColour = darkGrey,
		.errorTextColour = white,
		.errorBackColour = veryDarkGrey,

		.popupBorderColour = darkGrey,
		.popupBackColour = darkGrey,
		.popupTextColour = white,
		.popupButtonTextColour = white,
		.popupButtonBackColour = veryDarkGrey,
		.popupInfoTextColour = white,
		.popupInfoBackColour = darkGrey,

		.alertPopupBackColour = darkGrey,
		.alertPopupTextColour = white,

		.buttonTextColour = white,
		.buttonPressedTextColour = white,
		.buttonTextBackColour = midGrey,
		.buttonImageBackColour = grey,
		.buttonGradColour = 0,	//UTFT::fromRGB(8, 4, 8),
		.buttonPressedBackColour = grey,
		.buttonPressedGradColour = 0,	//UTFT::fromRGB(8, 8, 8),
		.buttonBorderColour = midGrey,
		.homedButtonBackColour = midGrey,
		.notHomedButtonBackColour = orange,
		.pauseButtonBackColour = darkOrange,
		.resumeButtonBackColour = darkYellow,
		.resetButtonBackColour = darkRed,

		.progressBarColour = midGrey,
		.progressBarBackColour = veryDarkGrey,
		.stopButtonTextColour = white,
		.stopButtonBackColour = red
	}
};

#endif

// End

