#ifndef SRC_UI_POPUP_HPP_
#define SRC_UI_POPUP_HPP_ 1

#include "FlashData.hpp"
#include "Icons/Icons.hpp"
#include "General/String.h"
#include "UI/Alert.hpp"
#include "UI/ColourSchemes.hpp"
#include "UI/Display.hpp"
#include "UI/Events.hpp"
#include "UI/MessageLog.hpp"
#include "UI/Strings.hpp"
#include "UI/UserInterfaceConstants.hpp"

class StandardPopupWindow : public PopupWindow
{
public:
	StandardPopupWindow(PixelNumber ph, PixelNumber pw, Colour pb, Colour pBorder, Colour textColour, Colour imageBackColour,
			const char * null title, PixelNumber topMargin = popupTopMargin);

protected:
	StaticTextField *titleField;
	IconButton *closeButton;
};

class AlertPopup : public StandardPopupWindow
{
public:
	AlertPopup(const ColourScheme& colours);
	void Set(const char *title, const char *text, int32_t mode, uint32_t controls);
	void ChangeLetter(const size_t index);

private:
	String<alertTitleLength> alertTitle;
	String<alertTextLength/3> alertText1, alertText2, alertText3;

	TextButton *okButton;
	TextButton *cancelButton;

	struct AxisMap {
		const char *letter;
		TextButton *button;
	} axisMap[10] = {
		{ .letter = "X", .button = nullptr },
		{ .letter = "Y", .button = nullptr },
		{ .letter = "Z", .button = nullptr },
		{ .letter = "U", .button = nullptr },
		{ .letter = "V", .button = nullptr },
		{ .letter = "W", .button = nullptr },
		{ .letter = "A", .button = nullptr },
		{ .letter = "B", .button = nullptr },
		{ .letter = "C", .button = nullptr },
		{ .letter = "D", .button = nullptr },
	};

	struct DirMap {
		const char *text;
		const char *param;
		TextButtonForAxis *button;
	} dirMap[6] {
		{ .text = LESS_ARROW "2.0", .param = "-2.0", .button = nullptr },
		{ .text = LESS_ARROW "0.2", .param = "-0.2", .button = nullptr },
		{ .text = LESS_ARROW "0.02", .param = "-0.02", .button = nullptr },
		{ .text = MORE_ARROW "0.02", .param = "0.02", .button = nullptr },
		{ .text = MORE_ARROW "0.2", .param = "0.2", .button = nullptr },
		{ .text = MORE_ARROW "2.0", .param = "2.0", .button = nullptr },
};

};

#endif /* ifndef SRC_UI_POPUP_HPP_ */
