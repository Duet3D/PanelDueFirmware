#ifndef UI_PROVEL_HPP
#define UI_PROVEL_HPP 1

#include "UI/ColourSchemes.hpp"
#include "UI/Display.hpp"
#include "UI/UserInterfaceConstants.hpp"

#include "Version.hpp"

namespace Provel {

class Screen;
class Element;

// generic classes

class Provel {

private:
	Screen *screens;

	const ColourScheme *colours;

public:
	Provel();
	~Provel();

	// window stack functions
	Screen *Push(Screen *screen);
	Screen *Pop();
	Screen *Reset(Screen *screen);

	// event functions
	int Update();
	int FindTouch(int x, int y);
	int ProcessTouch(ButtonPress &event);
};

class Screen {
	MainWindow root;
	Element *elements;

public:
	Screen *next;
	Screen *prev;

	// add and delete ui elements from screen
	int Add(Element *element);
	int Delete(Element *element);

	// init or shutdown state machine
	virtual int Init(const ColourScheme *colours);
	virtual int Update();
	virtual int Shutdown();

	Screen() {
		next = nullptr;
		prev = nullptr;
		elements = nullptr;
	};
	~Screen() {};
};

class Element {
	Element *next;

public:
	virtual int Init() { return 0; };
	virtual int Shutdown() { return 0; };

	virtual int Update() { return 0; };

	virtual DisplayField *Get() { return nullptr; };

	Element() {
		next = nullptr;
	};
	~Element();
};


class Button : public Element {
	TextButton button;

public:
	Button(PixelNumber x, PixelNumber y, PixelNumber width, PixelNumber height, const char *text, event_t e, int param) :
		button(x, y, width, text, e, param) {};
	~Button() {};

	DisplayField *Get() { return &button; };
};

class ButtonDouble : public Element {
	TextButton button_left;
	TextButton button_right;
};

class Text : public Element {
	StaticTextField text;

public:
	Text(
		PixelNumber x=0, PixelNumber y=0,
		PixelNumber width=100, PixelNumber height=100,
		const char *ptext="text");
	~Text() {};

	DisplayField *Get() { return &text; };
};

class Title : public Element {
	StaticTextField text;

public:
	Title(
		PixelNumber x=0, PixelNumber y=0,
		PixelNumber width=100, PixelNumber height=100,
		const char *title="title");
	~Title();

	DisplayField *Get() { return &text; };
};

class Status : public Element {
	ColourGradientField status;

	enum State {
		Unknown,
		Normal,
		Warning,
		Error
	} state;

	// will change dynamically depending on some state
public:
	Status(PixelNumber px, PixelNumber py, PixelNumber pw, PixelNumber ph) :
		status(py, px, pw, ph) {
			status.SetColours(UTFT::fromRGB(255,0,0), UTFT::fromRGB(0,128,0));
		};
	~Status() {};

	void SetState(enum State val) { state = val; };
	enum State GetState() { return state; };

	DisplayField *Get() { return &status; }

	virtual int Init() { return 0; };
	virtual int Shutdown() { return 0; };

	virtual int Update() { return 0; };
};


class ScreenSplash : public Screen {
	Title *title;
	Text *version;
	int timeout; // in ms

public:
	ScreenSplash();
	~ScreenSplash();
};

class ScreenHoming : public Screen {
	Title *title;
	Button *stop;
	Status *status;

public:
	ScreenHoming();
	~ScreenHoming();
};
}

#endif
