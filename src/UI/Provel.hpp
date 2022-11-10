#ifndef UI_PROVEL_HPP
#define UI_PROVEL_HPP 1

#include "UI/ColourSchemes.hpp"
#include "UI/Display.hpp"
#include "UI/UserInterfaceConstants.hpp"

#include "Version.hpp"

namespace Provel {

class Screen;
class Element;

class Touch {
public:
	uint16_t x;
	uint16_t y;
	enum State {
		Released = 0,
		Pressed = 1,
		Repeated = 2
	} state;

	Touch(uint16_t x, uint16_t y, enum State state) :
		x(x), y(y), state(state) {};
};

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
	int ProcessTouch(Touch &event);
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

	Element *Find(int x, int y);

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

public:
	Element *next;

	virtual int Init() { return 0; };
	virtual int Shutdown() { return 0; };

	virtual int ProcessTouch(Touch &event) {
		(void)event;
		return 0;
	};
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
	Button(
		PixelNumber x, PixelNumber y,
		PixelNumber width, PixelNumber height,
		const char *text, event_t e, int param
	) : button(x, y, width, text, e, param) {};
	~Button() {};

	DisplayField *Get() { return &button; };
	int ProcessTouch(Touch &event);
};

class ButtonDouble : public Element {
	TextButton button_left;
	TextButton button_right;
public:
	ButtonDouble(
			PixelNumber x=0, PixelNumber y=0,
			PixelNumber width=100, PixelNumber height=100,
			const char *text_left="btn: LEFT", const char *text_right="btn: RIGHT"
	) : button_left(x, y, width / 2, text_left, 0, 0),
	    button_right(x + width / 2, y, width / 2, text_right, 0, 0) {};
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

}

#endif
