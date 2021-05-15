/*
 * OneBitPort.hpp
 *
 * Created: 04/11/2014 11:28:28
 *  Author: David
 */

#ifndef ONEBITPORT_H_
#define ONEBITPORT_H_

#include "ecv.h"
#undef array
#undef result
#undef value
#include "asf.h"

class OneBitPort
{
public:
	enum PortMode { Output, Input, InputPullup };

	OneBitPort(unsigned int pin);

	void setMode(PortMode mode) const;

	void setLow() const
	{
		port->PIO_CODR = mask;
	}

	void setHigh() const
	{
		port->PIO_SODR = mask;
	}

	// Pulse the pin high
	void pulseHigh() const
	{
		setHigh();
		delay(delay_100ns);
		setLow();
	}

	// Pulse the pin low
	void pulseLow() const
	{
		setLow();
		delay(delay_100ns);
		setHigh();
	}

	bool read() const
	{
		return (port->PIO_PDSR & mask) != 0;
	}

	static void delay(uint8_t del);

	static const uint8_t delay_100ns = 1;		// delay argument for 100ns
	static const uint8_t delay_200ns = 2;		// delay argument for 200ns

private:

	Pio * const port;			// PIO address
	const uint32_t mask;		// bit mask
};

#endif /* ONEBITPORT_H_ */
