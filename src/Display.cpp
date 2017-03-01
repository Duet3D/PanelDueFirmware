/*
 * Display.cpp
 *
 * Created: 04/11/2014 09:42:47
 *  Author: David
 */ 

#include "Display.hpp"
#include "ColourSchemes.hpp"

#undef min
#undef max
#undef array
#undef result
#include <algorithm>
#define array _ecv_array
#define result _ecv_result

extern UTFT lcd;

// Static fields of class DisplayField
LcdFont DisplayField::defaultFont = nullptr;
Colour DisplayField::defaultFcolour = white;
Colour DisplayField::defaultBcolour = black;
Colour DisplayField::defaultButtonBorderColour = black;
Colour DisplayField::defaultGradColour = 0;
Colour DisplayField::defaultPressedBackColour = black;
Colour DisplayField::defaultPressedGradColour = 0;

DisplayField::DisplayField(PixelNumber py, PixelNumber px, PixelNumber pw)
	: y(py), x(px), width(pw), fcolour(defaultFcolour), bcolour(defaultBcolour),
		changed(true), visible(true), underlined(false), textRows(1), next(nullptr)
{
}

void DisplayField::SetTextRows(const char * array null t)
{
	unsigned int rows = 1;
	if (t != nullptr)
	{
		while (*t != 0)
		{
			if (*t == '\n')
			{
				++rows;
			}
			++t;
		}
	}
	textRows = rows;
}

/*static*/ void DisplayField::SetDefaultColours(Colour pf, Colour pb, Colour pbb, Colour pg, Colour pbp, Colour pgp)
{
	defaultFcolour = pf;
	defaultBcolour = pb;
	defaultButtonBorderColour = pbb;
	defaultGradColour = pg;
	defaultPressedBackColour = pbp;
	defaultPressedGradColour = pgp;
}

/*static*/ PixelNumber DisplayField::GetTextWidth(const char* array s, PixelNumber maxWidth)
{
	lcd.setTextPos(0, 9999, maxWidth);
	lcd.print(s);    // dummy print to get text width
	return lcd.getTextX();
}

void DisplayField::Show(bool v)
{
	if (visible != v)
	{
		visible = changed = v;
	}
}
	
// Find the best match to a touch event in a list of fields
ButtonPress DisplayField::FindEvent(PixelNumber x, PixelNumber y, DisplayField * null p)
{	
	const int maxXerror = 8, maxYerror = 8;		// set these to how close we need to be
	int bestError = maxXerror + maxYerror;
	ButtonPress best;;
	while (p != nullptr)
	{
		if (p->visible && p->GetEvent() != nullEvent)
		{
			int xError = (x < p->GetMinX()) ? p->GetMinX() - x
									: (x > p->GetMaxX()) ? x - p->GetMaxX()
										: 0;
			if (xError < maxXerror)
			{
				int yError = (y < p->GetMinY()) ? p->GetMinY() - y
										: (y > p->GetMaxY()) ? y - p->GetMaxY()
											: 0;
				if (yError < maxYerror && xError + yError < bestError)
				{
					bestError = xError + yError;
					best = ButtonPress(static_cast<ButtonBase*>(p), 0);
				}
			}
		}
		p = p->next;
	}
	return best;
}

void DisplayField::SetColours(Colour pf, Colour pb)
{
	if (fcolour != pf || bcolour != pb)
	{
		fcolour = pf;
		bcolour = pb;
		changed = true;	
	}
}

// ButtonPress class methods
ButtonPress::ButtonPress() : button(nullptr), index(0) { }

ButtonPress::ButtonPress(ButtonBase *b, unsigned int pi) : button(b), index(pi) { }

void ButtonPress::Clear()
{
	button = nullptr;
	index = 0;
}

event_t ButtonPress::GetEvent() const
{
	return button->GetEvent();
}

int ButtonPress::GetIParam() const
{
	return button->GetIParam(index);
}

const char* array ButtonPress::GetSParam() const
{
	return button->GetSParam(index);
}

bool ButtonPress::operator==(const ButtonPress& other) const { return button == other.button && index == other.index; }

// Window class methods
Window::Window(Colour pb)
	: root(nullptr), next(nullptr), backgroundColour(pb)
{
}

// Append a field to the list of displayed fields
void Window::AddField(DisplayField *d)
{
	d->next = root;
	root = d;
}

bool Window::ObscuredByPopup(const DisplayField *p) const
{
	return next != nullptr
			&& (  (   p->GetMaxY() >= next->Ypos() && p->GetMinY() < next->Ypos() + next->GetHeight() 
				   && p->GetMaxX() >= next->Xpos() && p->GetMinX() < next->Xpos() + next->GetWidth()
				  )
				|| next->ObscuredByPopup(p)
			   );
}

bool Window::Visible(const DisplayField *p) const
{
	return p->IsVisible() && !ObscuredByPopup(p);
}

// Get the field that has been touched, or nullptr if we can't find one
ButtonPress Window::FindEvent(PixelNumber x, PixelNumber y)
{
	return (x < Xpos() || y < Ypos()) ? ButtonPress()
			: (next != nullptr) ? next->FindEvent(x, y) 
				: DisplayField::FindEvent(x - Xpos(), y - Ypos(), root);
}

// Get the field that has been touched, but search only outside the popup
ButtonPress Window::FindEventOutsidePopup(PixelNumber x, PixelNumber y)
{
	if (next == nullptr) return ButtonPress();
	
	ButtonPress f = DisplayField::FindEvent(x, y, root);
	return (f.IsValid() && Visible(f.GetButton())) ? f : ButtonPress();
}

void Window::SetPopup(PopupWindow * p, PixelNumber px, PixelNumber py, bool redraw)
{
	if (px == AutoPlace)
	{
		px = (DisplayX - p->GetWidth())/2;
	}
	if (py == AutoPlace)
	{
		py = (DisplayY - p->GetHeight())/2;
	}
	p->SetPos(px, py);
	Window *pw = this;
	while (pw->next != nullptr)
	{
		if (pw->next == p)
		{
			if (redraw)
			{
				p->Refresh(true);
			}
			return;				// popup is already displayed
		}
		pw = pw->next;
	}
	p->next = nullptr;			// ensure no nested popup
	pw->next = p;
	if (redraw)
	{
		p->Refresh(true);
	}
}

void Window::ClearPopup(bool redraw, PopupWindow *whichOne)
{
	if (next != nullptr)
	{
		// Find the penultimate window
		Window *pw = this;
		while (pw->next->next != nullptr)
		{
			pw = pw->next;
		}
		
		if (whichOne == nullptr || whichOne == pw->next)
		{
			const PixelNumber xmin = pw->next->Xpos(), xmax = xmin + pw->next->GetWidth() - 1, ymin = pw->next->Ypos(), ymax = ymin + pw->next->GetHeight() - 1;
			bool popupWasContained = pw->Contains(xmin, ymin, xmax, ymax);
			if (popupWasContained)
			{
				// Clear the area that was occupied by the last window to the background colour of the penultimate window
				lcd.setColor(pw->backgroundColour);
				lcd.fillRoundRect(xmin, ymin, xmax, ymax);
			}

			// Detach the last window
			pw->next = nullptr;

			if (redraw)
			{
				if (popupWasContained)
				{
					// Re-display the fields of the penultimate window that were obscured
					for (DisplayField * null pp = pw->root; pp != nullptr; pp = pp->next)
					{
						if (pp->IsVisible())
						{
							pp->Refresh(true, pw->Xpos(), pw->Ypos());
						}
					}
				}
				else
				{
					Refresh(true);		// redraw everything
				}
			}
		}
	}
}

// Redraw the specified field
void Window::Redraw(DisplayField *f)
{
	for (DisplayField * null p = root; p != nullptr; p = p->next)
	{
		if (p == f)
		{
			// The field belongs to this window
			if (!ObscuredByPopup(p))
			{
				if (p->IsVisible())
				{
					p->Refresh(true, Xpos(), Ypos());
				}
				else
				{
					lcd.setColor(backgroundColour);
					lcd.fillRect(p->GetMinX() + Xpos(), p->GetMinY() + Ypos(), p->GetMaxX() + Xpos(), p->GetMaxY() + Ypos());
				}
			}
			return;
		}
	}
	
	// Else we didn't find the field in our window, so look in nested windows
	if (next != nullptr)
	{
		next->Redraw(f);
	}
}

void Window::Show(DisplayField * null f, bool v)
{
	if (f != nullptr && f->IsVisible() != v)
	{
		f->Show(v);

		// Check whether the field is currently in the display list, if so then show or hide it
		for (DisplayField *p = root; p != nullptr; p = p->next)
		{
			if (p == f)
			{
				if (ObscuredByPopup(f))
				{
					// nothing to do		
				}
				else if (v)
				{
					f->Refresh(true, Xpos(), Ypos());
				}
				else
				{
					lcd.setColor(backgroundColour);
					lcd.fillRect(f->GetMinX(), f->GetMinY(), f->GetMaxX(), f->GetMaxY());
				}
				return;
			}
		}
		
		// Else we didn't find it, so maybe it is in a popup field
		if (next != nullptr)
		{
			next->Redraw(f);
		}
	}
}

// Show the button as pressed or not
void Window::Press(ButtonPress bp, bool v)
{
	if (bp.IsValid())
	{
		bp.GetButton()->Press(v, bp.GetIndex());
		if (bp.GetButton()->IsVisible())		// need to check this in case we are releasing the button and it has gone invisible since we pressed it
		{
			Redraw(bp.GetButton());
		}
	}
}

MainWindow::MainWindow() : Window(black), staticLeftMargin(0)
{
}

void MainWindow::Init(Colour bc)
{
	backgroundColour = bc;
	ClearAll();
}

void MainWindow::ClearAll()
{
	lcd.fillScr(backgroundColour);
}

// Refresh all fields. If 'full' is true then we rewrite them all, else we just rewrite those that have changed.
void MainWindow::Refresh(bool full)
{
	if (full)
	{
		lcd.fillScr(backgroundColour, staticLeftMargin);
	}

	for (DisplayField * null pp = root; pp != nullptr; pp = pp->next)
	{
		if (Visible(pp))
		{
			pp->Refresh(full, 0, 0);
		}
	}
	if (next != nullptr)
	{
		next->Refresh(full);
	}
}

bool MainWindow::Contains(PixelNumber xmin, PixelNumber ymin, PixelNumber xmax, PixelNumber ymax) const
{
	UNUSED(xmin); UNUSED(ymin); UNUSED(xmax); UNUSED(ymax);
	return true;
}

void MainWindow::ClearAllPopups()
{
	while (next != nullptr)
	{
		ClearPopup(true);
	}
}

PopupWindow::PopupWindow(PixelNumber ph, PixelNumber pw, Colour pb, Colour pBorder)
	: Window(pb), height(ph), width(pw), borderColour(pBorder)
{
}

void PopupWindow::Refresh(bool full)
{
	if (full)
	{
		// Draw a rectangle inside the border
		lcd.setColor(backgroundColour);
		lcd.fillRoundRect(xPos + 1, yPos + 2, xPos + width - 2, yPos + height - 3);

		// Draw a double border
		lcd.setColor(borderColour);
		lcd.drawRoundRect(xPos, yPos, xPos + width - 1, yPos + height - 1);
		lcd.drawRoundRect(xPos + 1, yPos + 1, xPos + width - 2, yPos + height - 2);
	}
	
	for (DisplayField * null p = root; p != nullptr; p = p->next)
	{
		if (p->IsVisible() && (full || !ObscuredByPopup(p)))
		{
			p->Refresh(full, xPos, yPos);
		}
	}
	
	if (next != nullptr)
	{
		next->Refresh(full);
	}
}

bool PopupWindow::Contains(PixelNumber xmin, PixelNumber ymin, PixelNumber xmax, PixelNumber ymax) const
{
	return xPos + 2 <= xmin && yPos + 2 <= ymin && xPos + width >= xmax + 3 && yPos + height >= ymax + 3;
}

PixelNumber FieldWithText::GetHeight() const
{
	PixelNumber height = UTFT::GetFontHeight(font) * textRows;
	height += (textRows - 1) * 2;		// 2px space between lines
	if (underlined)
	{
		height += 2;					// one space and the underline
	}
	if (border)
	{
		height += 4;					// one space abd border top and bottom
	}
	return height;
}

void FieldWithText::Refresh(bool full, PixelNumber xOffset, PixelNumber yOffset)
{
	if (full || changed)
	{
		xOffset += x;
		yOffset += y;
		PixelNumber textWidth = width;
		if (border)
		{
			if (full)
			{
				lcd.setColor(fcolour);
				lcd.drawRect(xOffset, yOffset, xOffset + width - 1, yOffset + GetHeight() - 1);
			}
			xOffset += 2;
			yOffset += 2;
			textWidth -= 4;
		}

		lcd.setFont(font);
		lcd.setColor(fcolour);
		lcd.setBackColor(bcolour);

		// Do a dummy print to get the text width. Needed for underlining and for centre- or right-aligned text.
		lcd.setTextPos(0, 9999, textWidth);
		PrintText();
		const PixelNumber actualWidth = lcd.getTextX();
		const PixelNumber underlineY = yOffset + UTFT::GetFontHeight(font) + 1;
		if (underlined)
		{
			// Remove previous underlining
			lcd.setColor(bcolour);
			lcd.drawLine(xOffset, underlineY, xOffset + textWidth - 1, underlineY);
			lcd.setColor(fcolour);
		}

		lcd.setTextPos(xOffset, yOffset, xOffset + textWidth);
		if (align == TextAlignment::Left)
		{
			PrintText();
			lcd.clearToMargin();
			if (underlined)
			{
				lcd.drawLine(xOffset, underlineY, xOffset + actualWidth, underlineY);
			}
		}
		else
		{
			lcd.clearToMargin();
			PixelNumber spare = textWidth - actualWidth;
			if (align == TextAlignment::Centre)
			{
				const PixelNumber textX = xOffset + spare/2;
				lcd.setTextPos(textX, yOffset, xOffset + textWidth);
				PrintText();
				if (underlined)
				{
					lcd.drawLine(textX, underlineY, textX + actualWidth - 1, underlineY);
				}
			}
			else
			{
				// Must be right aligned. Try to add a right margin of up to 3 pixels for better appearance.
				if (spare <= 3)
				{
					spare = 0;
				}
				else
				{
					spare -= 3;
				}
				const PixelNumber textX = xOffset + spare;
				lcd.setTextPos(textX, yOffset, xOffset + textWidth);
				PrintText();
				if (underlined)
				{
					lcd.drawLine(textX, underlineY, textX + actualWidth, underlineY);
				}
			}
		}
		changed = false;
	}
}

void TextField::PrintText() const
{
	if (label != nullptr)
	{
		lcd.print(label);
	}
	if (text != nullptr)
	{
		lcd.print(text);
	}
}

void FloatField::PrintText() const
{
	if (label != nullptr)
	{
		lcd.print(label);
	}
	lcd.print(val, numDecimals);
	if (units != nullptr)
	{
		lcd.print(units);
	}
}

void IntegerField::PrintText() const
{
	if (label != nullptr)
	{
		lcd.print(label);
	}
	lcd.print(val);
	if (units != nullptr)
	{
		lcd.print(units);
	}
}

void StaticTextField::PrintText() const
{
	if (text != nullptr)
	{
		lcd.print(text);
	}
}

ButtonBase::ButtonBase(PixelNumber py, PixelNumber px, PixelNumber pw)
	: DisplayField(py, px, pw),
	  borderColour(defaultButtonBorderColour), gradColour(defaultGradColour),
	  pressedBackColour(defaultPressedBackColour), pressedGradColour(defaultPressedGradColour), evt(nullEvent), pressed(false)
{	
}

PixelNumber ButtonBase::textMargin = 1;
PixelNumber ButtonBase::iconMargin = 1;

void ButtonBase::DrawOutline(PixelNumber xOffset, PixelNumber yOffset, bool isPressed) const
{
	lcd.setColor((isPressed) ? pressedBackColour : bcolour);
	// Note that we draw the filled rounded rectangle with the full width but 2 pixels less height than the border.
	// This means that we start with the requested colour inside the border.
	lcd.fillRoundRect(x + xOffset, y + yOffset + 1, x + xOffset + width - 1, y + yOffset + GetHeight() - 2, (isPressed) ? pressedGradColour : gradColour, buttonGradStep);
	lcd.setColor(borderColour);
	lcd.drawRoundRect(x + xOffset, y + yOffset, x + xOffset + width - 1, y + yOffset + GetHeight() - 1);
}

SingleButton::SingleButton(PixelNumber py, PixelNumber px, PixelNumber pw)
	: ButtonBase(py, px, pw)
{
	param.sParam = nullptr;
}

void SingleButton::DrawOutline(PixelNumber xOffset, PixelNumber yOffset) const
{
	ButtonBase::DrawOutline(xOffset, yOffset, pressed);
}

/*static*/ LcdFont ButtonWithText::font;

PixelNumber ButtonWithText::GetHeight() const
{
	PixelNumber ret = (UTFT::GetFontHeight(font) + 2) * textRows - 2;	// height of the text
	ret += 2 * textMargin + 2;											// add the border height
	return ret;
}

void ButtonWithText::Refresh(bool full, PixelNumber xOffset, PixelNumber yOffset)
{
	if (full || changed)
	{
		DrawOutline(xOffset, yOffset);
		lcd.setTransparentBackground(true);
		lcd.setColor(fcolour);
		lcd.setFont(font);
		unsigned int rowsLeft = textRows;
		size_t offset = 0;
		PixelNumber rowY = y + yOffset + textMargin + 1;
		do
		{
			lcd.setTextPos(0, 9999, width - 6);
			PrintText(offset);							// dummy print to get text width
			PixelNumber spare = width - 6 - lcd.getTextX();
			lcd.setTextPos(x + xOffset + 3 + spare/2, rowY, x + xOffset + width - 3);	// text is always centre-aligned
			offset += PrintText(offset) + 1;
			rowY += UTFT::GetFontHeight(font) + 2;
		} while (--rowsLeft != 0);
		lcd.setTransparentBackground(false);
		changed = false;
	}
}

CharButton::CharButton(PixelNumber py, PixelNumber px, PixelNumber pw, char pc, event_t e)
	: ButtonWithText(py, px, pw)
{
	SetEvent(e, (int)pc);
}

size_t CharButton::PrintText(size_t offset) const
{
	UNUSED(offset);
	return lcd.write((char)GetIParam(0));
}

TextButton::TextButton(PixelNumber py, PixelNumber px, PixelNumber pw, const char * array null pt, event_t e, int param)
	: ButtonWithText(py, px, pw), text(pt)
{
	SetTextRows(pt);
	SetEvent(e, param);
}

TextButton::TextButton(PixelNumber py, PixelNumber px, PixelNumber pw, const char * array null pt, event_t e, const char * array param)
	: ButtonWithText(py, px, pw), text(pt)
{
	SetEvent(e, param);
}

size_t TextButton::PrintText(size_t offset) const
{
	if (text != nullptr)
	{
		return lcd.print(text + offset);
	}
	return 0;
}

IconButton::IconButton(PixelNumber py, PixelNumber px, PixelNumber pw, Icon ic, event_t e, int param)
	: SingleButton(py, px, pw), icon(ic)
{
	SetEvent(e, param);
}

IconButton::IconButton(PixelNumber py, PixelNumber px, PixelNumber pw, Icon ic, event_t e, const char * array param)
: SingleButton(py, px, pw), icon(ic)
{
	SetEvent(e, param);
}

void IconButton::Refresh(bool full, PixelNumber xOffset, PixelNumber yOffset)
{
	if (full || changed)
	{
		DrawOutline(xOffset, yOffset);
		const uint16_t sx = GetIconWidth(icon), sy = GetIconHeight(icon);
		lcd.setTransparentBackground(true);
		lcd.drawBitmap(xOffset + x + (width - sx)/2, yOffset + y + iconMargin + 1, sx, sy, GetIconData(icon));
		lcd.setTransparentBackground(false);
		changed = false;
	}
}

size_t IntegerButton::PrintText(size_t offset) const
{
	UNUSED(offset);
	size_t ret = 0;
	if (label != nullptr)
	{
		ret += lcd.print(label);
	}
	ret += lcd.print(val);
	if (units != nullptr)
	{
		ret += lcd.print(units);
	}
	return ret;
}

size_t FloatButton::PrintText(size_t offset) const
{
	UNUSED(offset);
	size_t ret = lcd.print(val, numDecimals);
	if (units != nullptr)
	{
		ret += lcd.print(units);
	}
	return ret;
}

#if 0	// not used yet
ButtonRow::ButtonRow(PixelNumber py, PixelNumber px, PixelNumber pw, PixelNumber ps, unsigned int nb, event_t e)
	: ButtonBase(py, px, pw), numButtons(nb), whichPressed(-1), step(ps)
{
	evt = e;
}

ButtonRowWithText::ButtonRowWithText(PixelNumber py, PixelNumber px, PixelNumber pw, PixelNumber ps, unsigned int nb, event_t e)
	: ButtonRow(py, px, pw, ps, nb, e)
{
}

void ButtonRowWithText::Refresh(bool full, PixelNumber xOffset, PixelNumber yOffset)
{
	if (full || changed)
	{
		for (unsigned int i = 0; i < numButtons; ++i)
		{
			const PixelNumber buttonXoffset = xOffset + i * step;
			DrawOutline(buttonXoffset, yOffset, (int)i == whichPressed);
			lcd.setTransparentBackground(true);
			lcd.setColor(fcolour);
			lcd.setFont(font);
			lcd.setTextPos(0, 9999, width - 6);
			PrintText(i);							// dummy print to get text width
			PixelNumber spare = width - 6 - lcd.getTextX();
			lcd.setTextPos(x + buttonXoffset + 3 + spare/2, y + yOffset + textMargin + 1, x + xOffset + width - 3);	// text is always centre-aligned
			PrintText(i);
			lcd.setTransparentBackground(false);
		}
		changed = false;
	}
}

void CharButtonRow::PrintText(unsigned int n) const
{
	lcd.write(text[n]);
}

CharButtonRow::CharButtonRow(PixelNumber py, PixelNumber px, PixelNumber pw, PixelNumber ps, const char * array s, event_t e)
	: ButtonRowWithText(py, px, pw, ps, strlen(s), e), text(s)
{
}
#endif

void ProgressBar::Refresh(bool full, PixelNumber xOffset, PixelNumber yOffset)
{
	if (full || changed)
	{
		PixelNumber pixelsSet = ((width - 2) * percent)/100;
		if (full)
		{
			lcd.setColor(fcolour);
			lcd.drawLine(x + xOffset, y, x + xOffset + width - 1, y + yOffset);
			lcd.drawLine(x + xOffset, y + yOffset + height - 1, x + xOffset + width - 1, y + yOffset + height - 1);
			lcd.drawLine(x + xOffset + width - 1, y + yOffset + 1, x + xOffset + width - 1, y + yOffset + height - 2);

			lcd.fillRect(x + xOffset, y + yOffset + 1, x + xOffset + pixelsSet, y + yOffset + height - 2);
			if (pixelsSet < width - 2)
			{
				lcd.setColor(bcolour);
				lcd.fillRect(x + xOffset + pixelsSet + 1, y + yOffset + 1, x + xOffset + width - 2, y + yOffset + height - 2);
			}
		}
		else if (pixelsSet > lastNumPixelsSet)
		{
			lcd.setColor(fcolour);
			lcd.fillRect(x + xOffset + lastNumPixelsSet, y + yOffset + 1, x + xOffset + pixelsSet, y + yOffset + height - 2);
		}
		else if (pixelsSet < lastNumPixelsSet)
		{
			lcd.setColor(bcolour);
			lcd.fillRect(x + xOffset + pixelsSet + 1, y + yOffset + 1, x + xOffset + lastNumPixelsSet, y + yOffset + height - 2);	
		}
		changed = false;
		lastNumPixelsSet = pixelsSet;
	}
}

void StaticImageField::Refresh(bool full, PixelNumber xOffset, PixelNumber yOffset)
{
	if (full || changed)
	{
		lcd.drawCompressedBitmap(x + xOffset, y + yOffset, width, height, data);
		changed = false;
	}
}

// End
