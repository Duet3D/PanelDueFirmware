#include "UI/Popup.hpp"

void AlertPopup::Set(const char *title, const char *text, int32_t mode, uint32_t controls)
{
	alertTitle.copy(title);

	// Split the alert text into 3 lines
	size_t splitPoint = MessageLog::FindSplitPoint(text, alertText1.Capacity(), (PixelNumber)(GetWidth() - 2 * popupSideMargin));
	alertText1.copy(text);
	alertText1.Truncate(splitPoint);
	text += splitPoint;
	splitPoint = MessageLog::FindSplitPoint(text, alertText2.Capacity(), GetWidth() - 2 * popupSideMargin);
	alertText2.copy(text);
	alertText2.Truncate(splitPoint);
	text += splitPoint;
	alertText3.copy(text);

	closeButton->Show(mode == 1);
	okButton->Show(mode >= 2);
	cancelButton->Show(mode == 3);
	const bool showZbuttons = (controls & (1u << 2)) != 0;
	zUpCourseButton->Show(showZbuttons);
	zUpMedButton->Show(showZbuttons);
	zUpFineButton->Show(showZbuttons);
	zDownCourseButton->Show(showZbuttons);
	zDownMedButton->Show(showZbuttons);
	zDownFineButton->Show(showZbuttons);
}

// Create a standard popup window with a title and a close button at the top right
StandardPopupWindow::StandardPopupWindow(PixelNumber ph, PixelNumber pw, Colour pb, Colour pBorder, Colour textColour, Colour imageBackColour, const char * null title, PixelNumber topMargin)
	: PopupWindow(ph, pw, pb, pBorder), titleField(nullptr)
{
	DisplayField::SetDefaultColours(textColour, pb);
	if (title != nullptr)
	{
		AddField(titleField = new StaticTextField(topMargin + labelRowAdjust, popupSideMargin + closeButtonWidth + popupFieldSpacing,
							pw - 2 * (popupSideMargin + closeButtonWidth + popupFieldSpacing), TextAlignment::Centre, title));
	}
	else
	{
		titleField = nullptr;
	}
	DisplayField::SetDefaultColours(textColour, imageBackColour);
	AddField(closeButton = new IconButton(popupTopMargin, pw - (closeButtonWidth + popupSideMargin), closeButtonWidth, IconCancel, evCancel));
}

AlertPopup::AlertPopup(const ColourScheme& colours)
	: StandardPopupWindow(alertPopupHeight, alertPopupWidth,
			colours.alertPopupBackColour, colours.popupBorderColour, colours.alertPopupTextColour, colours.buttonImageBackColour, "", popupTopMargin)		// title is present, but empty for now
{
	DisplayField::SetDefaultColours(colours.alertPopupTextColour, colours.alertPopupBackColour);
	titleField->SetValue(alertTitle.c_str(), true);
	AddField(new StaticTextField(popupTopMargin + 2 * rowTextHeight, popupSideMargin, GetWidth() - 2 * popupSideMargin, TextAlignment::Centre, alertText1.c_str()));
	AddField(new StaticTextField(popupTopMargin + 3 * rowTextHeight, popupSideMargin, GetWidth() - 2 * popupSideMargin, TextAlignment::Centre, alertText2.c_str()));
	AddField(new StaticTextField(popupTopMargin + 4 * rowTextHeight, popupSideMargin, GetWidth() - 2 * popupSideMargin, TextAlignment::Centre, alertText3.c_str()));

	// Calculate the button positions
	constexpr unsigned int numButtons = 6;
	constexpr PixelNumber buttonWidthUnits = 5;
	constexpr PixelNumber buttonSpacingUnits = 1;
	constexpr PixelNumber totalUnits = (numButtons * buttonWidthUnits) + ((numButtons - 1) * buttonSpacingUnits);
	constexpr PixelNumber unitWidth = (alertPopupWidth - 2 * popupSideMargin)/totalUnits;
	constexpr PixelNumber buttonWidth = buttonWidthUnits * unitWidth;
	constexpr PixelNumber buttonStep = (buttonWidthUnits + buttonSpacingUnits) * unitWidth;
	constexpr PixelNumber hOffset = popupSideMargin + (alertPopupWidth - 2 * popupSideMargin - totalUnits * unitWidth)/2;

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	AddField(zUpCourseButton =   new TextButtonForAxis(popupTopMargin + 6 * rowTextHeight, hOffset + 0 * buttonStep, buttonWidth, LESS_ARROW "2.0", evMoveAxis, "-2.0"));
	AddField(zUpMedButton =      new TextButtonForAxis(popupTopMargin + 6 * rowTextHeight, hOffset + 1 * buttonStep, buttonWidth, LESS_ARROW "0.2", evMoveAxis, "-0.2"));
	AddField(zUpFineButton =     new TextButtonForAxis(popupTopMargin + 6 * rowTextHeight, hOffset + 2 * buttonStep, buttonWidth, LESS_ARROW "0.02", evMoveAxis, "-0.02"));
	AddField(zDownFineButton =   new TextButtonForAxis(popupTopMargin + 6 * rowTextHeight, hOffset + 3 * buttonStep, buttonWidth, MORE_ARROW "0.02", evMoveAxis, "0.02"));
	AddField(zDownMedButton =    new TextButtonForAxis(popupTopMargin + 6 * rowTextHeight, hOffset + 4 * buttonStep, buttonWidth, MORE_ARROW "0.2", evMoveAxis, "0.2"));
	AddField(zDownCourseButton = new TextButtonForAxis(popupTopMargin + 6 * rowTextHeight, hOffset + 5 * buttonStep, buttonWidth, MORE_ARROW "2.0", evMoveAxis, "2.0"));
	zUpCourseButton->SetAxisLetter('Z');
	zUpMedButton->SetAxisLetter('Z');
	zUpFineButton->SetAxisLetter('Z');
	zDownFineButton->SetAxisLetter('Z');
	zDownMedButton->SetAxisLetter('Z');
	zDownCourseButton->SetAxisLetter('Z');

	AddField(okButton =          new TextButton(popupTopMargin + 6 * rowTextHeight + buttonHeight + moveButtonRowSpacing, hOffset + buttonStep,     buttonWidth + buttonStep, "OK", evCloseAlert, "M292 P0"));
	AddField(cancelButton =      new TextButton(popupTopMargin + 6 * rowTextHeight + buttonHeight + moveButtonRowSpacing, hOffset + 3 * buttonStep, buttonWidth + buttonStep, "Cancel", evCloseAlert, "M292 P1"));
}

