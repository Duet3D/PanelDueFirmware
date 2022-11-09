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

	dbg("\r\n");

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

	dbg("\r\n");

	assert(screens);

	head = screens;
	screens = screens->next;

	head->Shutdown();

	return head;
}

int Provel::Update()
{
	Screen *head = screens;

	//dbg("%p\r\n", head);

	if (!head)
		return 0;

	return head->Update();
}

int Provel::ProcessTouch(int x, int y, enum TouchState state)
{
	dbg("x %d y %d %d\r\n", x, y, state);
	Screen *head;
	Element *element;

	if (!screens)
		return 0;

	head = screens;

	element = head->Find(x, y);
	if (!element)
		return 0;

	dbg("done\r\n");

	return element->ProcessTouch(x, y, state);
}

Screen *Provel::Reset(Screen *screen)
{
	dbg("\r\n");

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
	dbg("\r\n");
	Element *tmp;

	assert(element);

	root.AddField(element->Get());
	tmp = elements;

	elements = element;

	element->next = tmp;

	return 0;
}

int Screen::Delete(Element *element)
{
	assert(element);

	Element *head = elements;

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

	dbg("\r\n");

	event = root.FindEvent(x, y);
	button = event.GetButton();

	if (!button)
		return nullptr;

	for (Element *elem = elements; elem; elem = elem->next) {
		if (elem->Get() == button)
			dbg("%p\r\n", elem);
			return elem;
	}

	return nullptr;
}


Text::Text(PixelNumber x, PixelNumber y, PixelNumber width, PixelNumber height, const char *ptext) :
	text(y, x, width, TextAlignment::Right, ptext, false)
{
}

Title::Title(PixelNumber x, PixelNumber y, PixelNumber width, PixelNumber height, const char *title) :
	text(y, x, width, TextAlignment::Left, title, false)
{
}

#define PROVEL_WIDTH 500

ScreenSplash::ScreenSplash()
{
	dbg("\r\n");
	timeout = 1000;
	title = new Title(100, 0, PROVEL_WIDTH, 100, "title: PROVEL");
	version = new Title(100, 100, PROVEL_WIDTH, 100, "text: " VERSION_TEXT);

	Add(title);
	Add(version);
}

ScreenSplash::~ScreenSplash()
{
}


ScreenHoming::ScreenHoming()
{
	title = new Title(100, 0, PROVEL_WIDTH, 100, "title: HOMING");
	stop = new Button(100, 100, PROVEL_WIDTH, 100, "button: STOP", 0, 0);
	status = new Status(100, 200, PROVEL_WIDTH, 100);

	Add(title);
	Add(stop);
	Add(status);
}

ScreenHoming::~ScreenHoming()
{
}

}
