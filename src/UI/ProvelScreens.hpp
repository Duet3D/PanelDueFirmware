#ifndef UI_PROVEL_SCREENS_HPP
#define UI_PROVEL_SCREENS_HPP 1

#include "Provel.hpp"

namespace Provel {

class ScreenSplash : public Screen {
	Title *title;
	Title *version;
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

class ScreenLoading : public Screen {
	Title *title;
	Button *cancel;

public:
	ScreenLoading();
	~ScreenLoading();
};

class ScreenFileLoaded : public Screen {
	Title *title;
	Button *cancel;

public:
	ScreenFileLoaded();
	~ScreenFileLoaded();
};

class ScreenIdle : public Screen {
	Title *title;
	Button *cancel;

public:
	ScreenIdle();
	~ScreenIdle();
};

class ScreenPrinting : public Screen {
	Title *title;
	Button *cancel;

public:
	ScreenPrinting();
	~ScreenPrinting();
};

class ScreenHeating : public Screen {
	Title *title;
	Button *cancel;

public:
	ScreenHeating();
	~ScreenHeating();
};

class ScreenZCalibrate : public Screen {
	Title *title;
	Button *cancel;

public:
	ScreenZCalibrate();
	~ScreenZCalibrate();
};

class ScreenPurging : public Screen {
	Title *title;
	Button *cancel;

public:
	ScreenPurging();
	~ScreenPurging();
};

class ScreenPrinter : public Screen {
	Title *title;
	Button *cancel;

public:
	ScreenPrinter();
	~ScreenPrinter();
};

class ScreenFault : public Screen {
	Title *title;
	Button *cancel;

public:
	ScreenFault();
	~ScreenFault();
};

class ScreenWarning : public Screen {
	Title *title;
	Button *cancel;

public:
	ScreenWarning();
	~ScreenWarning();
};

}


#endif
