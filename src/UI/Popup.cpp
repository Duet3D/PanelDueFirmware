#include "UI/Popup.hpp"

#include "General/SafeStrtod.h"
#include "General/SimpleMath.h"
#include "ObjectModel/Axis.hpp"
#include "Hardware/SerialIo.hpp"

#define DEBUG 0
#include "Debug.hpp"

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

	closeButton->Show(mode == Alert::Mode::InfoConfirm);

	okButton->Show(mode == Alert::Mode::ConfirmCancel ||
		       mode == Alert::Mode::NumberInt ||
		       mode == Alert::Mode::NumberFloat ||
		       mode == Alert::Mode::Text);

	cancelButton->Show((mode == Alert::Mode::ConfirmCancel) ||
			   (showCancelButton &&
			   (mode == Alert::Mode::NumberInt ||
			    mode == Alert::Mode::NumberFloat ||
			    mode == Alert::Mode::Text
			   )));

	warning->Show(false);
	value->Show(false);

	// hide all buttons
	for (size_t i = 0; i < ARRAY_SIZE(axisMap); i++)
	{
		axisMap[i]->Show(false);
	}
	driveLetterField->Show(false);

	for (size_t i = 0; i < ARRAY_SIZE(dirMap); i++)
	{
		dirMap[i].button->Show(false);
	}

	for (size_t i = 0; i < ARRAY_SIZE(selectionMap); i++)
	{
		selectionMap[i]->Show(false);
	}

	if (mode == Alert::Mode::InfoConfirm || mode == Alert::Mode::ConfirmCancel)
	{
		size_t axisIndex = 0;
		for (size_t i = 0; i < MaxTotalAxes; i++)
		{
			if (!(controls & (1u << i)))
			{
				continue;
			}

			OM::Axis *omAxis = OM::GetAxis(i);
			if (!omAxis)
			{
				continue;
			}

			TextButton *axis = axisMap[axisIndex];

			// select first axis if there is at least one
			if (axisIndex == 0)
			{
				ChangeLetter(i);
			}

			axisIndex++;

			axis->SetText(omAxis->letter);
			axis->SetEvent(evMoveSelectAxis, i);
			axis->Show(true);
		}
		driveLetterField->Show(controls ? true : false);

		// show jog buttons
		for (size_t i = 0; i < ARRAY_SIZE(dirMap); i++)
		{
			struct DirMap *dir = &dirMap[i];

			assert(dir->button);
			dir->button->Show(controls ? true : false);
		}
	}
}

void AlertPopup::Set(const Alert &alert)
{
	showCancelButton = alert.cancelButton;

	Set(alert.title.c_str(), alert.text.c_str(), alert.mode, alert.controls);

	mode = alert.mode;
	seq = alert.seq;
	limits = alert.limits;

	switch (mode)
	{
	case Alert::Mode::Info:
	case Alert::Mode::InfoClose:
	case Alert::Mode::InfoConfirm:
	case Alert::Mode::ConfirmCancel:
		break;
	case Alert::Mode::Choices:
#if 1
		for (size_t i = 0; i < ARRAY_SIZE(selectionMap); i++)
		{
			dbg("show choices %d size %d\n", i, ARRAY_SIZE(alert.choices));
			if (i < ARRAY_SIZE(alert.choices))
			{
				dbg("show text %d %s\n", i, alert.choices[i].c_str());
				selectionMap[i]->SetText(alert.choices[i].c_str());
				selectionMap[i]->Show(true);
			}
		}
#endif
		break;
	case Alert::Mode::NumberInt:
		valueText.printf("%ld", limits.numberInt.valueDefault);
		value->SetText(valueText.c_str());
		value->Show(true);
		dbg("showing int value\n");
		break;
	case Alert::Mode::NumberFloat:
		valueText.printf("%f", (double)limits.numberFloat.valueDefault);
		value->SetText(valueText.c_str());
		value->Show(true);
		break;
	case Alert::Mode::Text:
		valueText.copy(limits.text.valueDefault.c_str());
		value->SetText(valueText.c_str());
		value->Show(true);
		break;
	default:
		dbg("invalid mode %d\n", alert.mode);
		break;
	}
}


void AlertPopup::ProcessOkButton()
{
	dbg("\n");
	switch (mode)
	{
	case Alert::Mode::Info:
	case Alert::Mode::InfoClose:
	case Alert::Mode::Choices:
		break;
	case Alert::Mode::InfoConfirm:
	case Alert::Mode::ConfirmCancel:
		SerialIo::Sendf("M292 P0 S%lu\n", seq);
		break;
	case Alert::Mode::NumberFloat:
	case Alert::Mode::NumberInt:
	case Alert::Mode::Text:
		SerialIo::Sendf("M292 P0 R{%s} S%lu\n", valueText.c_str(), seq);
		break;
	default:
		dbg("invalid mode %d\n", mode);
		break;
	}
}


void AlertPopup::ProcessChoice(uint32_t choice)
{
	if (mode != Alert::Mode::Choices)
		return;

	SerialIo::Sendf("M292 R{%lu} S%lu\n", choice, seq);
}


void AlertPopup::UpdateData(const char *data)
{
	bool valid = false;

	valueText.copy(data);
	value->SetText(valueText.c_str());
	value->Show(true);

	switch (mode)
	{
	case Alert::Mode::NumberInt:
		{
			int valueInt = StrToI32(data);
			valid = Validate(valueInt);

			warningText.printf("out of range %ld <= value <= %ld",
					limits.numberInt.min, limits.numberInt.max);
		}
		break;
	case Alert::Mode::NumberFloat:
		{
			float valueFloat = SafeStrtof(data);
			valid = Validate(valueFloat);
			if (valid)
				break;
			warningText.printf("out of range %f <= value <= %f",
					(double)limits.numberFloat.min, (double)limits.numberFloat.max);
		}
		break;
	case Alert::Mode::Text:
		{
			valid = Validate(data);
			if (valid)
				break;
			warningText.printf("invalid length %ld <= length <= %ld",
					limits.text.min, limits.text.max);
		}
		break;
	default:
		{
			dbg("Error: mode does not support data updates\n");
			return;
		}
		break;
	}

	okButton->Show(valid);

	if (!valid)
	{
		// TODO show warning
		warning->SetValue(warningText.c_str(), true);
		warning->Show(true);
	}
}

bool AlertPopup::Validate(int value)
{
	if (value < limits.numberInt.min)
		return false;

	if (value > limits.numberInt.max)
		return false;

	return true;
}

bool AlertPopup::Validate(float value)
{
	if (value < limits.numberFloat.min)
		return false;

	if (value > limits.numberFloat.max)
		return false;

	return true;
}

bool AlertPopup::Validate(const char *value)
{
	if (strlen(value) < (size_t)limits.text.min)
		return false;

	if (strlen(value) > (size_t)limits.text.max)
		return false;

	return true;
}

void AlertPopup::ChangeLetter(const size_t index)
{
	if (index >= ARRAY_SIZE(axisMap))
	{
		return;
	}

	OM::Axis *axis = OM::GetAxis(index);

	if (!axis)
	{
		return;
	}

	driveLetter.copy(axis->letter);
	driveLetterField->SetValue(driveLetter.c_str(), true);

	for (size_t i = 0; i < ARRAY_SIZE(dirMap); i++)
	{
		assert(dirMap[i].button);
		dirMap[i].button->SetAxisLetter(axis->letter[0]);
	}
	
}

AlertPopup::AlertPopup(const ColourScheme& colours) :
	StandardPopupWindow(
			alertPopupHeight, alertPopupWidth, colours.alertPopupBackColour, colours.popupBorderColour,
			colours.alertPopupTextColour, colours.buttonImageBackColour, "", popupTopMargin),
	showCancelButton(false),
	mode(Alert::Mode::None)
{
	DisplayField::SetDefaultColours(colours.alertPopupTextColour, colours.alertPopupBackColour);
	titleField->SetValue(alertTitle.c_str(), true);
	AddField(new StaticTextField(popupTopMargin + 2 * rowTextHeight, popupSideMargin, GetWidth() - 2 * popupSideMargin, TextAlignment::Centre, alertText1.c_str()));
	AddField(new StaticTextField(popupTopMargin + 3 * rowTextHeight, popupSideMargin, GetWidth() - 2 * popupSideMargin, TextAlignment::Centre, alertText2.c_str()));
	AddField(new StaticTextField(popupTopMargin + 4 * rowTextHeight, popupSideMargin, GetWidth() - 2 * popupSideMargin, TextAlignment::Centre, alertText3.c_str()));
	warning = new StaticTextField(popupTopMargin + 5 * rowTextHeight, popupSideMargin, GetWidth() - 2 * popupSideMargin, TextAlignment::Centre, "");
	AddField(warning);

	// Calculate the button positions
	constexpr unsigned int numButtons = 7;
	constexpr PixelNumber buttonWidthUnits = 5;
	constexpr PixelNumber buttonSpacingUnits = 1;
	constexpr PixelNumber totalUnits = (numButtons * buttonWidthUnits) + ((numButtons - 1) * buttonSpacingUnits);
	constexpr PixelNumber unitWidth = (alertPopupWidth - 2 * popupSideMargin)/totalUnits;
	constexpr PixelNumber buttonWidth = buttonWidthUnits * unitWidth;
	constexpr PixelNumber buttonAxis = 72;
	constexpr PixelNumber buttonAxisWidth = 52;
	constexpr PixelNumber buttonChoice = 145;
	constexpr PixelNumber buttonChoiceWidth = 125;
	constexpr PixelNumber buttonValueWith = 220;
	constexpr PixelNumber buttonStep = (buttonWidthUnits + buttonSpacingUnits) * unitWidth;
	constexpr PixelNumber hOffset = popupSideMargin + (alertPopupWidth - 2 * popupSideMargin - totalUnits * unitWidth)/2;

	// add drive letter text field
	driveLetterField = new StaticTextField(
				popupTopMargin + 5 * rowTextHeight + buttonHeight + moveButtonRowSpacing,
				hOffset + ARRAY_SIZE(dirMap) / 2 * buttonStep,
				buttonWidth, TextAlignment::Centre, driveLetter.c_str(), true);
	AddField(driveLetterField);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);

	for (size_t i = 0; i < ARRAY_SIZE(axisMap); i++)
	{
		TextButton *button = new TextButton(
				popupTopMargin + 5 * rowTextHeight,
				hOffset + i * buttonAxis, buttonAxisWidth,
				"none", evMoveSelectAxis, i);
		assert(button);

		AddField(button);
		axisMap[i] = button;
	}

	for (size_t i = 0; i < ARRAY_SIZE(selectionMap) / 2; i++)
	{
		TextButton *button = new TextButton(
				popupTopMargin + 5 * rowTextHeight,
				hOffset + i * buttonChoice, buttonChoiceWidth,
				"none", evChoiceAlert, i);
		assert(button);

		AddField(button);
		selectionMap[i] = button;
	}

	for (size_t i = ARRAY_SIZE(selectionMap) / 2; i < ARRAY_SIZE(selectionMap); i++)
	{
		TextButton *button = new TextButton(
				popupTopMargin + 6 * rowTextHeight + rowTextHeight / 2,
				hOffset + (i - ARRAY_SIZE(selectionMap) / 2) * buttonChoice, buttonChoiceWidth,
				"none", evChoiceAlert, i);
		assert(button);

		AddField(button);
		selectionMap[i] = button;
	}

	for (size_t i = 0; i < ARRAY_SIZE(dirMap) / 2; i++)
	{
		struct DirMap *dir = &dirMap[i];

		TextButtonForAxis *button = new TextButtonForAxis(
				popupTopMargin + 5 * rowTextHeight + buttonHeight + moveButtonRowSpacing ,
				hOffset + i * buttonStep, buttonWidth,
				dir->text, evMoveAxis, dir->param);

		assert(button);
		AddField(button);
		dir->button = button;
	}

	for (size_t i = ARRAY_SIZE(dirMap) / 2; i < ARRAY_SIZE(dirMap); i++)
	{
		struct DirMap *dir = &dirMap[i];

		TextButtonForAxis *button = new TextButtonForAxis(
				popupTopMargin + 5 * rowTextHeight + buttonHeight + moveButtonRowSpacing ,
				hOffset + (i + 1) * buttonStep, buttonWidth,
				dir->text, evMoveAxis, dir->param);

		assert(button);
		AddField(button);
		dir->button = button;
	}

	value = new TextButton(
			popupTopMargin + 6 * rowTextHeight,
			(alertPopupWidth - buttonValueWith) / 2, buttonValueWith,
			"none", evEditAlert, 0);
	assert(value);
	AddField(value);

	constexpr PixelNumber controlButtonWidth = buttonWidth + buttonStep;
	constexpr PixelNumber hOkOffset = popupSideMargin + (alertPopupWidth - 3 * popupSideMargin - 2 * controlButtonWidth) / 2;
	constexpr PixelNumber hCancelOffset = hOkOffset + popupSideMargin + controlButtonWidth;

	AddField(okButton =          new TextButton(popupTopMargin + 7 * rowTextHeight + buttonHeight + moveButtonRowSpacing, hOkOffset,     controlButtonWidth, "OK", evOkAlert, "M292 P0"));
	AddField(cancelButton =      new TextButton(popupTopMargin + 7 * rowTextHeight + buttonHeight + moveButtonRowSpacing, hCancelOffset, controlButtonWidth, "Cancel", evCloseAlert, "M292 P1"));
}

