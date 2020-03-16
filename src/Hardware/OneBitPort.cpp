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
	
void OneBitPort::setMode(PortMode mode) const
{
	pio_configure(port, (mode == Output) ? PIO_OUTPUT_0 : PIO_INPUT, mask, (mode == InputPullup) ? PIO_PULLUP : 0);
}

/*static*/ void OneBitPort::delay(uint8_t del)
{
	do
	{
		asm volatile ("nop\n");
		--del;
	} while (del != 0);
}
	
// End
