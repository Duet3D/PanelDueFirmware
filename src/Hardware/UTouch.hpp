/*
  UTouch.cpp - library support for Color TFT LCD Touch screens on SAM3X 
  Originally based on Utouch library by Henning Karlsen.
  Rewritten by D Crocker using the approach described in TI app note http://www.ti.com/lit/pdf/sbaa036.
*/

#ifndef UTouch_h
#define UTouch_h

#include "asf.h"
#include "OneBitPort.hpp"
#include "DisplayOrientation.hpp"

class UTouch
{
public:
	UTouch(unsigned int tclk, unsigned int tcs, unsigned int tdin, unsigned int dout, unsigned int irq);

	void	init(uint16_t xp, uint16_t yp, DisplayOrientation orientationAdjust = Default);
	bool	read(uint16_t &x, uint16_t &y, uint16_t * null rawX = nullptr, uint16_t * null rawY = nullptr);
	void	calibrate(uint16_t xlow, uint16_t xhigh, uint16_t ylow, uint16_t yhigh, uint16_t margin);
	void	adjustOrientation(DisplayOrientation a) { orientAdjust = (DisplayOrientation) (orientAdjust ^ a); }
	DisplayOrientation getOrientation() const { return orientAdjust; }
    
private:
	OneBitPort portCLK, portCS, portDIN, portDOUT, portIRQ;
	DisplayOrientation orientAdjust;
	uint16_t disp_x_size, disp_y_size;
	uint16_t scaleX, scaleY;
	int16_t offsetX, offsetY;

	bool	getTouchData(bool wantY, uint16_t &rslt);
	void	touch_WriteCommand(uint8_t command);
	uint16_t touch_ReadData(uint8_t command);
	uint16_t diff(uint16_t a, uint16_t b) { return (a < b) ? b - a : a - b; }
};

#endif