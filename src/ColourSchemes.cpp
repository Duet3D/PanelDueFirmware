/*
 * ColourSchemes.cpp
 *
 * Created: 01/09/2016 13:13:29
 *  Author: David
 */ 

#include "ColourSchemes.hpp"

const Colour
	lightRed =  UTFT::fromRGB(255, 128, 128),
	lightOrange = UTFT::fromRGB(255, 224, 192),
	lightYellow = UTFT::fromRGB(255, 255, 128),
	lightGreen = UTFT::fromRGB(192, 255, 192),
	lightBlue = UTFT::fromRGB(224, 224, 255),
	
	midGreen =  UTFT::fromRGB(0, 160, 0),
	
	darkRed = UTFT::fromRGB(128, 0, 0),
	darkOrange = UTFT::fromRGB(128, 64, 0),
	darkYellow = UTFT::fromRGB(128, 128, 0),
	darkGreen = UTFT::fromRGB(0, 96, 0),
	darkBlue = UTFT::fromRGB(0, 0, 64);

#ifdef OEM_COLOURS
#include "OemColourSchemes.cpp"
#else

const size_t NumColourSchemes = 2;

const ColourScheme colourSchemes[NumColourSchemes] =
{
	// Light colour schema. As this one comes first, it is the default.
	{
		.name = "Light",
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
		.errorBackColour = magenta,

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
		.buttonBackColour = white,
		.buttonGradColour = UTFT::fromRGB(255-8-1, 255-4-1, 255-8),
		.buttonPressedBackColour = lightGreen,
		.buttonPressedGradColour = UTFT::fromRGB(255-8-1, 255-8-1, 255-8),
		.buttonBorderColour = black,
		.homedButtonBackColour = lightBlue,
		.notHomedButtonBackColour = lightOrange,
		.pauseButtonBackColour = lightOrange,
		.resumeButtonBackColour = lightYellow,
		.resetButtonBackColour = lightRed,

		.progressBarColour = midGreen,
		.progressBarBackColour = white
	},
	
	// Dark colour scheme

	{
		.name = "Dark",
		.titleBarTextColour = white,
		.titleBarBackColour = darkRed,
		.labelTextColour = white,
		.infoTextColour = white,
		.infoBackColour = darkBlue,
		.defaultBackColour = black,
		.activeBackColour = darkRed,
		.standbyBackColour = darkYellow,
		.tuningBackColour = darkGreen,
		.errorTextColour = white,
		.errorBackColour = magenta,

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
		.buttonBackColour = black,
		.buttonGradColour = UTFT::fromRGB(8, 4, 8),
		.buttonPressedBackColour = darkGreen,
		.buttonPressedGradColour = UTFT::fromRGB(8, 8, 8),
		.buttonBorderColour = white,
		.homedButtonBackColour = darkBlue,
		.notHomedButtonBackColour = darkOrange,
		.pauseButtonBackColour = lightOrange,
		.resumeButtonBackColour = lightYellow,
		.resetButtonBackColour = lightRed,

		.progressBarColour = midGreen,
		.progressBarBackColour = black
	}
};

#endif

static_assert(NumColourSchemes <= MaxColourSchemes, "Too many colour schemes");

// End

