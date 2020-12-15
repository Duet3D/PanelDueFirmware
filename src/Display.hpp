/*
 * Display.h
 *
 * Created: 04/11/2014 09:43:43
 *  Author: David
 */


#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "ecv.h"
#undef array
#undef result
#undef value
#include <cstring>
#include "Hardware/UTFT.hpp"
#include "DisplaySize.hpp"
#include <math.h>

#ifndef UNUSED
# define UNUSED(_x)	(void)(_x)
#endif

// Fonts are held as arrays of 8-bit data in flash.
typedef const uint8_t * _ecv_array LcdFont;

// An icon is stored an _ecv_array of uint16_t data normally held in flash memory. The first value is the width in pixels, the second is the height in pixels.
// After that comes the icon data, 16 bits per pixel, one row at a time.
typedef const uint8_t * _ecv_array Icon;

// Unicode strings for special characters in our font
#define DECIMAL_POINT	"\xC2\xB7"		// Unicode middle-dot, code point B7
#define DEGREE_SYMBOL	"\xC2\xB0"		// Unicode degree-symbol, code point B0
#define THIN_SPACE		"\xC2\x80"		// Unicode control character, code point 0x80, we use it as thin space
#define LEFT_ARROW		"\xC2\x81"		// Unicode control character, code point 0x81, we use it as up arrow
#define UP_ARROW		"\xC2\x82"		// Unicode control character, code point 0x82, we use it as up arrow
#define RIGHT_ARROW		"\xC2\x83"		// Unicode control character, code point 0x83, we use it as down arrow
#define DOWN_ARROW		"\xC2\x84"		// Unicode control character, code point 0x84, we use it as down arrow
#define MORE_ARROW		"\xC2\x85"
#define LESS_ARROW		"\xC2\x86"

const uint8_t buttonGradStep = 12;
const PixelNumber AutoPlace = 0xFFFF;
constexpr float epsilon = 0.001f;

typedef uint8_t event_t;
const event_t nullEvent = 0;

enum class TextAlignment : uint8_t { Left, Centre, Right };

class ButtonBase;

// Small by-value class to identify what button has been pressed
class ButtonPress
{
	ButtonBase * null button;
	unsigned int index;

public:
	ButtonPress();
	ButtonPress(ButtonBase *b, unsigned int pi);
	void Set(ButtonBase *b, unsigned int pi);
	void Clear();

	bool IsValid() const { return button != nullptr; }
	ButtonBase * GetButton() const { return button; }
	unsigned int GetIndex() const { return index; }

	event_t GetEvent() const;
	int GetIParam() const;
	const char* _ecv_array GetSParam() const;
	bool operator==(const ButtonPress& other) const;

	bool operator!=(const ButtonPress& other) const { return !operator==(other); }
};

// Base class for a displayable field
class DisplayField
{
protected:
	PixelNumber y, x;							// Coordinates of top left pixel, counting from the top left corner
	PixelNumber width;							// number of pixels wide
	Colour fcolour, bcolour;					// foreground and background colours
	uint16_t changed : 1,
			visible : 1,
			underlined : 1,						// really belongs in class FieldWithText, but stored here to save space
			border : 1,							// really belongs in class FieldWithText, but stored here to save space
			textRows : 2;						// really belongs in class FieldWithText, but stored here to save space

	static LcdFont defaultFont;
	static Colour defaultFcolour, defaultBcolour;
	static Colour defaultButtonBorderColour, defaultGradColour, defaultPressedBackColour, defaultPressedGradColour;
	static Palette defaultIconPalette;

protected:
	DisplayField(PixelNumber py, PixelNumber px, PixelNumber pw);

	void SetTextRows(const char * _ecv_array t);
	virtual PixelNumber GetHeight() const = 0;
	virtual void CheckEvent(PixelNumber x, PixelNumber y, int& bestError, ButtonPress& best) { UNUSED(x); UNUSED(y); UNUSED(bestError); UNUSED(best); }

public:
	DisplayField * null next;					// link to next field in list

	virtual bool IsButton() const { return false; }
	virtual bool IsVisible() const { return visible; }
	void Show(bool v);
	virtual void Refresh(bool full, PixelNumber xOffset, PixelNumber yOffset) = 0;
	void SetColours(Colour pf, Colour pb);
	void SetChanged() { changed = true; }
	bool HasChanged() const { return changed; }
	PixelNumber GetMinX() const { return x; }
	PixelNumber GetMaxX() const { return x + width - 1; }
	PixelNumber GetMinY() const { return y; }
	PixelNumber GetMaxY() const { return y + GetHeight() - 1; }

	void SetPositionAndWidth(PixelNumber newX, PixelNumber newWidth);
	void SetPosition(PixelNumber x, PixelNumber y);

	virtual event_t GetEvent() const { return nullEvent; }

	static void SetDefaultColours(Colour pf, Colour pb) { defaultFcolour = pf; defaultBcolour = pb; }
	static void SetDefaultColours(Colour pf, Colour pb, Colour pbb, Colour pg, Colour pbp, Colour pgp, Palette pal);
	static void SetDefaultFont(LcdFont pf) { defaultFont = pf; }
	static ButtonPress FindEvent(PixelNumber x, PixelNumber y, DisplayField * null p);

	// Icon management
	static PixelNumber GetIconWidth(Icon ic) { return ic[0]; }
	static PixelNumber GetIconHeight(Icon ic) { return ic[1]; }
	static const uint8_t * _ecv_array GetIconData(Icon ic) { return ic + 2; }

	static PixelNumber GetTextWidth(const char* _ecv_array s, PixelNumber maxWidth);						// find out how much width we need to print this text
	static PixelNumber GetTextWidth(const char* _ecv_array s, PixelNumber maxWidth, size_t maxChars);	// find out how much width we need to print this text
};

class PopupWindow;

class Window
{
protected:
	DisplayField * null root;
	PopupWindow * null next;
	Colour backgroundColour;

public:
	Window(Colour pb);
	virtual PixelNumber Xpos() const { return 0; }
	virtual PixelNumber Ypos() const { return 0; }
	void AddField(DisplayField *p);
	ButtonPress FindEvent(PixelNumber x, PixelNumber y);
	ButtonPress FindEventOutsidePopup(PixelNumber x, PixelNumber y);
	DisplayField * null GetRoot() const { return root; }
	virtual void Refresh(bool full) = 0;
	void Redraw(DisplayField *f);
	void Show(DisplayField * null f, bool v);
	void Press(ButtonPress bp, bool v);
	void SetPopup(PopupWindow * p, PixelNumber px = 0, PixelNumber py = 0, bool redraw = true);
	PopupWindow * null GetPopup() const { return next; }
	void ClearPopup(bool redraw = true, PopupWindow *whichOne = nullptr);
	bool ObscuredByPopup(const DisplayField *p) const;
	bool Visible(const DisplayField *p) const;
	virtual bool Contains(PixelNumber xmin, PixelNumber ymin, PixelNumber xmax, PixelNumber ymax) const = 0;
};

class MainWindow : public Window
{
	PixelNumber staticLeftMargin;

public:
	MainWindow();
	void Init(Colour pb);
	void Refresh(bool full) override;
	void SetRoot(DisplayField * null r) { root = r; }
	bool Contains(PixelNumber xmin, PixelNumber ymin, PixelNumber xmax, PixelNumber ymax) const override;
	void ClearAllPopups();
	void SetLeftMargin(PixelNumber m) { staticLeftMargin = m; }
};

class PopupWindow : public Window
{
private:
	PixelNumber height, width, xPos, yPos;
	Colour borderColour;
	bool roundedCorners;

public:
	PopupWindow(PixelNumber ph, PixelNumber pw, Colour pb, Colour pBorder, bool roundCorners = true);

	PixelNumber GetHeight() const { return height; }
	PixelNumber GetWidth() const { return width; }
	PixelNumber Xpos() const override { return xPos; }
	PixelNumber Ypos() const override { return yPos; }
	void Refresh(bool full) override;
	void SetPos(PixelNumber px, PixelNumber py) { xPos = px; yPos = py; }
	bool Contains(PixelNumber xmin, PixelNumber ymin, PixelNumber xmax, PixelNumber ymax) const override;
};

class ColourGradientField : public DisplayField
{
	PixelNumber height;

protected:
	PixelNumber GetHeight() const override { return height; }

public:
	ColourGradientField(PixelNumber py, PixelNumber px, PixelNumber pw, PixelNumber ph)
		: DisplayField(py, px, pw), height(ph)
	{
	}

	void Refresh(bool full, PixelNumber xOffset, PixelNumber yOffset) override;
};

// Base class for fields displaying text
class FieldWithText : public DisplayField
{
	LcdFont font;
	TextAlignment align;

protected:
	PixelNumber GetHeight() const override;

	virtual void PrintText() const = 0;

	FieldWithText(PixelNumber py, PixelNumber px, PixelNumber pw, TextAlignment pa, bool withBorder, bool isUnderlined = false)
		: DisplayField(py, px, pw), font(DisplayField::defaultFont), align(pa)
	{
		underlined = isUnderlined;
		border = withBorder;
		textRows = 1;
	}

public:
	void Refresh(bool full, PixelNumber xOffset, PixelNumber yOffset) override final;
};

// Class to display a fixed label and some variable text
class TextField : public FieldWithText
{
	const char* _ecv_array null label;
	const char* _ecv_array null text;

protected:
	void PrintText() const override;

public:
	TextField(PixelNumber py, PixelNumber px, PixelNumber pw, TextAlignment pa,
				const char * _ecv_array null pl, const char* _ecv_array null pt = nullptr, bool withBorder = false)
		: FieldWithText(py, px, pw, pa, withBorder), label(pl), text(pt)
	{
	}

	void SetValue(const char* _ecv_array s)
	{
		text = s;
		changed = true;
	}

	void SetLabel(const char* _ecv_array s)
	{
		label = s;
		changed = true;
	}
};

// Class to display an optional label, a floating point value, and an optional units string
class FloatField : public FieldWithText
{
	const char* _ecv_array null label;
	const char* _ecv_array null units;
	float val;
	uint8_t numDecimals;

protected:
	void PrintText() const override;

public:
	FloatField(PixelNumber py, PixelNumber px, PixelNumber pw, TextAlignment pa, uint8_t pd,
			const char * _ecv_array pl = nullptr, const char * _ecv_array null pu = nullptr, bool withBorder = false)
		: FieldWithText(py, px, pw, pa, withBorder), label(pl), units(pu), val(0.0), numDecimals(pd)
	{
	}

	float GetValue() const noexcept { return val; }

	void SetValue(float v)
	{
		if (fabsf(val - v) < epsilon)
		{
			return;
		}
		val = v;
		changed = true;
	}

	void SetLabel(const char* _ecv_array s)
	{
		if (strcmp(label, s) == 0)
		{
			return;
		}
		label = s;
		changed = true;
	}
};

// Class to display an optional label, an integer value, and an optional units string
class IntegerField : public FieldWithText
{
	const char* _ecv_array null label;
	const char* _ecv_array null units;
	int val;

protected:
	void PrintText() const override;

public:
	IntegerField(PixelNumber py, PixelNumber px, PixelNumber pw, TextAlignment pa,
					const char *pl = nullptr, const char *pu = nullptr, bool withBorder = false)
		: FieldWithText(py, px, pw, pa, withBorder), label(pl), units(pu), val(0)
	{
	}

	void SetValue(int v)
	{
		if (val == v)
		{
			return;
		}
		val = v;
		changed = true;
	}
};

// Class to display a text string only
class StaticTextField : public FieldWithText
{
	const char * _ecv_array null text;

protected:
	void PrintText() const override;

public:
	StaticTextField(PixelNumber py, PixelNumber px, PixelNumber pw, TextAlignment pa, const char * _ecv_array null pt, bool isUnderlined = false)
		: FieldWithText(py, px, pw, pa, false, isUnderlined), text(pt)
	{
		SetTextRows(pt);
	}

	// Change the value
	void SetValue(const char* _ecv_array null pt, bool forceUpdate = false)
	{
		if (!forceUpdate && strcmp(text, pt) == 0)
		{
			return;
		}
		text = pt;
		SetTextRows(pt);
		changed = true;
	}
};

class ButtonBase : public DisplayField
{
protected:
	Colour borderColour, gradColour, pressedBackColour, pressedGradColour;
	event_t evt;								// event number that is triggered by touching this field
	bool pressed;								// putting this here instead of in SingleButton saves 4 byes per button

	ButtonBase(PixelNumber py, PixelNumber px, PixelNumber pw);
	void DrawOutline(PixelNumber xOffset, PixelNumber yOffset, bool isPressed) const;
	void CheckEvent(PixelNumber x, PixelNumber y, int& bestError, ButtonPress& best) override;

	static PixelNumber textMargin;
	static PixelNumber iconMargin;

public:
	event_t GetEvent() const override { return evt; }
	virtual const char* null GetSParam(unsigned int index) const { UNUSED(index); return nullptr; }
	virtual int GetIParam(unsigned int index) const { UNUSED(index); return 0; }
	virtual void Press(bool p, int index) { UNUSED(p); UNUSED(index); }
};

class SingleButton : public ButtonBase
{
	union EventParameter
	{
		const char* null sParam;
		int iParam;
		//float fParam;
	};
	EventParameter param;

protected:
	SingleButton(PixelNumber py, PixelNumber px, PixelNumber pw);

	void DrawOutline(PixelNumber xOffset, PixelNumber yOffset) const;

public:
	bool IsButton() const override final { return true; }

	void SetEvent(event_t e, EventParameter p) { evt = e; param = p; }
	void SetEvent(event_t e, const char* null sp) { evt = e; param.sParam = sp; }
	void SetEvent(event_t e, int ip) { evt = e; param.iParam = ip; }
	//void SetEvent(event_t e, float fp) { evt = e; param.fParam = fp; }

	EventParameter GetUParam() const { return param; }
	const char* null GetSParam(unsigned int index) const override { UNUSED(index); return param.sParam; }
	int GetIParam(unsigned int index) const override { UNUSED(index); return param.iParam; }
	//float GetFParam() const { return param.fParam; }

	void Press(bool p, int index) override;

	static void SetTextMargin(PixelNumber p) { textMargin = p; }
	static void SetIconMargin(PixelNumber p) { iconMargin = p; }
};

class ButtonWithText : public SingleButton
{
	static LcdFont font;

protected:
	PixelNumber GetHeight() const override;

	virtual size_t PrintText(size_t offset) const = 0;

public:
	ButtonWithText(PixelNumber py, PixelNumber px, PixelNumber pw)
		: SingleButton(py, px, pw) {}

	void Refresh(bool full, PixelNumber xOffset, PixelNumber yOffset) override final;

	static void SetFont(LcdFont f) { font = f; }
};

// Character button. The character on the button is the same as the integer event parameter.
class CharButton : public ButtonWithText
{
protected:
	size_t PrintText(size_t offset) const override;

public:
	CharButton(PixelNumber py, PixelNumber px, PixelNumber pw, char pc, event_t e);
};

// Base class for a row of related buttons with the same event
class ButtonRow : public ButtonBase
{
protected:
	unsigned int numButtons;
	int whichPressed;
	PixelNumber step;

public:
	ButtonRow(PixelNumber py, PixelNumber px, PixelNumber pw, PixelNumber ps, unsigned int nb, event_t e);
};

class ButtonRowWithText : public ButtonRow
{
	static LcdFont font;

protected:
	PixelNumber GetHeight() const override;
	virtual void PrintText(unsigned int n) const = 0;

public:
	ButtonRowWithText(PixelNumber py, PixelNumber px, PixelNumber pw, PixelNumber ps, unsigned int nb, event_t e);

	void Refresh(bool full, PixelNumber xOffset, PixelNumber yOffset) override final;

	static void SetFont(LcdFont f) { font = f; }
};

// Row of character buttons, used to build a keyboard
class CharButtonRow : public ButtonRowWithText
{
	const char * _ecv_array text;

protected:
	void PrintText(unsigned int n) const override;
	void CheckEvent(PixelNumber x, PixelNumber y, int& bestError, ButtonPress& best) override;

public:
	CharButtonRow(PixelNumber py, PixelNumber px, PixelNumber pw, PixelNumber ps, const char * _ecv_array s, event_t e);
	int GetIParam(unsigned int index) const override { return (int)text[index]; }
	void Press(bool p, int index) override;
	void ChangeText(const char* _ecv_array s);
};

// Standard button with text
class TextButton : public ButtonWithText
{
	friend class ShadowTextButton;

	const char * _ecv_array null text;

protected:
	size_t PrintText(size_t offset) const override;

public:
	TextButton(PixelNumber py, PixelNumber px, PixelNumber pw, const char * _ecv_array null pt, event_t e, int param = 0);
	TextButton(PixelNumber py, PixelNumber px, PixelNumber pw, const char * _ecv_array null pt, event_t e, const char * _ecv_array param);

	// Hide any text buttons with null text
	bool IsVisible() const override { return text != nullptr && DisplayField::IsVisible(); }

	void SetText(const char* _ecv_array null pt)
	{
		if (strcmp(text, pt) == 0)
		{
			return;
		}
		text = pt;
		changed = true;
	}
};

class TextButtonWithLabel : public TextButton
{
	const char * _ecv_array null label;
protected:
	size_t PrintText(size_t offset) const override;
public:
	TextButtonWithLabel(PixelNumber py, PixelNumber px, PixelNumber pw, const char * _ecv_array null pt, event_t e, int param = 0, const char* _ecv_array null label = nullptr);
	TextButtonWithLabel(PixelNumber py, PixelNumber px, PixelNumber pw, const char * _ecv_array null pt, event_t e, const char * _ecv_array param, const char* _ecv_array null label = nullptr);

	// Hide any text buttons with null text and null label
	bool IsVisible() const override { return (label != nullptr && DisplayField::IsVisible()) || TextButton::IsVisible(); }

	void SetLabel(const char* _ecv_array null label)
	{
		if (strcmp(this->label, label) == 0)
		{
			return;
		}
		this->label = label;
		changed = true;
	}
};

class TextButtonForAxis : public TextButton
{
private:
	char axisLetter;
public:
	TextButtonForAxis(PixelNumber py, PixelNumber px, PixelNumber pw, const char * _ecv_array null pt, event_t e, int param = 0)
		: TextButton(py, px, pw, pt, e, param), axisLetter('\0') {}
	TextButtonForAxis(PixelNumber py, PixelNumber px, PixelNumber pw, const char * _ecv_array null pt, event_t e, const char * _ecv_array param)
		: TextButton(py, px, pw, pt, e, param), axisLetter('\0') {}

	char GetAxisLetter() const { return this->axisLetter; }
	void SetAxisLetter(char axisLetter) { this->axisLetter = axisLetter; }
};

// Standard button with an icon
class IconButton : public SingleButton
{

protected:
	Icon icon;
	PixelNumber GetHeight() const override { return GetIconHeight(icon) + 2 * iconMargin + 2; }

public:
	IconButton(PixelNumber py, PixelNumber px, PixelNumber pw, Icon ic, event_t e, int param = 0);
	IconButton(PixelNumber py, PixelNumber px, PixelNumber pw, Icon ic, event_t e, const char * _ecv_array param);

	void Refresh(bool full, PixelNumber xOffset, PixelNumber yOffset) override;
};

// Standard button with an icon
class IconButtonWithText : public IconButton
{
	LcdFont font;
	const char * _ecv_array null text;
	int val;
	bool printText;

protected:
	size_t PrintText() const;

public:
	IconButtonWithText(PixelNumber py, PixelNumber px, PixelNumber pw, Icon ic, event_t e, const char * text, int param = 0);
	IconButtonWithText(PixelNumber py, PixelNumber px, PixelNumber pw, Icon ic, event_t e, const char * text, const char * _ecv_array param);
	IconButtonWithText(PixelNumber py, PixelNumber px, PixelNumber pw, Icon ic, event_t e, int textVal, int param = 0);
	IconButtonWithText(PixelNumber py, PixelNumber px, PixelNumber pw, Icon ic, event_t e, int textVal, const char * _ecv_array param);

	void Refresh(bool full, PixelNumber xOffset, PixelNumber yOffset) override final;

	void SetIcon(Icon newIcon)
	{
		if (icon == newIcon)
		{
			return;
		}
		icon = newIcon;
		changed = true;
	}

	void SetText(const char * t)
	{
		if (strcmp(text, t) == 0)
		{
			return;
		}
		text = t;
		changed = true;
	}

	void SetIntVal(int newVal)
	{
		if (newVal == val)
		{
			return;
		}
		val = newVal;
		changed = true;
	}

	void SetPrintText(const bool pt)
	{
		printText = pt;
		changed = true;
	}
};

// Button that displays an integer value, optionally preceded by a label and followed by units
class IntegerButton : public ButtonWithText
{
	const char* _ecv_array null label;
	const char* _ecv_array null units;
	int val;

protected:
	size_t PrintText(size_t offset) const override;

public:
	IntegerButton(PixelNumber py, PixelNumber px, PixelNumber pw, const char * _ecv_array pl = nullptr, const char * _ecv_array pt = nullptr)
		: ButtonWithText(py, px, pw), label(pl), units(pt), val(0) {}

	int GetValue() const { return val; }

	void SetValue(int pv)
	{
		if (val == pv)
		{
			return;
		}
		val = pv;
		changed = true;
	}

	void Increment(int amount)
	{
		val += amount;
		changed = true;
	}
};

// Button that displays a float value, optionally followed by units
class FloatButton : public ButtonWithText
{
	const char * _ecv_array null units;
	float val;
	uint8_t numDecimals;

protected:
	size_t PrintText(size_t offset) const override;

public:
	FloatButton(PixelNumber py, PixelNumber px, PixelNumber pw, uint8_t pd, const char * _ecv_array pt = nullptr)
		: ButtonWithText(py, px, pw), units(pt), val(0.0), numDecimals(pd) {
	}

	float GetValue() const { return val; }

	void SetValue(float pv)
	{
		if (fabsf(val - pv) < epsilon)
		{
			return;
		}
		val = pv;
		changed = true;
	}

	void Increment(int amount)
	{
		val += amount;
		changed = true;
	}
};

class ProgressBar : public DisplayField
{
	PixelNumber lastNumPixelsSet;
	uint8_t height;
	uint8_t percent;

public:
	ProgressBar(uint16_t py, uint16_t px, uint8_t ph, uint16_t pw)
		: DisplayField(py, px, pw), lastNumPixelsSet(0), height(ph), percent(0)
	{
	}

	void Refresh(bool full, PixelNumber xOffset, PixelNumber yOffset) override;

	PixelNumber GetHeight() const override { return height; }

	void SetPercent(uint8_t pc)
	{
		if (percent == pc)
		{
			return;
		}
		percent = pc;
		changed = true;
	}
};

class StaticImageField: public DisplayField
{
	const uint16_t * _ecv_array data;					// compressed bitmap
	PixelNumber height;

public:
	StaticImageField(PixelNumber py, PixelNumber px, PixelNumber ph, PixelNumber pw, const uint16_t * _ecv_array imageData)
		: DisplayField(py, px, pw), data(imageData), height(ph)
	{
	}

	void Refresh(bool full, PixelNumber xOffset, PixelNumber yOffset) override;

	PixelNumber GetHeight() const override { return height; }
};

#endif /* DISPLAY_H_ */
