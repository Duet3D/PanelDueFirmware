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
	stop = new Button(100, 100, PROVEL_WIDTH, 100, "btn: STOP", 0, 0);
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

ScreenFileLoaded::~ScreenFileLoaded()
{
}

ScreenFault::ScreenFault()
{
	title = new Title(100, 0, PROVEL_WIDTH, 100, "title: FAULT");
	clear = new Button(100, 100, PROVEL_WIDTH, 100, "btn: CLEAR", 0, 0);
	status = new Status(100, 200, PROVEL_WIDTH, 100);

	Add(title);
	Add(clear);
	Add(status);
}

ScreenFault::~ScreenFault()
{
}

ScreenHeating::ScreenHeating()
{
	title = new Title(100, 0, PROVEL_WIDTH, 100, "title: HEATING");
	text = new Text(100, 200, PROVEL_WIDTH, 100, "text: TEXT");
	stop = new Button(100, 300, PROVEL_WIDTH, 100, "btn: STOP", 0, 0);
	status = new Status(100, 400, PROVEL_WIDTH, 100);

	Add(title);
	Add(text);
	Add(stop);
	Add(status);
}

ScreenHeating::~ScreenHeating()
{
}

ScreenIdle::ScreenIdle()
{
	Title *title;
	Button *load;
	Button *home;
	Button *printer;
	Button *purge;

	title = new Title(100, 0, PROVEL_WIDTH, 100, "title: IDLE");
	load = new Button(100, 100, PROVEL_WIDTH, 100, "btn: LOAD", 0, 0);
	home = new Button(100, 200, PROVEL_WIDTH, 100, "btn: HOME", 0, 0);
	printer = new Button(100, 300, PROVEL_WIDTH, 100, "btn: PRINTER", 0, 0);
	purge = new Button(100, 400, PROVEL_WIDTH, 100, "btn: PURGE", 0, 0);

	Add(title);
	Add(load);
	Add(home);
	Add(printer);
	Add(purge);
}

ScreenIdle::~ScreenIdle()
{
}

ScreenLoading::ScreenLoading()
{
	title = new Title(100, 0, PROVEL_WIDTH, 100, "title: LOADING FILE");
	cancel = new Button(100, 100, PROVEL_WIDTH, 100, "btn: CANCEL", 0, 0);

	Add(title);
	Add(cancel);
}

ScreenLoading::~ScreenLoading()
{
}


ScreenPrinter::ScreenPrinter()
{
	title = new Title(100, 0, PROVEL_WIDTH, 100, "title: PRINTER");
	plusMinus = new ButtonDouble(100, 100, PROVEL_WIDTH, 100, "btn: PLUS", "btn: MINUS");
	zCalibrate = new Button(100, 200, PROVEL_WIDTH, 100, "btn: ZCALIBRATE", 0, 0);
	indexCup = new Button(100, 300, PROVEL_WIDTH, 100, "btn: INDEXCUP", 0, 0);
	access = new Button(100, 400, PROVEL_WIDTH, 100, "btn: ACCESS", 0, 0);
	enterExit = new ButtonDouble(100, 500, PROVEL_WIDTH, 100, "btn: ENTER", "btn: EXIT");

	Add(title);
	Add(plusMinus);
	Add(zCalibrate);
	Add(indexCup);
	Add(access);
	Add(enterExit);
}

ScreenPrinter::~ScreenPrinter()
{
}

ScreenPrinting::ScreenPrinting()
{
	title = new Title(100, 0, PROVEL_WIDTH, 100, "title: PRINTING");
	text = new Text(100, 100, PROVEL_WIDTH, 100, "text: TEXT");
	status = new Status(100, 200, PROVEL_WIDTH, 100);

	Add(title);
	Add(text);
	Add(status);
}

ScreenPrinting::~ScreenPrinting()
{
}

}
