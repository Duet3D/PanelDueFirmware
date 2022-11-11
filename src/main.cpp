#include "main.hpp"

#include "UI/Provel.hpp"
#include "UI/ProvelScreens.hpp"

#include "Hardware/SysTick.hpp"

#define DEBUG 2
#include "Debug.hpp"

static Provel::Provel *ui;
static Provel::ScreenSplash *splash;
static Provel::ScreenHoming *home;
static Provel::ScreenFileLoaded *fileLoaded;
static Provel::ScreenFault *fault;
static Provel::ScreenHeating *heating;
static Provel::ScreenIdle *idle;
static Provel::ScreenLoading *loading;
static Provel::ScreenPrinter *printer;
static Provel::ScreenPrinting *printing;
static Provel::ScreenPurging *purging;
static Provel::ScreenWarning *warning;
static Provel::ScreenZCalibrate *zCalibrate;

int main_init()
{
	//UTFT lcd(DISPLAY_CONTROLLER, 15, 14, 0, 39);

	ui = new Provel::Provel();
	splash = new Provel::ScreenSplash();
	home = new Provel::ScreenHoming();
	fileLoaded = new Provel::ScreenFileLoaded();
	fault = new Provel::ScreenFault();
	heating = new Provel::ScreenHeating();
	idle = new Provel::ScreenIdle();
	loading = new Provel::ScreenLoading();
	printer = new Provel::ScreenPrinter();
	printing = new Provel::ScreenPrinting();
	purging = new Provel::ScreenPurging();
	warning = new Provel::ScreenWarning();
	zCalibrate = new Provel::ScreenZCalibrate();

	dbg("STARTING\r\n");

#if 0
	dbg("splash\r\n");
	ui->Push(splash);
	delay_ms(1000);

	dbg("home\r\n");
	ui->Push(home);
	delay_ms(1000);

	dbg("file loaded\r\n");
	ui->Push(fileLoaded);
	delay_ms(1000);

	dbg("fault\r\n");
	ui->Push(fault);
	delay_ms(1000);

	dbg("loading file\r\n");
	ui->Push(loading);
	delay_ms(1000);

	dbg("printing\r\n");
	ui->Push(printing);
	delay_ms(1000);

	dbg("heating\r\n");
	ui->Push(heating);
	delay_ms(1000);

	dbg("idle\r\n");
	ui->Push(idle);
	delay_ms(1000);

	dbg("printer\r\n");
	ui->Push(printer);
	delay_ms(1000);

	dbg("purging\r\n");
	ui->Push(purging);
	delay_ms(1000);

	dbg("warning\r\n");
	ui->Push(warning);
	delay_ms(1000);
#endif
	dbg("z calibrate\r\n");
	ui->Push(zCalibrate);
	delay_ms(1000);

	return 0;
}

static int main_touchUpdate(UTouch &touch, Provel::Touch &event)
{
	const uint32_t normalTouchDelay = 250;	// how long we ignore new touches for after pressing Set
	const uint32_t repeatTouchDelay = 100;	// how long we ignore new touches while pressing up/down, to get a reasonable repeat rate

	const uint32_t now = SystemTick::GetTickCount();

	static uint32_t lastTouchTime;

	uint16_t x, y;
	bool repeat;
	int touched = 0;

	// check for valid touch event
	if (touch.read(x, y, repeat)) {
		dbg("x %u y %u s %d\r\n", x, y, repeat);

		switch (event.state)
		{
			case Provel::Touch::State::Released:
				touched = 1;
				event.state = Provel::Touch::State::Pressed;
				break;
			case Provel::Touch::State::Pressed:
				if (now - lastTouchTime >= normalTouchDelay)
				{
					touched = 1;
					event.state = Provel::Touch::State::Repeated;
				}
				break;
			case Provel::Touch::State::Repeated:
				if (now - lastTouchTime >= repeatTouchDelay)
				{
					touched = 1;
				}
				break;
		}

		if (touched)
		{
			dbg("delta %d state %d\n", now - lastTouchTime, event.state);

			dbg("pressed\n");
			lastTouchTime = SystemTick::GetTickCount();

			event.x = x;
			event.y = y;
		}
	} else if (event.state != Provel::Touch::State::Released && now - lastTouchTime >= normalTouchDelay) {
		//dbg("released\n");
		touched = 1;
		event.state = Provel::Touch::State::Released;
	}

	return touched;
}

int main_run(UTouch &touch)
{

	static Provel::Touch event = Provel::Touch(0, 0, Provel::Touch::State::Repeated);
	int touched;
	int ret;

	touched = main_touchUpdate(touch, event);
	if (touched) {
		dbg("\r\n");
		ret = ui->ProcessTouch(event);
		if (ret) {
			dbg("failed to process touch event\r\n");
			return ret;
		}
	}
#if 1
	//dbg("\r\n");
	ret = ui->Update();
	if (ret) {
		dbg("failed to update ui\r\n");
		return ret;
	}
#endif
	//dbg("\r\n");

	return 0;
}
