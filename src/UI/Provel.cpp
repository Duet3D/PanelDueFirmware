/*
 * Provel.cpp
 *
 *  Created on: 07-11-2022
 *      Author: Michael Szafranek (faule.aepfel@gmail.com)
 */

#include "UI/Provel.hpp"

#ifdef assert
#undef assert
#endif
#include <cassert>

#include "Version.hpp"

#define DEBUG 2
#include "Debug.hpp"

//
// - generic screen
//   - 1 column
//   - 6 rows
//     - static/dynamic text fields
//     - button
//       - half width
//       - full full
//
// - testing
//   - good example screens
//     - splash
//     - homing
//     - print status
//
// - screens
//   - splash
//   - homing
//   - loading file
//   - file loaded
//   - idle
//   - printing
//   - heating
//   - z calibrate
//   - purging
//   - printer
//   - fault
//   - warning

// questions
//
// - static or dynamic screen allocation?
// - how to state information?
//   - a part is already stored in global variables
//   - a part is stored in object model structures
// - how is state stored?
//   - probably makes sense to have ui state and a printer state
//   - due to action triggered from the ui an itermediate state before transitioning from
//     one ui state to another might be needed
// - how are action transmitted to the printer?
//

namespace Provel {

Provel::Provel() : screens(nullptr)
{
	colours = &colourSchemes[0];

	DisplayField::SetDefaultFont(DEFAULT_FONT);
	DisplayField::SetDefaultColours(colours->infoTextColour, colours->infoBackColour);
	SingleButton::SetTextMargin(textButtonMargin);
	SingleButton::SetIconMargin(iconButtonMargin);
}

Provel::~Provel()
{
}

Screen *Provel::Push(Screen *screen)
{
	int ret;

	dbg("screen %p\r\n", screen);

	assert(screen);

	ret = screen->Init(colours);
	assert(ret == 0);

	screen->next = screens;
	screens = screen;

	return screens;
}

Screen *Provel::Pop()
{
	Screen *head;

	dbg("screen %p\r\n", screens);

	assert(screens);

	head = screens;
	screens = screens->next;

	head->Shutdown();

	return head;
}

int Provel::Update()
{
	Screen *head = screens;

	//dbg("screen %p\r\n", head);

	if (!head)
		return 0;

	return head->Update();
}

int Provel::ProcessTouch(Touch &event)
{
	Screen *head;
	Element *element;

	dbg("screen %p\r\n", screens);
	dbg("x %d y %d %d\r\n", event.x, event.y, event.state);

	if (!screens)
		return 0;

	head = screens;
	element = head->Find(event.x, event.y);
	if (!element)
		return 0;

	return element->ProcessTouch(event);
}

Screen *Provel::Reset(Screen *screen)
{
	dbg("screen %p\r\n", screen);

	for (Screen *head = screens; head; head = head->next) {
		int ret = head->Shutdown();
		assert(ret == 0);
	}

	return Push(screen);
}


int Screen::Init(const ColourScheme *colours)
{
	assert(colours);

	dbg("\r\n");

	root.Init(colours->defaultBackColour);
	root.Refresh(true);

	dbg("done\r\n");

	return 0;
}

int Screen::Update()
{
	//dbg("\r\n");

	root.Refresh(false);

	return 0;
};

int Screen::Shutdown()
{
	dbg("\r\n");

	return 0;
};

int Screen::Add(Element *element)
{
	Element *tmp;

	dbg("screen %p element %p\r\n", this, element);
	dbg("(x/y) %hu/%hu\r\n", element->Get()->GetMinX(), element->Get()->GetMaxY());
	dbg("(w/h) %hu/%hu\r\n", element->Get()->GetWidth(), element->Get()->GetHeight());

	assert(element);

	root.AddField(element->Get());

	tmp = elements;

	elements = element;

	element->next = tmp;

	return 0;
}

int Screen::Delete(Element *element)
{
	dbg("screen %p element %p\r\n", this, element);

	assert(element);

	if (elements == element) {
		elements = element->next;
		element->next = nullptr;

		return 0;
	}

	for (Element *elem = elements; elem; elem = elem->next) {
		if (elem->next == element) {
			elem->next = element->next;
			element->next = nullptr;

			return 0;
		}
	}

	return 0;
}

Element *Screen::Find(int x, int y)
{
	ButtonPress event;
	ButtonBase *button;

	dbg("screen %p (%hu/%hu) x %d y %d\r\n", this, root.Xpos(), root.Ypos(), x, y);

	event = root.FindEvent(x, y);
	button = event.GetButton();

	dbg("button %p\r\n", event.GetButton());

	if (!button)
		return nullptr;

	for (Element *elem = elements; elem; elem = elem->next) {
		if (!elem->Get())
			continue;

		dbg("element %p %p\r\n", elem, elem->Get());
		if (elem->Get()->Find(button)) {
			dbg("element %p found\r\n", elem);
			return elem;
		}
	}

	return nullptr;
}


int Element::ProcessTouch(Touch &event)
{
	dbg("element %p\r\n", this);

	(void)event;
	return 0;
};


Text::Text(PixelNumber x, PixelNumber y, PixelNumber width, PixelNumber height, const char *ptext) :
	text(y, x, width, TextAlignment::Right, ptext, false)
{
}

Title::Title(PixelNumber x, PixelNumber y, PixelNumber width, PixelNumber height, const char *title) :
	text(y, x, width, TextAlignment::Left, title, false)
{
}


int Button::ProcessTouch(Touch &event)
{
	button.Press(event.state == Touch::State::Pressed, 0);
	return 0;
}

ButtonDouble::ButtonDouble(
		PixelNumber x, PixelNumber y,
		PixelNumber width, PixelNumber height,
		const char *text_left, const char *text_right) :
	group(y, x, width, height),
	button_left(y, x, width / 2, text_left, nullEvent, 0),
	button_right(y, x + width / 2, width / 2, text_right, nullEvent, 0)
{
	group.AddChild(&button_left);
	group.AddChild(&button_right);
}

int ButtonDouble::ProcessTouch(Touch &event)
{
	dbg("\r\n");
	ButtonPress press = group.FindEvent(event.x, event.y, &group);
	ButtonBase *button = press.GetButton();

	if (!button)
		return 0;

	button->Press(event.state == Touch::State::Pressed, 0);

	return 0;
}

}
