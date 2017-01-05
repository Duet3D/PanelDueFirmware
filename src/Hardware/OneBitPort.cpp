/*
 * OneBitPort.cpp
 *
 * Created: 04/11/2014 11:28:10
 *  Author: David
 */ 

#include "OneBitPort.hpp"

OneBitPort::OneBitPort(unsigned int pin)
{
#if defined(SAM3S)
	// For the SAM3S we use the following pin numbers:
	// 0-31		PA0-PA31
	// 32-63	PB0-PB31
	// 64-95	PC0-PC31
	mask = 1u << (pin & 31);
	port = (pin < 32) ? PIOA : PIOB;		// no PORT C on the SAM3S4B
#else
	// Use Arduino pin numbers
	port = portOutputRegister(digitalPinToPort(pin));
	mask = digitalPinToBitMask(pin);
#endif
}
	
void OneBitPort::setMode(PortMode mode)
{
#if defined(SAM3S)
	pio_configure(port, (mode == Output) ? PIO_OUTPUT_0 : PIO_INPUT, mask, (mode == InputPullup) ? PIO_PULLUP : 0);
#else
	pinMode(pin, (mode == Output) ? OUTPUT : (mode == InputPullup) ? INPUT_PULLUP : INPUT);
#endif
}

/*static*/ void OneBitPort::delay(uint8_t del)
{
	while (del != 0)
	{
		asm volatile ("nop\n" "nop\n" "nop\n");
		--del;
	}
}
	
// End
