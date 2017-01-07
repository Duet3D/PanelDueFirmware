/*
 * OneBitPort.cpp
 *
 * Created: 04/11/2014 11:28:10
 *  Author: David
 */ 

#include "OneBitPort.hpp"

OneBitPort::OneBitPort(unsigned int pin)
	: port((pin < 32) ? PIOA : PIOB), mask(1u << (pin & 31))
{
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
