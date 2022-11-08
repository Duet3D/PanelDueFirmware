#include "main.hpp"

#include "UI/Provel.hpp"

#include "Hardware/SysTick.hpp"

#define DEBUG 2
#include "Debug.hpp"

int main_init()
{
	//UTFT lcd(DISPLAY_CONTROLLER, 15, 14, 0, 39);
	Provel::Provel ui;
	Provel::ScreenSplash splash;

	dbg("STARTING\r\n");
	dbg("splash init\r\n");

	ui.Push(&splash);
	ui.Update();

	delay_ms(1000);
	dbg("splash done\r\n");

#if 1
	Provel::ScreenHoming home;
	ui.Push(&home);
	ui.Update();
#endif

	return 0;
}

int main_run(UTouch &touch)
{
	const uint32_t normalTouchDelay = 250;				// how long we ignore new touches for after pressing Set
	const uint32_t repeatTouchDelay = 100;				// how long we ignore new touches while pressing up/down, to get a reasonable repeat rate

	const uint32_t now = SystemTick::GetTickCount();

	static uint32_t lastTouchTime;

	struct TouchEvent {
		uint32_t x;
		uint32_t y;
		enum {
			EventStateReleased = 0,
			EventStatePressed = 1,
			EventStateRepeated = 2
		} state;
	} event = { 0, 0, TouchEvent::EventStateReleased };
	uint16_t x, y;
	bool repeat;
	bool touched = false;

	// check for valid touch event
	if (touch.read(x, y, repeat))
	{
		switch (event.state)
		{
			case TouchEvent::EventStateReleased:
				touched = true;
				event.state = TouchEvent::EventStatePressed;
				break;
			case TouchEvent::EventStatePressed:
				if (now - lastTouchTime >= normalTouchDelay)
				{
					touched = true;
					event.state = TouchEvent::EventStateRepeated;
				}
				break;
			case TouchEvent::EventStateRepeated:
				if (now - lastTouchTime >= repeatTouchDelay)
				{
					touched = true;
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
	} else if (event.state != TouchEvent::EventStateReleased && now - lastTouchTime >= normalTouchDelay) {
		//dbg("released\n");
		touched = true;
		event.state = TouchEvent::EventStateReleased;
	}

	return 0;
}
