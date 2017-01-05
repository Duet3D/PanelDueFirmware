/*
  UTouch.cpp - library support for Color TFT LCD Touch screens on SAM3X 
  Originally based on Utouch library by Henning Karlsen.
  Rewritten by D Crocker using the approach described in TI app note http://www.ti.com/lit/pdf/sbaa036.
*/

#include "UTouch.hpp"

UTouch::UTouch(unsigned int tclk, unsigned int tcs, unsigned int din, unsigned int dout, unsigned int irq)
	: portCLK(tclk), portCS(tcs), portDIN(din), portDOUT(dout), portIRQ(irq)
{
}

void UTouch::init(uint16_t xp, uint16_t yp, DisplayOrientation orientationAdjust)
{
	orientAdjust			= orientationAdjust;
	disp_x_size				= xp;
	disp_y_size				= yp;
	offsetX					= 0;
	scaleX					= (uint16_t)(((uint32_t)(disp_x_size - 1) << 16)/4095);
	offsetY					= 0;
	scaleY					= (uint16_t)(((uint32_t)(disp_y_size - 1) << 16)/4095);
	
	portCLK.setMode(OneBitPort::Output);
	portCS.setMode(OneBitPort::Output);
	portDIN.setMode(OneBitPort::Output);
	portDOUT.setMode(OneBitPort::Input);
	portIRQ.setMode(OneBitPort::InputPullup);
	
	portCS.setHigh();
	portCLK.setHigh();
	portDIN.setHigh();
}

// If the panel is touched, return the coordinates in x and y and return true; else return false
bool UTouch::read(uint16_t &px, uint16_t &py, uint16_t * null rawX, uint16_t * null rawY)
{
	bool ret = false;
	if (!portIRQ.read())			// if screen is touched
	{
		portCS.setLow();
		delay_us(100);				// allow the screen to settle
		uint16_t tx;
		if (getTouchData(false, tx))
		{
			uint16_t ty;
			if (getTouchData(true, ty))
			{
				if (!portIRQ.read())
				{
					int16_t valx = (orientAdjust & SwapXY) ? ty : tx;
					if (orientAdjust & ReverseX)
					{
						valx = 4095 - valx;
					}
						
					int16_t cx = (int16_t)(((uint32_t)valx * (uint32_t)scaleX) >> 16) - offsetX;
					px = (cx < 0) ? 0 : (cx >= disp_x_size) ? disp_x_size - 1 : (uint16_t)cx;

					int16_t valy = (orientAdjust & SwapXY) ? tx : ty;
					if (orientAdjust & ReverseY)
					{
						valy = 4095 - valy;
					}
	
					int16_t cy = (int16_t)(((uint32_t)valy * (uint32_t)scaleY) >> 16) - offsetY;
					py = (cy < 0) ? 0 : (cy >= disp_y_size) ? disp_y_size - 1 : (uint16_t)cy;
					if (rawX != nullptr)
					{
						*rawX = valx;
					}
					if (rawY != nullptr)
					{
						*rawY = valy;
					}
					ret = true;
				}				
			}
		}
		portCS.setHigh();
	}
	return ret;
}

// Get data from the touch chip. CS has already been set low.
// We need to allow the touch chip ADC input to settle. See TI app note http://www.ti.com/lit/pdf/sbaa036.
bool UTouch::getTouchData(bool wantY, uint16_t &rslt)
{
	uint8_t command = (wantY) ? 0xD3 : 0x93;		// start, channel 5 (y) or 1 (x), 12-bit, differential mode, don't power down between conversions
	touch_WriteCommand(command);					// send the command
	touch_ReadData(command);						// discard the first result and send the same command again

	const size_t numReadings = 8;
	const uint16_t maxDiff = 25;					// needs to be big enough to handle jitter. 8 was OK for the 4.3 and 5 inch displays but not the 7 inch.
	const unsigned int maxAttempts = 16;

	uint16_t ring[numReadings];
	uint32_t sum = 0;
	
	// Take enough readings to fill the ring buffer
	for (size_t i = 0; i < numReadings; ++i)
	{
		uint16_t val = touch_ReadData(command);
		ring[i] = val;
		sum += val;
	}

	// Test whether every reading is within 'maxDiff' of the average reading.
	// If it is, return the average reading.
	// If not, take another reading and try again, up to 'maxAttempts' times.
	uint16_t avg;
	size_t last = 0;
	bool ok;
	for (unsigned int i = 0; i < maxAttempts; ++i)
	{
		avg = (uint16_t)(sum/numReadings);
		ok = true;
		for (size_t i = 0; ok && i < numReadings; ++i)
		{
			if (diff(avg, ring[i]) > maxDiff)
			{
				ok = false;
				break;
			}
		}
		if (ok)
		{
			break;
		}
		
		// Take another reading
		sum -= ring[last];
		uint16_t val = touch_ReadData(command);
		ring[last] = val;
		sum += val;
		last = (last + 1) % numReadings;
	}
	
	touch_ReadData(command & 0xF8);			// tell it to power down between conversions
	touch_ReadData(0);						// read the final data
	rslt = avg;
	return ok;
}

// Send the first command in a chain. The chip latches the data bit on the rising edge of the clock. We have already set CS low.
void UTouch::touch_WriteCommand(uint8_t command)
{
	for(uint8_t count=0; count<8; count++)
	{
		if (command & 0x80)
		{
			portDIN.setHigh();
		}
		else
		{
			portDIN.setLow();
		}
		command <<= 1;
		portCLK.pulseHigh();
	}
}

// Read the data, and write another command at the same time. We have already set CS low.
// The chip produces its data bit after the falling edge of the clock. After sending 8 clocks, we can send a command again.
uint16_t UTouch::touch_ReadData(uint8_t command)
{
	uint16_t cmd = (uint16_t)command;
	uint16_t data = 0;

	for (uint8_t count=0; count<16; count++)
	{
		if (cmd & 0x8000)
		{
			portDIN.setHigh();
		}
		else
		{
			portDIN.setLow();
		}
		cmd <<= 1;
		OneBitPort::delay(OneBitPort::delay_100ns);					// need 100ns setup time from writing data to clock rising edge
		portCLK.pulseHigh();
		if (count < 12)
		{
			OneBitPort::delay(OneBitPort::delay_200ns);				// need 200ns setup time form clock falling edge to reading data
			data <<= 1;
			if (portDOUT.read())
			{
				data++;
			}
		}
	}
	
	return(data);
}

void UTouch::calibrate(uint16_t xlow, uint16_t xhigh, uint16_t ylow, uint16_t yhigh, uint16_t margin)
{
	scaleX = (uint16_t)(((uint32_t)(disp_x_size - 1 - 2 * margin) << 16)/(xhigh - xlow));
	offsetX = (int16_t)(((uint32_t)xlow * (uint32_t)scaleX) >> 16) - (int16_t)margin;
	scaleY = (uint16_t)(((uint32_t)(disp_y_size - 1 - 2 * margin) << 16)/(yhigh - ylow));
	offsetY = (int16_t)(((uint32_t)ylow * (uint32_t)scaleY) >> 16) - (int16_t)margin;
}

// End
