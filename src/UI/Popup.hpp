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
	void Set(const Alert &alert);
	void ChangeLetter(const size_t index);

private:
	String<alertTitleLength> alertTitle;
	String<alertTextLength/3> alertText1, alertText2, alertText3;

	String<2> driveLetter;

	String<32> okCommand;
	String<32> cancelCommand;

	TextButton *okButton;
	TextButton *cancelButton;

	TextButton *axisMap[10] = {
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
	};

	StaticTextField *driveLetterField;

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

	TextButton *selectionMap[10];

	struct {
		struct {
			int32_t min;
			int32_t max;
			int32_t valueDefault;
		} numberInt;
		struct {
			float min;
			float max;
			float valueDefault;
		} numberFloat;
		struct {
			int32_t min;
			int32_t max;
			String<32> valueDefault;
		} text;
	} limits;
};

};

#endif /* ifndef SRC_UI_POPUP_HPP_ */
