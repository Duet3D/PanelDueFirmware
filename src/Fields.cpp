/*
 * Fields.cpp
 *
 * Created: 04/02/2015 11:43:36
 *  Author: David
 */ 

#include "Configuration.hpp"
#include "Library/Vector.hpp"
#include "Display.hpp"
#include "PanelDue.hpp"
#include "Hardware/Buzzer.hpp"
#include "Fields.hpp"
#include "Icons/Icons.hpp"
#include "Events.hpp"
#include "Library/Misc.hpp"
#include "MessageLog.hpp"

FloatField *currentTemps[maxHeaters], *fpHeightField, *fpLayerHeightField;
FloatField *axisPos[MAX_AXES];
IntegerButton *activeTemps[maxHeaters], *standbyTemps[maxHeaters];
IntegerButton *spd, *extrusionFactors[maxHeaters - 1], *fanSpeed, *baudRateButton, *volumeButton;
IntegerField *freeMem, *touchX, *touchY, *fpSizeField, *fpFilamentField, *fileListErrorField;
ProgressBar *printProgressBar;
SingleButton *tabControl, *tabPrint, *tabMsg, *tabSetup;
SingleButton *moveButton, *extrudeButton, *macroButton;

TextButton *filenameButtons[numDisplayedFiles], *languageButton, *coloursButton;
SingleButton *scrollFilesLeftButton, *scrollFilesRightButton, *filesUpButton, *changeCardButton;
SingleButton *homeButtons[MAX_AXES], *homeAllButton, *bedCompButton;
ButtonPress currentExtrudeRatePress, currentExtrudeAmountPress;
StaticTextField *nameField, *statusField, *touchCalibInstruction, *macroPopupTitleField, *debugField;
IntegerField *filePopupTitleField;
StaticTextField *messageTextFields[numMessageRows], *messageTimeFields[numMessageRows];
StaticTextField *fwVersionField, *settingsNotSavedField, *areYouSureTextField, *areYouSureQueryField;
ButtonBase *filesButton, *pauseButton, *resumeButton, *resetButton;
TextField *timeLeftField;
DisplayField *baseRoot, *commonRoot, *controlRoot, *printRoot, *messageRoot, *setupRoot;
ButtonBase * null currentTab = nullptr;
PopupWindow *setTempPopup, *movePopup, *extrudePopup, *fileListPopup, *filePopup, *baudPopup, *volumePopup, *areYouSurePopup, *keyboardPopup, *languagePopup, *coloursPopup, *brightnessPopup;
TextField *zProbe, *fpNameField, *fpGeneratedByField, *userCommandField;
PopupWindow *alertPopup;
StaticTextField *moveAxisRows[MAX_AXES];

const size_t machineNameLength = 30;
const size_t printingFileLength = 40;
const size_t zprobeBufLength = 12;
const size_t alertTextLength = 80;

String<machineNameLength> machineName;
String<printingFileLength> printingFile;
String<zprobeBufLength> zprobeBuf;
String<generatedByTextLength> generatedByText;
String<alertTextLength> alertText;

static const char* const languageNames[] = { "EN", "DE", "FR" };
static_assert(sizeof(languageNames)/sizeof(languageNames[0]) == numLanguages, "Wrong number of languages");
extern const char* const longLanguageNames[] = { "Keyboard EN", "Tastatur DE", "Clavier FR" };
static_assert(sizeof(longLanguageNames)/sizeof(longLanguageNames[0]) == numLanguages, "Wrong number of long languages");

const char* array settingsNotSavedText = "Some settings are not saved!";
const char*array  restartNeededText = "Touch Save & Restart to use new colour scheme";
const char* array const axisNames[] = { "X", "Y", "Z", "U", "V", "W" };

// Map of PrinterStatus status codes to text. The space at the end improves the appearance.
const char * const statusText[] =
{
	"Connecting",
	"Idle ",
	"Printing ",
	"Halted",
	"Starting up ",
	"Paused ",
	"Busy ",
	"Pausing ",
	"Resuming ",
	"Firmware upload",
	"Changing tool"
};

#if DISPLAY_X == 800
const Icon heaterIcons[maxHeaters] = { IconBed, IconNozzle1, IconNozzle2, IconNozzle3, IconNozzle4, IconNozzle5, IconNozzle6 };
#else
const Icon heaterIcons[maxHeaters] = { IconBed, IconNozzle1, IconNozzle2, IconNozzle3, IconNozzle4 };
#endif

namespace Fields
{
	static bool keyboardIsDisplayed = false;

	// Create a standard popup window with a title and a close button at the top right
	PopupWindow *CreatePopupWindow(PixelNumber ph, PixelNumber pw, Colour pb, Colour pBorder, Colour textColour, const char * null title, PixelNumber topMargin = popupTopMargin)
	{
		PopupWindow *window = new PopupWindow(ph, pw, pb, pBorder);
		DisplayField::SetDefaultColours(textColour, pb);
		if (title != nullptr)
		{
			window->AddField(new StaticTextField(topMargin + labelRowAdjust, popupSideMargin + closeButtonWidth + popupFieldSpacing, pw - 2 * (popupSideMargin + closeButtonWidth + popupFieldSpacing), TextAlignment::Centre, title));
		}
		window->AddField(new IconButton(popupTopMargin, pw - (closeButtonWidth + 
		popupSideMargin), closeButtonWidth, IconCancel, evCancel));	
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

	// Create a row of test buttons.
	// Optionally, set one to 'pressed' and return that one.
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

		movePopup = CreatePopupWindow(movePopupHeight, movePopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, "Move head");
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
			ShowAxis(i, i < MIN_AXES);

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

		extrudePopup = CreatePopupWindow(extrudePopupHeight, extrudePopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupButtonTextColour, "Extrusion amount (mm)");
		PixelNumber ypos = popupTopMargin + buttonHeight + extrudeButtonRowSpacing;
		DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
		currentExtrudeAmountPress = CreateStringButtonRow(extrudePopup, ypos, popupSideMargin, extrudePopupWidth - 2 * popupSideMargin, fieldSpacing, 6, extrudeAmountValues, extrudeAmountValues, evExtrudeAmount, 3);
		ypos += buttonHeight + extrudeButtonRowSpacing;
		DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
		extrudePopup->AddField(new StaticTextField(ypos + labelRowAdjust, popupSideMargin, extrudePopupWidth - 2 * popupSideMargin, TextAlignment::Centre, "Speed (mm/sec)"));
		ypos += buttonHeight + extrudeButtonRowSpacing;
		DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
		currentExtrudeRatePress = CreateStringButtonRow(extrudePopup, ypos, popupSideMargin, extrudePopupWidth - 2 * popupSideMargin, fieldSpacing, 5, extrudeSpeedValues, extrudeSpeedParams, evExtrudeRate, 4);
		ypos += buttonHeight + extrudeButtonRowSpacing;
		extrudePopup->AddField(new TextButton(ypos, popupSideMargin, extrudePopupWidth/3 - 2 * popupSideMargin, "Extrude", evExtrude));
		extrudePopup->AddField(new TextButton(ypos, (2 * extrudePopupWidth)/3 + popupSideMargin, extrudePopupWidth/3 - 2 * popupSideMargin, "Retract", evRetract));
	}
	
	// Create the popup used to list files and macros
	void CreateFileListPopup(const ColourScheme& colours)
	{
		fileListPopup = CreatePopupWindow(fileListPopupHeight, fileListPopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, nullptr);
		const PixelNumber closeButtonPos = fileListPopupWidth - closeButtonWidth - popupSideMargin;
		const PixelNumber navButtonWidth = (closeButtonPos - popupSideMargin)/7;
		const PixelNumber upButtonPos = closeButtonPos - navButtonWidth - fieldSpacing;
		const PixelNumber rightButtonPos = upButtonPos - navButtonWidth - fieldSpacing;
		const PixelNumber leftButtonPos = popupSideMargin;
		const PixelNumber textPos = popupSideMargin + navButtonWidth;
		const PixelNumber changeButtonPos = rightButtonPos - navButtonWidth - fieldSpacing;

		DisplayField::SetDefaultColours(colours.popupTextColour, colours.popupBackColour);
		fileListPopup->AddField(filePopupTitleField = new IntegerField(popupTopMargin + labelRowAdjust, textPos, changeButtonPos - textPos, TextAlignment::Centre, "Files on card ", nullptr));
		fileListPopup->AddField(macroPopupTitleField = new StaticTextField(popupTopMargin + labelRowAdjust, textPos, rightButtonPos - textPos, TextAlignment::Centre, "Macros"));

		DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
		fileListPopup->AddField(scrollFilesLeftButton = new TextButton(popupTopMargin, leftButtonPos, navButtonWidth, "<", evScrollFiles, -numFileRows));
		scrollFilesLeftButton->Show(false);
		fileListPopup->AddField(scrollFilesRightButton = new TextButton(popupTopMargin, rightButtonPos, navButtonWidth, ">", evScrollFiles, numFileRows));
		scrollFilesRightButton->Show(false);
		fileListPopup->AddField(filesUpButton = new IconButton(popupTopMargin, upButtonPos, navButtonWidth, IconUp, evNull));
		filesUpButton->Show(false);
		fileListPopup->AddField(changeCardButton = new TextButton(popupTopMargin, changeButtonPos, navButtonWidth, "<->", evChangeCard, 0));
		
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
								TextAlignment::Centre, "Error ", " accessing SD card");
		fileListErrorField->Show(false);
		fileListPopup->AddField(fileListErrorField);
	}

	// Create the popup window used to display the file dialog
	void CreateFileActionPopup(const ColourScheme& colours)
	{
		filePopup = CreatePopupWindow(fileInfoPopupHeight, fileInfoPopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupTextColour, "File information");
		PixelNumber ypos = popupTopMargin + (3 * rowTextHeight)/2;
		fpNameField = new TextField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, "Filename: ");
		ypos += rowTextHeight;
		fpSizeField = new IntegerField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, "Size: ", " bytes");
		ypos += rowTextHeight;
		fpLayerHeightField = new FloatField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, 2, "Layer height: ","mm");
		ypos += rowTextHeight;
		fpHeightField = new FloatField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, 1, "Object height: ", "mm");
		ypos += rowTextHeight;
		fpFilamentField = new IntegerField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, "Filament needed: ", "mm");
		ypos += rowTextHeight;
		fpGeneratedByField = new TextField(ypos, popupSideMargin, fileInfoPopupWidth - 2 * popupSideMargin, TextAlignment::Left, "Sliced by: ", generatedByText.c_str());
		filePopup->AddField(fpNameField);
		filePopup->AddField(fpSizeField);
		filePopup->AddField(fpLayerHeightField);
		filePopup->AddField(fpHeightField);
		filePopup->AddField(fpFilamentField);
		filePopup->AddField(fpGeneratedByField);

		// Add the buttons
		DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
		filePopup->AddField(new TextButton(popupTopMargin + 8 * rowTextHeight, popupSideMargin, fileInfoPopupWidth/3 - 2 * popupSideMargin, "Print", evPrint));
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
		static const char* const volumePopupText[Buzzer::MaxVolume + 1] = { "Off", "1", "2", "3", "4", "5" };
		volumePopup = CreateIntPopupBar(colours, fullPopupWidth, Buzzer::MaxVolume + 1, volumePopupText, nullptr, evAdjustVolume, evAdjustVolume);
	}

	// Create the colour scheme change popup
	void CreateColoursPopup(const ColourScheme& colours)
	{
		if (NumColourSchemes >= 2)
		{
			// Put all the colour scheme names in a single array for the call to CreateIntPopupBar
			const char* coloursPopupText[MaxColourSchemes];
			for (size_t i = 0; i < NumColourSchemes; ++i)
			{
				coloursPopupText[i] = colourSchemes[i].name;
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
		languagePopup = CreateIntPopupBar(colours, fullPopupWidth, numLanguages, languageNames, nullptr, evAdjustLanguage, evAdjustLanguage);
	}
	
	// Create the pop-up keyboard
	void CreateKeyboardPopup(uint32_t language, ColourScheme colours)
	{
		static const char* array const keysGB[4] = { "1234567890-+", "QWERTYUIOP", "ASDFGHJKL:", "ZXCVBNM./" };
		static const char* array const keysDE[4] = { "1234567890-+", "QWERTZUIOP", "ASDFGHJKL:", "YXCVBNM./" };
		static const char* array const keysFR[4] = { "1234567890-+", "AZERTWUIOP", "QSDFGHJKLM", "YXCVBN.:/" };
		static const char* array const * const keyboards[numLanguages] = { keysGB, keysDE, keysFR };

		keyboardPopup = CreatePopupWindow(keyboardPopupHeight, keyboardPopupWidth, colours.popupBackColour, colours.popupBorderColour, colours.popupInfoTextColour, nullptr, keyboardTopMargin);
		
		// Add the text area in which the command is built
		DisplayField::SetDefaultColours(colours.popupInfoTextColour, colours.popupInfoBackColour);		// need a different background colour
		userCommandField = new TextField(keyboardTopMargin + labelRowAdjust, popupSideMargin, keyboardPopupWidth - 2 * popupSideMargin - closeButtonWidth - popupFieldSpacing, TextAlignment::Left, nullptr, "_");
		keyboardPopup->AddField(userCommandField);

		if (language >= numLanguages)
		{
			language = 0;
		}
		const char* array const * array const keys = keyboards[language];
		DisplayField::SetDefaultColours(colours.popupButtonTextColour, colours.popupButtonBackColour);
		PixelNumber row = keyboardTopMargin + keyButtonVStep;
		for (size_t i = 0; i < 4; ++i)
		{
			PixelNumber column = popupSideMargin + (i * keyButtonHStep)/3;
			const char * s = keys[i];
			while (*s != 0)
			{
				keyboardPopup->AddField(new CharButton(row, column, keyButtonWidth, *s, evKey));
				++s;
				column += keyButtonHStep;
			}
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
			
		// Add the cancel, space and enter keys
		const PixelNumber keyButtonHSpace = keyButtonHStep - keyButtonWidth;
		const PixelNumber wideKeyButtonWidth = (keyboardPopupWidth - 2 * popupSideMargin - 2 * keyButtonHSpace)/5;
		keyboardPopup->AddField(new TextButton(row, popupSideMargin + wideKeyButtonWidth + keyButtonHSpace, 2 * wideKeyButtonWidth, nullptr, evKey, (int)' '));	
		keyboardPopup->AddField(new IconButton(row, popupSideMargin + 3 * wideKeyButtonWidth + 2 * keyButtonHSpace, wideKeyButtonWidth, IconEnter, evSendKeyboardCommand));
	}
	
	// Create the message popup window
	void CreateMessagePopup(const ColourScheme& colours)
	{
		alertPopup = CreatePopupWindow(alertPopupHeight, alertPopupWidth, colours.alertPopupBackColour, colours.popupBorderColour, colours.alertPopupTextColour, "Message");
		alertPopup->AddField(new StaticTextField(popupTopMargin + 2 * rowTextHeight, popupSideMargin, alertPopupWidth - 2 * popupSideMargin, TextAlignment::Centre,
								 alertText.c_str()));
	}
	
#ifdef OEM_LAYOUT

#include "OemFields.inc"

#else
	// Create the grid of heater icons and temperatures
	void CreateTemperatureGrid(const ColourScheme& colours)
	{
		// Add the labels
		DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
		mgr.AddField(debugField = new StaticTextField(row2 + labelRowAdjust, margin, bedColumn - fieldSpacing - margin, TextAlignment::Left, "debug"));
		mgr.AddField(new StaticTextField(row3 + labelRowAdjust, margin, bedColumn - fieldSpacing - margin, TextAlignment::Right, "Current" THIN_SPACE DEGREE_SYMBOL "C"));
		mgr.AddField(new StaticTextField(row4 + labelRowAdjust, margin, bedColumn - fieldSpacing - margin, TextAlignment::Right, "Active" THIN_SPACE DEGREE_SYMBOL "C"));
		mgr.AddField(new StaticTextField(row5 + labelRowAdjust, margin, bedColumn - fieldSpacing - margin, TextAlignment::Right, "Standby" THIN_SPACE DEGREE_SYMBOL "C"));
	
		// Add the grid
		for (unsigned int i = 0; i < maxHeaters; ++i)
		{
			PixelNumber column = ((tempButtonWidth + fieldSpacing) * i) + bedColumn;

			// Add the icon button
			SingleButton *b = new IconButton(row2, column, tempButtonWidth, heaterIcons[i], evSelectHead, i);
			mgr.AddField(b);
		
			// Add the current temperature field
			DisplayField::SetDefaultColours(colours.infoTextColour, colours.defaultBackColour);
			FloatField *f = new FloatField(row3 + labelRowAdjust, column, tempButtonWidth, TextAlignment::Centre, 1);
			f->SetValue(0.0);
			currentTemps[i] = f;
			mgr.AddField(f);
		
			// Add the active temperature button
			DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonBackColour);
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
		mgr.AddField(zProbe = new TextField(row6p3 + labelRowAdjust, column, DISPLAY_X - column - margin, TextAlignment::Left, "Pr", zprobeBuf.c_str()));

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
		DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonBackColour);
		bedCompButton = AddIconButton(row7p7, MAX_AXES + 1, MAX_AXES + 2, IconBedComp, evSendCommand, "G32");

		filesButton = AddIconButton(row8p7, 0, 4, IconFiles, evListFiles, nullptr);
		moveButton = AddTextButton(row8p7, 1, 4, "Move", evMovePopup, nullptr);
		extrudeButton = AddTextButton(row8p7, 2, 4, "Extrude", evExtrudePopup, nullptr);
		macroButton = AddTextButton(row8p7, 3, 4, "Macro", evListMacros, nullptr);

		controlRoot = mgr.GetRoot();
	}

	// Create the fields for the Printing tab
	void CreatePrintingTabFields(const ColourScheme& colours)
	{
		mgr.SetRoot(commonRoot);
	
		// Labels
		DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
		mgr.AddField(new StaticTextField(row6 + labelRowAdjust, margin, bedColumn - fieldSpacing, TextAlignment::Right, "Extruder" THIN_SPACE "%"));
		//		mgr.AddField(new StaticTextField(row7 + labelRowAdjust, margin, bedColumn - fieldSpacing, TextAlignment::Right, "% speed"));
		//		mgr.AddField(new StaticTextField(row7 + labelRowAdjust, ((tempButtonWidth + fieldSpacing) * 1) + bedColumn, tempButtonWidth, TextAlignment::Right, "fan"));
	
		// Extrusion factor buttons
		DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonBackColour);
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
		mgr.AddField(spd = new IntegerButton(row7, speedColumn, fanColumn - speedColumn - fieldSpacing, "Speed ", "%"));
		spd->SetValue(100);
		spd->SetEvent(evAdjustSpeed, "M220 S");
	
		// Fan button
		mgr.AddField(fanSpeed = new IntegerButton(row7, fanColumn, pauseColumn - fanColumn - fieldSpacing, "Fan ", "%"));
		fanSpeed->SetEvent(evAdjustFan, 0);
		fanSpeed->SetValue(0);
	
		DisplayField::SetDefaultColours(colours.buttonTextColour, colours.pauseButtonBackColour);
		pauseButton = new TextButton(row7, pauseColumn, DisplayX - pauseColumn - margin, "Pause print", evPausePrint, "M25");
		mgr.AddField(pauseButton);

		DisplayField::SetDefaultColours(colours.buttonTextColour, colours.resumeButtonBackColour);
		resumeButton = new TextButton(row7, resumeColumn, cancelColumn - resumeColumn - fieldSpacing, "Resume", evResumePrint, "M24");
		mgr.AddField(resumeButton);

		DisplayField::SetDefaultColours(colours.buttonTextColour, colours.resetButtonBackColour);
		resetButton = new TextButton(row7, cancelColumn, DisplayX - cancelColumn - margin, "Cancel", evReset, "M0");
		mgr.AddField(resetButton);

		//		DisplayField::SetDefaultColours(labelTextColour, defaultBackColour);
		//		mgr.AddField(printingField = new TextField(row8, margin, DisplayX, TextAlignment::Left, "printing ", printingFile.c_str()));
	
		DisplayField::SetDefaultColours(colours.progressBarColour,colours. progressBarBackColour);
		mgr.AddField(printProgressBar = new ProgressBar(row8 + (rowHeight - progressBarHeight)/2, margin, progressBarHeight, DisplayX - 2 * margin));
		mgr.Show(printProgressBar, false);
	
		DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
		mgr.AddField(timeLeftField = new TextField(row9, margin, DisplayX - 2 * margin, TextAlignment::Left, "time left: "));
		mgr.Show(timeLeftField, false);

		printRoot = mgr.GetRoot();
	}

	// Create the fields for the Message tab
	void CreateMessageTabFields(const ColourScheme& colours)
	{
		mgr.SetRoot(baseRoot);
		DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonBackColour);
		mgr.AddField(new IconButton(margin,  DisplayX - margin - keyboardButtonWidth, keyboardButtonWidth, IconKeyboard, evKeyboard));
		DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
		mgr.AddField(new StaticTextField(margin + labelRowAdjust, margin, DisplayX - 2 * margin - keyboardButtonWidth, TextAlignment::Centre, "Messages"));
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
		mgr.AddField(fwVersionField = new StaticTextField(row1, margin, DisplayX, TextAlignment::Left, "Panel Due firmware version " VERSION_TEXT));
		mgr.AddField(freeMem = new IntegerField(row2, margin, DisplayX/2 - margin, TextAlignment::Left, "Free RAM: "));
		mgr.AddField(touchX = new IntegerField(row2, DisplayX/2, DisplayX/4, TextAlignment::Left, "Touch: ", ","));
		mgr.AddField(touchY = new IntegerField(row2, (DisplayX * 3)/4, DisplayX/4, TextAlignment::Left));
	
		DisplayField::SetDefaultColours(colours.errorTextColour, colours.errorBackColour);
		mgr.AddField(settingsNotSavedField = new StaticTextField(row3, margin, DisplayX - 2 * margin, TextAlignment::Left, settingsNotSavedText));
		settingsNotSavedField->Show(false);

		DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonBackColour);
		baudRateButton = AddIntegerButton(row4, 0, 3, nullptr, " baud", evSetBaudRate);
		volumeButton = AddIntegerButton(row4, 1, 3, "Volume ", nullptr, evSetVolume);
		languageButton = AddTextButton(row4, 2, 3, longLanguageNames[language], evSetLanguage, nullptr);
		AddTextButton(row5, 0, 3, "Calibrate touch", evCalTouch, nullptr);
		AddTextButton(row5, 1, 3, "Mirror display", evInvertX, nullptr);
		AddTextButton(row5, 2, 3, "Invert display", evInvertY, nullptr);
		coloursButton = AddTextButton(row6, 0, 3, colours.name, evSetColours, nullptr);
		AddTextButton(row6, 1, 3, "Brightness -", evDimmer, nullptr);
		AddTextButton(row6, 2, 3, "Brightness +", evBrighter, nullptr);
		AddTextButton(row7, 0, 3, "Save settings", evSaveSettings, nullptr);
		AddTextButton(row7, 1, 3, "Clear settings", evFactoryReset, nullptr);
		AddTextButton(row7, 2, 3, "Save & restart", evRestart, nullptr);
	
		setupRoot = mgr.GetRoot();
	}

	// Create the fields that are displayed on all pages
	void CreateCommonFields(const ColourScheme& colours)
	{
		DisplayField::SetDefaultColours(colours.buttonTextColour, colours.buttonBackColour, colours.buttonBorderColour, colours.buttonGradColour,
										colours.buttonPressedBackColour, colours.buttonPressedGradColour);
		tabControl = AddTextButton(rowTabs, 0, 4, "Control", evTabControl, nullptr);
		tabPrint = AddTextButton(rowTabs, 1, 4, "Print", evTabPrint, nullptr);
		tabMsg = AddTextButton(rowTabs, 2, 4, "Console", evTabMsg, nullptr);
		tabSetup = AddTextButton(rowTabs, 3, 4, "Setup", evTabSetup, nullptr);
	}
	
	void CreateMainPages(uint32_t language, const ColourScheme& colours)
	{
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
#endif

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

		DisplayField::SetDefaultColours(colours.labelTextColour, colours.defaultBackColour);
		touchCalibInstruction = new StaticTextField(DisplayY/2 - 10, 0, DisplayX, TextAlignment::Centre, nullptr);		// the text is filled in within CalibrateTouch

		mgr.SetRoot(nullptr);
	}
	
	// Show or hide the field that warns about unsaved settings
	void SettingsAreSaved(bool areSaved, bool needRestart)
	pre(needRestart => !areSaved)
	{
		if (needRestart)
		{
			settingsNotSavedField->SetValue(restartNeededText);
			mgr.Show(settingsNotSavedField, true);
		}
		else if (areSaved)
		{
			mgr.Show(settingsNotSavedField, false);
		}
		else
		{
			settingsNotSavedField->SetValue(settingsNotSavedText);
			mgr.Show(settingsNotSavedField, true);
		}
	}
	
	void ShowFilesButton()
	{
		mgr.Show(resumeButton, false);
		mgr.Show(resetButton, false);
		mgr.Show(pauseButton, false);
		mgr.Show(filesButton, true);
	}
	
	void ShowPauseButton()
	{
		mgr.Show(resumeButton, false);
		mgr.Show(resetButton, false);
		mgr.Show(filesButton, false);
		mgr.Show(pauseButton, true);
	}
	
	void ShowResumeAndCancelButtons()
	{
		mgr.Show(pauseButton, false);
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
		if (heater < maxHeaters && currentTemps[heater] != nullptr)
		{
			currentTemps[heater]->SetValue(fval);
		}
	}

	void ShowHeater(size_t heater, bool show)
	{
		if (heater < maxHeaters)
		{
			mgr.Show(currentTemps[heater], show);
			mgr.Show(activeTemps[heater], show);
			mgr.Show(standbyTemps[heater], show);
			mgr.Show(extrusionFactors[heater - 1], show);
		}
	}

	void ShowHeaterStatus(size_t heater, int ival)
	{
		if (heater < maxHeaters && currentTemps[heater] != nullptr)
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
			timesLeftText.catFrom("n/a");
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
			timesLeftText.copy("file ");
			AppendTimeLeft(timesLeft[0]);
			timesLeftText.catFrom(", filament ");
			AppendTimeLeft(timesLeft[1]);
			if (DisplayX >= 800)
			{
				timesLeftText.catFrom(", layer ");
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
					mgr.SetPopup(keyboardPopup, margin, (DisplayX - keyboardPopupWidth)/2, false);
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
		mgr.SetPopup(keyboardPopup, keyboardPopupX, keyboardPopupY);
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
			Fields::ShowPauseButton();
		}
		else if (GetStatus() == PrinterStatus::paused)
		{
			Fields::ShowResumeAndCancelButtons();
		}
		else
		{
			Fields::ShowFilesButton();
		}

		mgr.Show(printProgressBar, PrintInProgress());
	//	mgr.Show(printingField, PrintInProgress());

		// Don't enable the time left field when we start printing, instead this will get enabled when we receive a suitable message
		if (!PrintInProgress())
		{
			mgr.Show(timeLeftField, false);
		}

		statusField->SetValue(statusText[(unsigned int)GetStatus()]);
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

	// process an alert message. If the data is empty then we should clear any existing alert.
	void ProcessAlert(const char data[])
	{
		if (data[0] == 0)
		{
			mgr.ClearPopup(true, alertPopup);
		}
		else
		{
			alertText.copy(data);
			mgr.SetPopup(alertPopup, (DisplayX - alertPopupWidth)/2, (DisplayY - alertPopupHeight)/2);
		}
	}
}

// End

