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
#include "Fields.hpp"
#include "Icons/Icons.hpp"
#include "Events.hpp"
#include "Library/Misc.hpp"
#include "MessageLog.hpp"

IntegerField *freeMem, *touchX, *touchY, *fileListErrorField;

TextButton *filenameButtons[numDisplayedFiles];
SingleButton *scrollFilesLeftButton, *scrollFilesRightButton, *filesUpButton, *changeCardButton;
StaticTextField *touchCalibInstruction, *macroPopupTitleField, *debugField;
IntegerField *filePopupTitleField;
StaticTextField *messageTextFields[numMessageRows], *messageTimeFields[numMessageRows];

namespace Fields
{
	// Create a standard popup window with a title and a close button at the top right
	PopupWindow *CreatePopupWindow(PixelNumber ph, PixelNumber pw, Colour pb, Colour pBorder, Colour textColour, const char * null title, PixelNumber topMargin)
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

	// Create a row of text buttons.
	// Optionally, set one to 'pressed' and return that one.
	ButtonPress CreateStringButtonRow(Window * pf, PixelNumber top, PixelNumber left, PixelNumber totalWidth, PixelNumber spacing, unsigned int numButtons,
								const char* array const text[], const char* array const params[], Event evt, int selected)
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


}

// End

