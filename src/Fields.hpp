/*
 * Fields.hpp
 *
 * Created: 04/02/2015 11:43:53
 *  Author: David
 */ 


#ifndef FIELDS_H_
#define FIELDS_H_

#include "Display.hpp"
#include "ColourSchemes.hpp"
#include "Events.hpp"
#include "PrinterStatus.hpp"

// From the display type, we determine the display controller type and touch screen orientation adjustment
#if DISPLAY_TYPE == DISPLAY_TYPE_ITDB02_32WD

# define DISPLAY_CONTROLLER		HX8352A
const DisplayOrientation DefaultDisplayOrientAdjust = static_cast<DisplayOrientation>(SwapXY | ReverseY | InvertBitmap);
const DisplayOrientation DefaultTouchOrientAdjust = static_cast<DisplayOrientation>(ReverseY);
const bool is24BitLcd = true;
# define DISPLAY_X				(400)
# define DISPLAY_Y				(240)

#elif DISPLAY_TYPE == DISPLAY_TYPE_ITDB02_43

# define DISPLAY_CONTROLLER		SSD1963_480
const DisplayOrientation DefaultDisplayOrientAdjust = static_cast<DisplayOrientation>(SwapXY | ReverseX | InvertBitmap);
const DisplayOrientation DefaultTouchOrientAdjust = SwapXY;
const bool is24BitLcd = true;
# define DISPLAY_X				(480)
# define DISPLAY_Y				(272)

#elif DISPLAY_TYPE == DISPLAY_TYPE_ITDB02_50

# define DISPLAY_CONTROLLER		SSD1963_800
const DisplayOrientation DefaultDisplayOrientAdjust = static_cast<DisplayOrientation>(SwapXY | ReverseX | InvertBitmap);
const DisplayOrientation DefaultTouchOrientAdjust = static_cast<DisplayOrientation>(SwapXY | ReverseY);
const bool is24BitLcd = true;
# define DISPLAY_X				(800)
# define DISPLAY_Y				(480)

#elif DISPLAY_TYPE == DISPLAY_TYPE_ITDB02_70

# define DISPLAY_CONTROLLER		SSD1963_800
const DisplayOrientation DefaultDisplayOrientAdjust = static_cast<DisplayOrientation>(SwapXY | ReverseX | ReverseY | InvertText | InvertBitmap);
const DisplayOrientation DefaultTouchOrientAdjust = static_cast<DisplayOrientation>(SwapXY | ReverseY);
const bool is24BitLcd = false;
# define DISPLAY_X				(800)
# define DISPLAY_Y				(480)

#else
# error DISPLAY_TYPE is not defined correctly
#endif

const PixelNumber DisplayX = DISPLAY_X;
const PixelNumber DisplayY = DISPLAY_Y;

// Define the row and column positions. Leave a gap of at least 1 pixel from the edges of the screen, so that we can highlight
// a field by drawing an outline.

#define MIN_AXES	(3)

#if DISPLAY_X == 480

const unsigned int maxHeaters = 5;
#define MAX_AXES	(4)

const PixelNumber margin = 2;
const PixelNumber textButtonMargin = 1;
const PixelNumber iconButtonMargin = 1;
const PixelNumber outlinePixels = 2;
const PixelNumber fieldSpacing = 6;
const PixelNumber statusFieldWidth = 156;
const PixelNumber bedColumn = 114;

const PixelNumber rowTextHeight = 21;	// height of the font we use
const PixelNumber rowHeight = 28;
const PixelNumber moveButtonRowSpacing = 12;
const PixelNumber extrudeButtonRowSpacing = 12;
const PixelNumber fileButtonRowSpacing = 8;
const PixelNumber keyboardButtonRowSpacing = 6;		// small enough to show 2 lines of messages

const PixelNumber speedTextWidth = 70;
const PixelNumber efactorTextWidth = 30;
const PixelNumber percentageWidth = 60;
const PixelNumber e1FactorXpos = 140, e2FactorXpos = 250;

const PixelNumber messageTimeWidth = 60;

const PixelNumber popupY = 192;
const PixelNumber popupSideMargin = 10;
const PixelNumber popupTopMargin = 10;
const PixelNumber keyboardTopMargin = 8;

const PixelNumber popupFieldSpacing = 10;

const PixelNumber axisLabelWidth = 26;
const PixelNumber firstMessageRow = margin + rowHeight + 3;		// adjust this to get a whole number of message rows below the keyboard

const PixelNumber progressBarHeight = 10;
const PixelNumber closeButtonWidth = 40;

const PixelNumber touchCalibMargin = 15;

extern uint8_t glcd19x21[];				// declare which fonts we will be using
#define DEFAULT_FONT	glcd19x21

#elif DISPLAY_X == 800

const unsigned int maxHeaters = 7;
#define MAX_AXES (6)

const PixelNumber margin = 4;
const PixelNumber textButtonMargin = 1;
const PixelNumber iconButtonMargin = 2;
const PixelNumber outlinePixels = 3;
const PixelNumber fieldSpacing = 12;
const PixelNumber statusFieldWidth = 228;
const PixelNumber bedColumn = 160;

const PixelNumber rowTextHeight = 32;	// height of the font we use
const PixelNumber rowHeight = 48;
const PixelNumber moveButtonRowSpacing = 20;
const PixelNumber extrudeButtonRowSpacing = 20;
const PixelNumber fileButtonRowSpacing = 12;
const PixelNumber keyboardButtonRowSpacing = 12;

const PixelNumber speedTextWidth = 105;
const PixelNumber efactorTextWidth = 45;
const PixelNumber percentageWidth = 90;
const PixelNumber e1FactorXpos = 220, e2FactorXpos = 375;

const PixelNumber messageTimeWidth = 90;

const PixelNumber popupY = 345;
const PixelNumber popupSideMargin = 20;
const PixelNumber popupTopMargin = 20;
const PixelNumber keyboardTopMargin = 20;
const PixelNumber popupFieldSpacing = 20;

const PixelNumber axisLabelWidth = 40;
const PixelNumber firstMessageRow = margin + rowHeight;		// adjust this to get a whole number of message rows below the keyboard

const PixelNumber progressBarHeight = 16;
const PixelNumber closeButtonWidth = 66;

const PixelNumber touchCalibMargin = 22;

extern uint8_t glcd28x32[];				// declare which fonts we will be using
extern uint8_t glcd28x32[];				// declare which fonts we will be using
#define DEFAULT_FONT	glcd28x32

#else

#error Unsupported DISPLAY_X value

#endif

const PixelNumber buttonHeight = rowTextHeight + 4;
const PixelNumber tempButtonWidth = (DISPLAY_X + fieldSpacing - bedColumn)/maxHeaters - fieldSpacing;

const PixelNumber row1 = 0;										// we don't need a top margin
const PixelNumber row2 = row1 + rowHeight - 2;					// the top row never has buttons so it can be shorter
const PixelNumber row3 = row2 + rowHeight;
const PixelNumber row4 = row3 + rowHeight;
const PixelNumber row5 = row4 + rowHeight;
const PixelNumber row6 = row5 + rowHeight;
const PixelNumber row6p3 = row6 + (rowHeight/3);
const PixelNumber row7 = row6 + rowHeight;
const PixelNumber row7p7 = row7 + ((2 * rowHeight)/3);
const PixelNumber row8 = row7 + rowHeight;
const PixelNumber row8p7 = row8 + ((2 * rowHeight)/3);
const PixelNumber row9 = row8 + rowHeight;
const PixelNumber rowTabs = DisplayY - rowTextHeight;			// place at bottom of screen with no margin
const PixelNumber labelRowAdjust = 2;							// how much to drop non-button fields to line up with buttons

const PixelNumber speedColumn = margin;
const PixelNumber fanColumn = DISPLAY_X/4 + 20;

const PixelNumber pauseColumn = DISPLAY_X/2 + 10 + fieldSpacing;
const PixelNumber resumeColumn = pauseColumn;
const PixelNumber cancelColumn = pauseColumn + (DISPLAY_X - pauseColumn - fieldSpacing - margin)/2 + fieldSpacing;

const PixelNumber fullPopupWidth = DisplayX - (2 * margin);
const PixelNumber fullPopupHeight = DisplayY - (2 * margin);
const PixelNumber fullWidthPopupX = (DisplayX - fullPopupWidth)/2;
const PixelNumber popupBarHeight = buttonHeight + (2 * popupTopMargin);

const PixelNumber tempPopupBarWidth = (3 * fullPopupWidth)/4;
const PixelNumber tempPopupX = (DisplayX - tempPopupBarWidth)/2;
const PixelNumber fileInfoPopupWidth = fullPopupWidth - (4 * margin),
				  fileInfoPopupHeight = (8 * rowTextHeight) + buttonHeight + (2 * popupTopMargin);
const PixelNumber areYouSurePopupWidth = DisplayX - 80,
				  areYouSurePopupHeight = (3 * rowHeight) + (2 * popupTopMargin);

const PixelNumber movePopupWidth = fullPopupWidth;
const PixelNumber movePopupHeight = ((MAX_AXES + 1) * buttonHeight) + (MAX_AXES * moveButtonRowSpacing) + (2 * popupTopMargin);
const PixelNumber movePopupX = (DisplayX - movePopupWidth)/2;
const PixelNumber movePopupY = (DisplayY - movePopupHeight)/2;

const PixelNumber extrudePopupWidth = fullPopupWidth;
const PixelNumber extrudePopupHeight = (5 * buttonHeight) + (4 * extrudeButtonRowSpacing) + (2 * popupTopMargin);
const PixelNumber extrudePopupX = (DisplayX - extrudePopupWidth)/2;
const PixelNumber extrudePopupY = (DisplayY - extrudePopupHeight)/2;

const PixelNumber keyboardButtonWidth = DisplayX/5;
const PixelNumber keyboardPopupWidth = fullPopupWidth;
const PixelNumber keyButtonWidth = (keyboardPopupWidth - 2 * popupSideMargin)/16;
const PixelNumber keyButtonHStep = (keyboardPopupWidth - 2 * popupSideMargin - keyButtonWidth)/11;
const PixelNumber keyButtonVStep = buttonHeight + keyboardButtonRowSpacing;
const PixelNumber keyboardPopupHeight = (5 * keyButtonVStep) + (2 * keyboardTopMargin) + buttonHeight;
const PixelNumber keyboardPopupX = fullWidthPopupX, keyboardPopupY = margin;

const unsigned int numFileColumns = 2;
const unsigned int numFileRows = (fullPopupHeight - (2 * popupTopMargin) + fileButtonRowSpacing)/(buttonHeight + fileButtonRowSpacing) - 1;
const unsigned int numDisplayedFiles = numFileColumns * numFileRows;
const PixelNumber fileListPopupWidth = fullPopupWidth;
const PixelNumber fileListPopupHeight = ((numFileRows + 1) * buttonHeight) + (numFileRows * fileButtonRowSpacing) + (2 * popupTopMargin);
const PixelNumber fileListPopupX = (DisplayX - fileListPopupWidth)/2;
const PixelNumber fileListPopupY = (DisplayY - fileListPopupHeight)/2;

const uint32_t numMessageRows = (rowTabs - margin - rowHeight)/rowTextHeight;
const PixelNumber messageTextX = margin + messageTimeWidth + 2;
const PixelNumber messageTextWidth = DisplayX - margin - messageTextX;

const unsigned int numLanguages = 3;
extern const char* const longLanguageNames[];

extern IntegerField *freeMem, *touchX, *touchY, *fileListErrorField;
extern TextButton *filenameButtons[numDisplayedFiles];
extern SingleButton *scrollFilesLeftButton, *scrollFilesRightButton, *filesUpButton, *changeCardButton;
extern StaticTextField *macroPopupTitleField, *debugField;
extern IntegerField *filePopupTitleField;
extern StaticTextField *touchCalibInstruction;
extern StaticTextField *messageTextFields[numMessageRows], *messageTimeFields[numMessageRows];

namespace Fields
{
	// Create a standard popup window with a title and a close button at the top right
	PopupWindow *CreatePopupWindow(PixelNumber ph, PixelNumber pw, Colour pb, Colour pBorder, Colour textColour,
									const char * null title, PixelNumber topMargin = popupTopMargin);

	// Add a text button
	TextButton *AddTextButton(PixelNumber row, unsigned int col, unsigned int numCols, const char* array text, Event evt, const char* param);

	// Add an integer button
	IntegerButton *AddIntegerButton(PixelNumber row, unsigned int col, unsigned int numCols, const char * array null label, const char * array null units, Event evt);

	// Add an icon button with a string parameter
	IconButton *AddIconButton(PixelNumber row, unsigned int col, unsigned int numCols, Icon icon, Event evt, const char* param);

	// Create a row of text buttons.
	// Optionally, set one to 'pressed' and return that one.
	ButtonPress CreateStringButtonRow(Window * pf, PixelNumber top, PixelNumber left, PixelNumber totalWidth, PixelNumber spacing, unsigned int numButtons,
										const char* array const text[], const char* array const params[], Event evt, int selected = -1);

	// Create a popup bar with string parameters
	PopupWindow *CreateStringPopupBar(const ColourScheme& colours, PixelNumber width, unsigned int numEntries,
										const char* const text[], const char* const params[], Event ev);

	// Create a popup bar with integer parameters
	// If the 'params' parameter is null then we use 0, 1, 2.. at the parameters
	PopupWindow *CreateIntPopupBar(const ColourScheme& colours, PixelNumber width, unsigned int numEntries,
									const char* const text[], const int * null params, Event ev, Event zeroEv);
}

#endif /* FIELDS_H_ */
