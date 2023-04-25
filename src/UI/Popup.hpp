#ifndef SRC_UI_POPUP_HPP_
#define SRC_UI_POPUP_HPP_ 1

#include "FlashData.hpp"
#include "Icons/Icons.hpp"
#include "General/SimpleMath.h"
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

	void UpdateData(const char *data);
	void ProcessOkButton();
	void ProcessChoice(uint32_t choice);

	bool Validate(int value);
	bool Validate(float value);
	bool Validate(const char *value);

private:
	String<alertTitleLength> alertTitle;
	String<alertTextLength/3> alertText1, alertText2, alertText3;

	String<alertTextLength/3> warningText;
	StaticTextField *warning;

	String<2> driveLetter;

	String<32> okCommand;
	String<32> cancelCommand;

	TextButton *okButton;
	TextButton *cancelButton;

	bool showCancelButton;

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

	struct {
		TextButton *button;
		String<32> text;
	} selectionMap[10];
	String<32> valueText;
	TextButton *value;

	static_assert(ARRAY_SIZE(Alert::choices) == ARRAY_SIZE(selectionMap));

	uint32_t seq;
	Alert::Mode mode;
	Alert::Limits limits;
};

#endif /* ifndef SRC_UI_POPUP_HPP_ */
