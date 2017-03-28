/*
 * UserInterface.cpp
 *
 *  Created on: 7 Jan 2017
 *      Author: David
 */

#ifndef OEM_LAYOUT

#include "UserInterface.hpp"

#include "UserInterfaceConstants.hpp"
#include "PanelDue.hpp"
#include "FileManager.hpp"
#include "MessageLog.hpp"
#include "Library/Misc.hpp"
#include "Library/Vector.hpp"
#include "Icons/Icons.hpp"
#include "Hardware/Buzzer.hpp"
#include "Hardware/Reset.hpp"
#include "Hardware/SerialIo.hpp"
#include "Strings.hpp"

const unsigned int numLanguages = 3;
static_assert(ARRAY_SIZE(LanguageTables) == numLanguages, "Wrong number of languages in LanguageTable");
static const char* array const axisNames[] = { "X", "Y", "Z", "U", "V", "W" };

#if DISPLAY_X == 800
const Icon heaterIcons[maxHeaters] = { IconBed, IconNozzle1, IconNozzle2, IconNozzle3, IconNozzle4, IconNozzle5, IconNozzle6 };
#else
const Icon heaterIcons[maxHeaters] = { IconBed, IconNozzle1, IconNozzle2, IconNozzle3, IconNozzle4 };
#endif

// Public fields
TextField *fwVersionField, *userCommandField;
IntegerField *freeMem, *touchX, *touchY;
TextButton *filenameButtons[numDisplayedFiles];
StaticTextField *touchCalibInstruction, *debugField;
StaticTextField *messageTextFields[numMessageRows], *messageTimeFields[numMessageRows];

// Private fields
static PopupWindow *setTempPopup, *movePopup, *extrudePopup, *fileListPopup, *filePopup, *baudPopup, *volumePopup, *areYouSurePopup, *keyboardPopup, *languagePopup, *coloursPopup;
static SingleButton *scrollFilesLeftButton, *scrollFilesRightButton, *filesUpButton, *changeCardButton;
static StaticTextField *areYouSureTextField, *areYouSureQueryField, *macroPopupTitleField;
static DisplayField *baseRoot, *commonRoot, *controlRoot, *printRoot, *messageRoot, *setupRoot;
static SingleButton *homeButtons[MAX_AXES], *toolButtons[maxHeaters], *homeAllButton, *bedCompButton;
static FloatField *axisPos[MAX_AXES];
static FloatField *currentTemps[maxHeaters];
static FloatField *fpHeightField, *fpLayerHeightField, *babystepOffsetField;
static IntegerField *fpSizeField, *fpFilamentField, *fileListErrorField, *filePopupTitleField;
static ProgressBar *printProgressBar;
static SingleButton *tabControl, *tabPrint, *tabMsg, *tabSetup;
static ButtonBase *filesButton, *pauseButton, *resumeButton, *resetButton, *babystepButton;
static TextField *timeLeftField, *zProbe;
static TextField *fpNameField, *fpGeneratedByField;
static StaticTextField *moveAxisRows[MAX_AXES];
static StaticTextField *nameField, *statusField, *settingsNotSavedField;
static IntegerButton *activeTemps[maxHeaters], *standbyTemps[maxHeaters];
static IntegerButton *spd, *extrusionFactors[maxHeaters - 1], *fanSpeed, *baudRateButton, *volumeButton;
static TextButton *languageButton, *coloursButton;
static SingleButton *moveButton, *extrudeButton, *macroButton;
static PopupWindow *alertPopup, *babystepPopup;

static ButtonBase * null currentTab = nullptr;

static ButtonPress currentButton;
static ButtonPress fieldBeingAdjusted;
static ButtonPress currentExtrudeRatePress, currentExtrudeAmountPress;

const size_t machineNameLength = 30;
const size_t printingFileLength = 40;
const size_t zprobeBufLength = 12;
const size_t alertTextLength = 80;
const size_t generatedByTextLength = 50;

static String<machineNameLength> machineName;
static String<printingFileLength> printingFile;
static String<zprobeBufLength> zprobeBuf;
static String<alertTextLength> alertText;
static String<generatedByTextLength> generatedByText;

const size_t maxUserCommandLength = 40;					// max length of a user gcode command
const size_t numUserCommandBuffers = 6;					// number of command history buffers plus one

static String<maxUserCommandLength> userCommandBuffers[numUserCommandBuffers];
static size_t currentUserCommandBuffer = 0, currentHistoryBuffer = 0;

static int oldIntValue;
static bool restartNeeded = false;
int heaterStatus[maxHeaters];
static Event eventToConfirm = evNull;

const char* array null currentFile = nullptr;			// file whose info is displayed in the file info popup
const StringTable * strings = &LanguageTables[0];
static bool keyboardIsDisplayed = false;

// Create a standard popup window with a title and a close button at the top right
PopupWindow *CreatePopupWindow(PixelNumber ph, PixelNumber pw, Colour pb, Colour pBorder, Colour textColour, Colour imageBackColour, const char * null title, PixelNumber topMargin = popupTopMargin)
{
	PopupWindow *window = new PopupWindow(ph, pw, pb, pBorder);
	DisplayField::SetDefaultColours(textColour, pb);
	if (title != nullptr)
	{
		window->AddField(new StaticTextField(topMargin + labelRowAdjust, popupSideMargin + closeButtonWidth + popupFieldSpacing,
							pw - 2 * (popupSideMargin + closeButtonWidth + popupFieldSpacing), TextAlignment::Centre, title));
	}
	DisplayField::SetDefaultColours(textColour, imageBackColour);
	window->AddField(new IconButton(popupTopMargin, pw - (closeButtonWidth + popupSideMargin), closeButtonWidth, IconCancel, evCancel));
	return window;
}

// Add a text button
TextButton *AddTextButton(PixelNumber row, unsigned int col, unsigned int numCols, const char* array text, Event evt, const char* param)
{
	PixelNumber width = (DisplayX - 2 * margin + fieldSpacing)/numCols - fieldSpacing;
	PixelNumber xpos = col * (width + fieldSpacing) + margin;
	TextButton *f = new TextButton(row - 2, xpos, width, text, evt, param);
	mgr.AddField(f);
	return f;
}

// Add an integer button
IntegerButton *AddIntegerButton(PixelNumber row, unsigned int col, unsigned int numCols, const char * array null label, const char * array null units, Event evt)
{
	PixelNumber width = (DisplayX - 2 * margin + fieldSpacing)/numCols - fieldSpacing;
	PixelNumber xpos = col * (width + fieldSpacing) + margin;
	IntegerButton *f = new IntegerButton(row - 2, xpos, width, label, units);
	f->SetEvent(evt, 0);
	mgr.AddField(f);
	return f;
}

// Add an icon button with a string parameter
IconButton *AddIconButton(PixelNumber row, unsigned int col, unsigned int numCols, Icon icon, Event evt, const char* param)
{
	PixelNumber width = (DisplayX - 2 * margin + fieldSpacing)/numCols - fieldSpacing;
	PixelNumber xpos = col * (width + fieldSpacing) + margin;
	IconButton *f = new IconButton(row - 2, xpos, width, icon, evt, param);
	mgr.AddField(f);
	return f;
}

// Create a row of text buttons.
// Optionally, set one to 'pressed' and return that one.
// Set the colours before calling this
ButtonPress CreateStringButtonRow(Window * pf, PixelNumber top, PixelNumber left, PixelNumber totalWidth, PixelNumber spacing, unsigned int numButtons,
									const char* array const text[], const char* array const params[], Event evt, int selected = -1)
{
	const PixelNumber step = (totalWidth + spacing)/numButtons;
	ButtonPress bp;
	for (unsigned int i = 0; i < numButtons; ++i)
	{
		TextButton *tp = new TextButton(top, left + i * step, step - spacing, text[i], evt, params[i]);
		pf->AddField(tp);
		if ((int)i == selected)
		{
			tp->Press(true, 0);
			bp = ButtonPress(tp, 0);
		}
	}
	return bp;
}

// Create a row of icon buttons.
// Set the colours before calling this
void CreateIconButtonRow(Window * pf, PixelNumber top, PixelNumber left, PixelNumber totalWidth, PixelNumber spacing, unsigned int numButtons,
									const Icon icons[], const char* array const params[], Event evt)
{
	const PixelNumber step = (totalWidth + spacing)/numButtons;
	for (unsigned int i = 0; i < numButtons; ++i)
	{
		pf->AddField(new IconButton(top, left + i * step, step - spacing, icons[i], evt, params[i]));
	}
}

// Create a popup bar with string parameters
PopupWindow *CreateStringPopupBar(const ColourScheme& colours, PixelNumber width, unsigned int numEntries, const char* const text[], const char* const params[], Event ev)
{
	PopupWindow *pf = new PopupWindow(popupBarHeight, width, colours.popupBackColour, colours.popupBorderColour);
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	PixelNumber step = (width - 2 * popupSideMargin + popupFieldSpacing)/numEntries;
	for (unsigned int i = 0; i < numEntries; ++i)
	{
		pf->AddField(new TextButton(popupTopMargin, popupSideMargin + i * step, step - popupFieldSpacing, text[i], ev, params[i]));
	}
	return pf;
}

// Create a popup bar with integer parameters
// If the 'params' parameter is null then we use 0, 1, 2.. at the parameters
PopupWindow *CreateIntPopupBar(const ColourScheme& colours, PixelNumber width, unsigned int numEntries, const char* const text[], const int * null params, Event ev, Event zeroEv)
{
	PopupWindow *pf = new PopupWindow(popupBarHeight, width, colours.popupBackColour, colours.popupBorderColour);
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	PixelNumber step = (width - 2 * popupSideMargin + popupFieldSpacing)/numEntries;
	for (unsigned int i = 0; i < numEntries; ++i)
	{
		const int iParam = (params == nullptr) ? (int)i : params[i];
		pf->AddField(new TextButton(popupSideMargin, popupSideMargin + i * step, step - popupFieldSpacing, text[i], (params[i] == 0) ? zeroEv : ev, iParam));
	}
	return pf;
}

// Nasty hack to work around bug in RepRapFirmware 1.09k and earlier
// The M23 and M30 commands don't work if we send the full path, because "0:/gcodes/" gets prepended regardless.
const char * array StripPrefix(const char * array dir)
{
	if ((GetFirmwareFeatures() && noGcodesFolder) == 0)			// if running RepRapFirmware
	{
		const size_t len = strlen(dir);
		if (len >= 8 && memcmp(dir, "/gcodes/", 8) == 0)
		{
			dir += 8;
		}
		else if (len >= 10 && memcmp(dir, "0:/gcodes/", 10) == 0)
		{
			dir += 10;
		}
		else if (strcmp(dir, "/gcodes") == 0 || strcmp(dir, "0:/gcodes") == 0)
		{
			dir += len;
		}
	}
	return dir;
}

// Adjust the brightness
void ChangeBrightness(bool up)
{
	int adjust = max<int>(1, GetBrightness()/16);
	if (!up)
	{
		adjust = -adjust;
	}
	SetBrightness(GetBrightness() + adjust);
}

// Update an integer field, provided it isn't the one being adjusted
// Don't update it if the value hasn't changed, because that makes the display flicker unnecessarily
void UpdateField(IntegerButton *f, int val)
{
	if (f != fieldBeingAdjusted.GetButton() && f->GetValue() != val)
	{
		f->SetValue(val);
	}
}

void PopupAreYouSure(Event ev, const char* text, const char* query = strings->areYouSure)
{
	eventToConfirm = ev;
	areYouSureTextField->SetValue(text);
	areYouSureQueryField->SetValue(query);
	mgr.SetPopup(areYouSurePopup, AutoPlace, AutoPlace);
}

void PopupRestart()
{
	PopupAreYouSure(evRestart, "Restart required", "Restart now?");
}

void CreateIntegerAdjustPopup(const ColourScheme& colours)
{
	// Create the popup window used to adjust temperatures, fan speed, extrusion factor etc.
	static const char* const tempPopupText[] = {"-5", "-1", "Set", "+1", "+5"};
	static const int tempPopupParams[] = { -5, -1, 0, 1, 5 };
	setTempPopup = CreateIntPopupBar(colours, tempPopupBarWidth, 5, tempPopupText, tempPopupParams, evAdjustInt, evSetInt);
}

// Create the movement popup window
void CreateMovePopup(const ColourScheme& colours)
{
	static const char * array const xyJogValues[] = { "-100", "-10", "-1", "-0.1", "0.1",  "1", "10", "100" };
	static const char * array const zJogValues[] = { "-50", "-5", "-0.5", "-0.05", "0.05",  "0.5", "5", "50" };

	movePopup = CreatePopupWindow(movePopupHeight, movePopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, colours.buttonImageBackColour, strings->moveHead);
	PixelNumber ypos = popupTopMargin + buttonHeight + moveButtonRowSpacing;
	const PixelNumber xpos = popupSideMargin + axisLabelWidth;
	Event e = evMoveX;
	for (size_t i = 0; i < MAX_AXES; ++i)
	{
		DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
		const char * array const * array values = (axisNames[i][0] == 'Z') ? zJogValues : xyJogValues;
		CreateStringButtonRow(movePopup, ypos, xpos, movePopupWidth - xpos - popupSideMargin, fieldSpacing, 8, values, values, e);

		// We create the label after the button row, so that the buttons follow it in the field order, which makes it easier to hide them
		DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
		StaticTextField * const tf = new StaticTextField(ypos + labelRowAdjust, popupSideMargin, axisLabelWidth, TextAlignment::Left, axisNames[i]);
		movePopup->AddField(tf);
		moveAxisRows[i] = tf;
		UI::ShowAxis(i, i < MIN_AXES);

		ypos += buttonHeight + moveButtonRowSpacing;
		e = (Event)((uint8_t)e + 1);
	}
}

// Create the extrusion controls popup
void CreateExtrudePopup(const ColourScheme& colours)
{
	static const char * array extrudeAmountValues[] = { "100", "50", "20", "10", "5",  "1" };
	static const char * array extrudeSpeedValues[] = { "50", "40", "20", "10", "5" };
	static const char * array extrudeSpeedParams[] = { "3000", "2400", "1200", "600", "300" };

	extrudePopup = CreatePopupWindow(extrudePopupHeight, extrudePopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupButtonTextColour, colours.buttonImageBackColour, strings->extrusionAmount);
	PixelNumber ypos = popupTopMargin + buttonHeight + extrudeButtonRowSpacing;
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	currentExtrudeAmountPress = CreateStringButtonRow(extrudePopup, ypos, popupSideMargin, extrudePopupWidth - 2 * popupSideMargin, fieldSpacing, 6, extrudeAmountValues, extrudeAmountValues, evExtrudeAmount, 3);
	ypos += buttonHeight + extrudeButtonRowSpacing;
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	extrudePopup->AddField(new StaticTextField(ypos + labelRowAdjust, popupSideMargin, extrudePopupWidth - 2 * popupSideMargin, TextAlignment::Centre, strings->extrusionSpeed));
	ypos += buttonHeight + extrudeButtonRowSpacing;
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	currentExtrudeRatePress = CreateStringButtonRow(extrudePopup, ypos, popupSideMargin, extrudePopupWidth - 2 * popupSideMargin, fieldSpacing, 5, extrudeSpeedValues, extrudeSpeedParams, evExtrudeRate, 4);
	ypos += buttonHeight + extrudeButtonRowSpacing;
	extrudePopup->AddField(new TextButton(ypos, popupSideMargin, extrudePopupWidth/3 - 2 * popupSideMargin, strings->extrude, evExtrude));
	extrudePopup->AddField(new TextButton(ypos, (2 * extrudePopupWidth)/3 + popupSideMargin, extrudePopupWidth/3 - 2 * popupSideMargin, strings->retract, evRetract));
}

// Create the popup used to list files and macros
void CreateFileListPopup(const ColourScheme& colours)
{
	fileListPopup = CreatePopupWindow(fileListPopupHeight, fileListPopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, colours.buttonImageBackColour, nullptr);
	const PixelNumber closeButtonPos = fileListPopupWidth - closeButtonWidth - popupSideMargin;
	const PixelNumber navButtonWidth = (closeButtonPos - popupSideMargin)/7;
	const PixelNumber upButtonPos = closeButtonPos - navButtonWidth - fieldSpacing;
	const PixelNumber rightButtonPos = upButtonPos - navButtonWidth - fieldSpacing;
	const PixelNumber leftButtonPos = rightButtonPos - navButtonWidth - fieldSpacing;
	const PixelNumber textPos = popupSideMargin + navButtonWidth;
	const PixelNumber changeButtonPos = popupSideMargin;

	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	fileListPopup->AddField(filePopupTitleField = new IntegerField(popupTopMargin + labelRowAdjust, textPos, leftButtonPos - textPos, TextAlignment::Centre, strings->filesOnCard, nullptr));
	fileListPopup->AddField(macroPopupTitleField = new StaticTextField(popupTopMargin + labelRowAdjust, textPos, leftButtonPos - textPos, TextAlignment::Centre, strings->macros));

	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.buttonImageBackColour);
	fileListPopup->AddField(changeCardButton = new IconButton(popupTopMargin, changeButtonPos, navButtonWidth, IconFiles, evChangeCard, 0));
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	fileListPopup->AddField(scrollFilesLeftButton = new TextButton(popupTopMargin, leftButtonPos, navButtonWidth, "<", evScrollFiles, -1));
	scrollFilesLeftButton->Show(false);
	fileListPopup->AddField(scrollFilesRightButton = new TextButton(popupTopMargin, rightButtonPos, navButtonWidth, ">", evScrollFiles, 1));
	scrollFilesRightButton->Show(false);
	fileListPopup->AddField(filesUpButton = new IconButton(popupTopMargin, upButtonPos, navButtonWidth, IconUp, evNull));
	filesUpButton->Show(false);

	const PixelNumber fileFieldWidth = (fileListPopupWidth + fieldSpacing - (2 * popupSideMargin))/numFileColumns;
	unsigned int fileNum = 0;
	for (unsigned int c = 0; c < numFileColumns; ++c)
	{
		PixelNumber row = popupTopMargin;
		for (unsigned int r = 0; r < numFileRows; ++r)
		{
			row += buttonHeight + fileButtonRowSpacing;
			TextButton *t = new TextButton(row, (fileFieldWidth * c) + popupSideMargin, fileFieldWidth - fieldSpacing, nullptr, evNull);
			t->Show(false);
			fileListPopup->AddField(t);
			filenameButtons[fileNum] = t;
			++fileNum;
		}
	}

	fileListErrorField = new IntegerField(popupTopMargin + 2 * (buttonHeight + fileButtonRowSpacing), popupSideMargin, fileListPopupWidth - (2 * popupSideMargin),
							TextAlignment::Centre, strings->error, strings->accessingSdCard);
	fileListErrorField->Show(false);
	fileListPopup->AddField(fileListErrorField);
}

// Create the popup window used to display the file dialog
void CreateFileActionPopup(const ColourScheme& colours)
{
	filePopup = CreatePopupWindow(fileInfoPopupHeight, fileInfoPopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, colours.buttonImageBackColour, "File information");
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	PixelNumber ypos = popupTopMargin + (3 * rowTextHeight)/2;
	fpNameField = new TextField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, strings->fileName);
	ypos += rowTextHeight;
	fpSizeField = new IntegerField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, strings->fileSize, " b");
	ypos += rowTextHeight;
	fpLayerHeightField = new FloatField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, 2, strings->layerHeight, "mm");
	ypos += rowTextHeight;
	fpHeightField = new FloatField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, 1, strings->objectHeight, "mm");
	ypos += rowTextHeight;
	fpFilamentField = new IntegerField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, strings->filamentNeeded, "mm");
	ypos += rowTextHeight;
	fpGeneratedByField = new TextField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, strings->generatedBy, generatedByText.c_str());
	filePopup->AddField(fpNameField);
	filePopup->AddField(fpSizeField);
	filePopup->AddField(fpLayerHeightField);
	filePopup->AddField(fpHeightField);
	filePopup->AddField(fpFilamentField);
	filePopup->AddField(fpGeneratedByField);

	// Add the buttons
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	filePopup->AddField(new TextButton(popupTopMargin + 8 * rowTextHeight, popupSideMargin, fileInfoPopupWidth/3 - 2 * popupSideMargin, strings->print, evPrint));
	filePopup->AddField(new IconButton(popupTopMargin + 8 * rowTextHeight, (2 * fileInfoPopupWidth)/3 + popupSideMargin, fileInfoPopupWidth/3 - 2 * popupSideMargin, IconTrash, evDeleteFile));
}

// Create the "Are you sure?" popup
void CreateAreYouSurePopup(const ColourScheme& colours)
{
	areYouSurePopup = new PopupWindow(areYouSurePopupHeight, areYouSurePopupWidth, colours.popupBackColour, colours.popupBorderColour);
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	areYouSurePopup->AddField(areYouSureTextField = new StaticTextField(popupSideMargin, margin, areYouSurePopupWidth - 2 * margin, TextAlignment::Centre, nullptr));
	areYouSurePopup->AddField(areYouSureQueryField = new StaticTextField(popupTopMargin + rowHeight, margin, areYouSurePopupWidth - 2 * margin, TextAlignment::Centre, nullptr));

	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	areYouSurePopup->AddField(new IconButton(popupTopMargin + 2 * rowHeight, popupSideMargin, areYouSurePopupWidth/2 - 2 * popupSideMargin, IconOk, evYes));
	areYouSurePopup->AddField(new IconButton(popupTopMargin + 2 * rowHeight, areYouSurePopupWidth/2 + 10, areYouSurePopupWidth/2 - 2 * popupSideMargin, IconCancel, evCancel));
}

// Create the baud rate adjustment popup
void CreateBaudRatePopup(const ColourScheme& colours)
{
	static const char* const baudPopupText[] = { "9600", "19200", "38400", "57600", "115200" };
	static const int baudPopupParams[] = { 9600, 19200, 38400, 57600, 115200 };
	baudPopup = CreateIntPopupBar(colours, fullPopupWidth, 5, baudPopupText, baudPopupParams, evAdjustBaudRate, evAdjustBaudRate);
}

// Create the volume adjustment popup
void CreateVolumePopup(const ColourScheme& colours)
{
	static_assert(Buzzer::MaxVolume == 5, "MaxVolume assumed to be 5 here");
	static const char* const volumePopupText[Buzzer::MaxVolume + 1] = { "0", "1", "2", "3", "4", "5" };
	volumePopup = CreateIntPopupBar(colours, fullPopupWidth, Buzzer::MaxVolume + 1, volumePopupText, nullptr, evAdjustVolume, evAdjustVolume);
}

// Create the colour scheme change popup
void CreateColoursPopup(const ColourScheme& colours)
{
	if (NumColourSchemes >= 2)
	{
		// Put all the colour scheme names in a single array for the call to CreateIntPopupBar
		const char* coloursPopupText[NumColourSchemes];
		for (size_t i = 0; i < NumColourSchemes; ++i)
		{
			coloursPopupText[i] = strings->colourSchemeNames[i];
		}
		coloursPopup = CreateIntPopupBar(colours, fullPopupWidth, NumColourSchemes, coloursPopupText, nullptr, evAdjustColours, evAdjustColours);
	}
	else
	{
		coloursPopup = nullptr;
	}
}

// Create the language popup (currently only affects the keyboard layout)
void CreateLanguagePopup(const ColourScheme& colours)
{
	languagePopup = new PopupWindow(popupBarHeight, fullPopupWidth, colours.popupBackColour, colours.popupBorderColour);
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	PixelNumber step = (fullPopupWidth - 2 * popupSideMargin + popupFieldSpacing)/numLanguages;
	for (unsigned int i = 0; i < numLanguages; ++i)
	{
		languagePopup->AddField(new TextButton(popupSideMargin, popupSideMargin + i * step, step - popupFieldSpacing, LanguageTables[i].languageName, evAdjustLanguage, i));
	}
}

// Create the pop-up keyboard
void CreateKeyboardPopup(uint32_t language, ColourScheme colours)
{
	static const char* array const keysEN[4] = { "1234567890-+", "QWERTYUIOP", "ASDFGHJKL:", "ZXCVBNM./" };
	static const char* array const keysDE[4] = { "1234567890-+", "QWERTZUIOP", "ASDFGHJKL:", "YXCVBNM./" };
	static const char* array const keysFR[4] = { "1234567890-+", "AZERTWUIOP", "QSDFGHJKLM", "YXCVBN.:/" };
	static const char* array const * const keyboards[numLanguages] = { keysEN, keysDE, keysFR /*, keysEN */ };		// Spain keyboard layout is same as English

	keyboardPopup = CreatePopupWindow(keyboardPopupHeight, keyboardPopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupInfoTextColour, colours.buttonImageBackColour, nullptr, keyboardTopMargin);

	// Add the text area in which the command is built
	DisplayField::SetDefaultColours(colours.popupInfoTextColour, colours.popupInfoBackColour);		// need a different background colour
	userCommandField = new TextField(keyboardTopMargin + labelRowAdjust, popupSideMargin, keyboardPopupWidth - 2 * popupSideMargin - closeButtonWidth - popupFieldSpacing, TextAlignment::Left, nullptr, "_");
	userCommandField->SetLabel(userCommandBuffers[currentUserCommandBuffer].c_str());	// set up to display the current user command
	keyboardPopup->AddField(userCommandField);

	if (language >= numLanguages)
	{
		language = 0;
	}
	const char* array const * array const keys = keyboards[language];
	PixelNumber row = keyboardTopMargin + keyButtonVStep;
	for (size_t i = 0; i < 4; ++i)
	{
		DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
		PixelNumber column = popupSideMargin + (i * keyButtonHStep)/3;
		const char * s = keys[i];
		while (*s != 0)
		{
			keyboardPopup->AddField(new CharButton(row, column, keyButtonWidth, *s, evKey));
			++s;
			column += keyButtonHStep;
		}
		DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.buttonImageBackColour);
		switch (i)
		{
		case 1:
			keyboardPopup->AddField(new IconButton(row, keyboardPopupWidth - popupSideMargin - 2 * keyButtonWidth, 2 * keyButtonWidth, IconBackspace, evBackspace));
			break;

		case 2:
			keyboardPopup->AddField(new IconButton(row, keyboardPopupWidth - popupSideMargin - (3 * keyButtonWidth)/2, (3 * keyButtonWidth)/2, IconUp, evUp));
			break;

		case 3:
			keyboardPopup->AddField(new IconButton(row, keyboardPopupWidth - popupSideMargin - (3 * keyButtonWidth)/2, (3 * keyButtonWidth)/2, IconDown, evDown));
			break;

		default:
			break;
		}
		row += keyButtonVStep;
	}

	// Add the space and enter keys
	const PixelNumber keyButtonHSpace = keyButtonHStep - keyButtonWidth;
	const PixelNumber wideKeyButtonWidth = (keyboardPopupWidth - 2 * popupSideMargin - 2 * keyButtonHSpace)/5;
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	keyboardPopup->AddField(new TextButton(row, popupSideMargin + wideKeyButtonWidth + keyButtonHSpace, 2 * wideKeyButtonWidth, nullptr, evKey, (int)' '));
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.buttonImageBackColour);
	keyboardPopup->AddField(new IconButton(row, popupSideMargin + 3 * wideKeyButtonWidth + 2 * keyButtonHSpace, wideKeyButtonWidth, IconEnter, evSendKeyboardCommand));
}

// Create the message popup window
void CreateMessagePopup(const ColourScheme& colours)
{
	alertPopup = CreatePopupWindow(alertPopupHeight, alertPopupWidth, colours.alertPopupBackColour, colours.popupBorderColour, colours.alertPopupTextColour, colours.buttonImageBackColour,
			strings->message);
	DisplayField::SetDefaultColours(colours.alertPopupTextColour, colours.alertPopupBackColour);
	alertPopup->AddField(new StaticTextField(popupTopMargin + 2 * rowTextHeight, popupSideMargin, alertPopupWidth - 2 * popupSideMargin, TextAlignment::Centre, alertText.c_str()));
}

// Create the babystep popup
void CreateBabystepPopup(const ColourScheme& colours)
{
	static const Icon babystepIcons[2] = {IconUp, IconDown };
	static const char * array const babystepCommands[2] = { "M290 S0.05", "M290 S-0.05" };
	babystepPopup = CreatePopupWindow(babystepPopupHeight, babystepPopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, colours.buttonImageBackColour,
			strings->babyStepping);
	PixelNumber ypos = popupTopMargin + babystepRowSpacing;
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	babystepPopup->AddField(babystepOffsetField = new FloatField(ypos, popupSideMargin, babystepPopupWidth - 2 * popupSideMargin, TextAlignment::Left, 3, strings->currentZoffset, "mm"));
	ypos += babystepRowSpacing;
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.buttonImageBackColour);
	CreateIconButtonRow(babystepPopup, ypos, popupSideMargin, babystepPopupWidth - 2 * popupSideMargin, fieldSpacing, 2, babystepIcons, babystepCommands, evBabyStepAmount);
}

// Create the grid of heater icons and temperatures
void CreateTemperatureGrid(const ColourScheme& colours)
{
	// Add the emergency stop button
	DisplayField::SetDefaultColours(colours.stopButtonTextColour, colours.stopButtonBackColour);
	mgr.AddField(new TextButton(row2, margin, bedColumn - fieldSpacing - margin - 16, strings->stop, evEmergencyStop));

	// Add the labels and the debug field
	DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
	mgr.AddField(debugField = new StaticTextField(row1 + labelRowAdjust, margin, bedColumn - fieldSpacing - margin, TextAlignment::Left, "debug"));
	mgr.AddField(new StaticTextField(row3 + labelRowAdjust, margin, bedColumn - fieldSpacing - margin, TextAlignment::Right, strings->current));
	mgr.AddField(new StaticTextField(row4 + labelRowAdjust, margin, bedColumn - fieldSpacing - margin, TextAlignment::Right, strings->active));
	mgr.AddField(new StaticTextField(row5 + labelRowAdjust, margin, bedColumn - fieldSpacing - margin, TextAlignment::Right, strings->standby));

	// Add the grid
	for (unsigned int i = 0; i < maxHeaters; ++i)
	{
		PixelNumber column = ((tempButtonWidth + fieldSpacing) * i) + bedColumn;

		// Add the icon button
		DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonImageBackColour);
		SingleButton *b = new IconButton(row2, column, tempButtonWidth, heaterIcons[i], evSelectHead, i);
		toolButtons[i] = b;
		mgr.AddField(b);

		// Add the current temperature field
		DisplayField::SetDefaultColours(colours.infoTextColour, colours.defaultBackColour);
		FloatField *f = new FloatField(row3 + labelRowAdjust, column, tempButtonWidth, TextAlignment::Centre, 1);
		f->SetValue(0.0);
		currentTemps[i] = f;
		mgr.AddField(f);

		// Add the active temperature button
		DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
		IntegerButton *ib = new IntegerButton(row4, column, tempButtonWidth);
		ib->SetEvent(evAdjustActiveTemp, i);
		ib->SetValue(0);
		activeTemps[i] = ib;
		mgr.AddField(ib);

		// Add the standby temperature button
		ib = new IntegerButton(row5, column, tempButtonWidth);
		ib->SetEvent(evAdjustStandbyTemp, i);
		ib->SetValue(0);
		standbyTemps[i] = ib;
		mgr.AddField(ib);
	}
}

// Create the extra fields for the Control tab
void CreateControlTabFields(const ColourScheme& colours)
{
	mgr.SetRoot(commonRoot);

	DisplayField::SetDefaultColours(colours.infoTextColour, colours.infoBackColour);
	PixelNumber column = margin;
	PixelNumber xyFieldWidth = (DISPLAY_X - (2 * margin) - (MAX_AXES * fieldSpacing))/(MAX_AXES + 1);
	for (size_t i = 0; i < MAX_AXES; ++i)
	{
		FloatField *f = new FloatField(row6p3 + labelRowAdjust, column, xyFieldWidth, TextAlignment::Left, (i == 2) ? 2 : 1, axisNames[i]);
		axisPos[i] = f;
		f->SetValue(0.0);
		mgr.AddField(f);
		f->Show(i < MIN_AXES);
		column += xyFieldWidth + fieldSpacing;
	}
	zprobeBuf[0] = 0;
	mgr.AddField(zProbe = new TextField(row6p3 + labelRowAdjust, column, DISPLAY_X - column - margin, TextAlignment::Left, "P", zprobeBuf.c_str()));

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.notHomedButtonBackColour);
	homeAllButton = AddIconButton(row7p7, 0, MAX_AXES + 2, IconHomeAll, evSendCommand, "G28");
	homeButtons[0] = AddIconButton(row7p7, 1, MAX_AXES + 2, IconHomeX, evSendCommand, "G28 X0");
	homeButtons[1] = AddIconButton(row7p7, 2, MAX_AXES + 2, IconHomeY, evSendCommand, "G28 Y0");
	homeButtons[2] = AddIconButton(row7p7, 3, MAX_AXES + 2, IconHomeZ, evSendCommand, "G28 Z0");
#if MAX_AXES > 3
	homeButtons[3] = AddIconButton(row7p7, 4, MAX_AXES + 2, IconHomeU, evSendCommand, "G28 U0");
	homeButtons[3]->Show(false);
#endif
#if MAX_AXES > 4
	homeButtons[4] = AddIconButton(row7p7, 5, MAX_AXES + 2, IconHomeV, evSendCommand, "G28 V0");
	homeButtons[4]->Show(false);
#endif
#if MAX_AXES > 5
	homeButtons[5] = AddIconButton(row7p7, 6, MAX_AXES + 2, IconHomeW, evSendCommand, "G28 W0");
	homeButtons[5]->Show(false);
#endif
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonImageBackColour);
	bedCompButton = AddIconButton(row7p7, MAX_AXES + 1, MAX_AXES + 2, IconBedComp, evSendCommand, "G32");

	filesButton = AddIconButton(row8p7, 0, 4, IconFiles, evListFiles, nullptr);
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	moveButton = AddTextButton(row8p7, 1, 4, strings->move, evMovePopup, nullptr);
	extrudeButton = AddTextButton(row8p7, 2, 4, strings->extrusion, evExtrudePopup, nullptr);
	macroButton = AddTextButton(row8p7, 3, 4, strings->macro, evListMacros, nullptr);

	controlRoot = mgr.GetRoot();
}

// Create the fields for the Printing tab
void CreatePrintingTabFields(const ColourScheme& colours)
{
	mgr.SetRoot(commonRoot);

	// Labels
	DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
	mgr.AddField(new StaticTextField(row6 + labelRowAdjust, margin, bedColumn - fieldSpacing, TextAlignment::Right, strings->extruderPercent));

	// Extrusion factor buttons
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	for (unsigned int i = 1; i < maxHeaters; ++i)
	{
		PixelNumber column = ((tempButtonWidth + fieldSpacing) * i) + bedColumn;

		IntegerButton *ib = new IntegerButton(row6, column, tempButtonWidth);
		ib->SetValue(100);
		ib->SetEvent(evExtrusionFactor, i);
		extrusionFactors[i - 1] = ib;
		mgr.AddField(ib);
	}

	// Speed button
	mgr.AddField(spd = new IntegerButton(row7, speedColumn, fanColumn - speedColumn - fieldSpacing, strings->speed, "%"));
	spd->SetValue(100);
	spd->SetEvent(evAdjustSpeed, "M220 S");

	// Fan button
	mgr.AddField(fanSpeed = new IntegerButton(row7, fanColumn, pauseColumn - fanColumn - fieldSpacing, strings->fan, "%"));
	fanSpeed->SetEvent(evAdjustFan, 0);
	fanSpeed->SetValue(0);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.pauseButtonBackColour);
	pauseButton = new TextButton(row7, pauseColumn, babystepColumn - pauseColumn - fieldSpacing, strings->pause, evPausePrint, "M25");
	mgr.AddField(pauseButton);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	babystepButton = new TextButton(row7, babystepColumn, DisplayX - babystepColumn - fieldSpacing, strings->babystep, evBabyStepPopup);
	mgr.AddField(babystepButton);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.resumeButtonBackColour);
	resumeButton = new TextButton(row7, resumeColumn, cancelColumn - resumeColumn - fieldSpacing, strings->resume, evResumePrint, "M24");
	mgr.AddField(resumeButton);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.resetButtonBackColour);
	resetButton = new TextButton(row7, cancelColumn, DisplayX - cancelColumn - margin, strings->cancel, evReset, "M0");
	mgr.AddField(resetButton);

	//		DisplayField::SetDefaultColours(labelTextColour, defaultBackColour);
	//		mgr.AddField(printingField = new TextField(row8, margin, DisplayX, TextAlignment::Left, "printing ", printingFile.c_str()));

	DisplayField::SetDefaultColours(colours.progressBarColour,colours. progressBarBackColour);
	mgr.AddField(printProgressBar = new ProgressBar(row8 + (rowHeight - progressBarHeight)/2, margin, progressBarHeight, DisplayX - 2 * margin));
	mgr.Show(printProgressBar, false);

	DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
	mgr.AddField(timeLeftField = new TextField(row9, margin, DisplayX - 2 * margin, TextAlignment::Left, strings->timeRemaining));
	mgr.Show(timeLeftField, false);

	printRoot = mgr.GetRoot();
}

// Create the fields for the Message tab
void CreateMessageTabFields(const ColourScheme& colours)
{
	mgr.SetRoot(baseRoot);
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonImageBackColour);
	mgr.AddField(new IconButton(margin,  DisplayX - margin - keyboardButtonWidth, keyboardButtonWidth, IconKeyboard, evKeyboard));
	DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
	mgr.AddField(new StaticTextField(margin + labelRowAdjust, margin, DisplayX - 2 * margin - keyboardButtonWidth, TextAlignment::Centre, strings->messages));
	PixelNumber row = firstMessageRow;
	for (unsigned int r = 0; r < numMessageRows; ++r)
	{
		StaticTextField *t = new StaticTextField(row, margin, messageTimeWidth, TextAlignment::Left, nullptr);
		mgr.AddField(t);
		messageTimeFields[r] = t;
		t = new StaticTextField(row, messageTextX, messageTextWidth, TextAlignment::Left, nullptr);
		mgr.AddField(t);
		messageTextFields[r] = t;
		row += rowTextHeight;
	}
	messageRoot = mgr.GetRoot();
}

// Create the fields for the Setup tab
void CreateSetupTabFields(uint32_t language, const ColourScheme& colours)
{
	mgr.SetRoot(baseRoot);
	DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
	// The firmware version field doubles up as an area for displaying debug messages, so make it the full width of the display
	mgr.AddField(fwVersionField = new TextField(row1, margin, DisplayX, TextAlignment::Left, strings->firmwareVersion, VERSION_TEXT));
	mgr.AddField(freeMem = new IntegerField(row2, margin, DisplayX/2 - margin, TextAlignment::Left, "Free RAM: "));
	mgr.AddField(touchX = new IntegerField(row2, DisplayX/2, DisplayX/4, TextAlignment::Left, "Touch: ", ","));
	mgr.AddField(touchY = new IntegerField(row2, (DisplayX * 3)/4, DisplayX/4, TextAlignment::Left));

	DisplayField::SetDefaultColours(colours.errorTextColour, colours.errorBackColour);
	mgr.AddField(settingsNotSavedField = new StaticTextField(row3, margin, DisplayX - 2 * margin, TextAlignment::Left, strings->settingsNotSavedText));
	settingsNotSavedField->Show(false);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	baudRateButton = AddIntegerButton(row4, 0, 3, nullptr, " baud", evSetBaudRate);
	baudRateButton->SetValue(GetBaudRate());
	volumeButton = AddIntegerButton(row4, 1, 3, strings->volume, nullptr, evSetVolume);
	volumeButton->SetValue(GetVolume());
	languageButton = AddTextButton(row4, 2, 3, LanguageTables[language].languageName, evSetLanguage, nullptr);
	AddTextButton(row5, 0, 3, strings->calibrateTouch, evCalTouch, nullptr);
	AddTextButton(row5, 1, 3, strings->mirrorDisplay, evInvertX, nullptr);
	AddTextButton(row5, 2, 3, strings->invertDisplay, evInvertY, nullptr);
	coloursButton = AddTextButton(row6, 0, 3, strings->colourSchemeNames[colours.index], evSetColours, nullptr);
	coloursButton->SetText(strings->colourSchemeNames[colours.index]);
	AddTextButton(row6, 1, 3, strings->brightnessDown, evDimmer, nullptr);
	AddTextButton(row6, 2, 3, strings->brightnessUp, evBrighter, nullptr);
	AddTextButton(row7, 0, 3, strings->saveSettings, evSaveSettings, nullptr);
	AddTextButton(row7, 1, 3, strings->clearSettings, evFactoryReset, nullptr);
	AddTextButton(row7, 2, 3, strings->saveAndRestart, evRestart, nullptr);
	setupRoot = mgr.GetRoot();
}

// Create the fields that are displayed on all pages
void CreateCommonFields(const ColourScheme& colours)
{
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour, colours.buttonBorderColour, colours.buttonGradColour,
									colours.buttonPressedBackColour, colours.buttonPressedGradColour);
	tabControl = AddTextButton(rowTabs, 0, 4, strings->control, evTabControl, nullptr);
	tabPrint = AddTextButton(rowTabs, 1, 4, strings->print, evTabPrint, nullptr);
	tabMsg = AddTextButton(rowTabs, 2, 4, strings->console, evTabMsg, nullptr);
	tabSetup = AddTextButton(rowTabs, 3, 4, strings->setup, evTabSetup, nullptr);
}

void CreateMainPages(uint32_t language, const ColourScheme& colours)
{
	if (language >= ARRAY_SIZE(LanguageTables))
	{
		language = 0;
	}
	strings = &LanguageTables[language];
	CreateCommonFields(colours);
	baseRoot = mgr.GetRoot();		// save the root of fields that we usually display

	// Create the fields that are common to the Control and Print pages
	DisplayField::SetDefaultColours(colours.titleBarTextColour, colours.titleBarBackColour);
	mgr.AddField(nameField = new StaticTextField(row1, 0, DisplayX - statusFieldWidth, TextAlignment::Centre, machineName.c_str()));
	mgr.AddField(statusField = new StaticTextField(row1, DisplayX - statusFieldWidth, statusFieldWidth, TextAlignment::Right, nullptr));
	CreateTemperatureGrid(colours);
	commonRoot = mgr.GetRoot();		// save the root of fields that we display on more than one page

	// Create the pages
	CreateControlTabFields(colours);
	CreatePrintingTabFields(colours);
	CreateMessageTabFields(colours);
	CreateSetupTabFields(language, colours);
}

namespace UI
{
	static void Adjusting(ButtonPress bp)
	{
		fieldBeingAdjusted = bp;
		if (bp == currentButton)
		{
			currentButton.Clear();		// to stop it being released
		}
	}

	static void StopAdjusting()
	{
		if (fieldBeingAdjusted.IsValid())
		{
			mgr.Press(fieldBeingAdjusted, false);
			fieldBeingAdjusted.Clear();
		}
	}

	static void CurrentButtonReleased()
	{
		if (currentButton.IsValid())
		{
			mgr.Press(currentButton, false);
			currentButton.Clear();
		}
	}

	// Return the number of supported languages
	extern unsigned int GetNumLanguages()
	{
		return numLanguages;
	}

	// Create all the fields we ever display
	void CreateFields(uint32_t language, const ColourScheme& colours)
	{
		// Set up default colours and margins
		mgr.Init(colours.defaultBackColour);
		DisplayField::SetDefaultFont(DEFAULT_FONT);
		ButtonWithText::SetFont(DEFAULT_FONT);
		SingleButton::SetTextMargin(textButtonMargin);
		SingleButton::SetIconMargin(iconButtonMargin);

		// Create the pages
		CreateMainPages(language, colours);

		// Create the popup fields
		CreateIntegerAdjustPopup(colours);
		CreateMovePopup(colours);
		CreateExtrudePopup(colours);
		CreateFileListPopup(colours);
		CreateFileActionPopup(colours);
		CreateVolumePopup(colours);
		CreateBaudRatePopup(colours);
		CreateColoursPopup(colours);
		CreateAreYouSurePopup(colours);
		CreateKeyboardPopup(language, colours);
		CreateLanguagePopup(colours);
		CreateMessagePopup(colours);
		CreateBabystepPopup(colours);

		DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
		touchCalibInstruction = new StaticTextField(DisplayY/2 - 10, 0, DisplayX, TextAlignment::Centre, strings->touchTheSpot);

		mgr.SetRoot(nullptr);
	}

	// Show or hide the field that warns about unsaved settings
	void CheckSettingsAreSaved()
	{
		if (IsSaveAndRestartNeeded())
		{
			settingsNotSavedField->SetValue(strings->restartNeededText);
			mgr.Show(settingsNotSavedField, true);
		}
		else if (IsSaveNeeded())
		{
			settingsNotSavedField->SetValue(strings->settingsNotSavedText);
			mgr.Show(settingsNotSavedField, true);
		}
		else
		{
			mgr.Show(settingsNotSavedField, false);
		}
	}

	void ShowFilesButton()
	{
		mgr.Show(resumeButton, false);
		mgr.Show(resetButton, false);
		mgr.Show(pauseButton, false);
		mgr.Show(babystepButton, false);
		mgr.Show(filesButton, true);
	}

	void ShowPauseButton()
	{
		mgr.Show(resumeButton, false);
		mgr.Show(resetButton, false);
		mgr.Show(filesButton, false);
		mgr.Show(pauseButton, true);
		mgr.Show(babystepButton, true);
	}

	void ShowResumeAndCancelButtons()
	{
		mgr.Show(pauseButton, false);
		mgr.Show(babystepButton, false);
		mgr.Show(filesButton, false);
		mgr.Show(resumeButton, true);
		mgr.Show(resetButton, true);
	}

	// Show or hide an axis on the move button grid and on the axis display
	void ShowAxis(size_t axis, bool b)
	{
		// The table gives us a pointer to the label field, which is followed by 8 buttons. So we need to show or hide 9 fields.
		DisplayField *f = moveAxisRows[axis];
		for (int i = 0; i < 9 && f != nullptr; ++i)
		{
			f->Show(b);
			f = f->next;
		}
		axisPos[axis]->Show(b);
	}

	void UpdateAxisPosition(size_t axis, float fval)
	{
		if (axis < MAX_AXES && axisPos[axis] != nullptr)
		{
			axisPos[axis]->SetValue(fval);
		}
	}

	void UpdateCurrentTemperature(size_t heater, float fval)
	{
		if (currentTemps[heater] != nullptr)
		{
			currentTemps[heater]->SetValue(fval);
		}
	}

	void ShowHeater(size_t heater, bool show)
	{
		mgr.Show(currentTemps[heater], show);
		mgr.Show(activeTemps[heater], show);
		mgr.Show(standbyTemps[heater], show);
		mgr.Show(extrusionFactors[heater - 1], show);
	}

	void UpdateHeaterStatus(size_t heater, int ival)
	{
		heaterStatus[heater] = ival;
		if (currentTemps[heater] != nullptr)
		{
			Colour c = (ival == 1) ? colours->standbyBackColour
						: (ival == 2) ? colours->activeBackColour
						: (ival == 3) ? colours->errorBackColour
						: (ival == 4) ? colours->tuningBackColour
						: colours->defaultBackColour;
			currentTemps[heater]->SetColours((ival == 3) ? colours->errorTextColour : colours->infoTextColour, c);
		}
	}

	static int timesLeft[3];
	static String<50> timesLeftText;

	void ChangeStatus(PrinterStatus oldStatus, PrinterStatus newStatus)
	{
		switch (newStatus)
		{
		case PrinterStatus::printing:
			if (oldStatus != PrinterStatus::paused && oldStatus != PrinterStatus::resuming)
			{
				// Starting a new print, so clear the times
				timesLeft[0] = timesLeft[1] = timesLeft[2] = 0;
			}
			// no break
		case PrinterStatus::paused:
		case PrinterStatus::pausing:
		case PrinterStatus::resuming:
			if (oldStatus == PrinterStatus::connecting || oldStatus == PrinterStatus::idle)
			{
				ChangePage(tabPrint);
			}
			else if (currentTab == tabPrint)
			{
				nameField->SetValue(printingFile.c_str());
			}
			break;

		case PrinterStatus::idle:
			printingFile.clear();
			nameField->SetValue(machineName.c_str());		// if we are on the print tab then it may still be set to the file that was being printed
			// no break
		case PrinterStatus::configuring:
			if (oldStatus == PrinterStatus::flashing)
			{
				mgr.ClearAllPopups();						// clear the firmware update message
			}
			break;

		case PrinterStatus::connecting:
			printingFile.clear();
			// We no longer clear the machine name here
			mgr.ClearAllPopups();
			break;

		default:
			nameField->SetValue(machineName.c_str());
			break;
		}
	}

	// Append an amount of time to timesLeftText
	static void AppendTimeLeft(int t)
	{
		if (t <= 0)
		{
			timesLeftText.catFrom(strings->notAvailable);
		}
		else if (t < 60)
		{
			timesLeftText.catf("%ds", t);
		}
		else if (t < 60 * 60)
		{
			timesLeftText.catf("%dm %02ds", t/60, t%60);
		}
		else
		{
			t /= 60;
			timesLeftText.catf("%dh %02dm", t/60, t%60);
		}
	}

	void UpdateTimesLeft(size_t index, unsigned int seconds)
	{
		if (index < (int)ARRAY_SIZE(timesLeft))
		{
			timesLeft[index] = seconds;
			timesLeftText.copy(strings->file);
			AppendTimeLeft(timesLeft[0]);
			timesLeftText.catFrom(strings->filament);
			AppendTimeLeft(timesLeft[1]);
			if (DisplayX >= 800)
			{
				timesLeftText.catFrom(strings->layer);
				AppendTimeLeft(timesLeft[2]);
			}
			timeLeftField->SetValue(timesLeftText.c_str());
			mgr.Show(timeLeftField, true);
		}
	}

	// Change to the page indicated. Return true if the page has a permanently-visible button.
	bool ChangePage(ButtonBase *newTab)
	{
		if (newTab != currentTab)
		{
			if (currentTab != nullptr)
			{
				currentTab->Press(false, 0);			// remove highlighting from the old tab
			}
			newTab->Press(true, 0);						// highlight the new tab
			currentTab = newTab;
			mgr.ClearAllPopups();
			switch(newTab->GetEvent())
			{
			case evTabControl:
				mgr.SetRoot(controlRoot);
				nameField->SetValue(machineName.c_str());
				break;
			case evTabPrint:
				mgr.SetRoot(printRoot);
				nameField->SetValue(PrintInProgress() ? printingFile.c_str() : machineName.c_str());
				break;
			case evTabMsg:
				mgr.SetRoot(messageRoot);
				if (keyboardIsDisplayed)
				{
					mgr.SetPopup(keyboardPopup, AutoPlace, keyboardPopupY, false);
				}
				break;
			case evTabSetup:
				mgr.SetRoot(setupRoot);
				break;
			default:
				mgr.SetRoot(commonRoot);
				break;
			}
			mgr.Refresh(true);
		}
		return true;
	}

	// Pop up the keyboard
	void ShowKeyboard()
	{
		mgr.SetPopup(keyboardPopup, AutoPlace, keyboardPopupY);
		keyboardIsDisplayed = true;
	}

	// This is called when the Cancel button on a popup is pressed
	void PopupCancelled()
	{
		if (mgr.GetPopup() == keyboardPopup)
		{
			keyboardIsDisplayed = false;
		}
	}

	// Return true if polling should be performed
	bool DoPolling()
	{
		return currentTab != tabSetup;			// don't poll while we are on the Setup page
	}

	// This is called in the main spin loop
	void Spin()
	{
		if (currentTab == tabMsg)
		{
			MessageLog::UpdateMessages(false);
		}
	}

	// This is called when we have just started a file print
	void PrintStarted()
	{
		ChangePage(tabPrint);
	}

	// This is called when we have just received the name of the file being printed
	void PrintingFilenameChanged(const char data[])
	{
		if (!printingFile.similar(data))
		{
			printingFile.copy(data);
			if (currentTab == tabPrint && PrintInProgress())
			{
				nameField->SetChanged();
			}
		}
	}

	// This is called just before the main polling loop starts. Display the default page.
	void ShowDefaultPage()
	{
		ChangePage(tabControl);
	}

	// Update the fields that are to do with the printing status
	void UpdatePrintingFields()
	{
		if (GetStatus() == PrinterStatus::printing)
		{
			ShowPauseButton();
		}
		else if (GetStatus() == PrinterStatus::paused)
		{
			ShowResumeAndCancelButtons();
		}
		else
		{
			ShowFilesButton();
		}

		mgr.Show(printProgressBar, PrintInProgress());
	//	mgr.Show(printingField, PrintInProgress());

		// Don't enable the time left field when we start printing, instead this will get enabled when we receive a suitable message
		if (!PrintInProgress())
		{
			mgr.Show(timeLeftField, false);
		}

		statusField->SetValue(strings->statusValues[(unsigned int)GetStatus()]);
	}

	// Set the percentage of print completed
	void SetPrintProgressPercent(unsigned int percent)
	{
		printProgressBar->SetPercent((uint8_t)percent);
	}

	// Update the geometry or the number of axes
	void UpdateGeometry(unsigned int numAxes, bool isDelta)
	{
		for (size_t i = 0; i < MAX_AXES; ++i)
		{
			mgr.Show(homeButtons[i], !isDelta && i < numAxes);
			ShowAxis(i, i < numAxes);
		}
	}

	// Update the homed status of the specified axis. If the axis is -1 then it represents the "all homed" status.
	void UpdateHomedStatus(int axis, bool isHomed)
	{
		SingleButton *homeButton = nullptr;
		if (axis < 0)
		{
			homeButton = homeAllButton;
		}
		else if (axis < MAX_AXES)
		{
			homeButton = homeButtons[axis];
		}
		if (homeButton != nullptr)
		{
			homeButton->SetColours(colours->buttonTextColour, (isHomed) ? colours->homedButtonBackColour : colours->notHomedButtonBackColour);
		}
	}

	// UIpdate the Z probe text
	void UpdateZProbe(const char data[])
	{
		zprobeBuf.copy(data);
		zProbe->SetChanged();
	}

	// Update the machine name
	void UpdateMachineName(const char data[])
	{
		machineName.copy(data);
		nameField->SetChanged();
	}

	// Update the fan RPM
	void UpdateFanPercent(int rpm)
	{
		UpdateField(fanSpeed, rpm);
	}

	// Update an active temperature
	void UpdateActiveTemperature(size_t index, int ival)
	{
		UpdateField(activeTemps[index], ival);
	}

	// Update a standby temperature
	void UpdateStandbyTemperature(size_t index, int ival)
	{
		UpdateField(standbyTemps[index], ival);
	}

	// Update an extrusion factor
	void UpdateExtrusionFactor(size_t index, int ival)
	{
		UpdateField(extrusionFactors[index], ival);
	}

	// Update the print speed factor
	void UpdateSpeedPercent(int ival)
	{
		UpdateField(spd, ival);
	}

	// Process an alert message. If the data is empty then we should clear any existing alert.
	void ProcessAlert(const char data[])
	{
		if (data[0] == 0)
		{
			mgr.ClearPopup(true, alertPopup);
		}
		else
		{
			alertText.copy(data);
			mgr.SetPopup(alertPopup, AutoPlace, AutoPlace);
		}
	}

	// This is called when the user selects a new file from a list of SD card files
	void FileSelected(const char * array null fileName)
	{
		fpNameField->SetValue(fileName);
		// Clear out the old field values, they relate to the previous file we looked at until we process the response
		fpSizeField->SetValue(0);						// would be better to make it blank
		fpHeightField->SetValue(0.0);					// would be better to make it blank
		fpLayerHeightField->SetValue(0.0);				// would be better to make it blank
		fpFilamentField->SetValue(0);					// would be better to make it blank
		generatedByText.clear();
		fpGeneratedByField->SetChanged();
	}

	// This is called when the "generated by" file information has been received
	void UpdateFileGeneratedByText(const char data[])
	{
		generatedByText.copy(data);
		fpGeneratedByField->SetChanged();
	}

	// This is called when the object height information for the file has been received
	void UpdateFileObjectHeight(float f)
	{
		fpHeightField->SetValue(f);
	}

	// This is called when the layer height information for the file has been received
	void UpdateFileLayerHeight(float f)
	{
		fpLayerHeightField->SetValue(f);
	}

	// This is called when the size of the file has been received
	void UpdateFileSize(int size)
	{
		fpSizeField->SetValue(size);
	}

	// This is called when the filament needed by the file has been received
	void UpdateFileFilament(int len)
	{
		fpFilamentField->SetValue(len);
	}

	// Return true if we are displaying file information
	bool IsDisplayingFileInfo()
	{
		return currentFile != nullptr;
	}

	// This is called when the host firmware changes
	void FirmwareFeaturesChanged(FirmwareFeatures newFeatures)
	{
		// Some firmwares don't support tool standby temperatures
		for (size_t i = 0; i < maxHeaters; ++i)
		{
			mgr.Show(standbyTemps[i], (newFeatures & noStandbyTemps) == 0);
		}
	}

	// Process a touch event
	void ProcessTouch(ButtonPress bp)
	{
		if (bp.IsValid())
		{
			ButtonBase *f = bp.GetButton();
			currentButton = bp;
			mgr.Press(bp, true);
			Event ev = (Event)(f->GetEvent());
			switch(ev)
			{
			case evEmergencyStop:
				{
					SerialIo::SendString("M112\n");
					Delay(1000);
					SerialIo::SendString("M999\n");
					Reconnect();
				}
				break;

			case evTabControl:
			case evTabPrint:
			case evTabMsg:
			case evTabSetup:
				if (ChangePage(f))
				{
					currentButton.Clear();						// keep the button highlighted after it is released
				}
				break;

			case evAdjustActiveTemp:
			case evAdjustStandbyTemp:
				if (static_cast<IntegerButton*>(f)->GetValue() < 0)
				{
					static_cast<IntegerButton*>(f)->SetValue(0);
				}
				Adjusting(bp);
				mgr.SetPopup(setTempPopup, AutoPlace, popupY);
				break;

			case evAdjustSpeed:
			case evExtrusionFactor:
			case evAdjustFan:
				oldIntValue = static_cast<IntegerButton*>(bp.GetButton())->GetValue();
				Adjusting(bp);
				mgr.SetPopup(setTempPopup, AutoPlace, popupY);
				break;

			case evSetInt:
				if (fieldBeingAdjusted.IsValid())
				{
					int val = static_cast<const IntegerButton*>(fieldBeingAdjusted.GetButton())->GetValue();
					switch(fieldBeingAdjusted.GetEvent())
					{
					case evAdjustActiveTemp:
						{
							int heater = fieldBeingAdjusted.GetIParam();
							if (heater == 0)
							{
								SerialIo::SendString("M140 S");
								SerialIo::SendInt(val);
								SerialIo::SendChar('\n');
							}
							else
							{
								SerialIo::SendString(((GetFirmwareFeatures() & noG10Temps) == 0) ? "G10 P" : "M104 T");
								SerialIo::SendInt(heater - 1);
								SerialIo::SendString(" S");
								SerialIo::SendInt(val);
								SerialIo::SendChar('\n');
							}
						}
						break;

					case evAdjustStandbyTemp:
						{
							int heater = fieldBeingAdjusted.GetIParam();
							if (heater > 0)
							{
								SerialIo::SendString("G10 P");
								SerialIo::SendInt(heater - 1);
								SerialIo::SendString(" R");
								SerialIo::SendInt(val);
								SerialIo::SendChar('\n');
							}
						}
						break;

					case evExtrusionFactor:
						{
							int heater = fieldBeingAdjusted.GetIParam();
							SerialIo::SendString("M221 P");
							SerialIo::SendInt(heater);
							SerialIo::SendString(" S");
							SerialIo::SendInt(val);
							SerialIo::SendChar('\n');
						}
						break;

					case evAdjustFan:
						SerialIo::SendString("M106 S");
						SerialIo::SendInt((256 * val)/100);
						SerialIo::SendChar('\n');
						break;

					default:
						{
							const char* null cmd = fieldBeingAdjusted.GetSParam();
							if (cmd != nullptr)
							{
								SerialIo::SendString(cmd);
								SerialIo::SendInt(val);
								SerialIo::SendChar('\n');
							}
						}
						break;
					}
					mgr.ClearPopup();
					StopAdjusting();
				}
				break;

			case evAdjustInt:
				if (fieldBeingAdjusted.IsValid())
				{
					IntegerButton *ib = static_cast<IntegerButton*>(fieldBeingAdjusted.GetButton());
					int newValue = ib->GetValue() + bp.GetIParam();
					switch(fieldBeingAdjusted.GetEvent())
					{
					case evAdjustActiveTemp:
					case evAdjustStandbyTemp:
						newValue = constrain<int>(newValue, 0, 300);
						break;

					case evAdjustFan:
						newValue = constrain<int>(newValue, 0, 100);
						break;

					default:
						break;
					}
					ib->SetValue(newValue);
					ShortenTouchDelay();
				}
				break;

			case evMovePopup:
				mgr.SetPopup(movePopup, AutoPlace, AutoPlace);
				break;

			case evMoveX:
			case evMoveY:
			case evMoveZ:
			case evMoveU:
			case evMoveV:
			case evMoveW:
				{
					const uint8_t axis = ev - evMoveX;
					const char c = (axis < 3) ? 'X' + axis : ('U' - 3) + axis;
					SerialIo::SendString("G91\nG1 ");
					SerialIo::SendChar(c);
					SerialIo::SendString(bp.GetSParam());
					SerialIo::SendString(" F6000\nG90\n");
				}
				break;

			case evExtrudePopup:
				mgr.SetPopup(extrudePopup, AutoPlace, AutoPlace);
				break;

			case evExtrudeAmount:
				mgr.Press(currentExtrudeAmountPress, false);
				mgr.Press(bp, true);
				currentExtrudeAmountPress = bp;
				currentButton.Clear();						// stop it being released by the timer
				break;

			case evExtrudeRate:
				mgr.Press(currentExtrudeRatePress, false);
				mgr.Press(bp, true);
				currentExtrudeRatePress = bp;
				currentButton.Clear();						// stop it being released by the timer
				break;

			case evExtrude:
			case evRetract:
				if (currentExtrudeAmountPress.IsValid() && currentExtrudeRatePress.IsValid())
				{
					SerialIo::SendString("G92 E0\nG1 E");
					if (ev == evRetract)
					{
						SerialIo::SendChar('-');
					}
					SerialIo::SendString(currentExtrudeAmountPress.GetSParam());
					SerialIo::SendString(" F");
					SerialIo::SendString(currentExtrudeRatePress.GetSParam());
					SerialIo::SendChar('\n');
				}
				break;

			case evBabyStepPopup:
				mgr.SetPopup(babystepPopup, AutoPlace, AutoPlace);
				break;

			case evBabyStepAmount:
				SerialIo::SendString("M290 ");
				SerialIo::SendString(bp.GetSParam());
				SerialIo::SendChar('\n');
				break;

			case evListFiles:
				FileManager::DisplayFilesList();
				break;

			case evListMacros:
				FileManager::DisplayMacrosList();
				break;

			case evCalTouch:
				CalibrateTouch();
				CheckSettingsAreSaved();
				break;

			case evFactoryReset:
				PopupAreYouSure(ev, strings->confirmFactoryReset);
				break;

			case evRestart:
				PopupAreYouSure(ev, strings->confirmRestart);
				break;

			case evSaveSettings:
				SaveSettings();
				if (restartNeeded)
				{
					PopupRestart();
				}
				break;

			case evSelectHead:
				{
					int head = bp.GetIParam();
					if (head == 0)
					{
						if (heaterStatus[0] == 2)			// if bed is active
						{
							SerialIo::SendString("M144\n");
						}
						else
						{
							SerialIo::SendString("M140 S");
							SerialIo::SendInt(activeTemps[0]->GetValue());
							SerialIo::SendChar('\n');
						}
					}
					else if (head < (int)maxHeaters)
					{
						if (heaterStatus[head] == 2)		// if head is active
						{
							SerialIo::SendString("T-1\n");
						}
						else
						{
							SerialIo::SendChar('T');
							SerialIo::SendInt(head - 1);
							SerialIo::SendChar('\n');
						}
					}
				}
				break;

			case evFile:
				{
					const char * array fileName = bp.GetSParam();
					if (fileName != nullptr)
					{
						if (fileName[0] == '*')
						{
							// It's a directory
							FileManager::RequestFilesSubdir(fileName + 1);
							//??? need to pop up a "wait" box here
						}
						else
						{
							// It's a regular file
							currentFile = fileName;
							SerialIo::SendString(((GetFirmwareFeatures() & noM20M36) == 0) ? "M36 " : "M408 S36 P");			// ask for the file info
							SerialIo::SendFilename(CondStripDrive(FileManager::GetFilesDir()), currentFile);
							SerialIo::SendChar('\n');
							FileSelected(currentFile);
							mgr.SetPopup(filePopup, AutoPlace, AutoPlace);
						}
					}
					else
					{
						ErrorBeep();
					}
				}
				break;

			case evFilesUp:
				FileManager::RequestFilesParentDir();
				break;

			case evMacrosUp:
				FileManager::RequestMacrosParentDir();
				break;

			case evMacro:
				{
					const char *fileName = bp.GetSParam();
					if (fileName != nullptr)
					{
						if (fileName[0] == '*')		// if it's a directory
						{
							FileManager::RequestMacrosSubdir(fileName + 1);
							//??? need to pop up a "wait" box here
						}
						else
						{
							SerialIo::SendString("M98 P");
							SerialIo::SendFilename(CondStripDrive(FileManager::GetMacrosDir()), fileName);
							SerialIo::SendChar('\n');
						}
					}
					else
					{
						ErrorBeep();
					}
				}
				break;

			case evPrint:
				mgr.ClearPopup();			// clear the file info popup
				mgr.ClearPopup();			// clear the file list popup
				if (currentFile != nullptr)
				{
					SerialIo::SendString("M32 ");
					SerialIo::SendFilename(CondStripDrive(StripPrefix(FileManager::GetFilesDir())), currentFile);
					SerialIo::SendChar('\n');
					PrintingFilenameChanged(currentFile);
					currentFile = nullptr;							// allow the file list to be updated
					CurrentButtonReleased();
					PrintStarted();
				}
				break;

			case evCancel:
				eventToConfirm = evNull;
				currentFile = nullptr;
				CurrentButtonReleased();
				PopupCancelled();
				mgr.ClearPopup();
				break;

			case evDeleteFile:
				CurrentButtonReleased();;
				PopupAreYouSure(ev, strings->confirmFileDelete);
				break;

			case evSendCommand:
			case evPausePrint:
			case evResumePrint:
			case evReset:
				SerialIo::SendString(bp.GetSParam());
				SerialIo::SendChar('\n');
				break;

			case evScrollFiles:
				FileManager::Scroll(bp.GetIParam() * GetNumScrolledFiles());
				ShortenTouchDelay();
				break;

			case evChangeCard:
				(void)FileManager::NextCard();
				break;

			case evKeyboard:
				ShowKeyboard();
				break;

			case evInvertX:
				MirrorDisplay();
				CalibrateTouch();
				CheckSettingsAreSaved();
				break;

			case evInvertY:
				InvertDisplay();
				CalibrateTouch();
				CheckSettingsAreSaved();
				break;

			case evSetBaudRate:
				Adjusting(bp);
				mgr.SetPopup(baudPopup, AutoPlace, popupY);
				break;

			case evAdjustBaudRate:
				{
					const int rate = bp.GetIParam();
					SetBaudRate(rate);
					baudRateButton->SetValue(rate);
				}
				CheckSettingsAreSaved();
				CurrentButtonReleased();
				mgr.ClearPopup();
				StopAdjusting();
				break;

			case evSetVolume:
				Adjusting(bp);
				mgr.SetPopup(volumePopup, AutoPlace, popupY);
				break;

			case evSetColours:
				if (coloursPopup != nullptr)
				{
					Adjusting(bp);
					mgr.SetPopup(coloursPopup, AutoPlace, popupY);
				}
				break;

			case evBrighter:
			case evDimmer:
				ChangeBrightness(ev == evBrighter);
				CheckSettingsAreSaved();
				ShortenTouchDelay();
				break;

			case evAdjustVolume:
				{
					const int newVolume = bp.GetIParam();
					SetVolume(newVolume);
					volumeButton->SetValue(newVolume);
				}
				TouchBeep();									// give audible feedback of the touch at the new volume level
				CheckSettingsAreSaved();
				break;

			case evAdjustColours:
				{
					const int newColours = bp.GetIParam();
					SetColourScheme(newColours);
					coloursButton->SetText(strings->colourSchemeNames[newColours]);
				}
				CheckSettingsAreSaved();
				break;

			case evSetLanguage:
				Adjusting(bp);
				mgr.SetPopup(languagePopup, AutoPlace, popupY);
				break;

			case evAdjustLanguage:
				{
					const int newLanguage = bp.GetIParam();
					SetLanguage(newLanguage);
					languageButton->SetText(LanguageTables[newLanguage].languageName);
				}
				CheckSettingsAreSaved();						// not sure we need this because we are going to reset anyway
				break;

			case evYes:
				CurrentButtonReleased();
				mgr.ClearPopup();								// clear the yes/no popup
				switch (eventToConfirm)
				{
				case evFactoryReset:
					FactoryReset();
					break;

				case evDeleteFile:
					if (currentFile != nullptr)
					{
						mgr.ClearPopup();						// clear the file info popup
						SerialIo::SendString("M30 ");
						SerialIo::SendFilename(CondStripDrive(StripPrefix(FileManager::GetFilesDir())), currentFile);
						SerialIo::SendChar('\n');
						FileManager::RefreshFilesList();
						currentFile = nullptr;
					}
					break;

				case evRestart:
					if (IsSaveNeeded())
					{
						SaveSettings();
					}
					Restart();
					break;

				default:
					break;
				}
				eventToConfirm = evNull;
				currentFile = nullptr;
				break;

			case evKey:
				if (!userCommandBuffers[currentUserCommandBuffer].full())
				{
					userCommandBuffers[currentUserCommandBuffer].add((char)bp.GetIParam());
					userCommandField->SetChanged();
				}
				break;

			case evBackspace:
				if (!userCommandBuffers[currentUserCommandBuffer].isEmpty())
				{
					userCommandBuffers[currentUserCommandBuffer].erase(userCommandBuffers[currentUserCommandBuffer].size() - 1);
					userCommandField->SetChanged();
					ShortenTouchDelay();
				}
				break;

			case evUp:
				currentHistoryBuffer = (currentHistoryBuffer + numUserCommandBuffers - 1) % numUserCommandBuffers;
				if (currentHistoryBuffer == currentUserCommandBuffer)
				{
					userCommandBuffers[currentUserCommandBuffer].clear();
				}
				else
				{
					userCommandBuffers[currentUserCommandBuffer].copy(userCommandBuffers[currentHistoryBuffer]);
				}
				userCommandField->SetChanged();
				break;

			case evDown:
				currentHistoryBuffer = (currentHistoryBuffer + 1) % numUserCommandBuffers;
				if (currentHistoryBuffer == currentUserCommandBuffer)
				{
					userCommandBuffers[currentUserCommandBuffer].clear();
				}
				else
				{
					userCommandBuffers[currentUserCommandBuffer].copy(userCommandBuffers[currentHistoryBuffer]);
				}
				userCommandField->SetChanged();
				break;

			case evSendKeyboardCommand:
				if (userCommandBuffers[currentUserCommandBuffer].size() != 0)
				{
					SerialIo::SendString(userCommandBuffers[currentUserCommandBuffer].c_str());
					SerialIo::SendChar('\n');

					// Add the command to the history if it was different frmo the previous command
					size_t prevBuffer = (currentUserCommandBuffer + numUserCommandBuffers - 1) % numUserCommandBuffers;
					if (strcmp(userCommandBuffers[currentUserCommandBuffer].c_str(), userCommandBuffers[prevBuffer].c_str()) != 0)
					{
						currentUserCommandBuffer = (currentUserCommandBuffer + 1) % numUserCommandBuffers;
					}
					currentHistoryBuffer = currentUserCommandBuffer;
					userCommandBuffers[currentUserCommandBuffer].clear();
					userCommandField->SetLabel(userCommandBuffers[currentUserCommandBuffer].c_str());
				}
				break;

			default:
				break;
			}
		}
	}

	// Process a touch event outside the popup on the field being adjusted
	void ProcessTouchOutsidePopup(ButtonPress bp)
	{
		if (bp == fieldBeingAdjusted)
		{
			DelayTouchLong();	// by default, ignore further touches for a long time
			TouchBeep();
			switch(fieldBeingAdjusted.GetEvent())
			{
			case evAdjustSpeed:
			case evExtrusionFactor:
			case evAdjustFan:
				static_cast<IntegerButton*>(fieldBeingAdjusted.GetButton())->SetValue(oldIntValue);
				mgr.ClearPopup();
				StopAdjusting();
				break;

			case evAdjustActiveTemp:
			case evAdjustStandbyTemp:
			case evSetBaudRate:
			case evSetVolume:
			case evSetColours:
				mgr.ClearPopup();
				StopAdjusting();
				break;

			case evSetLanguage:
				mgr.ClearPopup();
				StopAdjusting();
				if (IsSaveAndRestartNeeded())
				{
					restartNeeded = true;
					PopupRestart();
				}
				break;
			}
		}
		else
		{
			switch(bp.GetEvent())
			{
			case evTabControl:
			case evTabPrint:
			case evTabMsg:
			case evTabSetup:
				StopAdjusting();
				DelayTouchLong();	// by default, ignore further touches for a long time
				TouchBeep();
				{
					ButtonBase *btn = bp.GetButton();
					if (ChangePage(btn))
					{
						currentButton.Clear();						// keep the button highlighted after it is released
					}
				}
				break;

			case evSetBaudRate:
			case evSetVolume:
			case evSetColours:
			case evSetLanguage:
			case evCalTouch:
			case evInvertX:
			case evInvertY:
			case evSaveSettings:
			case evFactoryReset:
			case evRestart:
				// On the Setup tab, we allow any other button to be pressed to exit the current popup
				StopAdjusting();
				DelayTouchLong();	// by default, ignore further touches for a long time
				TouchBeep();
				mgr.ClearPopup();
				ProcessTouch(bp);
				break;

			default:
				break;
			}
		}
	}

	// This is called when a button press times out
	void OnButtonPressTimeout()
	{
		if (currentButton.IsValid())
		{
			CurrentButtonReleased();
		}
	}

	void UpdateFilesListTitle(int cardNumber, unsigned int numVolumes, bool isFilesList)
	{
		filePopupTitleField->SetValue(cardNumber);
		filePopupTitleField->Show(isFilesList);
		macroPopupTitleField->Show(!isFilesList);
		changeCardButton->Show(isFilesList && numVolumes > 1);
		filesUpButton->SetEvent((isFilesList) ? evFilesUp : evMacrosUp, nullptr);
		mgr.SetPopup(fileListPopup, AutoPlace, AutoPlace);
	}

	void FileListLoaded(int errCode)
	{
		if (errCode == 0)
		{
			mgr.Show(fileListErrorField, false);
		}
		else
		{
			fileListErrorField->SetValue(errCode);
			mgr.Show(fileListErrorField, true);
		}
	}

	void EnableFileNavButtons(bool scrollEarlier, bool scrollLater, bool parentDir)
	{
		mgr.Show(scrollFilesLeftButton, scrollEarlier);
		mgr.Show(scrollFilesRightButton, scrollLater);
		mgr.Show(filesUpButton, parentDir);
	}

	unsigned int GetNumScrolledFiles()
	{
		return numFileRows;
	}

	void SetNumTools(unsigned int n)
	{
		// Tool button 0 is the bed, hence we use <= instead of < in the following
		for (size_t i = 1; i < maxHeaters; ++i)
		{
			mgr.Show(toolButtons[i], i <= n);
		}
	}

	void SetBabystepOffset(float f)
	{
		babystepOffsetField->SetValue(f);
	}
}

#endif

// End
