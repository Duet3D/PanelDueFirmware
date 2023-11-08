/*
 * UserInterface.cpp
 *
 *  Created on: 7 Jan 2017
 *      Author: David
 */

#include <UI/UserInterface.hpp>

#include <ctype.h>

#include "Configuration.hpp"
#include "FileManager.hpp"
#include "FlashData.hpp"

#include "Hardware/Buzzer.hpp"
#include "Hardware/Reset.hpp"
#include "Hardware/SerialIo.hpp"
#include "Hardware/SysTick.hpp"

#include "Icons/Icons.hpp"
#include "Library/Misc.hpp"
#include "ObjectModel/BedOrChamber.hpp"
#include "ObjectModel/PrinterStatus.hpp"
#include "PanelDue.hpp"
#include "Version.hpp"

#include <General/SafeVsnprintf.h>
#include <General/SimpleMath.h>
#include <General/String.h>
#include <General/StringFunctions.h>

#include <ObjectModel/Axis.hpp>
#include <ObjectModel/Utils.hpp>

#include <UI/MessageLog.hpp>
#include <UI/Popup.hpp>
#include <UI/UserInterfaceConstants.hpp>

MainWindow mgr;

#define DEBUG 0
#include "Debug.hpp"

// Public fields
TextField *fwVersionField, *userCommandField, *ipAddressField;
IntegerField *freeMem;
StaticTextField *touchCalibInstruction, *debugField;
StaticTextField *messageTextFields[numMessageRows], *messageTimeFields[numMessageRows];

static const ColourScheme *colours;

// Private fields
static const size_t machineNameLength = 30;
static const size_t printingFileLength = 40;
static const size_t zprobeBufLength = 12;
static const size_t generatedByTextLength = 50;
static const size_t lastModifiedTextLength = 20;
static const size_t printTimeTextLength = 12;		// e.g. 11h 55m
static const size_t controlPageMacroTextLength = 50;
static const size_t ipAddressLength = 45;	// IPv4 needs max 15 but IPv6 can go up to 45

static String<ipAddressLength> ipAddress;

struct FileListButtons
{
	SingleButton *scrollLeftButton, *scrollRightButton, *folderUpButton;
	IntegerField *errorField;
};

static StaticTextField *fileListPopupNoFiles;

static FileListButtons filesListButtons, macrosListButtons;
static SingleButton *changeCardButton;

static TextButton *filenameButtons[NumDisplayedFiles];
static TextButton *macroButtons[NumDisplayedMacros];
static TextButton *controlPageMacroButtons[NumControlPageMacroButtons];
static String<controlPageMacroTextLength> controlPageMacroText[NumControlPageMacroButtons];

static PopupWindow *setTempPopup, *setRPMPopup, *movePopup, *extrudePopup, *fileListPopup, *macrosPopup, *fileDetailPopup, *baudPopup,
		*volumePopup, *infoTimeoutPopup, *screensaverTimeoutPopup, *babystepAmountPopup, *feedrateAmountPopup, *areYouSurePopup, *keyboardPopup, *languagePopup, *coloursPopup, *screensaverPopup;
static StaticTextField *areYouSureTextField, *areYouSureQueryField;
static DisplayField *emptyRoot, *baseRoot, *commonRoot, *controlRoot, *printRoot, *messageRoot, *setupRoot;
static SingleButton *homeAllButton, *bedCompButton;
static IconButtonWithText *homeButtons[MaxDisplayableAxes], *toolButtons[MaxSlots];
static FloatField *controlTabAxisPos[MaxDisplayableAxes];
#if DISPLAY_X == 800
static FloatField *printTabAxisPos[MaxDisplayableAxes];
#endif
static FloatField *movePopupAxisPos[MaxDisplayableAxes];
static FloatField *currentTemps[MaxSlots];
static FloatField *fpHeightField, *fpLayerHeightField, *babystepOffsetField;
static TextButtonWithLabel *babystepMinusButton, *babystepPlusButton;
static IntegerField *fpSizeField, *fpFilamentField, *filePopupTitleField;
static ProgressBar *printProgressBar;
static SingleButton *tabControl, *tabStatus, *tabMsg, *tabSetup;
static ButtonBase *filesButton, *pauseButton, *resumeButton, *cancelButton, *babystepButton, *reprintButton;
static TextField *timeLeftField, *zProbe;
static TextField *fpNameField, *fpGeneratedByField, *fpLastModifiedField, *fpPrintTimeField;
DrawDirect *fpThumbnail;
static StaticTextField *moveAxisRows[MaxDisplayableAxes];
static StaticTextField *nameField, *statusField;
static StaticTextField *screensaverText;
static IntegerButton *activeTemps[MaxSlots], *standbyTemps[MaxSlots];
static IntegerButton *spd, *extrusionFactors[MaxSlots], *fanSpeed, *baudRateButton, *volumeButton, *infoTimeoutButton, *screensaverTimeoutButton, *feedrateAmountButton;
static TextButton *languageButton, *coloursButton, *dimmingTypeButton, *heaterCombiningButton, *logLevelButton;
static TextButtonWithLabel *babystepAmountButton;
static SingleButton *moveButton, *extrudeButton, *macroButton;
static PopupWindow *babystepPopup;
static AlertPopup *alertPopup;
static CharButtonRow *keyboardRows[4];
static const char* _ecv_array const * _ecv_array currentKeyboard;
static void (*keyboardDataHandler)(const char *data) = nullptr;

static ButtonBase * null currentTab = nullptr;

static ButtonPress currentButton;
static ButtonPress fieldBeingAdjusted;
static ButtonPress currentExtrudeRatePress, currentExtrudeAmountPress;

static String<machineNameLength> machineName;
static String<printingFileLength> printingFile;
static bool lastJobFileNameAvailable = false;
static String<zprobeBufLength> zprobeBuf;
static String<generatedByTextLength> generatedByText;
static String<lastModifiedTextLength> lastModifiedText;
static String<printTimeTextLength> printTimeText;

const size_t maxUserCommandLength = 40;					// max length of a user gcode command
const size_t numUserCommandBuffers = 6;					// number of command history buffers plus one

static String<maxUserCommandLength> userCommandBuffers[numUserCommandBuffers];
static size_t currentUserCommandBuffer = 0, currentHistoryBuffer = 0;

static unsigned int numToolColsUsed = 0;
static unsigned int numHeaterAndToolColumns = 0;
static int oldIntValue;
static Event eventToConfirm = evNull;
static uint8_t numVisibleAxes = 0;						// initialise to 0 so we refresh the macros list when we receive the number of axes
static uint8_t numDisplayedAxes = 0;
static bool isDelta = false;

const char* _ecv_array null currentFile = nullptr;			// file whose info is displayed in the file info popup
const StringTable * strings = &LanguageTables[0];
static bool keyboardIsDisplayed = false;
static bool keyboardShifted = false;

int32_t alertMode = -1;									// the mode of the current alert, or -1 if no alert displayed
uint32_t alertTicks = 0;
uint32_t infoTimeout = DefaultInfoTimeout;				// info timeout in seconds, 0 means don't display into messages at all
uint32_t whenAlertReceived;
bool displayingResponse = false;						// true if displaying a response

static PixelNumber screensaverTextWidth = 0;
static uint32_t lastScreensaverMoved = 0;

static uint8_t currentWorkplaceNumber = OM::MaxTotalWorkplaces;
static int8_t currentTool = -2;							// Initialized to a value never returned by RRF to have the logic for "no tool" applied at startup
static bool allAxesHomed = false;
static const bool isLandscape = true; 					// Once portrait mode is enabled, this needs to be de-const-ed

#ifdef SUPPORT_ENCODER

# include "Hardware/RotaryEncoder.hpp"

static RotaryEncoder *encoder;
static uint32_t lastEncoderCommandSentAt = 0;
#endif

inline PixelNumber CalcWidth(unsigned int numCols, PixelNumber displayWidth = DisplayX)
{
	return (displayWidth - 2 * margin + fieldSpacing)/numCols - fieldSpacing;
}

inline PixelNumber CalcXPos(unsigned int col, PixelNumber width, int offset = 0)
{
	return col * (width + fieldSpacing) + margin + offset;
}

// Add a text button with a string parameter
TextButton *AddTextButton(PixelNumber row, unsigned int col, unsigned int numCols, const char* _ecv_array text, Event evt, const char* param, PixelNumber displayWidth = DisplayX)
{
	PixelNumber width = CalcWidth(numCols, displayWidth);
	PixelNumber xpos = CalcXPos(col, width);
	TextButton *f = new TextButton(row - 2, xpos, width, text, evt, param);
	mgr.AddField(f);
	return f;
}

// Add a text button with an int parameter
TextButton *AddTextButton(PixelNumber row, unsigned int col, unsigned int numCols, const char* _ecv_array text, Event evt, int param, PixelNumber displayWidth = DisplayX)
{
	PixelNumber width = CalcWidth(numCols, displayWidth);
	PixelNumber xpos = CalcXPos(col, width);
	TextButton *f = new TextButton(row - 2, xpos, width, text, evt, param);
	mgr.AddField(f);
	return f;
}

// Add an integer button
IntegerButton *AddIntegerButton(PixelNumber row, unsigned int col, unsigned int numCols, const char * _ecv_array null label, const char * _ecv_array null units, Event evt, PixelNumber displayWidth = DisplayX)
{
	PixelNumber width = CalcWidth(numCols, displayWidth);
	PixelNumber xpos = CalcXPos(col, width);
	IntegerButton *f = new IntegerButton(row - 2, xpos, width, label, units);
	f->SetEvent(evt, 0);
	mgr.AddField(f);
	return f;
}

// Add an icon button with a string parameter
IconButton *AddIconButton(PixelNumber row, unsigned int col, unsigned int numCols, Icon icon, Event evt, const char* param, PixelNumber displayWidth = DisplayX)
{
	PixelNumber width = CalcWidth(numCols, displayWidth);
	PixelNumber xpos = CalcXPos(col, width);
	IconButton *f = new IconButton(row - 2, xpos, width, icon, evt, param);
	mgr.AddField(f);
	return f;
}

// Add an icon button with an int parameter
IconButton *AddIconButton(PixelNumber row, unsigned int col, unsigned int numCols, Icon icon, Event evt, int param, PixelNumber displayWidth = DisplayX)
{
	PixelNumber width = CalcWidth(numCols, displayWidth);
	PixelNumber xpos = CalcXPos(col, width);
	IconButton *f = new IconButton(row - 2, xpos, width, icon, evt, param);
	mgr.AddField(f);
	return f;
}

// Add an icon button with a string parameter
IconButtonWithText *AddIconButtonWithText(PixelNumber row, unsigned int col, unsigned int numCols, Icon icon, Event evt, const char * text, const char* param, PixelNumber displayWidth = DisplayX)
{
	PixelNumber width = CalcWidth(numCols, displayWidth);
	PixelNumber xpos = CalcXPos(col, width);
	IconButtonWithText *f = new IconButtonWithText(row - 2, xpos, width, icon, evt, text, param);
	mgr.AddField(f);
	return f;
}

// Add an icon button with an int parameter
IconButtonWithText *AddIconButtonWithText(PixelNumber row, unsigned int col, unsigned int numCols, Icon icon, Event evt, int intVal, const int param, PixelNumber displayWidth = DisplayX)
{
	PixelNumber width = CalcWidth(numCols, displayWidth);
	PixelNumber xpos = CalcXPos(col, width);
	IconButtonWithText *f = new IconButtonWithText(row - 2, xpos, width, icon, evt, intVal, param);
	mgr.AddField(f);
	return f;
}

// Add an icon button with an int parameter
IconButtonWithText *AddIconButtonWithText(PixelNumber row, unsigned int col, unsigned int numCols, Icon icon, Event evt, const char * text, const int param, PixelNumber displayWidth = DisplayX)
{
	PixelNumber width = CalcWidth(numCols, displayWidth);
	PixelNumber xpos = CalcXPos(col, width);
	IconButtonWithText *f = new IconButtonWithText(row - 2, xpos, width, icon, evt, text, param);
	mgr.AddField(f);
	return f;
}

// Create a row of text buttons.
// Optionally, set one to 'pressed' and return that one.
// Set the colours before calling this
ButtonPress CreateStringButtonRow(
		Window * parentWindow,
		PixelNumber top,
		PixelNumber left,
		PixelNumber totalWidth,
		PixelNumber spacing,
		unsigned int numButtons,
		const char* _ecv_array const text[],
		const char* _ecv_array const params[],
		Event evt,
		int selected = -1,
		bool textButtonForAxis = false,
		DisplayField** firstButton = nullptr)
{
	const PixelNumber step = (totalWidth + spacing)/numButtons;
	ButtonPress bp;
	// Since Window->AddField prepends fields in the linked list we start with the last element
	for (int i = numButtons - 1; i >= 0; --i)
	{
		TextButton *tp =
				textButtonForAxis
				? new TextButtonForAxis(top, left + i * step, step - spacing, text[i], evt, params[i])
				: new TextButton(		top, left + i * step, step - spacing, text[i], evt, params[i]);
		parentWindow->AddField(tp);
		if ((int)i == selected)
		{
			tp->Press(true, 0);
			bp = ButtonPress(tp, 0);
		}
		if (firstButton != nullptr && i == 0)
		{
			*firstButton = tp;
		}
	}
	return bp;
}

#if 0  // currently unused
// Create a row of icon buttons.
// Set the colours before calling this
void CreateIconButtonRow(Window * pf, PixelNumber top, PixelNumber left, PixelNumber totalWidth, PixelNumber spacing, unsigned int numButtons,
									const Icon icons[], const char* _ecv_array const params[], Event evt)
{
	const PixelNumber step = (totalWidth + spacing)/numButtons;
	for (unsigned int i = 0; i < numButtons; ++i)
	{
		pf->AddField(new IconButton(top, left + i * step, step - spacing, icons[i], evt, params[i]));
	}
}
#endif

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
		pf->AddField(new TextButton(popupSideMargin, popupSideMargin + i * step, step - popupFieldSpacing, text[i], (params && params[i] == 0) ? zeroEv : ev, iParam));
	}
	return pf;
}

// Nasty hack to work around bug in RepRapFirmware 1.09k and earlier
// The M23 and M30 commands don't work if we send the full path, because "0:/gcodes/" gets prepended regardless.
const char * _ecv_array StripPrefix(const char * _ecv_array dir)
{
	if (GetFirmwareFeatures().IsBitSet(noGcodesFolder))			// if running RepRapFirmware
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


static void SendGcode(const char *data)
{
	SerialIo::Sendf("%s\n", data);
}

static void PopupEditData(const char *data)
{
	alertPopup->UpdateData(data);
	dbg("received data %s\n", data);
	mgr.ClearPopup(true, keyboardPopup);
}

// Adjust the brightness
static void ChangeBrightness(bool up)
{
	int adjust = max<int>(1, nvData.GetBrightness() / 5);
	if (!up)
	{
		adjust = -adjust;
	}
	SetBrightness(nvData.GetBrightness() + adjust);
}

// Cycle through available display dimmer types
static void ChangeDisplayDimmerType()
{
	DisplayDimmerType newType = (DisplayDimmerType) ((uint8_t)nvData.GetDisplayDimmerType() + 1);
	if (newType == DisplayDimmerType::NumTypes)
	{
		newType = (DisplayDimmerType)0;
	}
	nvData.SetDisplayDimmerType(newType);
}

// Cyce through available heater combine types and repaint
static void ChangeHeaterCombineType()
{
	HeaterCombineType newType = (HeaterCombineType) ((uint8_t)nvData.GetHeaterCombineType() + 1);
	if (newType == HeaterCombineType::NumTypes)
	{
		newType = (HeaterCombineType)0;
	}
	nvData.SetHeaterCombineType(newType);
	UI::AllToolsSeen();
}

// Update an integer field, provided it isn't the one being adjusted
// Don't update it if the value hasn't changed, because that makes the display flicker unnecessarily
static void UpdateField(IntegerButton *f, int val)
{
	if (f != fieldBeingAdjusted.GetButton() && f->GetValue() != val)
	{
		f->SetValue(val);
	}
}

static void PopupAreYouSure(Event ev, const char* text, const char* query = strings->areYouSure)
{
	eventToConfirm = ev;
	if (isLandscape)
	{
		areYouSureTextField->SetValue(text);
		areYouSureQueryField->SetValue(query);
		mgr.SetPopup(areYouSurePopup, AutoPlace, AutoPlace);
	}
}

static void CreateIntegerAdjustPopup(const ColourScheme& colours)
{
	// Create the popup window used to adjust temperatures, fan speed, extrusion factor etc.
	static const char* const tempPopupText[] = {"-5", "-1", strings->set, "+1", "+5"};
	static const int tempPopupParams[] = { -5, -1, 0, 1, 5 };
	setTempPopup = CreateIntPopupBar(colours, tempPopupBarWidth, 5, tempPopupText, tempPopupParams, evAdjustInt, evSetInt);
}

static void CreateIntegerRPMAdjustPopup(const ColourScheme& colours)
{
	// Create the popup window used to adjust temperatures, fan speed, extrusion factor etc.
	static const char* const rpmPopupText[] = {"-1000", "-100", "-10", strings->set, "+10", "+100", "+1000"};
	static const int rpmPopupParams[] = { -1000, -100, -10, 0, 10, 100, 1000 };
	setRPMPopup = CreateIntPopupBar(colours, rpmPopupBarWidth, 7, rpmPopupText, rpmPopupParams, evAdjustInt, evSetInt);
}

// Create the movement popup window
static void CreateMovePopup(const ColourScheme& colours)
{
	static const char * _ecv_array const xyJogValues[] = { "-100", "-10", "-1", "-0.1", "0.1",  "1", "10", "100" };
	static const char * _ecv_array const zJogValues[] = { "-50", "-5", "-0.5", "-0.05", "0.05",  "0.5", "5", "50" };

	movePopup = new StandardPopupWindow(movePopupHeight, movePopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, colours.buttonImageBackColour, strings->moveHead);
	PixelNumber ypos = popupTopMargin + buttonHeight + moveButtonRowSpacing;
	const PixelNumber axisPosYpos = ypos + (MaxDisplayableAxes - 1) * (buttonHeight + moveButtonRowSpacing);
	const PixelNumber xpos = popupSideMargin + axisLabelWidth;
	PixelNumber column = popupSideMargin + margin;
	PixelNumber xyFieldWidth = (DISPLAY_X - (2 * margin) - (MaxDisplayableAxes * fieldSpacing))/(MaxDisplayableAxes + 1);

	for (size_t i = 0; i < MaxDisplayableAxes; ++i)
	{
		DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
		const char * _ecv_array const * _ecv_array values = (axisNames[i][0] == 'Z') ? zJogValues : xyJogValues;
		CreateStringButtonRow(movePopup, ypos, xpos, movePopupWidth - xpos - popupSideMargin, fieldSpacing, 8, values, values, evMoveAxis, -1, true);

		// We create the label after the button row, so that the buttons follow it in the field order, which makes it easier to hide them
		DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
		StaticTextField * const tf = new StaticTextField(ypos + labelRowAdjust, popupSideMargin, axisLabelWidth, TextAlignment::Left, axisNames[i]);
		movePopup->AddField(tf);
		moveAxisRows[i] = tf;
		UI::ShowAxis(i, i < MIN_AXES, axisNames[i]);

		DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupInfoBackColour);
		FloatField *f = new FloatField(axisPosYpos, column, xyFieldWidth, TextAlignment::Left, 2, axisNames[i]);
		movePopupAxisPos[i] = f;
		movePopup->AddField(f);
		f->Show(i < MIN_AXES);
		column += xyFieldWidth + fieldSpacing;

		ypos += buttonHeight + moveButtonRowSpacing;
	}
}

// Create the extrusion controls popup
static void CreateExtrudePopup(const ColourScheme& colours)
{
	static const char * _ecv_array extrudeAmountValues[] = { "100", "50", "20", "10", "5",  "1" };
	static const char * _ecv_array extrudeSpeedValues[] = { "50", "20", "10", "5", "2", "1", "0.5" };
	static const char * _ecv_array extrudeSpeedParams[] = { "3000", "1200", "600", "300", "120", "60", "30" };		// must be extrudeSpeedValues * 60

	extrudePopup = new StandardPopupWindow(extrudePopupHeight, extrudePopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, colours.buttonImageBackColour, strings->extrusionAmount);
	PixelNumber ypos = popupTopMargin + buttonHeight + extrudeButtonRowSpacing;
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	currentExtrudeAmountPress = CreateStringButtonRow(extrudePopup, ypos, popupSideMargin, extrudePopupWidth - 2 * popupSideMargin, fieldSpacing, 6, extrudeAmountValues, extrudeAmountValues, evExtrudeAmount, 3);
	ypos += buttonHeight + extrudeButtonRowSpacing;
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	extrudePopup->AddField(new StaticTextField(ypos + labelRowAdjust, popupSideMargin, extrudePopupWidth - 2 * popupSideMargin, TextAlignment::Centre, strings->extrusionSpeed));
	ypos += buttonHeight + extrudeButtonRowSpacing;
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	currentExtrudeRatePress = CreateStringButtonRow(
			extrudePopup,
			ypos,
			popupSideMargin,
			extrudePopupWidth - 2 * popupSideMargin,
			fieldSpacing,
			ARRAY_SIZE(extrudeSpeedValues),
			extrudeSpeedValues,
			extrudeSpeedParams,
			evExtrudeRate,
			ARRAY_SIZE(extrudeSpeedValues) / 2);

	ypos += buttonHeight + extrudeButtonRowSpacing;
	extrudePopup->AddField(new TextButton(ypos, popupSideMargin, extrudePopupWidth/3 - 2 * popupSideMargin, strings->extrude, evExtrude));
	extrudePopup->AddField(new TextButton(ypos, (2 * extrudePopupWidth)/3 + popupSideMargin, extrudePopupWidth/3 - 2 * popupSideMargin, strings->retract, evRetract));
}

// Create a popup used to list files pr macros
PopupWindow *CreateFileListPopup(FileListButtons& controlButtons, TextButton ** _ecv_array fileButtons, unsigned int numRows, unsigned int numCols, const ColourScheme& colours, bool filesNotMacros,
		PixelNumber popupHeight = fileListPopupHeight, PixelNumber popupWidth = fileListPopupWidth)
pre(fileButtons.lim == numRows * numCols)
{
	PopupWindow * const popup = new StandardPopupWindow(popupHeight, popupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, colours.buttonImageBackColour, nullptr);
	const PixelNumber closeButtonPos = popupWidth - closeButtonWidth - popupSideMargin;
	const PixelNumber navButtonWidth = (closeButtonPos - popupSideMargin)/7;
	const PixelNumber upButtonPos = closeButtonPos - navButtonWidth - fieldSpacing;
	const PixelNumber rightButtonPos = upButtonPos - navButtonWidth - fieldSpacing;
	const PixelNumber leftButtonPos = rightButtonPos - navButtonWidth - fieldSpacing;
	const PixelNumber textPos = popupSideMargin + navButtonWidth;
	const PixelNumber changeButtonPos = popupSideMargin;

	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	if (filesNotMacros)
	{
		popup->AddField(filePopupTitleField = new IntegerField(popupTopMargin + labelRowAdjust, textPos, leftButtonPos - textPos, TextAlignment::Centre, strings->filesOnCard, nullptr));
		popup->AddField(fileListPopupNoFiles = new StaticTextField(popupHeight / 2 - popupTopMargin, popupSideMargin, popupWidth, TextAlignment::Centre, strings->noFilesFound));
		fileListPopupNoFiles->Show(false);
	}
	else
	{
		popup->AddField(new StaticTextField(popupTopMargin + labelRowAdjust, textPos, leftButtonPos - textPos, TextAlignment::Centre, strings->macros));
	}

	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.buttonImageBackColour);
	if (filesNotMacros)
	{
		popup->AddField(changeCardButton = new IconButton(popupTopMargin, changeButtonPos, navButtonWidth, IconFiles, evChangeCard, 0));
	}

	const Event scrollEvent = (filesNotMacros) ? evScrollFiles : evScrollMacros;

	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	popup->AddField(controlButtons.scrollLeftButton = new TextButton(popupTopMargin, leftButtonPos, navButtonWidth, LEFT_ARROW, scrollEvent, -1));
	controlButtons.scrollLeftButton->Show(false);
	popup->AddField(controlButtons.scrollRightButton = new TextButton(popupTopMargin, rightButtonPos, navButtonWidth, RIGHT_ARROW, scrollEvent, 1));
	controlButtons.scrollRightButton->Show(false);
	popup->AddField(controlButtons.folderUpButton = new TextButton(popupTopMargin, upButtonPos, navButtonWidth, UP_ARROW, (filesNotMacros) ? evFilesUp : evMacrosUp));
	controlButtons.folderUpButton->Show(false);

	const PixelNumber fileFieldWidth = (popupWidth + fieldSpacing - (2 * popupSideMargin))/numCols;
	for (unsigned int c = 0; c < numCols; ++c)
	{
		PixelNumber row = popupTopMargin;
		for (unsigned int r = 0; r < numRows; ++r)
		{
			row += buttonHeight + fileButtonRowSpacing;
			TextButton *t = new TextButton(row, (fileFieldWidth * c) + popupSideMargin, fileFieldWidth - fieldSpacing, nullptr, evNull);
			t->Show(false);
			popup->AddField(t);
			*fileButtons = t;
			++fileButtons;
		}
	}

	controlButtons.errorField = new IntegerField(popupTopMargin + 2 * (buttonHeight + fileButtonRowSpacing), popupSideMargin, popupWidth - (2 * popupSideMargin),
							TextAlignment::Centre, strings->error, strings->accessingSdCard);
	controlButtons.errorField->Show(false);
	popup->AddField(controlButtons.errorField);
	return popup;
}

static void ThumbnailRefreshNotify(bool full, bool changed)
{
	UNUSED(changed);

	if (!full || !currentFile)
		return;

	dbg("full %d changed %d currentFile %s\n", full, changed, currentFile);
	SerialIo::Sendf(GetFirmwareFeatures().IsBitSet(noM20M36) ? "M408 S36 P" : "M36 ");			// ask for the file info
	SerialIo::SendFilename(CondStripDrive(FileManager::GetFilesDir()), currentFile);
	SerialIo::SendChar('\n');
}

// Create the popup window used to display the file dialog
static void CreateFileActionPopup(const ColourScheme& colours)
{
	PixelNumber y_start, height;
	PixelNumber x_start, width;

	fileDetailPopup = new StandardPopupWindow(fileInfoPopupHeight, fileInfoPopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, colours.buttonImageBackColour, nullptr);
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);

	PixelNumber ypos = popupTopMargin + 1;
	fpNameField = new TextField(ypos, popupSideMargin, fileInfoPopupWidth - closeButtonWidth - 3 * popupSideMargin, TextAlignment::Left, strings->fileName);
	ypos += rowTextHeight + 3;
	fpGeneratedByField = new TextField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, strings->generatedBy, generatedByText.c_str());
	ypos += rowTextHeight;

	y_start = ypos + 3;
	height = 7 * rowTextHeight + (2 * rowTextHeight) / 3;

	x_start = fileInfoPopupWidth - popupSideMargin * 3 / 2 - fileInfoPopupWidth / 3;
	width = fileInfoPopupWidth / 3 + 5;

	fpThumbnail = new DrawDirect(y_start, x_start, height, width, ThumbnailRefreshNotify);

	dbg("y_start %d x_start %d height %d width %d\n", y_start, x_start, height, width);
	dbg("text height %d\n", rowTextHeight);

	fpSizeField = new IntegerField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin - fileInfoPopupWidth / 3, TextAlignment::Left, strings->fileSize, " b");
	ypos += rowTextHeight;
	fpLayerHeightField = new FloatField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin - fileInfoPopupWidth / 3, TextAlignment::Left, 2, strings->layerHeight, "mm");
	ypos += rowTextHeight;
	fpHeightField = new FloatField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin - fileInfoPopupWidth / 3, TextAlignment::Left, 1, strings->objectHeight, "mm");
	ypos += rowTextHeight;
	fpFilamentField = new IntegerField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin - fileInfoPopupWidth / 3, TextAlignment::Left, strings->filamentNeeded, "mm");
	ypos += rowTextHeight;
	fpLastModifiedField = new TextField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin - fileInfoPopupWidth / 3, TextAlignment::Left, strings->lastModified, lastModifiedText.c_str());
	ypos += rowTextHeight;
	fpPrintTimeField = new TextField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin - fileInfoPopupWidth / 3, TextAlignment::Left, strings->estimatedPrintTime, printTimeText.c_str());
	fileDetailPopup->AddField(fpNameField);
	fileDetailPopup->AddField(fpSizeField);
	fileDetailPopup->AddField(fpLayerHeightField);
	fileDetailPopup->AddField(fpHeightField);
	fileDetailPopup->AddField(fpFilamentField);
	fileDetailPopup->AddField(fpGeneratedByField);
	fileDetailPopup->AddField(fpLastModifiedField);
	fileDetailPopup->AddField(fpPrintTimeField);
	fileDetailPopup->AddField(fpThumbnail);

	// Add the buttons
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	fileDetailPopup->AddField(new TextButton(popupTopMargin + 10 * rowTextHeight, popupSideMargin, fileInfoPopupWidth/3 - 2 * popupSideMargin, strings->print, evPrintFile));
	fileDetailPopup->AddField(new TextButton(popupTopMargin + 10 * rowTextHeight, fileInfoPopupWidth/3 + popupSideMargin, fileInfoPopupWidth/3 - 2 * popupSideMargin, strings->simulate, evSimulateFile));
	fileDetailPopup->AddField(new IconButton(popupTopMargin + 10 * rowTextHeight, (2 * fileInfoPopupWidth)/3 + popupSideMargin, fileInfoPopupWidth/3 - 2 * popupSideMargin, IconTrash, evDeleteFile));
}

// Create the "Are you sure?" popup
static void CreateAreYouSurePopup(const ColourScheme& colours)
{
	areYouSurePopup = new PopupWindow(areYouSurePopupHeight, areYouSurePopupWidth, colours.popupBackColour, colours.popupBorderColour);
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	areYouSurePopup->AddField(areYouSureTextField = new StaticTextField(popupSideMargin, margin, areYouSurePopupWidth - 2 * margin, TextAlignment::Centre, nullptr));
	areYouSurePopup->AddField(areYouSureQueryField = new StaticTextField(popupTopMargin + rowHeight, margin, areYouSurePopupWidth - 2 * margin, TextAlignment::Centre, nullptr));

	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	areYouSurePopup->AddField(new IconButton(popupTopMargin + 2 * rowHeight, popupSideMargin, areYouSurePopupWidth/2 - 2 * popupSideMargin, IconOk, evYes));
	areYouSurePopup->AddField(new IconButton(popupTopMargin + 2 * rowHeight, areYouSurePopupWidth/2 + 10, areYouSurePopupWidth/2 - 2 * popupSideMargin, IconCancel, evCancel));
}

static void CreateScreensaverPopup()
{
	screensaverPopup = new PopupWindow(max(DisplayX, DisplayY), max(DisplayX, DisplayY), black, black, false);
	DisplayField::SetDefaultColours(white, black);
	static const char * text = "Touch to wake up";
	screensaverTextWidth = DisplayField::GetTextWidth(text, DisplayX);
	screensaverPopup->AddField(screensaverText = new StaticTextField(row1, margin, screensaverTextWidth, TextAlignment::Left, text));
}

// Create the baud rate adjustment popup
static void CreateBaudRatePopup(const ColourScheme& colours)
{
	static const char* const baudPopupText[] = { "9600", "19200", "38400", "57600", "115200" };
	static const int baudPopupParams[] = { 9600, 19200, 38400, 57600, 115200 };
	baudPopup = CreateIntPopupBar(colours, fullPopupWidth, 5, baudPopupText, baudPopupParams, evAdjustBaudRate, evAdjustBaudRate);
}

// Create the volume adjustment popup
static void CreateVolumePopup(const ColourScheme& colours)
{
	static_assert(Buzzer::MaxVolume == 5, "MaxVolume assumed to be 5 here");
	static const char* const volumePopupText[Buzzer::MaxVolume + 1] = { "0", "1", "2", "3", "4", "5" };
	volumePopup = CreateIntPopupBar(colours, fullPopupWidth, ARRAY_SIZE(volumePopupText), volumePopupText, nullptr, evAdjustVolume, evAdjustVolume);
}

// Create the volume adjustment popup
static void CreateInfoTimeoutPopup(const ColourScheme& colours)
{
	static const char* const infoTimeoutPopupText[Buzzer::MaxVolume + 1] = { "0", "2", "5", "10" };
	static const int values[] = { 0, 2, 5, 10 };
	infoTimeoutPopup = CreateIntPopupBar(colours, fullPopupWidth, ARRAY_SIZE(infoTimeoutPopupText), infoTimeoutPopupText, values, evAdjustInfoTimeout, evAdjustInfoTimeout);
}

// Create the screensaver timeout adjustment popup
static void CreateScreensaverTimeoutPopup(const ColourScheme& colours)
{
	static const char* const screensaverTimeoutPopupText[Buzzer::MaxVolume + 1] = { "off", "60", "120", "180", "240", "300" };
	static const int values[] = { 0, 60, 120, 180, 240, 300 };
	screensaverTimeoutPopup = CreateIntPopupBar(colours, fullPopupWidth, ARRAY_SIZE(screensaverTimeoutPopupText), screensaverTimeoutPopupText, values, evAdjustScreensaverTimeout, evAdjustScreensaverTimeout);
}

// Create the babystep amount adjustment popup
static void CreateBabystepAmountPopup(const ColourScheme& colours)
{
	static const int values[] = { 0, 1, 2, 3 };
	babystepAmountPopup = CreateIntPopupBar(colours, fullPopupWidth, ARRAY_SIZE(babystepAmounts), babystepAmounts, values, evAdjustBabystepAmount, evAdjustBabystepAmount);
}

// Create the feedrate amount adjustment popup
static void CreateFeedrateAmountPopup(const ColourScheme& colours)
{
	static const char* const feedrateText[] = {"600", "1200", "2400", "6000", "12000"};
	static const int values[] = { 600, 1200, 2400, 6000, 12000 };
	feedrateAmountPopup = CreateIntPopupBar(colours, fullPopupWidth, ARRAY_SIZE(feedrateText), feedrateText, values, evAdjustFeedrate, evAdjustFeedrate);
}

// Create the colour scheme change popup
static void CreateColoursPopup(const ColourScheme& colours)
{
	if (NumColourSchemes >= 2)
	{
		// Put all the colour scheme names in a single _ecv_array for the call to CreateIntPopupBar
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
static void CreateLanguagePopup(const ColourScheme& colours)
{
	languagePopup = new PopupWindow(popupBarHeight, fullPopupWidth, colours.popupBackColour, colours.popupBorderColour);
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	PixelNumber step = (fullPopupWidth - 2 * popupSideMargin + popupFieldSpacing)/NumLanguages;
	for (unsigned int i = 0; i < NumLanguages; ++i)
	{
		languagePopup->AddField(new TextButton(popupSideMargin, popupSideMargin + i * step, step - popupFieldSpacing, LanguageTables[i].languageName, evAdjustLanguage, i));
	}
}

// Create the pop-up keyboard
static void CreateKeyboardPopup(uint32_t language, ColourScheme colours)
{
	static const char* _ecv_array const keysEN[8] = { "1234567890-+", "QWERTYUIOP[]", "ASDFGHJKL:@", "ZXCVBNM,./", "!\"#$%^&*()_=", "qwertyuiop{}", "asdfghjkl;'", "zxcvbnm<>?" };
	static const char* _ecv_array const keysDE[8] = { "1234567890-+", "QWERTZUIOP[]", "ASDFGHJKL:@", "YXCVBNM,./", "!\"#$%^&*()_=", "qwertzuiop{}", "asdfghjkl;'", "yxcvbnm<>?" };
	static const char* _ecv_array const keysFR[8] = { "1234567890-+", "AZERTWUIOP[]", "QSDFGHJKLM@", "YXCVBN.,:/", "!\"#$%^&*()_=", "azertwuiop{}", "qsdfghjklm'", "yxcvbn<>;?" };
	static const char* _ecv_array const * const keyboards[] = {
			keysEN,	// English
			keysDE,	// German
			keysFR,	// French
			keysEN,	// Spanish
			keysEN,	// Czech
			keysEN,	// Italian
			keysEN,	// Dutch
			keysEN,	// Polish
#if USE_CYRILLIC_CHARACTERS
			keysEN,	// Ukrainian
			keysEN,	// Russian
#elif USE_JAPANESE_CHARACTERS
			keysEN, // Japanese
#endif
	};

	static_assert(ARRAY_SIZE(keyboards) >= NumLanguages, "Wrong number of keyboard entries");

	keyboardPopup = new StandardPopupWindow(keyboardPopupHeight, keyboardPopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupInfoTextColour, colours.buttonImageBackColour, nullptr, keyboardTopMargin);

	// Add the text area in which the command is built
	DisplayField::SetDefaultColours(colours.popupInfoTextColour, colours.popupInfoBackColour);		// need a different background colour
	userCommandField = new TextField(keyboardTopMargin + labelRowAdjust, popupSideMargin, keyboardPopupWidth - 2 * popupSideMargin - closeButtonWidth - popupFieldSpacing, TextAlignment::Left, nullptr, "_");
	userCommandField->SetLabel(userCommandBuffers[currentUserCommandBuffer].c_str());	// set up to display the current user command
	keyboardPopup->AddField(userCommandField);

	if (language >= NumLanguages)
	{
		language = 0;
	}

	currentKeyboard = keyboards[language];
	PixelNumber row = keyboardTopMargin + keyButtonVStep;

	for (size_t i = 0; i < 4; ++i)
	{
		DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
		// New code using CharButtonRow to economise on RAM at the expense of more flash memory usage
		const PixelNumber column = popupSideMargin + (i * keyButtonHStep)/3;
		keyboardRows[i] = new CharButtonRow(row, column, keyButtonWidth, keyButtonHStep, currentKeyboard[i], evKey);
		keyboardPopup->AddField(keyboardRows[i]);
		DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.buttonImageBackColour);
		switch (i)
		{
		case 0:
			keyboardPopup->AddField(new IconButton(row, keyboardPopupWidth - popupSideMargin - (5 * keyButtonWidth)/4, (5 * keyButtonWidth)/4, IconBackspace, evBackspace));
			break;

		case 2:
			keyboardPopup->AddField(new TextButton(row, keyboardPopupWidth - popupSideMargin - (3 * keyButtonWidth)/2, (3 * keyButtonWidth)/2, UP_ARROW, evUp));
			break;

		case 3:
			keyboardPopup->AddField(new TextButton(row, keyboardPopupWidth - popupSideMargin - (3 * keyButtonWidth)/2, (3 * keyButtonWidth)/2, DOWN_ARROW, evDown));
			break;

		default:
			break;
		}
		row += keyButtonVStep;
	}

	// Add the shift, space and enter keys
	const PixelNumber keyButtonHSpace = keyButtonHStep - keyButtonWidth;
	const PixelNumber wideKeyButtonWidth = (keyboardPopupWidth - 2 * popupSideMargin - 2 * keyButtonHSpace)/5;
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
	keyboardPopup->AddField(new TextButton(row, popupSideMargin, wideKeyButtonWidth, "Shift", evShift, 0));
	keyboardPopup->AddField(new TextButton(row, popupSideMargin + wideKeyButtonWidth + keyButtonHSpace, 2 * wideKeyButtonWidth, "", evKey, (int)' '));
	DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.buttonImageBackColour);
	keyboardPopup->AddField(new IconButton(row, popupSideMargin + 3 * wideKeyButtonWidth + 2 * keyButtonHSpace, wideKeyButtonWidth, IconEnter, evSendKeyboardCommand));

	keyboardDataHandler = SendGcode;
}

// Create the babystep popup
static void CreateBabystepPopup(const ColourScheme& colours)
{
	babystepPopup = new StandardPopupWindow(babystepPopupHeight, babystepPopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, colours.buttonImageBackColour,
			strings->babyStepping);
	PixelNumber ypos = popupTopMargin + babystepRowSpacing;
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
	babystepPopup->AddField(babystepOffsetField = new FloatField(ypos, popupSideMargin, babystepPopupWidth - 2 * popupSideMargin, TextAlignment::Left, 3, strings->currentZoffset, "mm"));
	ypos += babystepRowSpacing;
	DisplayField::SetDefaultColours(colours.popupTextColour, colours.buttonImageBackColour);
	const PixelNumber width = CalcWidth(2, babystepPopupWidth - 2 * popupSideMargin);
	babystepPopup->AddField(babystepMinusButton = new TextButtonWithLabel(ypos, CalcXPos(0, width, popupSideMargin), width, babystepAmounts[nvData.GetBabystepAmountIndex()], evBabyStepMinus, nullptr, LESS_ARROW " "));
	babystepPopup->AddField(babystepPlusButton = new TextButtonWithLabel(ypos, CalcXPos(1, width, popupSideMargin), width, babystepAmounts[nvData.GetBabystepAmountIndex()], evBabyStepPlus, nullptr, MORE_ARROW " "));
}

// Create the grid of heater icons and temperatures
static void CreateTemperatureGrid(const ColourScheme& colours)
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
	for (unsigned int i = 0; i < MaxSlots; ++i)
	{
		const PixelNumber column = ((tempButtonWidth + fieldSpacing) * i) + bedColumn;

		// Add the icon button
		DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonImageBackColour);
		IconButtonWithText * const b = new IconButtonWithText(row2, column, tempButtonWidth, i == 0 ? IconBed : IconNozzle, evSelectHead, i, i);
		b->Show(false);
		toolButtons[i] = b;
		mgr.AddField(b);

		// Add the current temperature field
		DisplayField::SetDefaultColours(colours.infoTextColour, colours.defaultBackColour);
		FloatField * const f = new FloatField(row3 + labelRowAdjust, column, tempButtonWidth, TextAlignment::Centre, 1);
		f->Show(false);
		currentTemps[i] = f;
		mgr.AddField(f);

		// Add the active temperature button
		DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
		IntegerButton *ib = new IntegerButton(row4, column, tempButtonWidth);
		ib->SetEvent(evAdjustToolActiveTemp, i);
		ib->SetValue(0);
		ib->Show(false);
		activeTemps[i] = ib;
		mgr.AddField(ib);

		// Add the standby temperature button
		ib = new IntegerButton(row5, column, tempButtonWidth);
		ib->SetEvent(evAdjustToolStandbyTemp, i);
		ib->SetValue(0);
		ib->Show(false);
		standbyTemps[i] = ib;
		mgr.AddField(ib);
	}
}

// Create the extra fields for the Control tab
static void CreateControlTabFields(const ColourScheme& colours)
{
	mgr.SetRoot(commonRoot);

	DisplayField::SetDefaultColours(colours.infoTextColour, colours.infoBackColour);
	PixelNumber column = margin;
	PixelNumber xyFieldWidth = (DISPLAY_X - (2 * margin) - (MaxDisplayableAxes * fieldSpacing))/(MaxDisplayableAxes + 1);
	for (size_t i = 0; i < MaxDisplayableAxes; ++i)
	{
		FloatField * const f = new FloatField(row6p3 + labelRowAdjust, column, xyFieldWidth, TextAlignment::Left, 2, axisNames[i]);
		controlTabAxisPos[i] = f;
		mgr.AddField(f);
		f->Show(i < MIN_AXES);
		column += xyFieldWidth + fieldSpacing;
	}
	zprobeBuf[0] = 0;
	mgr.AddField(zProbe = new TextField(row6p3 + labelRowAdjust, column, DISPLAY_X - column - margin, TextAlignment::Left, "P", zprobeBuf.c_str()));

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.notHomedButtonBackColour);
	homeAllButton = AddIconButton(row7p7, 0, MaxDisplayableAxes + 2, IconHomeAll, evSendCommand, "G28");
	homeButtons[0] = AddIconButtonWithText(row7p7, 1, MaxDisplayableAxes + 2, IconHomeAll, evHomeAxis, axisNames[0], axisNames[0]);
	homeButtons[1] = AddIconButtonWithText(row7p7, 2, MaxDisplayableAxes + 2, IconHomeAll, evHomeAxis, axisNames[1], axisNames[1]);
	homeButtons[2] = AddIconButtonWithText(row7p7, 3, MaxDisplayableAxes + 2, IconHomeAll, evHomeAxis, axisNames[2], axisNames[2]);
#if MaxDisplayableAxes > 3
	homeButtons[3] = AddIconButtonWithText(row7p7, 4, MaxDisplayableAxes + 2, IconHomeAll, evHomeAxis, axisNames[3], axisNames[3]);
	homeButtons[3]->Show(false);
#endif
#if MaxDisplayableAxes > 4
	homeButtons[4] = AddIconButtonWithText(row7p7, 5, MaxDisplayableAxes + 2, IconHomeAll, evHomeAxis, axisNames[4], axisNames[4]);
	homeButtons[4]->Show(false);
#endif
#if MaxDisplayableAxes > 5
	homeButtons[5] = AddIconButtonWithText(row7p7, 6, MaxDisplayableAxes + 2, IconHomeAll, evHomeAxis, axisNames[5], axisNames[5]);
	homeButtons[5]->Show(false);
#endif
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonImageBackColour);
	bedCompButton = AddIconButton(row7p7, MaxDisplayableAxes + 1, MaxDisplayableAxes + 2, IconBedComp, evSendCommand, "G32");

	filesButton = AddIconButton(row8p7, 0, 4, IconFiles, evListFiles, nullptr);
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	moveButton = AddTextButton(row8p7, 1, 4, strings->move, evMovePopup, nullptr);
	extrudeButton = AddTextButton(row8p7, 2, 4, strings->extrusion, evExtrudePopup, nullptr);
	macroButton = AddTextButton(row8p7, 3, 4, strings->macro, evListMacros, nullptr);

	// When there is room, we also display a few macro buttons on the right hand side
	for (size_t i = 0; i < NumControlPageMacroButtons; ++i)
	{
		// The position and width of the buttons will get corrected when we know how many tools we have
		TextButton * const b = controlPageMacroButtons[i] = new TextButton(row2 + i * rowHeight, 999, 99, nullptr, evNull);
		b->Show(false);			// hide them until we have loaded the macros
		mgr.AddField(b);
	}

	controlRoot = mgr.GetRoot();
}

// Create the fields for the Printing tab
static void CreatePrintingTabFields(const ColourScheme& colours)
{
	mgr.SetRoot(commonRoot);

	// Labels
	DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
	mgr.AddField(new StaticTextField(row6 + labelRowAdjust, margin, bedColumn - fieldSpacing - margin, TextAlignment::Right, strings->extruderPercent));

	// Extrusion factor buttons
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	for (unsigned int i = 0; i < MaxSlots; ++i)
	{
		const PixelNumber column = ((tempButtonWidth + fieldSpacing) * i) + bedColumn;

		IntegerButton * const ib = new IntegerButton(row6, column, tempButtonWidth);
		ib->SetValue(100);
		ib->SetEvent(evExtrusionFactor, i);
		ib->Show(false);
		extrusionFactors[i] = ib;
		mgr.AddField(ib);
	}

	// Speed button
	mgr.AddField(spd = new IntegerButton(row7, speedColumn, stateColumnWdith - fieldSpacing, strings->speed, "%"));
	spd->SetValue(100);
	spd->SetEvent(evAdjustSpeed, "M220 S");

	// Fan button
	mgr.AddField(fanSpeed = new IntegerButton(row7, fanColumn, stateColumnWdith - fieldSpacing, strings->fan, "%"));
	fanSpeed->SetEvent(evAdjustFan, 0);
	fanSpeed->SetValue(0);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	babystepButton = new TextButton(row7, babystepColumn, stateColumnWdith - fieldSpacing, strings->babystep, evBabyStepPopup);
	mgr.AddField(babystepButton);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.resetButtonBackColour);
	cancelButton = new TextButton(row7, cancelColumn, stateColumnWdith - fieldSpacing, strings->cancel, evReset, "M0");
	mgr.AddField(cancelButton);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.pauseButtonBackColour);
	pauseButton = new TextButton(row7, pauseColumn, stateColumnWdith - (2 * margin), strings->pause, evPausePrint, "M25");
	mgr.AddField(pauseButton);

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.resumeButtonBackColour);
	resumeButton = new TextButton(row7, resumeColumn, stateColumnWdith - (2 * margin), strings->resume, evResumePrint, "M24");
	mgr.AddField(resumeButton);

#if DISPLAY_X == 800
	// On 5" and 7" screens there is room to show the current position on the Print page
	const PixelNumber offset = rowHeight - 20;
	DisplayField::SetDefaultColours(colours.infoTextColour, colours.infoBackColour);
	PixelNumber column = margin;
	PixelNumber xyFieldWidth = (DISPLAY_X - (2 * margin) - (MaxDisplayableAxes * fieldSpacing))/(MaxDisplayableAxes + 1);
	for (size_t i = 0; i < MaxDisplayableAxes; ++i)
	{
		FloatField * const f = new FloatField(row8 + labelRowAdjust - 4, column, xyFieldWidth, TextAlignment::Left, 2, axisNames[i]);
		printTabAxisPos[i] = f;
		mgr.AddField(f);
		f->Show(i < MIN_AXES);
		column += xyFieldWidth + fieldSpacing;
	}
#else
	const PixelNumber offset = 0;
#endif

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	const PixelNumber reprintRow =
#if DISPLAY_X == 800
			row9
#else
			row8
#endif
			;
	reprintButton = new TextButton(reprintRow, speedColumn, 2 * stateColumnWdith - fieldSpacing, strings->reprint, evReprint);
	reprintButton->Show(false);
	mgr.AddField(reprintButton);

	DisplayField::SetDefaultColours(colours.progressBarColour,colours. progressBarBackColour);
	mgr.AddField(printProgressBar = new ProgressBar(row8 + offset + (rowHeight - progressBarHeight)/2, margin, progressBarHeight, DisplayX - 2 * margin));
	mgr.Show(printProgressBar, false);

	DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
	mgr.AddField(timeLeftField = new TextField(row9 + offset, margin, DisplayX - 2 * margin, TextAlignment::Left, strings->timeRemaining));
	mgr.Show(timeLeftField, false);

	printRoot = mgr.GetRoot();
}

// Create the fields for the Message tab
static void CreateMessageTabFields(const ColourScheme& colours)
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
static void CreateSetupTabFields(uint32_t language, const ColourScheme& colours)
{
	mgr.SetRoot(baseRoot);
	DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
	// The firmware version field doubles up as an area for displaying debug messages, so make it the full width of the display
	mgr.AddField(fwVersionField = new TextField(row1, margin, DisplayX, TextAlignment::Left, strings->firmwareVersion, VERSION_TEXT));
	mgr.AddField(freeMem = new IntegerField(row2, margin, DisplayX/2 - margin, TextAlignment::Left, "Free RAM: "));
	mgr.AddField(new ColourGradientField(ColourGradientTopPos, ColourGradientLeftPos, ColourGradientWidth, ColourGradientHeight));

	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour);
	baudRateButton = AddIntegerButton(row3, 0, 3, nullptr, " baud", evSetBaudRate);
	baudRateButton->SetValue(nvData.GetBaudRate());
	volumeButton = AddIntegerButton(row3, 1, 3, strings->volume, nullptr, evSetVolume);
	volumeButton->SetValue(nvData.GetVolume());
	languageButton = AddTextButton(row3, 2, 3, LanguageTables[language].languageName, evSetLanguage, nullptr);
	AddTextButton(row4, 0, 3, strings->calibrateTouch, evCalTouch, nullptr);
	AddTextButton(row4, 1, 3, strings->mirrorDisplay, evInvertX, nullptr);
	AddTextButton(row4, 2, 3, strings->invertDisplay, evInvertY, nullptr);
	coloursButton = AddTextButton(row5, 0, 3, strings->colourSchemeNames[colours.index], evSetColours, nullptr);
	AddTextButton(row5, 1, 3, strings->brightnessDown, evDimmer, nullptr);
	AddTextButton(row5, 2, 3, strings->brightnessUp, evBrighter, nullptr);
	dimmingTypeButton = AddTextButton(row6, 0, 3, strings->displayDimmingNames[(unsigned int)nvData.GetDisplayDimmerType()], evSetDimmingType, nullptr);
	infoTimeoutButton = AddIntegerButton(row6, 1, 3, strings->infoTimeout, nullptr, evSetInfoTimeout);
	infoTimeoutButton->SetValue(infoTimeout);
	AddTextButton(row6, 2, 3, strings->clearSettings, evFactoryReset, nullptr);
	screensaverTimeoutButton = AddIntegerButton(row7, 0, 3, strings->screensaverAfter, nullptr, evSetScreensaverTimeout);
	screensaverTimeoutButton->SetValue(nvData.GetScreensaverTimeout() / 1000);

	const PixelNumber width = CalcWidth(3);
	mgr.AddField(babystepAmountButton = new TextButtonWithLabel(row7, CalcXPos(1, width), width, babystepAmounts[nvData.GetBabystepAmountIndex()], evSetBabystepAmount, nullptr, strings->babystepAmount));

	feedrateAmountButton = AddIntegerButton(row7, 2, 3, strings->feedrate, nullptr, evSetFeedrate);
	feedrateAmountButton->SetValue(nvData.GetFeedrate());

	heaterCombiningButton  = AddTextButton(row8, 0, 3, strings->heaterCombineTypeNames[(unsigned int)nvData.GetHeaterCombineType()], evSetHeaterCombineType, nullptr);
	logLevelButton = AddTextButton(row8, 1, 3, strings->logLevelNames[(unsigned int)MessageLog::LogLevelGet()], evSetLogLevel, nullptr);

	DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
	mgr.AddField(ipAddressField = new TextField(row9, margin, DisplayX/2 - margin, TextAlignment::Left, "IP: ", ipAddress.c_str()));
	setupRoot = mgr.GetRoot();
}

// Create the fields that are displayed on all pages
static void CreateCommonFields(const ColourScheme& colours)
{
	DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonTextBackColour, colours.buttonBorderColour, colours.buttonGradColour,
									colours.buttonPressedBackColour, colours.buttonPressedGradColour, colours.pal);
	tabControl = AddTextButton(rowTabs, 0, 4, strings->control, evTabControl, nullptr);
	tabStatus = AddTextButton(rowTabs, 1, 4, strings->status, evTabStatus, nullptr);
	tabMsg = AddTextButton(rowTabs, 2, 4, strings->console, evTabMsg, nullptr);
	tabSetup = AddTextButton(rowTabs, 3, 4, strings->setup, evTabSetup, nullptr);
}

static void CreateMainPages(uint32_t language, const ColourScheme& colours)
{
	if (language >= ARRAY_SIZE(LanguageTables))
	{
		language = 0;
	}
	emptyRoot = mgr.GetRoot();
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
	CreateScreensaverPopup();
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

	static void ClearAlertOrResponse();

	// Return the number of supported languages
	unsigned int GetNumLanguages()
	{
		return NumLanguages;
	}

	void InitColourScheme(const ColourScheme *scheme)
	{
		colours = scheme;
	}

	// Create all the fields we ever display
	void CreateFields(uint32_t language, const ColourScheme& colours, uint32_t p_infoTimeout)
	{
		infoTimeout = p_infoTimeout;

		// Set up default colours and margins
		mgr.Init(colours.defaultBackColour);
		DisplayField::SetDefaultFont(DEFAULT_FONT);
		ButtonWithText::SetFont(DEFAULT_FONT);
		CharButtonRow::SetFont(DEFAULT_FONT);
		SingleButton::SetTextMargin(textButtonMargin);
		SingleButton::SetIconMargin(iconButtonMargin);

		// Create the pages
		CreateMainPages(language, colours);

		// Create the popup fields
		CreateIntegerAdjustPopup(colours);
		CreateIntegerRPMAdjustPopup(colours);
		CreateMovePopup(colours);
		CreateExtrudePopup(colours);
		fileListPopup = CreateFileListPopup(filesListButtons, filenameButtons, NumFileRows, NumFileColumns, colours, true);
		macrosPopup = CreateFileListPopup(macrosListButtons, macroButtons, NumMacroRows, NumMacroColumns, colours, false);
		CreateFileActionPopup(colours);
		CreateVolumePopup(colours);
		CreateInfoTimeoutPopup(colours);
		CreateScreensaverTimeoutPopup(colours);
		CreateBabystepAmountPopup(colours);
		CreateFeedrateAmountPopup(colours);
		CreateBaudRatePopup(colours);
		CreateColoursPopup(colours);
		CreateAreYouSurePopup(colours);
		CreateKeyboardPopup(language, colours);
		CreateLanguagePopup(colours);
		alertPopup = new AlertPopup(colours);
		CreateBabystepPopup(colours);

		DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
		touchCalibInstruction = new StaticTextField(DisplayY/2 - 10, 0, DisplayX, TextAlignment::Centre, strings->touchTheSpot);

		mgr.SetRoot(nullptr);

#ifdef SUPPORT_ENCODER
		encoder = new RotaryEncoder(2, 3, 32+6);			// PA2, PA3 and PB6
		encoder->Init(4);
#endif
	}

	// This is called when no job is active/paused
	void ShowFilesButton()
	{
		// First hide everything removed then show everything new
		// otherwise remnants of the to-be-hidden might remain
		mgr.Show(resumeButton,		false);
		mgr.Show(cancelButton,		false);
		mgr.Show(pauseButton,		false);
		mgr.Show(printProgressBar,	false);

		mgr.Show(babystepButton,	true);
		mgr.Show(reprintButton,		lastJobFileNameAvailable);
		mgr.Show(filesButton,		true);
	}

	// This is called when a job is active
	void ShowPauseButton()
	{
		// First hide everything removed then show everything new
		// otherwise remnants of the to-be-hidden might remain
		mgr.Show(resumeButton,		false);
		mgr.Show(cancelButton,		false);
		mgr.Show(filesButton,		false);
		mgr.Show(reprintButton,		false);

		mgr.Show(pauseButton,		true);
		mgr.Show(babystepButton,	true);
		mgr.Show(printProgressBar,	true);
	}

	// This is called when a job is paused
	void ShowResumeAndCancelButtons()
	{
		// First hide everything removed then show everything new
		// otherwise remnants of the to-be-hidden might remain
		mgr.Show(filesButton,		false);
		mgr.Show(pauseButton,		false);
		mgr.Show(reprintButton,		false);
		mgr.Show(babystepButton,	false);

		mgr.Show(cancelButton,		true);
		mgr.Show(resumeButton,		true);
		mgr.Show(printProgressBar,	true);
	}

	// Show or hide an axis on the move button grid and on the axis display
	void ShowAxis(size_t slot, bool b, const char* axisLetter)
	{
		if (slot >= MaxDisplayableAxes)
		{
			return;
		}
		// The table gives us a pointer to the label field, which is followed by 8 buttons. So we need to show or hide 9 fields.
		DisplayField *f = moveAxisRows[slot];
		for (int i = 0; i < 9 && f != nullptr; ++i)
		{
			mgr.Show(f, b);
			if (i > 0) // actual move buttons
			{
				TextButtonForAxis *textButton = static_cast<TextButtonForAxis*>(f);
				textButton->SetAxisLetter(axisLetter[0]);
			}
			f = f->next;
		}
		mgr.Show(controlTabAxisPos[slot], b);
#if DISPLAY_X == 800
		mgr.Show(printTabAxisPos[slot], b);
#endif
		if (numDisplayedAxes < MaxDisplayableAxes)
		{
			mgr.Show(movePopupAxisPos[slot], b);		// the move popup axis positions occupy the last axis row of the move popup
		}
		else
		{
			// This is incremental and we might end up that this row is no longer available
			for (size_t i = 0; i < MaxDisplayableAxes; ++i)
			{
				mgr.Show(movePopupAxisPos[i], false);
			}
		}
	}

	void UpdateAxisPosition(size_t axisIndex, float fval)
	{
		if (axisIndex < MaxTotalAxes)
		{
			auto axis = OM::GetAxis(axisIndex);
			if (axis != nullptr && axis->slot < MaxDisplayableAxes)
			{
				size_t slot = axis->slot;
				controlTabAxisPos[slot]->SetValue(fval);
#if DISPLAY_X == 800
				printTabAxisPos[slot]->SetValue(fval);
#endif
				movePopupAxisPos[slot]->SetValue(fval);
			}
		}
	}

	void UpdateCurrentTemperature(size_t heaterIndex, float fval)
	{
		OM::Slots heaterSlots;
		OM::GetHeaterSlots(heaterIndex, heaterSlots);
		if (!heaterSlots.IsEmpty())
		{
			const size_t count = heaterSlots.Size();
			for (size_t i = 0; i < count; ++i)
			{
				currentTemps[heaterSlots[i]]->SetValue(fval);
			}

			heaterSlots.Clear();
		}
	}

	void UpdateHeaterStatus(const size_t heaterIndex, const OM::HeaterStatus status)
	{
		OM::Slots heaterSlots;
		OM::GetHeaterSlots(heaterIndex, heaterSlots);
		const Colour foregroundColour =	(status == OM::HeaterStatus::fault)
					? colours->errorTextColour
					: colours->infoTextColour;
		const Colour backgroundColour =
					  (status == OM::HeaterStatus::standby) ? colours->standbyBackColour
					: (status == OM::HeaterStatus::active)  ? colours->activeBackColour
					: (status == OM::HeaterStatus::fault)   ? colours->errorBackColour
					: (status == OM::HeaterStatus::tuning)  ? colours->tuningBackColour
					: colours->defaultBackColour;
		if (!heaterSlots.IsEmpty())
		{
			const Colour bedOrChamberBgColor = (backgroundColour == colours->defaultBackColour)
				? colours->buttonImageBackColour
				: backgroundColour;
			const size_t count = heaterSlots.Size();
			for (size_t i = 0; i < count; ++i)
			{
				const size_t slot = heaterSlots[i];
				currentTemps[slot]->SetColours(foregroundColour, backgroundColour);

				// If it's a bed or a chamber we update colors for the tool button as well
				OM::IterateBedsWhile([&heaterIndex, &status, &foregroundColour, &bedOrChamberBgColor, &slot](OM::Bed*& bed, size_t) {
					if (bed->heater == (int)heaterIndex)
					{
						bed->heaterStatus = status;
						toolButtons[slot]->SetColours(foregroundColour, bedOrChamberBgColor);
						return false;	// This will lead to getting out of the iteration on first hit - is that really what we want?
					}
					return true;
				});
				OM::IterateChambersWhile([&heaterIndex, &status, &foregroundColour, &bedOrChamberBgColor, &slot](OM::Chamber*& chamber, size_t) {
					if (chamber->heater == (int)heaterIndex)
					{
						chamber->heaterStatus = status;
						toolButtons[slot]->SetColours(foregroundColour, bedOrChamberBgColor);
						return false;	// This will lead to getting out of the iteration on first hit - is that really what we want?
					}
					return true;
				});
			}
			heaterSlots.Clear();
		}
	}

	void SetCurrentTool(int32_t ival)
	{
		if (ival == currentTool)
		{
			return;
		}
		currentTool = ival;
	}

	enum TimesLeft { file, filament, slicer, max };
	static int timesLeft[TimesLeft::max];
	static uint32_t simulatedTime;
	static uint32_t jobDuration;
	static uint32_t jobWarmUpDuration;
	static String<50> timesLeftText;

	static const char *GetStatusString(OM::PrinterStatus status)
	{
		unsigned int index = (unsigned int)status;
		if (index >= ARRAY_SIZE(strings->statusValues) || !strings->statusValues[index])
		{
			return "unknown status";
		}

		return strings->statusValues[index];
	}

	void ChangeStatus(OM::PrinterStatus oldStatus, OM::PrinterStatus newStatus)
	{

		if (oldStatus != newStatus)
		{
			const char *fromStatus = GetStatusString(oldStatus);
			const char *toStatus = GetStatusString(newStatus);

			MessageLog::AppendMessageF(MessageLog::LogLevel::Verbose,
					"Info: status changed from %s to %s.", fromStatus, toStatus);
		}

		switch (newStatus)
		{
		case OM::PrinterStatus::printing:
		case OM::PrinterStatus::simulating:
			if (oldStatus != OM::PrinterStatus::paused && oldStatus != OM::PrinterStatus::resuming)
			{
				// Starting a new print, so clear the times
				timesLeft[0] = timesLeft[1] = timesLeft[2] = 0;
				simulatedTime = 0;
			}
			SetLastFileSimulated(newStatus == OM::PrinterStatus::simulating);
			if (oldStatus != newStatus)
			{
				PrintStarted();
			}
			[[fallthrough]];
		case OM::PrinterStatus::paused:
		case OM::PrinterStatus::pausing:
		case OM::PrinterStatus::resuming:
			if (currentTab == tabStatus)
			{
				nameField->SetValue(printingFile.c_str());
			}
			break;

		case OM::PrinterStatus::idle:
			printingFile.Clear();
			nameField->SetValue(machineName.c_str());		// if we are on the print tab then it may still be set to the file that was being printed
			if (IsPrintingStatus(oldStatus))
			{
				mgr.Refresh(true);		// Ending a print creates a popup and that will prevent removing some of the elements hidden so force it here
			}
			[[fallthrough]];
		case OM::PrinterStatus::configuring:
			if (oldStatus == OM::PrinterStatus::flashing)
			{
				mgr.ClearAllPopups();						// clear the firmware update message
			}
			break;

		case OM::PrinterStatus::connecting:
			printingFile.Clear();
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
			timesLeftText.cat(strings->notAvailable);
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

	void UpdateTimesLeftText()
	{
		if (!PrintInProgress())
		{
			return;
		}
		size_t count = 0;
		timesLeftText.Clear();
		if (simulatedTime > 0)
		{
			timesLeftText.copy(strings->simulated);
			AppendTimeLeft(simulatedTime + jobWarmUpDuration - jobDuration);
			++count;
		}
		if (timesLeft[TimesLeft::slicer] > 0)
		{
			if (count > 0) {
				timesLeftText.cat(", ");
			}
			timesLeftText.cat(strings->slicer);
			AppendTimeLeft(timesLeft[TimesLeft::slicer]);
			++count;
		}
		if ((count < 2 || (DisplayX >= 800 && count < 3)) && timesLeft[TimesLeft::filament] > 0)
		{
			if (count > 0) {
				timesLeftText.cat(", ");
			}
			timesLeftText.cat(strings->filament);
			AppendTimeLeft(timesLeft[TimesLeft::filament]);
			++count;
		}
		if ((count < 2 || (DisplayX >= 800 && count < 3)) && timesLeft[TimesLeft::file] > 0)
		{
			if (count > 0) {
				timesLeftText.cat(", ");
			}
			timesLeftText.cat(strings->file);
			AppendTimeLeft(timesLeft[TimesLeft::file]);
			++count;
		}

		timeLeftField->SetValue(timesLeftText.c_str());
		mgr.Show(timeLeftField, true);
	}

	void UpdateTimesLeft(size_t index, unsigned int seconds)
	{
		if (index < (int)ARRAY_SIZE(timesLeft))
		{
			timesLeft[index] = seconds;
			UpdateTimesLeftText();
		}
	}

	void UpdateDuration(uint32_t duration)
	{
		jobDuration = duration;
		UpdateTimesLeftText();
	}

	void UpdateWarmupDuration(uint32_t warmupDuration)
	{
		jobWarmUpDuration = warmupDuration;
		UpdateTimesLeftText();
	}

	void SetSimulatedTime(uint32_t simdTime)
	{
		simulatedTime = simdTime;
		UpdateTimesLeftText();
	}

	void SwitchToTab(ButtonBase *newTab) {
		switch (newTab->GetEvent()) {
		case evTabControl:
			mgr.SetRoot(controlRoot);
			nameField->SetValue(machineName.c_str());
			break;
		case evTabStatus:
			mgr.SetRoot(printRoot);
			nameField->SetValue(
					PrintInProgress() ? printingFile.c_str() : machineName.c_str());
			break;
		case evTabMsg:
			mgr.SetRoot(messageRoot);
			if (keyboardIsDisplayed) {
				keyboardDataHandler = SendGcode;
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

	// Change to the page indicated. Return true if the page has a permanently-visible button.
	static bool ChangePage(ButtonBase *newTab)
	{
		if (newTab == currentTab)
		{
			mgr.ClearAllPopups();						// if already on the correct page, just clear popups
		}
		else
		{
			if (currentTab != nullptr)
			{
				currentTab->Press(false, 0);			// remove highlighting from the old tab
				if (currentTab->GetEvent() == evTabSetup && nvData.IsSaveNeeded())
				{
					SaveSettings();						// leaving the Control tab and we have changed settings, so save them
				}
			}
			newTab->Press(true, 0);						// highlight the new tab
			currentTab = newTab;
			mgr.ClearAllPopups();
			SwitchToTab(newTab);
		}
		return true;
	}

	void ActivateScreensaver()
	{
		mgr.Show(screensaverText, isLandscape);
		mgr.SetPopup(screensaverPopup);
		lastScreensaverMoved = SystemTick::GetTickCount();
	}

	bool DeactivateScreensaver()
	{
		if (!screensaverPopup->IsPopupActive())
			return false;

		mgr.ClearPopup(true, screensaverPopup);

		return true;
	}

	void AnimateScreensaver()
	{
		if (SystemTick::GetTickCount() - lastScreensaverMoved >= ScreensaverMoveTime)
		{
			static unsigned int seed = SystemTick::GetTickCount();
			const PixelNumber width = isLandscape ? DisplayX : DisplayXP;
			const PixelNumber height = isLandscape ? DisplayY : DisplayYP;
			const PixelNumber availableWidth = (width - 2*margin - screensaverTextWidth);
			const PixelNumber availableHeight = (height - 2*margin - rowTextHeight);
			const PixelNumber x = (rand_r(&seed) % availableWidth);
			const PixelNumber y = (rand_r(&seed) % availableHeight);
			if (isLandscape)
			{
				mgr.Show(screensaverText, false);
				screensaverText->SetPosition(x + margin, y + margin);
				mgr.Show(screensaverText, true);
			}
			lastScreensaverMoved = SystemTick::GetTickCount();
		}
	}

	// Pop up the keyboard
	void ShowKeyboard()
	{
		keyboardDataHandler = SendGcode;
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
	bool IsSetupTab()
	{
		return currentTab == tabSetup;			// don't poll while we are on the Setup page
	}

	void Tick()
	{
#ifdef SUPPORT_ENCODER
		encoder->Poll();
#endif
	}

#ifdef SUPPORT_ENCODER
	void HandleEncoderChange(const int change)
	{
		bool sent = false;
		if (sent) {
			lastEncoderCommandSentAt = SystemTick::GetTickCount();
		}
	}
#endif

	// This is called in the main spin loop
	void Spin()
	{
#ifdef SUPPORT_ENCODER
		if (SystemTick::GetTickCount() - lastEncoderCommandSentAt >= MinimumEncoderCommandInterval)
		{
			// Check encoder and command movement
			const int ch = encoder->GetChange();
			if (ch != 0)
			{
				HandleEncoderChange(ch);
			}
		}
#endif

		if (currentTab == tabMsg)
		{
			MessageLog::UpdateMessages(false);
		}
		if (alertTicks != 0 && SystemTick::GetTickCount() - whenAlertReceived >= alertTicks)
		{
			ClearAlertOrResponse();
		}
	}

	// This is called when we have just started a file print
	void PrintStarted()
	{
		if (isLandscape)
		{
			ChangePage(tabStatus);
		}
	}

	// This is called when we have just received the name of the file being printed
	void PrintingFilenameChanged(const char data[])
	{
		if (!printingFile.Similar(data))
		{
			printingFile.copy(data);
			if (currentTab == tabStatus && PrintInProgress())
			{
				nameField->SetChanged();
			}
		}
	}

	void LastJobFileNameAvailable(const bool available)
	{
		lastJobFileNameAvailable = available;
		if (!PrintInProgress())
		{
			mgr.Show(reprintButton, available);
		}
	}

	void SetLastFileSimulated(const bool lastFileSimulated)
	{
		TextButton* redoButton = static_cast<TextButton*>(reprintButton);
		redoButton->SetEvent(lastFileSimulated ? evResimulate : evReprint, 0);
		redoButton->SetText(lastFileSimulated ? strings->resimulate : strings->reprint);
	}

	// This is called just before the main polling loop starts. Display the default page.
	void ShowDefaultPage()
	{
		ChangePage(tabControl);
	}

	// Update the fields that are to do with the printing status
	void UpdatePrintingFields()
	{
		OM::PrinterStatus status = GetStatus();
		if (status == OM::PrinterStatus::printing || status == OM::PrinterStatus::simulating)
		{
			ShowPauseButton();
		}
		else if (status == OM::PrinterStatus::paused)
		{
			ShowResumeAndCancelButtons();
		}
		else
		{
			ShowFilesButton();
		}

		// Don't enable the time left field when we start printing, instead this will get enabled when we receive a suitable message
		if (!PrintInProgress())
		{
			mgr.Show(timeLeftField, false);
		}

		const OM::PrinterStatus stat = GetStatus();
		statusField->SetValue(((unsigned int)stat < ARRAY_SIZE(strings->statusValues) && strings->statusValues[(unsigned int)stat]) ? strings->statusValues[(unsigned int)stat] : "unknown status");
	}

	// Set the percentage of print completed
	void SetPrintProgressPercent(unsigned int percent)
	{
		printProgressBar->SetPercent((uint8_t)percent);
	}

	// Update the geometry or the number of axes
	void UpdateGeometry(unsigned int p_numAxes, bool p_isDelta)
	{
		if (p_numAxes != numVisibleAxes || p_isDelta != isDelta)
		{
			numVisibleAxes = p_numAxes;
			isDelta = p_isDelta;
			FileManager::RefreshMacrosList();
			numDisplayedAxes = 0;
			OM::IterateAxesWhile([](OM::Axis*& axis, size_t)
			{
				axis->slot = MaxTotalAxes;
				if (!axis->visible)
				{
					return true;
				}
				const char * letter = axis->letter;
				if (numDisplayedAxes < MaxDisplayableAxes)
				{
					axis->slot = numDisplayedAxes;
					++numDisplayedAxes;

					// Update axis letter everywhere we display it
					const uint8_t slot = axis->slot;
					controlTabAxisPos	[slot]->SetLabel(letter);
					moveAxisRows		[slot]->SetValue(letter);
#if DISPLAY_X == 800
					printTabAxisPos		[slot]->SetLabel(letter);
#endif
					movePopupAxisPos	[slot]->SetLabel(letter);
					homeButtons			[slot]->SetText(letter);

					// Update axis letter to be sent for homing commands
					homeButtons[slot]->SetEvent(homeButtons[slot]->GetEvent(), letter);
					homeButtons[slot]->SetColours(colours->buttonTextColour, (axis->homed) ? colours->homedButtonBackColour : colours->notHomedButtonBackColour);

					mgr.Show(homeButtons[slot], !isDelta);
					ShowAxis(slot, true, axis->letter);
				}
				// When we get here it's likely to be the initialisation phase
				// and we won't have the babystep amount set
				if (axis->letter[0] == 'Z')
				{
					babystepOffsetField->SetValue(axis->babystep);
				}
				return true;
			});
			// Hide axes possibly shown before
			for (size_t i = numDisplayedAxes; i < MaxDisplayableAxes; ++i)
			{
				mgr.Show(homeButtons[i], false);
				ShowAxis(i, false);
			}
		}
	}

	void UpdateAllHomed()
	{
		bool allHomed = true;
		OM::IterateAxesWhile([&allHomed](OM::Axis*& axis, size_t) {
			if (axis->visible && !axis->homed)
			{
				allHomed = false;
				return false;
			}
			return true;
		});
		if (allHomed != allAxesHomed)
		{
			allAxesHomed = allHomed;
			homeAllButton->SetColours(colours->buttonTextColour, (allAxesHomed) ? colours->homedButtonBackColour : colours->notHomedButtonBackColour);
		}
	}

	// Update the homed status of the specified axis. If the axis is -1 then it represents the "all homed" status.
	void UpdateHomedStatus(size_t axisIndex, bool isHomed)
	{
		OM::Axis *axis = OM::GetOrCreateAxis(axisIndex);
		if (axis == nullptr)
		{
			return;
		}
		axis->homed = isHomed;
		const size_t slot = axis->slot;
		if (slot < MaxDisplayableAxes)
		{
			homeButtons[slot]->SetColours(colours->buttonTextColour, (isHomed) ? colours->homedButtonBackColour : colours->notHomedButtonBackColour);
		}

		UpdateAllHomed();
	}

	// Update the Z probe text
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

	// Update the IP address fiels on Setup tab
	void UpdateIP(const char data[])
	{
		ipAddress.copy(data);
		ipAddressField->SetChanged();
	}

	// Update the fan RPM
	void UpdateFanPercent(size_t fanIndex, int rpm)
	{
		if (currentTool == NoTool)
		{
			if (fanIndex == 0)
			{
				UpdateField(fanSpeed, rpm);
			}
		}
		else
		{
			// There might be multiple tools using the same fan and one of them might
			// be the active one but not necessarily the first one so we need to iterate
			OM::IterateToolsWhile([&fanIndex, &rpm](OM::Tool*& tool, size_t) {
				if (tool->index == currentTool && tool->fans.IsBitSet(fanIndex) && tool->fans.LowestSetBit() == fanIndex)
				{
					UpdateField(fanSpeed, rpm);
				}
				return true;
			});
		}
	}

	void UpdateToolTemp(size_t toolIndex, size_t toolHeaterIndex, int32_t temp, bool active)
	{
		OM::Tool *tool = OM::GetOrCreateTool(toolIndex);

		// If we do not handle this tool back off
		if (tool == nullptr)
		{
			return;
		}

		tool->UpdateTemp(toolHeaterIndex, temp, active);
		if (toolHeaterIndex == 0 || nvData.GetHeaterCombineType() == HeaterCombineType::notCombined)
		{
			if (tool->slot + toolHeaterIndex < MaxSlots)
			{
				UpdateField((active ? activeTemps : standbyTemps)[tool->slot + toolHeaterIndex], temp);
			}
		}
	}

	void UpdateTemperature(size_t heaterIndex, int ival, IntegerButton** fields)
	{
		OM::Slots heaterSlots;
		OM::GetHeaterSlots(heaterIndex, heaterSlots, false);	// Ignore tools
		if (!heaterSlots.IsEmpty())
		{
			const size_t count = heaterSlots.Size();
			for (size_t i = 0; i < count; ++i)
			{
				UpdateField(fields[heaterSlots[i]], ival);
			}

			heaterSlots.Clear();
		}
	}

	// Update an active temperature
	void UpdateActiveTemperature(size_t index, int ival)
	{
		UpdateTemperature(index, ival, activeTemps);
	}

	// Update a standby temperature
	void UpdateStandbyTemperature(size_t index, int ival)
	{
		UpdateTemperature(index, ival, standbyTemps);
	}

	// Update an extrusion factor
	void UpdateExtrusionFactor(size_t index, int ival)
	{
		OM::IterateToolsWhile([&index, &ival](OM::Tool*& tool, size_t) {
			if (tool->extruders.IsBitSet(index) && tool->slot < MaxSlots)
			{
				UpdateField(extrusionFactors[tool->slot], ival);
			}
			return tool->slot < MaxSlots;
		});
	}

	// Update the print speed factor
	void UpdateSpeedPercent(int ival)
	{
		UpdateField(spd, ival);
	}

	// Process a new message box alert, clearing any existing one
	void ProcessAlert(const Alert& alert)
	{
		if (isLandscape)
		{
			alertPopup->Set(alert);
			mgr.SetPopup(alertPopup, AutoPlace, AutoPlace);
		}
		alertMode = alert.mode;
		displayingResponse = false;
		whenAlertReceived = SystemTick::GetTickCount();
		alertTicks = (alertMode < 2) ? (uint32_t)(alert.timeout * 1000.0) : 0;
	}

	// Process a command to clear a message box alert
	void ClearAlert()
	{
		if (alertMode >= 0)
		{
			alertTicks = 0;
			mgr.ClearPopup(true, alertPopup);
			CurrentAlertModeClear();
			alertMode = -1;
		}
	}

	// Clear a message box alert or response. Called when the user presses the close button or the alert or response times out.
	void ClearAlertOrResponse()
	{
		if (alertMode >= 0 || displayingResponse)
		{
			alertTicks = 0;
			mgr.ClearPopup(true, alertPopup);
			CurrentAlertModeClear();
			alertMode = -1;
			displayingResponse = false;
		}
	}

	bool CanDimDisplay()
	{
		return alertMode < 2;
	}

	void ProcessSimpleAlert(const char* _ecv_array text)
	{
		if (alertMode < 2)												// if the current alert doesn't require acknowledgement
		{
			if (isLandscape)
			{
				alertPopup->Set(strings->message, text, 1, 0);
				mgr.SetPopup(alertPopup, AutoPlace, AutoPlace);
			}
			alertMode = 1;												// a simple alert is like a mode 1 alert without a title
			displayingResponse = false;
			whenAlertReceived = SystemTick::GetTickCount();
			alertTicks = 0;												// no timeout
		}
	}

	// Process a new response. This is treated like a simple alert except that it times out and isn't cleared by a "clear alert" command from the host.
	void NewResponseReceived(const char* _ecv_array text)
	{
		const bool isErrorMessage = StringStartsWith(text, "Error");
		if (   alertMode < 2											// if the current alert doesn't require acknowledgement
			&& !(currentTab == tabSetup || currentTab == tabMsg)		// don't show on setup tab or on console tab
			&& (isErrorMessage || infoTimeout != 0)
		   )
		{
			if (isLandscape)
			{
				alertPopup->Set(strings->response, text, 1, 0);
				mgr.SetPopup(alertPopup, AutoPlace, AutoPlace);
			}
			alertMode = -1;												// make sure that a call to ClearAlert doesn't clear us
			displayingResponse = true;
			whenAlertReceived = SystemTick::GetTickCount();
			alertTicks = isErrorMessage ? 0 : infoTimeout * SystemTick::TicksPerSecond;				// time out if it isn't an error message
		}
	}

	// This is called when the user selects a new file from a list of SD card files
	void FileSelected(const char * _ecv_array null fileName)
	{
		fpNameField->SetValue(fileName);
		// Clear out the old field values, they relate to the previous file we looked at until we process the response
		fpSizeField->SetValue(0);						// would be better to make it blank
		fpHeightField->SetValue(0.0);					// would be better to make it blank
		fpLayerHeightField->SetValue(0.0);				// would be better to make it blank
		fpFilamentField->SetValue(0);					// would be better to make it blank
		generatedByText.Clear();
		fpGeneratedByField->SetChanged();
		lastModifiedText.Clear();
		fpLastModifiedField->SetChanged();
		printTimeText.Clear();
		fpPrintTimeField->SetChanged();
	}

	// This is called when the "generated by" file information has been received
	void UpdateFileGeneratedByText(const char data[])
	{
		generatedByText.copy(data);
		fpGeneratedByField->SetChanged();
	}

	// This is called when the "last modified" file information has been received
	void UpdateFileLastModifiedText(const char data[])
	{
		lastModifiedText.copy(data);
		lastModifiedText.Replace('T', ' ');
		lastModifiedText.Replace('+', '\0');		// ignore time zone if present
		lastModifiedText.Replace('.', '\0');		// ignore decimal seconds if present (DCS 2.0.0 sends them)
		fpLastModifiedField->SetChanged();
	}

	// This is called when the "last modified" file information has been received
	void UpdatePrintTimeText(uint32_t seconds, bool isSimulated)
	{
		bool update = false;
		if (isSimulated)
		{
			printTimeText.Clear();					// prefer simulated to estimated print time
			fpPrintTimeField->SetLabel(strings->simulatedPrintTime);
			update = true;
		}
		else if (printTimeText.IsEmpty())
		{
			fpPrintTimeField->SetLabel(strings->estimatedPrintTime);
			update = true;
		}
		if (update)
		{
			unsigned int minutes = (seconds + 50)/60;
			printTimeText.printf("%dh %02dm", minutes / 60, minutes % 60);
			fpPrintTimeField->SetChanged();
		}
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

	bool UpdateFileThumbnailChunk(const struct Thumbnail &thumbnail, uint32_t pixels_offset, const qoi_rgba_t *pixels, size_t pixels_count)
	{
		dbg("offset %d pixels %08x count %d\n", pixels_offset, pixels, pixels_count);
		if (!mgr.IsPopupActive(fileDetailPopup))
		{
			return false;
		}
#define DRAW_TEST 0
#if DRAW_TEST == 1
		qoi_rgba_t pixel[100];

		//memset(pixel, 0xaa, sizeof(pixel));
		for (size_t i = 0; i < ARRAY_SIZE(pixel); i++)
		{
			pixel[i].v = 0;
			pixel[i].rgba.r = 0xaa;
		}

		fpThumbnail->DrawRect(thumbnail.width, thumbnail.height, pixels_offset, pixel, ARRAY_SIZE(pixel));
#elif DRAW_TEST == 2
		int line = 64;
		for (int i = 0; i < line; i++) {
			qoi_rgba_t test_pixels[64];

			for (size_t p = 0; p < ARRAY_SIZE(test_pixels); p++) {
				test_pixels[p].v = 0;
				test_pixels[p].rgba.r = 128 + p;
				test_pixels[p].rgba.g = 64 + i;
			}

			fpThumbnail->DrawRect(ARRAY_SIZE(test_pixels), 1, i * ARRAY_SIZE(test_pixels), test_pixels, ARRAY_SIZE(test_pixels));
		}
#else
		fpThumbnail->DrawRect(thumbnail.width, thumbnail.height, pixels_offset, pixels, pixels_count);
#endif
		return true;
	}

	// Return true if we are displaying file information
	bool IsDisplayingFileInfo()
	{
		return currentFile != nullptr;
	}

	static void DoEmergencyStop()
	{
		// We send M112 for the benefit of old firmware, and F0 0F (an invalid UTF8 sequence) for new firmware
		SerialIo::Sendf("M112 ;" "\xF0" "\x0F" "\n");
		TouchBeep();											// needed when we are called from ProcessTouchOutsidePopup
		Delay(1000);
		SerialIo::Sendf("M999\n");
		Delay(1000);
	}

	// Make this into a template if we need something else than IntegerButton** as list
	size_t GetButtonSlot(IntegerButton** buttonList, ButtonBase* button)
	{
		size_t slot = MaxSlots;
		for (size_t i = 0; i < MaxSlots; ++i)
		{
			if (buttonList[i] == button)
			{
				slot = i;
				break;
			}
		}
		return slot;
	}

	void ProcessRelease(ButtonPress bp)
	{
		if (!bp.IsValid())
		{
			return;
		}

		ButtonBase *f = bp.GetButton();
		Event ev = (Event)(f->GetEvent());

		switch(ev)
		{
		case evTabControl:
		case evTabStatus:
		case evTabMsg:
		case evTabSetup:

		case evExtrudeAmount:
		case evExtrudeRate:

		case evAdjustBaudRate:
		case evAdjustVolume:
		case evAdjustInfoTimeout:
		case evAdjustScreensaverTimeout:
		case evAdjustBabystepAmount:
		case evAdjustFeedrate:
		case evAdjustColours:
		case evAdjustLanguage:
			break;
		case evOkAlert:
		case evCloseAlert:
		case evChoiceAlert:
			mgr.Press(bp, false);
			ClearAlertOrResponse();
			break;
		default:
			mgr.Press(bp, false);
			break;
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


			if (bp.GetEvent() != evAdjustVolume)
			{
				TouchBeep();		// give audible feedback of the touch, unless adjusting the volume
			}

			switch(ev)
			{
			case evEmergencyStop:
				DoEmergencyStop();
				break;

			case evTabControl:
			case evTabStatus:
			case evTabMsg:
			case evTabSetup:
				if (ChangePage(f))
				{
					currentButton.Clear();						// keep the button highlighted after it is released
				}
				break;

			case evAdjustToolActiveTemp:
			case evAdjustToolStandbyTemp:
			case evAdjustBedActiveTemp:
			case evAdjustBedStandbyTemp:
			case evAdjustChamberActiveTemp:
			case evAdjustChamberStandbyTemp:
				if (static_cast<IntegerButton*>(f)->GetValue() < 0)
				{
					static_cast<IntegerButton*>(f)->SetValue(0);
				}
				Adjusting(bp);
				if (isLandscape)
				{
					mgr.SetPopup(setTempPopup, AutoPlace, popupY);
				}
				break;

			case evAdjustActiveRPM:
				Adjusting(bp);
				if (isLandscape)
				{
					mgr.SetPopup(setRPMPopup, AutoPlace, popupY);
				}
				break;

			case evAdjustSpeed:
			case evExtrusionFactor:
			case evAdjustFan:
				oldIntValue = static_cast<IntegerButton*>(bp.GetButton())->GetValue();
				Adjusting(bp);
				if (isLandscape)
				{
					mgr.SetPopup(setTempPopup, AutoPlace, popupY);
				}
				break;

			case evSetInt:
				if (fieldBeingAdjusted.IsValid())
				{
					int val = static_cast<const IntegerButton*>(fieldBeingAdjusted.GetButton())->GetValue();
					const event_t eventOfFieldBeingAdjusted = fieldBeingAdjusted.GetEvent();
					switch (eventOfFieldBeingAdjusted)
					{
					case evAdjustBedActiveTemp:
					case evAdjustChamberActiveTemp:
						{
							int index = fieldBeingAdjusted.GetIParam();
							const bool isBed = eventOfFieldBeingAdjusted == evAdjustBedActiveTemp;
							SerialIo::Sendf("%s P%d S%d\n", isBed ? "M140" : "M141", index, val);
						}
						break;

					case evAdjustBedStandbyTemp:
					case evAdjustChamberStandbyTemp:
						{
							int index = fieldBeingAdjusted.GetIParam();
							const bool isBed = eventOfFieldBeingAdjusted == evAdjustBedStandbyTemp;
							SerialIo::Sendf("%s P%d R%d\n", isBed ? "M140" : "M141", index, val);
						}
						break;

					case evAdjustToolActiveTemp:
						{
							int toolNumber = fieldBeingAdjusted.GetIParam();
							OM::Tool* tool = OM::GetTool(toolNumber);
							if (tool == nullptr)
							{
								break;
							}

							const bool useM568 = GetFirmwareFeatures().IsBitSet(m568TempAndRPM);
							if (nvData.GetHeaterCombineType() == HeaterCombineType::combined)
							{
								tool->UpdateTemp(0, val, true);
								SerialIo::Sendf("%s P%d S%d\n", (useM568 ? "M568" : "G10"), toolNumber, tool->heaters[0]->activeTemp);
							}
							else
							{

								// Find the slot for this button to determine which heater index it is
								{
									size_t slot = GetButtonSlot(activeTemps, fieldBeingAdjusted.GetButton());
									if (slot >= MaxSlots || (slot - tool->slot) >= MaxSlots)
									{
										break;
									}
									tool->UpdateTemp(slot - tool->slot, val, true);
								}

								String<maxUserCommandLength> heaterTemps;
								if (tool->GetHeaterTemps(heaterTemps.GetRef(), true))
								{
									SerialIo::Sendf("%s P%d S%s\n", (useM568 ? "M568" : "G10"), toolNumber, heaterTemps.c_str());
								}
							}
						}
						break;

					case evAdjustToolStandbyTemp:
						{
							int toolNumber = fieldBeingAdjusted.GetIParam();
							OM::Tool* tool = OM::GetTool(toolNumber);
							if (tool == nullptr)
							{
								break;
							}

							const bool useM568 = GetFirmwareFeatures().IsBitSet(m568TempAndRPM);
							if (nvData.GetHeaterCombineType() == HeaterCombineType::combined)
							{
								tool->UpdateTemp(0, val, false);
								SerialIo::Sendf("%s P%d R%d\n", (useM568 ? "M568" : "G10"), toolNumber, tool->heaters[0]->standbyTemp);
							}
							else
							{

								// Find the slot for this button to determine which heater index it is
								{
									size_t slot = GetButtonSlot(standbyTemps, fieldBeingAdjusted.GetButton());
									if (slot >= MaxSlots || (slot - tool->slot) >= MaxSlots)
									{
										break;
									}
									tool->UpdateTemp(slot - tool->slot, val, false);
								}

								String<maxUserCommandLength> heaterTemps;
								if (tool->GetHeaterTemps(heaterTemps.GetRef(), false))
								{
									SerialIo::Sendf("%s P%d R%s\n", (useM568 ? "M568" : "G10"), toolNumber, heaterTemps.c_str());
								}
							}
						}
						break;

					case evAdjustActiveRPM:
						{
							auto spindle = OM::GetSpindle(fieldBeingAdjusted.GetIParam());
							if (val == 0)
							{
								SerialIo::Sendf("M5 P%d\n", spindle->index);
							}
							else
							{
								SerialIo::Sendf("M%d P%d S%d\n", val < 0 ? 4 : 3, spindle->index, abs(val));
							}
						}
						break;

					case evExtrusionFactor:
						{
							const int extruder = fieldBeingAdjusted.GetIParam();
							SerialIo::Sendf("M221 D%d S%d\n", extruder, val);
						}
						break;

					case evAdjustFan:
						SerialIo::Sendf("M106 S%d\n", (256 * val)/100);
						break;

					default:
						{
							const char* null cmd = fieldBeingAdjusted.GetSParam();
							if (cmd != nullptr)
							{
								SerialIo::Sendf("%s%d\n", cmd, val);
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
					const int change = bp.GetIParam();
					int newValue = ib->GetValue() + change;
					switch(fieldBeingAdjusted.GetEvent())
					{
					case evAdjustToolActiveTemp:
					case evAdjustToolStandbyTemp:
					case evAdjustBedActiveTemp:
					case evAdjustBedStandbyTemp:
					case evAdjustChamberActiveTemp:
					case evAdjustChamberStandbyTemp:
						newValue = constrain<int>(newValue, 0, 1600);		// some users want to print at high temperatures
						break;

					case evAdjustFan:
						newValue = constrain<int>(newValue, 0, 100);
						break;

					case evAdjustActiveRPM:
						{
							auto spindle = OM::GetSpindle(fieldBeingAdjusted.GetIParam());
							newValue = constrain<int>(newValue, -spindle->max, spindle->max);

							// If a change will lead us below the min speed for spindle skip to the other side
							if (newValue > (int)-spindle->min && newValue < (int)spindle->min)
							{
								newValue = (change < 0) ? -spindle->min : spindle->min;
							}
						}
						break;

					default:
						break;
					}
					ib->SetValue(newValue);
				}
				break;

			case evMovePopup:
				mgr.SetPopup(movePopup, AutoPlace, AutoPlace);
				break;

			case evMoveSelectAxis:
				{
					alertPopup->ChangeLetter(bp.GetIParam());
				}
				break;
			case evMoveAxis:
				{
					TextButtonForAxis *textButton = static_cast<TextButtonForAxis*>(bp.GetButton());
					const char letter = textButton->GetAxisLetter();
					SerialIo::Sendf("G91 G1 %s%c%s F%d G90\n", islower(letter) ? "'" : "", letter, bp.GetSParam(), nvData.GetFeedrate());
				}
				break;

			case evExtrudePopup:
				if (isLandscape)
				{
					mgr.SetPopup(extrudePopup, AutoPlace, AutoPlace);
				}
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
					SerialIo::Sendf("M120 M83 G1 E%s%s F%s M121\n",
							(ev == evRetract ? "-" : ""),
							currentExtrudeAmountPress.GetSParam(),
							currentExtrudeRatePress.GetSParam());
				}
				break;

			case evBabyStepPopup:
				mgr.SetPopup(babystepPopup, AutoPlace, AutoPlace);
				break;

			case evBabyStepMinus:
			case evBabyStepPlus:
				{
					SerialIo::Sendf("M290 Z%s%s\n", (ev == evBabyStepMinus ? "-" : ""), babystepAmounts[nvData.GetBabystepAmountIndex()]);
					float currentBabystepAmount = babystepOffsetField->GetValue();
					if (ev == evBabyStepMinus)
					{
						currentBabystepAmount -= babystepAmountsF[nvData.GetBabystepAmountIndex()];
					}
					else
					{
						currentBabystepAmount += babystepAmountsF[nvData.GetBabystepAmountIndex()];
					}
					babystepOffsetField->SetValue(currentBabystepAmount);
				}
				break;

			case evListFiles:
				FileManager::DisplayFilesList();
				break;

			case evListMacros:
				FileManager::DisplayMacrosList();
				break;

			case evCalTouch:
				CalibrateTouch();
				break;

			case evFactoryReset:
				PopupAreYouSure(ev, strings->confirmFactoryReset);
				break;

			case evSelectBed:
				{
					int bedIndex = bp.GetIParam();
					const OM::Bed* bed = OM::GetBed(bedIndex);
					if (bed == nullptr || bed->slot >= MaxSlots)
					{
						break;
					}
					const auto slot = bed->slot;
					if (bed->heaterStatus == OM::HeaterStatus::active)			// if bed is active
					{
						SerialIo::Sendf("M144 P%d\n", bedIndex);
					}
					else
					{
						SerialIo::Sendf("M140 P%d S%d\n", bedIndex, activeTemps[slot]->GetValue());
					}
				}
				break;

			case evSelectChamber:
				{
					const int chamberIndex = bp.GetIParam();
					const OM::Chamber* chamber = OM::GetChamber(chamberIndex);
					if (chamber == nullptr || chamber->slot >= MaxSlots)
					{
						break;
					}
					const auto slot = chamber->slot;
					SerialIo::Sendf("M141 P%d S%d\n",
							chamberIndex,
							(chamber->heaterStatus == OM::HeaterStatus::active ? -274 : activeTemps[slot]->GetValue()));
				}
				break;

			case evSelectHead:
				{
					int head = bp.GetIParam();
					// pressing a evSeelctHead button in the middle of active printing is almost always accidental (and fatal to the print job)
					if (GetStatus() != OM::PrinterStatus::printing && GetStatus() != OM::PrinterStatus::simulating)
					{
						if (head == currentTool)		// if head is active
						{
							SerialIo::Sendf("T-1\n");
						}
						else
						{
							SerialIo::Sendf("T%d\n", head);
						}
					}
				}
				break;

			case evFile:
				{
					const char * _ecv_array fileName = bp.GetSParam();
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
							FileSelected(currentFile);
							mgr.SetPopup(fileDetailPopup, AutoPlace, AutoPlace);
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
			case evMacroControlPage:
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
							SerialIo::Sendf("M98 P");
							const char * _ecv_array const dir = (ev == evMacroControlPage) ? FileManager::GetMacrosRootDir() : FileManager::GetMacrosDir();
							SerialIo::SendFilename(CondStripDrive(dir), fileName);
							SerialIo::SendChar('\n');
						}
					}
					else
					{
						ErrorBeep();
					}
				}
				break;

			case evPrintFile:
			case evSimulateFile:
				mgr.ClearPopup();			// clear the file info popup
				mgr.ClearPopup();			// clear the file list popup
				if (currentFile != nullptr)
				{
					SerialIo::Sendf((ev == evSimulateFile) ? "M37 P" : "M32 ");
					SerialIo::SendFilename(CondStripDrive(StripPrefix(FileManager::GetFilesDir())), currentFile);
					SerialIo::SendChar('\n');
					PrintingFilenameChanged(currentFile);
					currentFile = nullptr;							// allow the file list to be updated
					CurrentButtonReleased();
					PrintStarted();
				}
				break;

			case evReprint:
			case evResimulate:
				if (lastJobFileNameAvailable)
				{
					SerialIo::Sendf("%s{job.lastFileName}\n", (ev == evResimulate) ? "M37 P" : "M32 ");
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
				CurrentButtonReleased();
				PopupAreYouSure(ev, strings->confirmFileDelete);
				break;

			case evSendCommand:
			case evPausePrint:
			case evResumePrint:
			case evReset:
				SerialIo::Sendf("%s\n", bp.GetSParam());
				break;

			case evHomeAxis:
				{
					const char letter = bp.GetSParam()[0];
					SerialIo::Sendf("G28 %s%c0\n", islower(letter) ? "'" : "", letter);
				}
				break;

			case evScrollFiles:
				FileManager::ScrollFiles(bp.GetIParam() * NumFileRows);
				break;

			case evScrollMacros:
				FileManager::ScrollMacros(bp.GetIParam() * NumMacroRows);
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
				break;

			case evInvertY:
				InvertDisplay();
				CalibrateTouch();
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
				CurrentButtonReleased();
				mgr.ClearPopup();
				StopAdjusting();
				break;

			case evSetVolume:
				Adjusting(bp);
				mgr.SetPopup(volumePopup, AutoPlace, popupY);
				break;

			case evSetInfoTimeout:
				Adjusting(bp);
				mgr.SetPopup(infoTimeoutPopup, AutoPlace, popupY);
				break;

			case evSetScreensaverTimeout:
				Adjusting(bp);
				mgr.SetPopup(screensaverTimeoutPopup, AutoPlace, popupY);
				break;

			case evSetBabystepAmount:
				Adjusting(bp);
				mgr.SetPopup(babystepAmountPopup, AutoPlace, popupY);
				break;

			case evSetFeedrate:
				Adjusting(bp);
				mgr.SetPopup(feedrateAmountPopup, AutoPlace, popupY);
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
				break;

			case evAdjustVolume:
				{
					const int newVolume = bp.GetIParam();
					nvData.SetVolume(newVolume);
					volumeButton->SetValue(newVolume);
				}
				TouchBeep();									// give audible feedback of the touch at the new volume level
				break;

			case evAdjustInfoTimeout:
				{
					infoTimeout = bp.GetIParam();
					nvData.SetInfoTimeout(infoTimeout);
					infoTimeoutButton->SetValue(infoTimeout);
				}
				TouchBeep();									// give audible feedback of the touch at the new volume level
				break;

			case evAdjustScreensaverTimeout:
				{
					uint32_t screensaverTimeout = bp.GetIParam();
					nvData.SetScreensaverTimeout(screensaverTimeout * 1000);
					screensaverTimeoutButton->SetValue(screensaverTimeout);
				}
				TouchBeep();									// give audible feedback of the touch at the new volume level
				break;

			case evAdjustBabystepAmount:
				{
					uint32_t babystepAmountIndex = bp.GetIParam();
					nvData.SetBabystepAmountIndex(babystepAmountIndex);
					babystepAmountButton->SetText(babystepAmounts[babystepAmountIndex]);
					babystepMinusButton->SetText(babystepAmounts[babystepAmountIndex]);
					babystepPlusButton->SetText(babystepAmounts[babystepAmountIndex]);
				}
				TouchBeep();									// give audible feedback of the touch at the new volume level
				break;

			case evAdjustFeedrate:
				{
					uint32_t feedrate = bp.GetIParam();
					nvData.SetFeedrate(feedrate);
					feedrateAmountButton->SetValue(feedrate);
				}
				TouchBeep();									// give audible feedback of the touch at the new volume level
				break;

			case evAdjustColours:
				{
					const uint8_t newColours = (uint8_t)bp.GetIParam();
					if (nvData.SetColourScheme(newColours))
					{
						SaveSettings();
						Reset();
					}
				}
				mgr.ClearPopup();
				break;

			case evSetLanguage:
				Adjusting(bp);
				mgr.SetPopup(languagePopup, AutoPlace, popupY);
				break;

			case evAdjustLanguage:
				{
					const uint8_t newLanguage = (uint8_t)bp.GetIParam();
					if (nvData.SetLanguage(newLanguage))
					{
						SaveSettings();
						Reset();
					}
				}
				mgr.ClearPopup();
				break;

			case evSetDimmingType:
				ChangeDisplayDimmerType();
				dimmingTypeButton->SetText(strings->displayDimmingNames[(unsigned int)nvData.GetDisplayDimmerType()]);
				break;

			case evSetHeaterCombineType:
				ChangeHeaterCombineType();
				heaterCombiningButton->SetText(strings->heaterCombineTypeNames[(unsigned int)nvData.GetHeaterCombineType()]);
				break;

			case evSetLogLevel:
				{
					MessageLog::LogLevel logLevel = MessageLog::LogLevelGet();

					logLevel = (MessageLog::LogLevel)(((int)logLevel + 1) % (int)MessageLog::LogLevel::NumTypes);

					logLevelButton->SetText(strings->logLevelNames[(unsigned int)logLevel]);

					nvData.SetLogLevel(logLevel);

					MessageLog::LogLevelSet(logLevel);
				}
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
						SerialIo::Sendf("M30 ");
						SerialIo::SendFilename(CondStripDrive(StripPrefix(FileManager::GetFilesDir())), currentFile);
						SerialIo::SendChar('\n');
						FileManager::RefreshFilesList();
						currentFile = nullptr;
					}
					break;

				default:
					break;
				}
				eventToConfirm = evNull;
				currentFile = nullptr;
				break;

			case evKey:
				if (!userCommandBuffers[currentUserCommandBuffer].cat((char)bp.GetIParam()))
				{
					userCommandField->SetChanged();
				}
				break;

			case evShift:
				{
					size_t rowOffset;
					if (keyboardShifted)
					{
						bp.GetButton()->Press(false, 0);
						rowOffset = 0;
					}
					else
					{
						rowOffset = 4;
					}
					for (size_t i = 0; i < 4; ++i)
					{
						keyboardRows[i]->ChangeText(currentKeyboard[i + rowOffset]);
					}
				}
				keyboardShifted = !keyboardShifted;
				currentButton.Clear();				// make the key sticky
				break;

			case evBackspace:
				if (!userCommandBuffers[currentUserCommandBuffer].IsEmpty())
				{
					userCommandBuffers[currentUserCommandBuffer].Erase(userCommandBuffers[currentUserCommandBuffer].strlen() - 1);
					userCommandField->SetChanged();
				}
				break;

			case evUp: // TODO new events for moving editor one left or right
				currentHistoryBuffer = (currentHistoryBuffer + numUserCommandBuffers - 1) % numUserCommandBuffers;
				if (currentHistoryBuffer == currentUserCommandBuffer)
				{
					userCommandBuffers[currentUserCommandBuffer].Clear();
				}
				else
				{
					userCommandBuffers[currentUserCommandBuffer].copy(userCommandBuffers[currentHistoryBuffer].c_str());
				}
				userCommandField->SetChanged();
				break;

			case evDown:
				currentHistoryBuffer = (currentHistoryBuffer + 1) % numUserCommandBuffers;
				if (currentHistoryBuffer == currentUserCommandBuffer)
				{
					userCommandBuffers[currentUserCommandBuffer].Clear();
				}
				else
				{
					userCommandBuffers[currentUserCommandBuffer].copy(userCommandBuffers[currentHistoryBuffer].c_str());
				}
				userCommandField->SetChanged();
				break;

			case evSendKeyboardCommand:
				if (userCommandBuffers[currentUserCommandBuffer].strlen() != 0)
				{
					if (keyboardDataHandler)
					{
						keyboardDataHandler(userCommandBuffers[currentUserCommandBuffer].c_str());
					}

					// Add the command to the history if it was different frmo the previous command
					size_t prevBuffer = (currentUserCommandBuffer + numUserCommandBuffers - 1) % numUserCommandBuffers;
					if (strcmp(userCommandBuffers[currentUserCommandBuffer].c_str(), userCommandBuffers[prevBuffer].c_str()) != 0)
					{
						currentUserCommandBuffer = (currentUserCommandBuffer + 1) % numUserCommandBuffers;
					}
					currentHistoryBuffer = currentUserCommandBuffer;
					userCommandBuffers[currentUserCommandBuffer].Clear();
					userCommandField->SetLabel(userCommandBuffers[currentUserCommandBuffer].c_str());
				}
				break;

			case evOkAlert:
				alertPopup->ProcessOkButton();
				break;

			case evCloseAlert:
				SerialIo::Sendf("%s\n", bp.GetSParam());
				break;

			case evChoiceAlert:
				alertPopup->ProcessChoice(bp.GetIParam());
				break;

			case evEditAlert:
				keyboardDataHandler = PopupEditData;
				mgr.SetPopup(keyboardPopup, AutoPlace, keyboardPopupY);
				keyboardIsDisplayed = true;
				break;

			default:
				break;
			}
		}
	}

	// Process a touch event outside the popup on the field being adjusted
	void ProcessTouchOutsidePopup(ButtonPress bp)
	{
		if (!IsSetupTab())
		{
			return;
		}

		if (bp == fieldBeingAdjusted)
		{
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

			case evAdjustToolActiveTemp:
			case evAdjustToolStandbyTemp:
			case evAdjustBedActiveTemp:
			case evAdjustBedStandbyTemp:
			case evAdjustChamberActiveTemp:
			case evAdjustChamberStandbyTemp:
			case evAdjustActiveRPM:
			case evSetBaudRate:
			case evSetVolume:
			case evSetInfoTimeout:
			case evSetScreensaverTimeout:
			case evSetFeedrate:
			case evSetBabystepAmount:
			case evSetColours:
				mgr.ClearPopup();
				StopAdjusting();
				break;

			case evSetLanguage:
				mgr.ClearPopup();
				StopAdjusting();
				break;
			}
		}
		else
		{
			switch(bp.GetEvent())
			{
			case evEmergencyStop:
				mgr.Press(bp, true);
				DoEmergencyStop();
				mgr.Press(bp, false);
				break;

			case evTabControl:
			case evTabStatus:
			case evTabMsg:
			case evTabSetup:
				StopAdjusting();
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
			case evSetInfoTimeout:
			case evSetScreensaverTimeout:
			case evSetFeedrate:
			case evSetBabystepAmount:
			case evSetColours:
			case evSetLanguage:
			case evCalTouch:
			case evInvertX:
			case evInvertY:
			case evFactoryReset:
				// On the Setup tab, we allow any other button to be pressed to exit the current popup
				StopAdjusting();
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

	void DisplayFilesPopup(int cardNumber, unsigned int numVolumes)
	{
		filePopupTitleField->SetValue(cardNumber);
		mgr.Show(changeCardButton, numVolumes > 1);

		if (isLandscape)
		{
			for (size_t i = 0; i < ARRAY_SIZE(filenameButtons); i++)
			{
				filenameButtons[i]->Press(false, 0);
				filenameButtons[i]->Show(false);
			}
			fileListPopupNoFiles->Show(true);
			mgr.SetPopup(fileListPopup, AutoPlace, AutoPlace);
		}
	}

	void FileListCardButtonUpdate(unsigned int numVolumes)
	{
		mgr.Show(changeCardButton, numVolumes > 1);
	}

	void DisplayMacrosPopup()
	{
		if (isLandscape)
		{
			for (size_t i = 0; i < ARRAY_SIZE(macroButtons); i++)
			{
				macroButtons[i]->Press(false, 0);
				macroButtons[i]->Show(false);
			}
			mgr.SetPopup(macrosPopup, AutoPlace, AutoPlace);
		}
	}

	void FileListLoaded(bool filesNotMacros, int errCode)
	{
		FileListButtons& buttons = (filesNotMacros) ? filesListButtons : macrosListButtons;
		if (errCode == 0)
		{
			mgr.Show(buttons.errorField, false);
		}
		else
		{
			buttons.errorField->SetValue(errCode);
			mgr.Show(buttons.errorField, true);
		}
	}

	void EnableFileNavButtons(bool filesNotMacros, bool scrollEarlier, bool scrollLater, bool parentDir)
	{
		FileListButtons& buttons = (filesNotMacros) ? filesListButtons : macrosListButtons;
		mgr.Show(buttons.scrollLeftButton, scrollEarlier);
		mgr.Show(buttons.scrollRightButton, scrollLater);
		mgr.Show(buttons.folderUpButton, parentDir);
	}

	// Update the specified button in the file or macro buttons list. If 'text' is nullptr then hide the button, else display it.
	void UpdateFileButton(bool filesNotMacros, unsigned int buttonIndex, const char * _ecv_array null text, const char * _ecv_array null param)
	{
		if (filesNotMacros && text)
		{
			mgr.Show(fileListPopupNoFiles, false);
		}

		if (text && buttonIndex < ((filesNotMacros) ? NumDisplayedFiles : NumDisplayedMacros))
		{
			TextButton * const f = ((filesNotMacros) ? filenameButtons : macroButtons)[buttonIndex];
			f->SetText(text);
			f->SetEvent((text == nullptr) ? evNull : (filesNotMacros) ? evFile : evMacro, param);
			mgr.Show(f, text != nullptr);
		}
	}

	// Update the specified button in the macro short list. If 'fileName' is nullptr then hide the button, else display it.
	// Return true if this should be called again for the next button.
	bool UpdateMacroShortList(unsigned int buttonIndex, const char * _ecv_array null fileName)
	{
#if (DISPLAY_X == 480)
		const bool tooFewSpace = numToolColsUsed >= (MaxSlots - 1);
#else
		const bool tooFewSpace = numToolColsUsed > (MaxSlots - 2);
#endif

		if (buttonIndex >= ARRAY_SIZE(controlPageMacroButtons) || numToolColsUsed == 0 || tooFewSpace)
		{
			return false;
		}

		String<controlPageMacroTextLength>& str = controlPageMacroText[buttonIndex];
		str.Clear();
		const bool isFile = (fileName != nullptr);
		if (isFile)
		{
			str.copy(fileName);
		}
		TextButton * const f = controlPageMacroButtons[buttonIndex];
		f->SetText(SkipDigitsAndUnderscore(str.c_str()));
		f->SetEvent((isFile) ? evMacroControlPage : evNull, str.c_str());
		mgr.Show(f, isFile);
		return true;
	}

	unsigned int GetNumScrolledFiles(bool filesNotMacros)
	{
		return (filesNotMacros) ? NumFileRows : NumMacroRows;
	}

	void AdjustControlPageMacroButtons()
	{
		const unsigned int n = numToolColsUsed;

		if (n != numHeaterAndToolColumns)
		{
			numHeaterAndToolColumns = n;

			// Adjust the width of the control page macro buttons, or hide them completely if insufficient room
			PixelNumber controlPageMacroButtonsColumn = (PixelNumber)(((tempButtonWidth + fieldSpacing) * n) + bedColumn + fieldSpacing);
			PixelNumber controlPageMacroButtonsWidth = (PixelNumber)((controlPageMacroButtonsColumn >= DisplayX - margin) ? 0 : DisplayX - margin - controlPageMacroButtonsColumn);
			if (controlPageMacroButtonsWidth > maxControlPageMacroButtonsWidth)
			{
				controlPageMacroButtonsColumn += controlPageMacroButtonsWidth - maxControlPageMacroButtonsWidth;
				controlPageMacroButtonsWidth = maxControlPageMacroButtonsWidth;
			}

			bool showControlPageMacroButtons = controlPageMacroButtonsWidth >= minControlPageMacroButtonsWidth;

			for (TextButton *& b : controlPageMacroButtons)
			{
				if (showControlPageMacroButtons)
				{
					b->SetPositionAndWidth(controlPageMacroButtonsColumn, controlPageMacroButtonsWidth);
				}
				mgr.Show(b, showControlPageMacroButtons);
			}

			if (currentTab == tabControl)
			{
				mgr.Refresh(true);
			}
		}
	}

	void ResetToolAndHeaterStates() noexcept
	{
		for (size_t i = 0; i < numToolColsUsed; ++i)
		{
			toolButtons[i]->SetColours(colours->buttonTextColour, colours->buttonImageBackColour);
			currentTemps[i]->SetColours(colours->infoTextColour, colours->defaultBackColour);
		}
	}

	void ManageCurrentActiveStandbyFields(
			size_t& slot,
			const bool showCurrent = false,
			const Event activeEvent = evNull,
			const int activeEventValue = -1,
			const Event standbyEvent = evNull,
			const int standbyEventValue = -1
			)
	{
		mgr.Show(currentTemps[slot], showCurrent);
		mgr.Show(activeTemps[slot], activeEvent != evNull);
		mgr.Show(standbyTemps[slot], standbyEvent != evNull);

		activeTemps[slot]->SetEvent(activeEvent, activeEventValue);
		activeTemps[slot]->SetValue(0);
		standbyTemps[slot]->SetEvent(standbyEvent, standbyEventValue);
		standbyTemps[slot]->SetValue(0);
	}

	size_t AddBedOrChamber(OM::BedOrChamber *bedOrChamber, size_t &slot, const bool isBed = true) {
		const size_t count = (isBed ? OM::GetBedCount() : OM::GetChamberCount());
		bedOrChamber->slot = MaxSlots;
		if (slot < MaxSlots && bedOrChamber->heater > -1) {
			bedOrChamber->slot = slot;
			mgr.Show(toolButtons[slot], true);
			ManageCurrentActiveStandbyFields(
					slot,
					true,
					isBed ? evAdjustBedActiveTemp : evAdjustChamberActiveTemp,
					bedOrChamber->index,
					isBed ? evAdjustBedStandbyTemp : evAdjustChamberStandbyTemp,
					bedOrChamber->index
					);
			mgr.Show(extrusionFactors[slot], false);
			toolButtons[slot]->SetEvent(isBed ? evSelectBed : evSelectChamber, bedOrChamber->index);
			toolButtons[slot]->SetIcon(isBed ? IconBed : IconChamber);
			toolButtons[slot]->SetIntVal(bedOrChamber->index);
			toolButtons[slot]->SetPrintText(count > 1);

			++slot;
		}
		return count;
	}

	void AllToolsSeen()
	{
		size_t slot = 0;
		size_t bedCount = 0;
		size_t chamberCount = 0;
		auto firstBed = OM::GetFirstBed();
		if (firstBed != nullptr)
		{
			bedCount = AddBedOrChamber(firstBed, slot);
		}
		OM::IterateToolsWhile([&slot](OM::Tool*& tool, size_t)
		{
			tool->slot = slot;
			const bool hasHeater = tool->heaters[0] != nullptr;
			const bool hasSpindle = tool->spindle != nullptr;
			const bool hasExtruder = tool->extruders.IsNonEmpty();
			if (slot < MaxSlots)
			{
				toolButtons[slot]->SetEvent(evSelectHead, tool->index);
				toolButtons[slot]->SetIntVal(tool->index);
				toolButtons[slot]->SetPrintText(true);
				toolButtons[slot]->SetIcon(hasSpindle ? IconSpindle : IconNozzle);
				mgr.Show(toolButtons[slot], true);

				mgr.Show(extrusionFactors[slot], hasExtruder);
				if (hasExtruder)
				{
					extrusionFactors[slot]->SetEvent(extrusionFactors[slot]->GetEvent(), (int) tool->extruders.LowestSetBit());
				}

				// Spindle takes precedence
				if (hasSpindle)
				{
					ManageCurrentActiveStandbyFields(slot, true, evAdjustActiveRPM, tool->spindle->index);
					++slot;
				}
				else if (hasHeater)
				{
					if (nvData.GetHeaterCombineType() == HeaterCombineType::notCombined)
					{
						tool->IterateHeaters([&slot, &tool](OM::ToolHeater*, size_t)
						{
							// only one heater per slot can be displayed
							if (slot >= MaxSlots)
							{
								return;
							}
							ManageCurrentActiveStandbyFields(
									slot,
									true,
									evAdjustToolActiveTemp, tool->index,
									evAdjustToolStandbyTemp, tool->index);
							++slot;
						});
					}
					else
					{
						ManageCurrentActiveStandbyFields(
								slot,
								true,
								evAdjustToolActiveTemp, tool->index,
								evAdjustToolStandbyTemp, tool->index);
						++slot;
					}
				}
				else
				{
					// Hides everything by default
					ManageCurrentActiveStandbyFields(slot);
					++slot;
				}
			}
			return slot < MaxSlots;
		});
		auto firstChamber = OM::GetFirstChamber();
		if (firstChamber != nullptr)
		{
			chamberCount = AddBedOrChamber(firstChamber, slot, false);
		}

		// Fill remaining space with additional beds
		if (slot < MaxSlots && bedCount > 1)
		{
			OM::IterateBedsWhile([&slot](OM::Bed*& bed, size_t) {
				AddBedOrChamber(bed, slot);
				return slot < MaxSlots;
			}, 1);
		}

		// Fill remaining space with additional chambers
		if (slot < MaxSlots && chamberCount > 1)
		{
			OM::IterateChambersWhile([&slot](OM::Chamber*& chamber, size_t) {
				AddBedOrChamber(chamber, slot, false);
				return slot < MaxSlots;
			}, 1);
		}

		numToolColsUsed = slot;
		for (size_t i = slot; i < MaxSlots; ++i)
		{
			mgr.Show(toolButtons[i], false);
			mgr.Show(currentTemps[i], false);
			mgr.Show(activeTemps[i], false);
			mgr.Show(standbyTemps[i], false);
			mgr.Show(extrusionFactors[i], false);
		}
		ResetToolAndHeaterStates();
		AdjustControlPageMacroButtons();
	}

	void SetSpindleActive(size_t spindleIndex, int32_t activeRpm)
	{
		auto spindle = OM::GetOrCreateSpindle(spindleIndex);
		if (spindle == nullptr)
		{
			return;
		}
		spindle->active = abs(activeRpm);
		if (!GetFirmwareFeatures().IsBitSet(m568TempAndRPM))
		{
			if (activeRpm == 0)
			{
				spindle->state = OM::SpindleState::stopped;
			}
			else if (activeRpm > 0)
			{
				spindle->state = OM::SpindleState::forward;
			}
			else
			{
				spindle->state = OM::SpindleState::reverse;
			}
		}

		OM::IterateToolsWhile([spindle](OM::Tool*& tool, size_t) {
			if (tool->slot < MaxSlots && tool->spindle == spindle)
			{
				activeTemps[tool->slot]->SetValue(tool->spindle->active);
			}
			return tool->slot < MaxSlots;
		});
	}

	void UpdateSpindleCurrent(OM::Spindle* spindle)
	{
		OM::IterateToolsWhile([spindle](OM::Tool*& tool, size_t) {
			if (tool->slot < MaxSlots && tool->spindle == spindle)
			{
				const OM::SpindleState state = spindle->state;
				currentTemps[tool->slot]->SetValue(
						(state == OM::SpindleState::stopped)
							? 0
							: (state == OM::SpindleState::forward)
							  	  ? spindle->current
							  	  : -spindle->current);
			}
			return tool->slot < MaxSlots;
		});
	}

	void SetSpindleCurrent(size_t spindleIndex, int32_t current)
	{
		auto spindle = OM::GetOrCreateSpindle(spindleIndex);
		if (spindle == nullptr)
		{
			return;
		}
		spindle->current = abs(current);
		if (!GetFirmwareFeatures().IsBitSet(m568TempAndRPM))
		{
			if (current == 0)
			{
				spindle->state = OM::SpindleState::stopped;
			}
			else if (current > 0)
			{
				spindle->state = OM::SpindleState::forward;
			}
			else
			{
				spindle->state = OM::SpindleState::reverse;
			}
		}
		UpdateSpindleCurrent(spindle);
	}

	void SetSpindleLimit(size_t spindleIndex, uint32_t value, bool max)
	{
		OM::Spindle *spindle = OM::GetOrCreateSpindle(spindleIndex);
		if (spindle != nullptr)
		{
			if (max)
			{
				spindle->max = value;
			}
			else
			{
				spindle->min = value;
			}
		}
	}

	void SetSpindleState(size_t spindleIndex, OM::SpindleState state)
	{
		OM::Spindle* spindle = OM::GetOrCreateSpindle(spindleIndex);
		if (spindle == nullptr)
		{
			return;
		}
		const bool changed = spindle->state != state;
		spindle->state = state;
		if (changed)
		{
			UpdateSpindleCurrent(spindle);
		}
	}

	// This handles the old path where tools were assigned to spindles
	void SetSpindleTool(int8_t spindleNumber, int8_t toolIndex)
	{
		auto sp = OM::GetOrCreateSpindle(spindleNumber);
		if (sp == nullptr)
		{
			return;
		}
		if (toolIndex == -1)
		{
			OM::IterateToolsWhile([sp](OM::Tool*& tool, size_t) {
				if (tool->spindle == sp)
				{
					tool->spindle = nullptr;
				}
				return true;
			});
		}
		else
		{
			OM::Tool *tool = OM::GetOrCreateTool(toolIndex);
			if (tool != nullptr)
			{
				tool->spindle = sp;
			}
		}
	}

	void UpdateToolStatus(size_t toolIndex, OM::ToolStatus status)
	{
		auto tool = OM::GetTool(toolIndex);
		if (tool == nullptr)
		{
			return;
		}
		tool->status = status;
		Colour c = /*(status == OM::ToolStatus::standby) ? colours->standbyBackColour : */
					(status == OM::ToolStatus::active) ? colours->activeBackColour
					: colours->buttonImageBackColour;
		if (tool->slot < MaxSlots)
		{
			toolButtons[tool->slot]->SetColours(colours->buttonTextColour, c);
		}
	}

	void SetToolExtruder(size_t toolIndex, uint8_t extruder)
	{
		OM::Tool *tool = OM::GetOrCreateTool(toolIndex);
		if (tool != nullptr)
		{
			tool->extruders.SetBit(extruder);
		}
	}

	void SetToolFan(size_t toolIndex, uint8_t fan)
	{
		OM::Tool *tool = OM::GetOrCreateTool(toolIndex);
		if (tool != nullptr)
		{
			tool->fans.SetBit(fan);
		}
	}

	bool RemoveToolHeaters(const size_t toolIndex, const uint8_t firstIndexToDelete)
	{
		OM::Tool *tool = OM::GetOrCreateTool(toolIndex);
		if (tool == nullptr)
		{
			return false;
		}
		return tool->RemoveHeatersFrom(firstIndexToDelete) > 0;
	}

	void SetToolHeater(size_t toolIndex, uint8_t toolHeaterIndex, uint8_t heaterIndex)
	{
		OM::Tool *tool = OM::GetOrCreateTool(toolIndex);
		if (tool == nullptr)
		{
			return;
		}
		OM::ToolHeater *toolHeater = tool->GetOrCreateHeater(toolHeaterIndex);
		if (toolHeater == nullptr)
		{
			return;
		}
		toolHeater->heaterIndex = heaterIndex;
	}

	void SetToolOffset(size_t toolIndex, size_t axisIndex, float offset)
	{
		if (axisIndex < MaxTotalAxes)
		{
			OM::Tool *tool = OM::GetOrCreateTool(toolIndex);
			if (tool != nullptr)
			{
				tool->offsets[axisIndex] = offset;
			}
		}
	}

	// This handles the new path were spindles are assigned to tools
	void SetToolSpindle(int8_t toolIndex, int8_t spindleNumber)
	{
		// Old spindles[].tool is handled by SetSpindleTool
		OM::Tool *tool = OM::GetOrCreateTool(toolIndex);
		if (tool == nullptr)
		{
			return;
		}
		if (spindleNumber == -1)
		{
			tool->spindle = nullptr;
		}
		else
		{
			OM::Spindle* spindle = OM::GetSpindle(spindleNumber);
			if (spindle == nullptr)
			{
				return;
			}
			tool->spindle = spindle;
		}
	}

	void SetBabystepOffset(size_t index, float f)
	{
		if (index < MaxTotalAxes)
		{
			OM::Axis *axis = OM::GetOrCreateAxis(index);
			if (axis == nullptr)
			{
				return;
			}
			axis->babystep = f;
			// In first initialization we will see babystep before letter
			// so this won;t be true hence it is also set in UpdateGeometry
			if (axis->letter[0] == 'Z')
			{
				babystepOffsetField->SetValue(f);
			}
		}
	}

	void SetAxisLetter(size_t index, char l)
	{
		if (index < MaxTotalAxes)
		{
			OM::Axis *axis = OM::GetOrCreateAxis(index);
			if (axis != nullptr)
			{
				axis->letter[0] = l;
			}
		}
	}

	void SetAxisVisible(size_t index, bool v)
	{
		if (index < MaxTotalAxes)
		{
			OM::Axis *axis = OM::GetOrCreateAxis(index);
			if (axis != nullptr)
			{
				axis->visible = v;
			}
		}
	}

	void SetAxisWorkplaceOffset(size_t axisIndex, size_t workplaceIndex, float offset)
	{
		if (axisIndex < MaxTotalAxes && workplaceIndex < OM::Workplaces::MaxTotalWorkplaces)
		{
			OM::Axis *axis = OM::GetOrCreateAxis(axisIndex);
			if (axis != nullptr)
			{
				axis->workplaceOffsets[workplaceIndex] = offset;
			}
		}
	}

	void SetCurrentWorkplaceNumber(uint8_t workplaceNumber)
	{
		if (currentWorkplaceNumber == workplaceNumber || workplaceNumber >= OM::Workplaces::MaxTotalWorkplaces)
		{
			return;
		}
		currentWorkplaceNumber = workplaceNumber;
	}

	void SetBedOrChamberHeater(const uint8_t heaterIndex, const int8_t heaterNumber, bool bed)
	{
		if (bed)
		{
			auto bed = OM::GetOrCreateBed(heaterIndex);
			if (bed != nullptr)
			{
				bed->heater = heaterNumber;
			}
		}
		else
		{
			auto chamber = OM::GetOrCreateChamber(heaterIndex);
			if (chamber != nullptr)
			{
				chamber->heater = heaterNumber;
			}
		}
	}
}

// End
