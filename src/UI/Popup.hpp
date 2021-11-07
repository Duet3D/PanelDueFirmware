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
	TextButtonForAxis *axis[3];
	TextButtonForAxis *steps[6];

	String<alertTitleLength> alertTitle;
	String<alertTextLength/3> alertText1, alertText2, alertText3;

	TextButton *okButton;
	TextButton *cancelButton;
};

#endif /* ifndef SRC_UI_POPUP_HPP_ */
