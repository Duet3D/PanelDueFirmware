#include "ProvelScreens.hpp"

#define PROVEL_WIDTH 500

#define DEBUG 2
#include "Debug.hpp"

namespace Provel {

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


ScreenFileLoaded::ScreenFileLoaded()
{
	title = new Title(100, 0, PROVEL_WIDTH, 100, "title: FILE LOADED");
	startOrClear = new ButtonDouble(100, 100, PROVEL_WIDTH, 100, "btn: START", "btn: CLEAR");

	Add(title);
	Add(startOrClear);
}

}
